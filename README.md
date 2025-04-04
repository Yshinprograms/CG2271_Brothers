**The Big Picture**

This program makes the KL25Z microcontroller act as the "body" of the robot, responding to simple commands received wirelessly (via an ESP32 acting as the "ears"). It uses a Real-Time Operating System (RTOS) called CMSIS-RTOS2 to manage different tasks concurrently: listening for commands, controlling motors, managing LED feedback, and playing sounds on a buzzer.

**Core Concepts (How it Manages Multiple Things)**

1.  **RTOS (CMSIS-RTOS2):** Think of it as a manager that lets different parts of the code (Tasks) run seemingly at the same time, switching between them very quickly. This allows the robot to move, flash LEDs, and play sounds simultaneously without one function completely blocking the others.
2.  **Tasks (Threads):** These are independent blocks of code, each responsible for a specific job:
    *   `tBrain`: The decision-maker. Processes incoming commands.
    *   `tMotorControl`: The driver. Executes motor actions.
    *   `led_control_thread`: The light show manager. Controls green and red LEDs based on status.
    *   `audio_thread`: The sound engineer. Plays tunes on the buzzer.
3.  **Interrupt (`UART2_IRQHandler`):** A high-priority function that runs *immediately* when a character arrives via UART (from the ESP32). Its only job is to grab the character quickly and pass it on.
4.  **Message Queues:** Like virtual mailboxes. Used for safe communication *between* tasks or from an ISR to a task.
    *   `uart_msg_queue_id`: The ISR drops received characters here for `tBrain` to pick up.
    *   `motor_cmd_queue_id`: `tBrain` sends processed motor commands here for `tMotorControl` to execute.
5.  **Mutex (`robot_state_mutex`):** A "talking stick". Ensures that only one task can access shared information (like the robot's current status) at any given moment, preventing confusion or errors.
6.  **Shared Variables:** Information needed by multiple tasks:
    *   `robot_state`: Stores whether the robot is `ROBOT_STATIONARY` or `ROBOT_MOVING` (or more specific states). Crucial for LED control. (Defined in `main.c`, declared `extern` elsewhere).
    *   `runComplete`: A flag (`true`/`false`) indicating if the 'Done' command ('D') has been received. Used by the audio task. (Defined in `main.c`, declared `extern` elsewhere).

**How a Command Flows Through the System (Example: 'F' command)**

1.  **Receive (`uart.c` - `UART2_IRQHandler`):**
    *   The ESP32 sends 'F' via UART.
    *   The KL25Z UART hardware receives it, triggering the `UART2_IRQHandler`.
    *   The ISR reads 'F' from the UART data register.
    *   The ISR puts 'F' into the `uart_msg_queue_id` mailbox.

2.  **Process (`main.c` - `tBrain` Task):**
    *   `tBrain` was waiting for a message on `uart_msg_queue_id`. It wakes up and receives 'F'.
    *   It recognizes 'F' means "Forward".
    *   It acquires the `robot_state_mutex` (grabs the talking stick).
    *   It sets the global `robot_state` to `ROBOT_MOVING`.
    *   It sets `runComplete` to `false`.
    *   It releases the `robot_state_mutex` (puts the stick back).
    *   It creates a `CMD_FORWARD` command message.
    *   It puts `CMD_FORWARD` into the `motor_cmd_queue_id` mailbox.

3.  **Execute (`main.c` - `tMotorControl` Task & `motor.c`):**
    *   `tMotorControl` was waiting for a command on `motor_cmd_queue_id`. It wakes up and receives `CMD_FORWARD`.
    *   It calls the `moveForward()` function (defined in `motor.c`), which sets the correct motor pins high/low to make both wheels spin forward.
    *   **CRITICAL:** It then calls `osDelay(500)`, pausing *only this task* for 0.5 seconds while the motors run.
    *   After the delay, it calls `stopMotors()` (from `motor.c`) to halt the wheels.
    *   It then goes back to waiting for the next command.

4.  **Feedback (Parallel Actions):**
    *   **LEDs (`led.c` - `led_control_thread` Task):**
        *   This task runs periodically (every ~10ms).
        *   It acquires `robot_state_mutex`, reads `robot_state` (which is now `ROBOT_MOVING`), and releases the mutex.
        *   Seeing the "moving" state, it starts the "running" green LED pattern (one LED on, cycling) and the "moving" red LED flashing pattern (500ms on, 500ms off). It manages its own timing internally using `osKernelGetTickCount` so it doesn't block.
    *   **Audio (`audio.c` - `audio_thread` Task):**
        *   This task is independently playing `melody1` note by note using PWM on the buzzer.
        *   Periodically (usually between notes), it acquires `robot_state_mutex`, checks `runComplete` (which is `false`), releases the mutex, and continues playing `melody1`. It is unaffected by the 'F' command itself.

**Special Commands:**

*   **'S' (Stop):** Goes through `tBrain` -> `tMotorControl`. Sets state to `ROBOT_STATIONARY`, sends `CMD_STOP`. `tMotorControl` calls `stopMotors()` immediately (no delay). `runComplete` is *not* set to true.
*   **'D' (Done):** Goes through `tBrain` -> `tMotorControl`. Sets state to `ROBOT_STATIONARY`, sends `CMD_STOP`. `tMotorControl` calls `stopMotors()` immediately. **Crucially, `tBrain` also sets `runComplete` to `true`**. The `audio_thread` will eventually detect this, stop `melody1`, play `melody2` once, and then stop making sound.

**Where to Find What:**

*   **Overall Setup, Task Creation, Global Variables:** `main.c`
*   **UART Initialization & Receiving Interrupt:** `uart.c`, `uart.h`
*   **Command Decoding & State Logic:** `tBrain` task within `main.c`
*   **Motor Command Execution & Delays:** `tMotorControl` task within `main.c`
*   **Low-Level Motor Pin Control Functions:** `motor.c`, `motor.h`
*   **LED Pin Definitions & Initialization:** `led.h`, `init_leds()` in `led.c`
*   **LED Patterns & Control Logic:** `led_control_thread` task within `led.c`
*   **Audio PWM Setup:** `initPWM()` in `audio.c`
*   **Melody Definitions & Audio Playback Logic:** `audio_thread` task, `melodyX_notes`/`_durations` arrays in `audio.c`
*   **Robot State Definitions (Enum):** `motor.h` (included by others)
*   **Pin Definitions:** `led.h`, `motor.h`, `audio.c` (PTB pins)

**How to Adapt / Modify:**

*   **Change Movement Duration:** Modify the `osDelay(500)` value in `tMotorControl` (`main.c`).
*   **Change LED Patterns/Speed:** Edit the logic inside `led_control_thread` (`led.c`), specifically the conditions checking `current_state` and the timing values (`green_running_interval`, `red_moving_interval`, etc.).
*   **Change Melodies:** Edit the `melody1_notes`, `melody1_durations`, `melody2_notes`, `melody2_durations` arrays in `audio.c`. Ensure the number of notes matches the number of durations.
*   **Add a New Movement Command (e.g., 'U' for U-turn):**
    1.  Add a case for 'U' in `tBrain` (`main.c`).
    2.  Define a new command like `CMD_UTURN` in the `MotorCommand_t` enum (`main.c`).
    3.  Add a corresponding case for `CMD_UTURN` in `tMotorControl` (`main.c`).
    4.  Implement the U-turn logic (e.g., call `sharpTurnLeft()` from `motor.c`, perhaps with a different `osDelay`).
    5.  Decide how `tBrain` should set `robot_state` for this command.
*   **Change Motor/LED Pins:** Update the `#define`s in `motor.h` or `led.h` AND update the corresponding `init_Motor()`/`init_leds()` functions (`motor.c`/`led.c`) to configure the new pins correctly.

**Thread Functions**
Okay, let's break down the thread handling and data handovers for each major feature in more detail. This should help in understanding the precise flow and potential bottlenecks or interaction points.

**1. Feature: Motor Movement (Responding to 'F', 'B', 'L', 'R', 'S', 'D')**

*   **Goal:** Receive a command character, translate it into a motor action, execute it for 0.5 seconds (for F/B/L/R), and then stop.
*   **Threads/ISR Involved:** `UART2_IRQHandler` (ISR), `tBrain` (Task), `tMotorControl` (Task).
*   **Data/Control Flow:**
    1.  **Reception (ISR):** `UART2_IRQHandler` fires *immediately* upon character arrival.
        *   **Action:** Reads the single character (e.g., 'F') from the UART hardware (`UART2->D`).
        *   **Handover:** Uses `osMessageQueuePut` to send this `char` to `uart_msg_queue_id`. This is a non-blocking operation from the ISR's perspective (using 0 timeout) – it quickly drops the message and finishes.
    2.  **Processing (`tBrain` Task):**
        *   **Action:** This task is likely blocked, waiting inside `osMessageQueueGet(uart_msg_queue_id, ...)` for a message to arrive.
        *   **Wake-up:** When the ISR puts the character ('F') in the queue, the RTOS wakes up `tBrain`.
        *   **Action:** `tBrain` receives 'F'. It translates 'F' into the `MotorCommand_t` enum value `CMD_FORWARD`.
        *   **State Update:** It acquires `robot_state_mutex`, sets `robot_state = ROBOT_MOVING`, sets `runComplete = false`, and releases the mutex.
        *   **Handover:** Uses `osMessageQueuePut` to send the `CMD_FORWARD` command to `motor_cmd_queue_id`. This is typically non-blocking for `tBrain` unless the motor queue is full (unlikely here). `tBrain` is now free to loop back and wait for the next UART character.
    3.  **Execution (`tMotorControl` Task):**
        *   **Action:** This task is likely blocked, waiting inside `osMessageQueueGet(motor_cmd_queue_id, ...)` for a command.
        *   **Wake-up:** When `tBrain` puts `CMD_FORWARD` in the queue, the RTOS wakes up `tMotorControl`.
        *   **Action:** `tMotorControl` receives `CMD_FORWARD`. It enters its `switch` statement and calls `moveForward()` (defined in `motor.c`) to activate the motor hardware pins.
        *   **Timing/Blocking:** It then calls `osDelay(500)`. **This is crucial:** `tMotorControl` task *yields control* to the RTOS scheduler and enters a waiting state for 500ms. Other tasks (like `led_control_thread`, `audio_thread`) can run during this time. `tMotorControl` itself cannot process any new commands from `motor_cmd_queue_id` while it's delayed.
        *   **Action:** After 500ms, the RTOS wakes `tMotorControl` again. It proceeds to call `stopMotors()`.
        *   **Action:** It then loops back to `osMessageQueueGet`, ready to wait for the next command.
    *   **Handling 'S'/'D':** If the command was `CMD_STOP` (from 'S' or 'D'), `tMotorControl` calls `stopMotors()` *immediately* without the `osDelay(500)`.
*   **Summary:** Fast ISR-to-task handover via queue. Fast task-to-task handover via another queue. The execution step in `tMotorControl` involves an *intentional blocking delay* (`osDelay`) that serializes movement commands into 0.5s bursts, preventing overlap but also preventing smooth transitions if commands are sent very rapidly.

**2. Feature: LED Display Control**

*   **Goal:** Show a "running" green pattern and specific flashing red pattern when moving; show solid green and a different flashing red pattern when stationary.
*   **Threads Involved:** `tBrain` (Task - modifies state), `led_control_thread` (Task - reads state and controls LEDs).
*   **Data/Control Flow:**
    1.  **State Change (`tBrain` Task):**
        *   **Action:** When processing a command ('F', 'B', 'L', 'R', 'S', 'D'), `tBrain` decides the new logical state of the robot.
        *   **Synchronization:** It acquires `robot_state_mutex`.
        *   **Action:** Updates the shared global variable `robot_state` (e.g., to `ROBOT_MOVING` or `ROBOT_STATIONARY`).
        *   **Synchronization:** Releases `robot_state_mutex`. (This whole critical section is very short).
    2.  **Polling and Execution (`led_control_thread` Task):**
        *   **Action:** This task runs in its own independent loop, controlled primarily by `osDelay(10)` at the end of its loop, making it run roughly every 10ms (+ execution time).
        *   **Synchronization:** At the start of its logic cycle, it acquires `robot_state_mutex`.
        *   **Action:** Reads the current value of the shared global variable `robot_state` into a local variable (`current_state`).
        *   **Synchronization:** Releases `robot_state_mutex`.
        *   **Action:** Based on the `current_state` it just read:
            *   If `ROBOT_MOVING` (or specific move states): It calls functions like `running_green_leds()` and uses internal logic (checking `osKernelGetTickCount()` against `last_red_toggle_tick` and `red_moving_ticks`) to decide whether to toggle the red LEDs ON/OFF according to the 500ms/500ms moving pattern.
            *   If `ROBOT_STATIONARY`: It calls `all_green_leds_on()` and uses similar internal timing logic but with `red_stationary_ticks` for the 250ms/250ms stationary red pattern.
        *   **Timing:** The running green effect and the red flashing are managed using non-blocking timer comparisons (`osKernelGetTickCount`) within this thread. It doesn't use `osDelay` for the flashing itself, allowing the loop to remain responsive.
*   **Summary:** State is updated by `tBrain` and protected by a mutex. `led_control_thread` *polls* this shared state periodically. The LED patterns themselves are generated using non-blocking timing within `led_control_thread`. There's a potential small latency (up to ~10ms) between the state changing in `tBrain` and the LEDs visually updating.

**3. Feature: Audio Playback (Buzzer)**

*   **Goal:** Play a continuous tune (`melody1`) while the run is active. Play a completion tune (`melody2`) once the 'Done' command is received.
*   **Threads Involved:** `tBrain` (Task - sets completion flag), `audio_thread` (Task - reads flag and controls buzzer PWM).
*   **Data/Control Flow:**
    1.  **Continuous Play (`audio_thread` Task):**
        *   **Action:** This task starts and enters its main `for (;;)` loop for `melody1`.
        *   **Action:** Inside the loop, it gets the next `frequency` and `duration` from the `melody1_notes` and `melody1_durations` arrays.
        *   **Action:** If `frequency > 0`, it calls `initPWM(frequency)` which reconfigures the TPM timer's MOD value (to set pitch) and ensures the PWM channel is enabled. If `frequency == 0` (rest), it disables the TPM counter (`TPM1->SC &= ~TPM_SC_CMOD_MASK`).
        *   **Timing/Blocking:** It calls `osDelay(duration)` to wait for the note's length. This blocks *only the `audio_thread`*.
        *   **State Check (Interleaved):** *Before* getting the next note (or sometimes implicitly just after the `osDelay`), it acquires `robot_state_mutex`, reads the `runComplete` flag, and releases the mutex.
        *   **Action:** If `runComplete` is `false`, it continues to the next note in `melody1`, wrapping around the array.
    2.  **Completion Trigger (`tBrain` Task):**
        *   **Action:** When `tBrain` processes the 'D' character.
        *   **Synchronization:** Acquires `robot_state_mutex`.
        *   **Action:** Sets the shared global variable `runComplete = true`.
        *   **Synchronization:** Releases `robot_state_mutex`.
    3.  **Completion Execution (`audio_thread` Task):**
        *   **Action:** During its next check of `runComplete` (after acquiring the mutex), `audio_thread` finds it's `true`.
        *   **Action:** It `break`s out of the `melody1` loop.
        *   **Action:** It explicitly stops the PWM (`TPM1->SC &= ~TPM_SC_CMOD_MASK`), pauses briefly (`osDelay(50)`).
        *   **Action:** It enters a new loop to play `melody2` note by note, using `initPWM()` and `osDelay(duration)` similar to how `melody1` was played.
        *   **Action:** After the `melody2` loop finishes, it stops the PWM again.
        *   **Termination:** Enters an infinite wait (`for(;;) { osDelay(osWaitForever); }`) or could terminate (`osThreadTerminate`).
*   **Summary:** Audio playback runs largely independently, blocking itself with `osDelay` for note durations. The completion is triggered by `tBrain` setting a shared flag (`runComplete`) protected by a mutex. `audio_thread` polls this flag between notes. This polling introduces potential latency in starting the completion tone, dependent on the duration of the `melody1` note playing when the 'D' command is processed. The buzzer pitch is controlled by dynamically changing the PWM frequency via `initPWM`.