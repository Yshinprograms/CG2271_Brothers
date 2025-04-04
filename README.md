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