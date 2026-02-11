#ifndef PINGPONG_H
#define PINGPONG_H

#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define SDL_Flags (SDL_INIT_VIDEO | SDL_INIT_AUDIO)
#define PLAYER_WIDTH 10         // each racket is 10 pixels long
#define PLAYER_X 100
#define PLAYER_SPEED 500.0f
#define BALL_SPEED 500.0f
#define BALL_MAX_SPEED 750.0f
#define BALL_DIMENTIONS 50.0f


// ============================================================================
// ========================== Function Declarations ===========================
// ============================================================================

int update(void * appstate);
void render(void * appstate);

// ============================================================================
// ========================== Structs =========================================
// ============================================================================

typedef struct {
    int dir;
    int y;
    int player_length;  // length of the player's racket
} Player;

typedef struct {
    SDL_FRect rect;
    float vx;
    float vy;
} Ball;


typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Surface * surface;
    SDL_Texture * texture;

    Player player1;
    Player player2;
    Ball ball;

    int screenHeight;
    int screenWidth;
    int screenFPS;

    float elapsedTime;
    Uint64 previousTime;

    // Movement
    bool p1_up;
    bool p1_down;
    bool p2_up;
    bool p2_down;
} Appstate;

#endif