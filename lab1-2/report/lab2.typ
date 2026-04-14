#align(center)[
= ECE3849 Lab 2: Multi-Screen Dashboard
=== Real-Time Embedded Systems
=== Benni Valentine, Leo Riesenbach
=== April 8, 2026
]

#let TODO = highlight[TODOTODOTODOTODOTODO]

#set heading(numbering: "1.1.1  ", )

= Introduction
In this lab, we further refined the stopwatch project, and added a microphone volume level dashboard screen to it. We converted the stopwatch button inputs to be interrupt-driven instead of polled. Additionally, we added multi-screen functionality navigated using the joystick, and a microphone screen that shows the current volume level measured by the microphone. The stopwatch continues running while the microphone is visible.


= Analysis Questions

== Question 1: Benefit of Multi-File Refactor
One concrete change that could be made to BuzzerTask is splitting the beep function out to multiple different pitches and durations instead of passing those as parameters. This would've been harder in a monolithic `main.c` because we'd have to add the declarations at the top and move stuff around. This is cleaner and more self-contained.

== Question 2: Shared State of `gCurrentScreen`
Sharing the value of `gCurrentScreen` between the reader `DisplayTask` and the writer `JoystickTask` is okay in this scenario because the `gCurrentScreen` variable is a `uint8_t`, smaller than the processor's word size, so the read and write to it are atomic. With a single writer, an atomic read/write is safe even with reader(s) in different tasks.

== Question 3: Binary Sempahore for Push-Buttons
Using a binary semaphore is better than polling for the push-buttons for this program because the scheduler is not required to enter `ButtonTask` to check if the buttons have been pushed yet. Only when the buttons are pushed and an ISR executes to unblock `ButtonTask` will the scheduler re-enter `ButtonTask`, as its `xSemaphoreTake()` call will finally be able to exit.

== Question 4: Multitasking between Screens
The stopwatch should continue running during the microphone screen because the user might bump the joystick accidentally while using the stopwatch, and it would be better not to lose their time. There is no technical reason to stop the stopwatch in the microphone screen, since all the tasks can just keep running with no difference in the backend. All tasks are still running in this case, and `DisplayTask` changes what's showing on the screen.

== Question 5: `vTaskDelay` vs Software Timer
`vTaskDelay(1)` does not create a stable 1000hz sample rate because it is subject to scheduler jitter. There may be an interrupt running at the time that the 1ms delay is over, which would delay the microphone sampling until it is over. It also does not factor in the time spent sampling and doing the calculation for the microphone reading (although this part could be easily mitigated with `vTaskDelayUntil()`)

== Question 6: Maximum Sample Rate with Software Timer
The maximum software-timer based event frequency that you could achieve is the same as the FreeRTOS tick frequency, which in this case is 1000hz. We can only sample at a maximum of 1000hz, meaning that the highest frequency signal we could analyze as per the Nyquist frequency is 500hz.

== Question 7: Window Size Trade-Off
The microphone screen is updated every `WINDOW_SIZE` samples. If we double this, then the display would update half as often. The RMS would stay stable, since the task would be getting less frequent, not more. The memory usage would double, since we would have to store twice as many samples.

= Conclusion
This lab was completed successfully. The stopwatch firmware now has significantly more functionality in the microphone screen, and is implemented better with the button interrupts. The codebase is cleaner now as well, thanks to the multi-file architecture.