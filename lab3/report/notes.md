1. Bug 1: vSnakeTask writes to snake[] and snakeLength. vRenderTask
   reads them at the same time. Is there any protection? What does the
   ARM Cortex-M4 guarantee about multi-word struct accesses across
   tasks?
    - No, there's no protection, and there's no guarantees about
      atomicity of multi-word struct accesses.
2. Bug 2: vSnakeTask reads gameState.currentDirection while vInputTask
   writes it. Is a volatile declaration enough to make this safe? Why or
   why not?
    - Yes, that is safe because it is word sized, so reads and writes to
      it are atomic.
3. Bug 3: vRenderTask wakes up every 33 ms regardless of whether
   anything changed. How many unnecessary redraws per second does this
    cause when the game is paused?
    - This causes 30 unnecessary redraws per second when the game is paused.
