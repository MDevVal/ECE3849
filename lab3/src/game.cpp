extern "C" {
    #include "FreeRTOS.h"
    #include "semphr.h"
    #include "task.h"
}
#include "game.h"
#include "app_objects.h"
#include <cstdlib>

// Global state definitions
volatile SnakeGameState gameState = {
    .currentDirection = RIGHT,
    .gFood = {0, 0},
    .state = PAUSED,
    .mutex = NULL,
    .needsReset = false,
    .score = 0
};

Position snake[MAX_LEN];
uint8_t snakeLength = 4;

// Internal helpers (caller must hold gameState.mutex)
static void addNewFood_nolock(void);
static bool isCollidingWithSnake_nolock(Position pos);

void ResetGame(void)
{
    if (gameState.mutex == NULL) {
        gameState.mutex = xSemaphoreCreateMutex();
    }
    xSemaphoreTake(gameState.mutex, portMAX_DELAY);

    // Place snake centered, heading right
    snakeLength = 4;
    gameState.score = 0;
    uint8_t cx = GRID_SIZE / 2;
    uint8_t cy = GRID_SIZE / 2;
    for (uint8_t i = 0; i < snakeLength; ++i) {
        snake[i].x = (uint8_t)(cx - i);
        snake[i].y = cy;
    }
    gameState.currentDirection = RIGHT;
    gameState.state = RUNNING;
    gameState.needsReset = false;

    addNewFood_nolock();

    xSemaphoreGive(gameState.mutex);
}

void moveSnake()
{
    xSemaphoreTake(gameState.mutex, portMAX_DELAY);


    // Shift body so each segment follows the previous one
    for (uint8_t i = snakeLength; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Update head position based on direction with wrap-around
    switch (gameState.currentDirection) {
        case UP:
            if (snake[0].y == 0) {
                snake[0].y = (uint8_t)(GRID_SIZE - 1);
            } else {
                snake[0].y = (uint8_t)(snake[0].y - 1);
            }
            break;
        case DOWN:
            if (snake[0].y == GRID_SIZE - 1) {
                snake[0].y = 0;
            } else {
                snake[0].y = (uint8_t)(snake[0].y + 1);
            }
            break;
        case LEFT:
            if (snake[0].x == 0) {
                snake[0].x = (uint8_t)(GRID_SIZE - 1);
            } else {
                snake[0].x = (uint8_t)(snake[0].x - 1);
            }
            break;
        case RIGHT:
            if (snake[0].x == GRID_SIZE - 1) {
                snake[0].x = 0;
            } else {
                snake[0].x = (uint8_t)(snake[0].x + 1);
            }
            break;
    }

    
    if (gameState.gFood.x == snake[0].x && gameState.gFood.y == snake[0].y) {
        snakeLength++;
        gameState.score++;
        addNewFood_nolock();
        xEventGroupSetBits(xGameEvents, EVT_FOOD_EATEN);
    }
    
    if (isCollidingWithSnake_nolock(snake[0])) {
        gameState.state = GAMEOVER;
        xEventGroupSetBits(xGameEvents, EVT_GAME_OVER);
    }

    xSemaphoreGive(gameState.mutex);
}

void addNewFood() {
    xSemaphoreTake(gameState.mutex, portMAX_DELAY);
    addNewFood_nolock();
    xSemaphoreGive(gameState.mutex);
}

static void addNewFood_nolock() {
    Position new_food_pos;
    do {
        new_food_pos.x = rand() % GRID_SIZE;
        new_food_pos.y = rand() % GRID_SIZE;
    } while (isCollidingWithSnake_nolock(new_food_pos));

    gameState.gFood.x = new_food_pos.x;
    gameState.gFood.y = new_food_pos.y;
}

bool isCollidingWithSnake(Position pos) {
    xSemaphoreTake(gameState.mutex, portMAX_DELAY);
    bool collides = isCollidingWithSnake_nolock(pos);
    xSemaphoreGive(gameState.mutex);
    return collides;
}

static bool isCollidingWithSnake_nolock(Position pos) {
    for (size_t i = 1; i < snakeLength - 2; i++) {
        if (pos.x == snake[i].x && pos.y == snake[i].y) {
            return true;
        }
    }
    return false;
}