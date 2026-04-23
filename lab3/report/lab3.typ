#align(center)[
= ECE3849 Lab 3: Snake Game --- Mutexes, Task Notifications, and Event Groups
=== Real-Time Embedded Systems
=== Benni Valentine, Leo Riesenbach
=== April 8, 2026
]

#let TODO = highlight[TODOTODOTODOTODOTODO]

#import "@preview/cetz:0.3.4"

#set heading(numbering: "1.1.1  ", )

= Introduction
In this lab, we implemented a snake game on the embedded TI system using FreeRTOS tasks and synchronization constructs. The project made use of task notifications, event groups, and mutexes for shared data. These multithreading tools were used to maximize the time-precision and responsiveness of the game, as well as to reduce excess work by the display task, preventing unnecessary re-draws when nothing's changed.

= System Architecture

The application is organized as four FreeRTOS tasks communicating through a mutex-guarded shared state struct, a shared snake-body array, and an event group for one-shot audio events. The `vSnakeTask` drives game progression and signals `vRenderTask` via a direct-to-task notification whenever a redraw is warranted. Hardware inputs (buttons and joystick) are polled by `vInputTask`, and outputs are the Crystalfontz 128×128 LCD driven by `vRenderTask` and the PWM-driven piezo buzzer driven by `BuzzerTask`.

#figure(
  cetz.canvas(length: 1cm, {
    import cetz.draw: *

    set-style(
      content: (padding: 0.12),
      stroke: 0.6pt,
      mark: (fill: black, scale: 0.8),
    )

    // --- Hardware inputs (left column) ---
    rect((0, 6.9), (2.6, 8.1), name: "btns", fill: rgb("#e8e8e8"))
    content("btns", [Buttons S1/S2])

    rect((0, 4.8), (2.6, 6.0), name: "joy", fill: rgb("#e8e8e8"))
    content("joy", [Joystick])

    // --- Tasks (center-left column) ---
    rect((3.6, 6.9), (6.6, 8.1), name: "inp", fill: rgb("#cfe2ff"))
    content("inp", [vInputTask\ (prio 2, 10 ms)])

    rect((3.6, 4.8), (6.6, 6.0), name: "snk_t", fill: rgb("#cfe2ff"))
    content("snk_t", [vSnakeTask\ (prio 2, adaptive)])

    rect((3.6, 2.5), (6.6, 3.7), name: "rnd", fill: rgb("#cfe2ff"))
    content("rnd", [vRenderTask\ (prio 1, on notify)])

    rect((3.6, -0.4), (6.6, 0.8), name: "buz", fill: rgb("#cfe2ff"))
    content("buz", [BuzzerTask\ (prio 2, on event)])

    // --- Shared state (center-right column) ---
    rect((7.8, 6.9), (10.8, 8.1), name: "gs", fill: rgb("#fff3cd"))
    content("gs", [gameState\ (mutex)])

    rect((7.8, 4.8), (10.8, 6.0), name: "snk_a", fill: rgb("#fff3cd"))
    content("snk_a", [snake\[\] + length\ (mutex)])

    rect((0.5, 1.0), (3.5, 2.2), name: "evt", fill: rgb("#d4edda"))
    content("evt", [xGameEvents\ (event group)])

    // --- Hardware outputs (right column) ---
    rect((12.0, 2.5), (14.6, 3.7), name: "lcd", fill: rgb("#e8e8e8"))
    content("lcd", [LCD 128×128])

    rect((12.0, -0.4), (14.6, 0.8), name: "pwm", fill: rgb("#e8e8e8"))
    content("pwm", [PWM0 Buzzer])

    // --- Dataflow arrows ---

    // HW inputs -> Input Task
    line("btns.east", "inp.west", mark: (end: "straight"))
    line("joy.east", "inp.south-west", mark: (end: "straight"))

    // Input Task <-> gameState
    line("inp.east", "gs.west",
      mark: (start: "straight", end: "straight"))

    // Snake Task <-> snake[] (same row, straight)
    line("snk_t.east", "snk_a.west",
      mark: (start: "straight", end: "straight"))

    // Snake Task <-> gameState
    line("snk_t.north-east", "gs.south-west",
      mark: (start: "straight", end: "straight"))

    // Snake Task -> xGameEvents (one-way: sets FOOD_EATEN / GAME_OVER).
    // Route below snake[], down the right side of the shared-state column
    // (between snake[]/evt and the LCD column), into evt from the east.
    line("snk_t.south-west",
      "evt.north", mark: (end: "straight"))

    // Snake Task -> Render Task (direct-to-task notification)
    line("snk_t.south", "rnd.north", mark: (end: "straight"))

    // Render Task <- gameState (read-only).
    // Route down the narrow gap between tasks and shared-state columns.
    line("gs.east", (11.2, 7.5), (11.2, 3.1), "rnd.east",
      mark: (end: "straight"))

    // Render Task <- snake[] (read-only)
    line("snk_a.south-west", "rnd.north-east", mark: (end: "straight"))

    // Render Task -> LCD (direct horizontal, same row)
    line("rnd.east", "lcd.west", mark: (end: "straight"))

    // xGameEvents -> Buzzer Task (waits on bits)
    line("evt.south", "buz.west", mark: (end: "straight"))

    // Buzzer Task -> PWM (direct horizontal, same row, below evt)
    line("buz.east", "pwm.west", mark: (end: "straight"))

    // --- Legend ---
    rect((0, -2.0), (0.5, -1.6), fill: rgb("#cfe2ff"))
    content((2.5, -1.8), [FreeRTOS task])
    rect((4.5, -2.0), (5.0, -1.6), fill: rgb("#fff3cd"))
    content((6.8, -1.8), [Mutex-guarded data])
    rect((8.8, -2.0), (9.3, -1.6), fill: rgb("#d4edda"))
    content((10.8, -1.8), [Event group])
    rect((12.5, -2.0), (13.0, -1.6), fill: rgb("#e8e8e8"))
    content((14.0, -1.8), [Hardware])
  }),
  caption: [System architecture. Tasks (blue) communicate through mutex-guarded shared state (yellow) and an event group (green). `vSnakeTask` signals `vRenderTask` via a direct-to-task notification; `BuzzerTask` blocks on the event group.],
) <fig:arch>

= Analysis Questions

== Question 1: Debugging Template Bugs
=== Race condition of `snake` and `snakeLength` access
There's no race condition protection for multi-thread access  of `snake` and `snakeLength`. Since `snake` is an array of many elements that in sum is far bigger than a word, there are no guarantees about access atomicity. The solution was adding a mutex for access to `snake` and `snakeLength`.

=== Race condition of `gameState.currentDirection` access
This is a safe atomic access because `currentDirection` is word-sized, so reads and writes are atomic. As long as the variable is marked `volatile`, this isn't a problem.

=== Unnecessary wake of `vRenderTask`
The unnecessary wake of `vRenderTask` during a pause causes an extra 30 redraws per second when nothing about the game is changing.

== Question 2: Polling vs Task Notifications for Rendering
In `vRenderTask`, polling for every 33ms with `vTaskDelayUntil` is worse than using task notifications because the polling approach means that the system will re-render every 33ms unconditionally. With the task notifications approach, the current display frame will only be redrawn if there is something new to draw on it. This is done by the snake task which controls the overall game state. Whenever the game state changes, then the render task is notified and the game display is redrawn.

== Question 3: Mutex Usage
We used two mutexes, one for the `snake` array and one for the `gameState`. The `snake` mutex protects reading or writing the snake's actual list of positions. The `gameState` mutex protects reading or writing the current food position, score, and running/paused/game-over state. They are both held only briefly by the display task or game task in each of their cycles. If a task held the mutex for 200ms, the whole game would be frozen for 200ms, as either the screen wouldn't be able to update or the snake wouldn't be able to move and inputs unable to register.

== Question 4: Event Group Bits Both Set
If both `EVT_FOOD_EATEN` and `EVT_GAME_OVER` are set, then `xEventGroupWaitBits` would return `0b11`, indicating that two bits are set. Here, the program as we designed it will play both tones in sequence. Arguably, it should play as a chord or maybe only play one or the other, but this is how we chose to do it.

= Design Decisions
We decided to add an additional mutex around `gameState` so that the different tasks wouldn't cause tearing between states. The alternative would be careful planning around the shared data in a lock-free manner, or using critical sections to prevent pre-emption. We chose this approach because it seemed easiest and fit with how the shared `snake` state was being handled.

We also decided to make the snake get faster with a higher score, using a linear function where at zero score the snake moves every 150ms, and it gets 2ms faster every score tick. There are other functions you could use for scaling the speed, because this one has problems such as making the game nearly impossible at high scores. This was easiest and simplest to implement for our purposes though.

We chose to have an infinite timeout on the render task taking a task notification, instead of forcing an occasional redraw. This was chosen because the application is relatively simple, so we could be quite confident that there are no conditions where the game progressed without redrawing the screen. In a more complex app, however, a timeout may be preferred to catch edge cases and make sure the screen is up-to-date.

= Conclusion
The hardest part of this lab was debugging in the embedded environment. We had one case where there was a very fickle race condition around when the mutexes were initialized, so we added a check for if they were NULL before accessing them. This was very difficult to diagnose though, as it is quite difficult to get any feedback out of an embedded system. 

If the system had twice as many tasks at the same priorities, we would likely run into cases of priority inversion around the mutexes, or high-priority tasks being blocked. There would also probably be less idle time, so more likely that a low-priority task doesn't run for a long time, making the game seem laggy.