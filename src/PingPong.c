#include "PingPong.h"
#include <stdlib.h>

SDL_AppResult tick(void * appstate) {

    Appstate * state = appstate;

    int r = update(state);
    // render(); // then based on the values of the location, we render the scene for a particular tick.
    render(state);

    if (r == 1) return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

int update(void * appstate) {
    Appstate * state = appstate;

    // Move Player 1
    if (state->player1.y <= 0.0f) state->player1.y = 0.0f;
    if (state->player1.y + state->player1.player_length >= state->screenHeight)
        state->player1.y = state->screenHeight - state->player1.player_length;

    // Move Player 2
    if (state->player2.y <= 0.0f) state->player2.y = 0.0f;
    if (state->player2.y + state->player2.player_length >= state->screenHeight)
        state->player2.y = state->screenHeight - state->player2.player_length;


    // Ball Movement ----------------------------------------------------------------
    // state->player1.y -= PLAYER_SPEED * (state->elapsedTime / 1000.0f);

    state->ball.rect.x += state->ball.vx * (state->elapsedTime / 1000.0f);
    state->ball.rect.y += state->ball.vy * (state->elapsedTime / 1000.0f);

    // Wall bounce checking
    if (state->ball.rect.y <= 0.0f) {
        state->ball.rect.y = 0.0f;
        state->ball.vy = -state->ball.vy;
    }
    if (state->ball.rect.y + state->ball.rect.h >= (float)state->screenHeight) {
        state->ball.rect.y = (float)state->screenHeight - state->ball.rect.h;
        state->ball.vy = -state->ball.vy;
    }

    // Score checking
    if (state->ball.rect.x + state->ball.rect.w < 0.0f || state->ball.rect.x > (float)state->screenWidth)
        return 1;


    // Collision checking ------------------------------------------------------------
    if (state->ball.rect.x + BALL_DIMENTIONS < 0 || state->ball.rect.x > state->screenWidth)
        return 1;

    // if (state->ball.rect.y <= 0) state->ball.vy = 1;
    // if (state->ball.rect.y + BALL_DIMENTIONS >= state->screenHeight) state->ball.vy = 0;


    // Right paddle
    if (state->ball.rect.x + BALL_DIMENTIONS > state->screenWidth - PLAYER_X
        && state->ball.rect.x < state->screenWidth - PLAYER_X + PLAYER_WIDTH
        && state->ball.rect.y + BALL_DIMENTIONS > state->player2.y
        && state->ball.rect.y < state->player2.y + state->player2.player_length)
    {
        state->ball.vx = -state->ball.vx; // bounce back left

        // Bud Fix: Avoide recollision, ball stuck on paddle
        state->ball.rect.x -= 20;

        // Impart racket momentum to ball
        float paddle_VelY = (float)state->player2.dir * (float)PLAYER_SPEED; // momentum (and it's direction)
        state->ball.vy = paddle_VelY * 0.9f;        // factor by which ball is affected.

        // also change vx, so paddle movement feels "वज़नदार"
        float vxSign = (state->ball.vx > 0.0f) ? 1.0f : -1.0f;
        state->ball.vx += vxSign * (SDL_fabsf(paddle_VelY) * 0.25f);
        if (state->ball.vx > BALL_MAX_SPEED) {
            state->ball.vx = BALL_MAX_SPEED;
        }
    }

    // Left paddle
    if (state->ball.rect.x < PLAYER_X + PLAYER_WIDTH
        && state->ball.rect.x + BALL_DIMENTIONS > PLAYER_X
        && state->ball.rect.y + BALL_DIMENTIONS > state->player1.y
        && state->ball.rect.y < state->player1.y + state->player1.player_length)
    {
        state->ball.vx = -state->ball.vx; // bounce back right

        // Bud Fix: Avoide recollision, ball stuck on paddle
        state->ball.rect.x += 20;

        // impart racket momentum to ball
        float paddle_VelY = (float)state->player1.dir * (float)PLAYER_SPEED; // momentum (and it's direction)
        state->ball.vy = paddle_VelY * 0.9f;        // factor by which ball is affected.

        // also change vx, so paddle movement feels "वज़नदार"
        float vxSign = (state->ball.vx > 0.0f) ? 1.0f : -1.0f;
        state->ball.vx += vxSign * (SDL_fabsf(paddle_VelY) * 0.25f);
        if (state->ball.vx > BALL_MAX_SPEED) {
            state->ball.vx = BALL_MAX_SPEED;
        }
    }


    return 0;
}

void render(void * appstate) {
    Appstate *state = appstate;

    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    // Create Rectangle
    const SDL_FRect Player_1 = { PLAYER_X, state->player1.y, PLAYER_WIDTH, state->player1.player_length};
    const SDL_FRect Player_2 = { state->screenWidth - PLAYER_X, state->player2.y, PLAYER_WIDTH, state->player2.player_length};
    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(state->renderer, &Player_1);
    SDL_RenderFillRect(state->renderer, &Player_2);

    SDL_FRect Ball;
    Ball.x = state->ball.rect.x;
    Ball.y = state->ball.rect.y;
    Ball.w = state->ball.rect.w;
    Ball.h = state->ball.rect.h;

    SDL_RenderTexture(state->renderer, state->texture, NULL, &Ball);  // ****************** IMPORTANT ********************

    SDL_RenderPresent(state->renderer);
}


// ============================================================================
// ========================== Callback functions ==============================
// ============================================================================

SDL_AppResult SDL_AppInit(void **appstate, int argc, char* argv[]) {

    Appstate * state = SDL_malloc(sizeof(Appstate));


    if (!SDL_SetAppMetadata("Ping Pong game", "1.0", "SDL ping pong project")) {
        SDL_Log("Failed to start SDL: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_Init(SDL_Flags)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_DisplayMode *sScreen = SDL_GetCurrentDisplayMode(1);
    if (!sScreen) {
        SDL_Log("SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->screenWidth = sScreen->w;
    state->screenHeight = sScreen->h;
    state->screenFPS = sScreen->refresh_rate;     // not implementing the use of this yet.

    state->window = SDL_CreateWindow("Ping Pong - Window", state->screenWidth, state->screenHeight, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
    if (!state->window) {
        SDL_Log("Window could not be created: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->renderer = SDL_CreateRenderer(state->window, NULL);
    if (!state->renderer) {
        SDL_Log("Renderer could not be created: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Use max FPS and reduce jitter between frames
    SDL_SetRenderVSync(state->renderer, true);

    char path[1024];
    const char *base = SDL_GetBasePath();
    if (base) {
        SDL_snprintf(path, sizeof(path), "%simages/ball.bmp", base);
        SDL_free((void*)base);
    }
    else {
        SDL_snprintf(path, sizeof(path), "images/ball.bmp");
    }

    state->surface = SDL_LoadBMP(path);
    if (!state->surface) {
        SDL_Log("SDL_LoadBMP failed: %s -- path: %s\n", SDL_GetError(), path);
        return SDL_APP_FAILURE;
    }

    // Get colour map for magenta (the one I want to remove)
    Uint32 color_key = SDL_MapSurfaceRGB(state->surface, 255, 0, 255);
    if (!SDL_SetSurfaceColorKey(state->surface, true, color_key)) {
        SDL_Log("SDL_SetSurfaceColorKey failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->texture = SDL_CreateTextureFromSurface(state->renderer, state->surface);
    if (!state->texture) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // free surface (texture already created)
    SDL_DestroySurface(state->surface);
    state->surface = NULL;


    // Initialise Players
    state->player1.y = state->player2.y = state->screenHeight / 2;
    state->player1.dir = state->player2.dir = 0;
    state->player1.player_length = state->player2.player_length = (int)(state->screenHeight / 8);

    // Randomly initialise Ball
    state->ball.rect.w = BALL_DIMENTIONS;
    state->ball.rect.h = BALL_DIMENTIONS;
    // state->ball.rect.x = (SDL_randf() * ((float)(state->screenWidth - 500))) + 200;
    // state->ball.rect.y = (SDL_randf() * ((float)(state->screenHeight - 500))) + 200;
    state->ball.rect.x = state->screenWidth / 2;
    state->ball.rect.y = state->screenHeight / 2;

    state->ball.vx = (SDL_randf() > 0.5f) ? BALL_SPEED : -BALL_SPEED;
    state->ball.vy = (SDL_randf() > 0.5f) ? BALL_SPEED : -BALL_SPEED;

    // SDL_Log("### \n stats \n ###");
    // SDL_Log("Screen Height %d", state->screen.screenHeight);
    // SDL_Log("Screen Width %d", state->screen.screenWidth);
    // SDL_Log("Player Length %d", state->player1.player_length);

    state->previousTime = SDL_GetTicks();
    *appstate = state;

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    
    Appstate * state = appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_TERMINATING:
            return SDL_APP_SUCCESS;
            break;

        case SDL_EVENT_KEY_DOWN:
        {
            SDL_Scancode sc = event->key.scancode;

            if (sc == SDL_SCANCODE_ESCAPE) return SDL_APP_SUCCESS;
            if (sc == SDL_SCANCODE_UP) state->p2_up = true;
            if (sc == SDL_SCANCODE_DOWN) state->p2_down = true;
            if (sc == SDL_SCANCODE_W) state->p1_up = true;
            if (sc == SDL_SCANCODE_S) state->p1_down = true;

            //update player dirs (up = -1, down = +1, both/none = 0)
            if (state->p1_up && !state->p1_down) state->player1.dir = -1;
            else if (state->p1_down && !state->p1_up) state->player1.dir = 1;
            else state->player1.dir = 0;\

            if (state->p2_up && !state->p2_down) state->player2.dir = -1;
            else if (state->p2_down && !state->p2_up) state->player2.dir = 1;
            else state->player2.dir = 0;

            return SDL_APP_CONTINUE;
        }

        case SDL_EVENT_KEY_UP:
        {
            SDL_Scancode sc = event->key.scancode;

            if (sc == SDL_SCANCODE_W) state->p1_up = false;
            if (sc == SDL_SCANCODE_S) state->p1_down = false;
            if (sc == SDL_SCANCODE_UP) state->p2_up = false;
            if (sc == SDL_SCANCODE_DOWN) state->p2_down = false;

            // update player dirs
            if (state->p1_up && !state->p1_down) state->player1.dir = -1;
            else if (state->p1_down && !state->p1_up) state->player1.dir = 1;
            else state->player1.dir = 0;

            if (state->p2_up && !state->p2_down) state->player2.dir = -1;
            else if (state->p2_down && !state->p2_up) state->player2.dir = 1;
            else state->player2.dir = 0;

            return SDL_APP_CONTINUE;
        }


        // For mobile handeling...
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_MOTION:
        {
            // event->tfinger.x and y are normalised (0 to 1)
            float y_px = 0.0f;
            y_px = event->tfinger.y * (float)state->screenHeight;

            if (event->tfinger.x < 0.5f)        // left half - for player 1 movement
                state->player1.y = (int)(y_px - state->player1.player_length * 0.5f);
            else
                state->player2.y = (int)(y_px - state->player2.player_length * 0.5f);

            return SDL_APP_CONTINUE;
        }
        case SDL_EVENT_FINGER_UP:
            return SDL_APP_CONTINUE;


        default:
            return SDL_APP_CONTINUE;
            break;
    }

    return SDL_APP_CONTINUE;
}

/**
 * Logic, have a tick function to handle each tick
//  * tick calls input(), update() and render();
 * input() handles the events... keyboard strokes, and touch
 * update() handles the game logic and all
 * render() will actually render the stuff out using .. renderDrawColour, renderClear,  render shapes and all, then renderPresent
 * 
 * for ticks, use currentTime and previousTime
 */
SDL_AppResult SDL_AppIterate(void *appstate) {

    Appstate * state = appstate;

    if (state->player1.dir < 0)
        state->player1.y -= PLAYER_SPEED * (state->elapsedTime / 1000.0f);
    else if (state->player1.dir > 0)
        state->player1.y += PLAYER_SPEED * (state->elapsedTime / 1000.0f);

    if (state->player2.dir < 0)
        state->player2.y -= PLAYER_SPEED * (state->elapsedTime / 1000.0f);
    else if (state->player2.dir > 0)
        state->player2.y += PLAYER_SPEED * (state->elapsedTime / 1000.0f);

    // 1 second = 1000 miliseconds
    // current framerate = 60 frames per second
    // therefore 1 frame every 1000/60 = 16 miliseconds
    const Uint64 currentTime = SDL_GetTicks();
    state->elapsedTime = currentTime - state->previousTime; // for movement multiplication
    state->previousTime = currentTime;

    SDL_AppResult AppResult = tick(appstate);

    return AppResult;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    Appstate * state = appstate;
    if (!state) return;
    if (state->texture) SDL_DestroyTexture(state->texture);
    if (state->surface) SDL_DestroySurface(state->surface);
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    SDL_free(state);
}