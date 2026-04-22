#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "semphr.h"

// Direction for snake movement
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;
typedef struct { uint8_t x, y; } Position;
typedef enum { PAUSED, RUNNING, GAMEOVER } GameState;

// High-level game state flags
typedef struct SnakeGameState {
    Direction currentDirection;
    GameState state;
    bool needsReset;
    Position gFood;
    uint32_t score;
    SemaphoreHandle_t mutex;
} SnakeGameState;

#define EVT_FOOD_EATEN  (1 << 0)
#define EVT_GAME_OVER   (1 << 1)

extern volatile SnakeGameState gameState;

// Grid configuration (128x128 display, 8x8 cells)
#define GRID_SIZE 16
#define CELL_SIZE 8
#define MAX_LEN   256  // Maximum snake length (16x16 grid)

// Snake representation
extern Position snake[MAX_LEN];
extern uint8_t snakeLength;

// Basic API
void ResetGame(void);
void moveSnake(void);
void addNewFood();
bool isCollidingWithSnake(Position pos);
