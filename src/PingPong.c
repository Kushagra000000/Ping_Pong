#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define SDL_Flags (SDL_INIT_VIDEO | SDL_INIT_AUDIO)
#define PLAYER_WIDTH 10         // each racket is 10 pixels long
#define PLAYER_X 100
#define PLAYER_SPEED 500
#define BALL_SPEED 500
#define BALL_DIMENTIONS 50

static Uint64 previousTime;


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
    int x;
    int y;
    bool x_vel;     // True means moving towards player 2
    bool y_vel;     // True means moving downwards
} Ball;

typedef struct {
    int screenHeight;
    int screenWidth;
    int screenFPS;
} Screen;

typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Surface * surface;
    SDL_Texture * texture;
    Player player1;
    Player player2;
    Ball ball;
    Screen screen;
    float elapsedTime;
} Appstate;

// ============================================================================
// ========================== My functions ====================================
// ============================================================================

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

    // Set Player 1 Properly
    if (state->player1.y <= 0) state->player1.y = 0;
    if (state->player1.y + state->player1.player_length >= state->screen.screenHeight)
        state->player1.y = state->screen.screenHeight - state->player1.player_length;

    // Set Player 2 Properly
    if (state->player2.y <= 0) state->player2.y = 0;
    if (state->player2.y + state->player2.player_length >= state->screen.screenHeight)
        state->player2.y = state->screen.screenHeight - state->player2.player_length;


    // Ball Movement ----------------------------------------------------------------
    // state->player1.y -= PLAYER_SPEED * (state->elapsedTime / 1000.0f);
    if (state->ball.x_vel) {
        state->ball.x += BALL_SPEED * (state->elapsedTime / 1000.0f);
    }
    else {
        state->ball.x -= BALL_SPEED * (state->elapsedTime / 1000.0f);
    }

    if (state->ball.y_vel) {
        state->ball.y += BALL_SPEED * (state->elapsedTime / 1000.0f);
    }
    else {
        state->ball.y -= BALL_SPEED * (state->elapsedTime / 1000.0f);
    }

    // Collision checking ------------------------------------------------------------
    if (state->ball.x + BALL_DIMENTIONS < 0 || state->ball.x > state->screen.screenWidth)
        return 1;

    if (state->ball.y <= 0) state->ball.y_vel = 1;
    if (state->ball.y + BALL_DIMENTIONS >= state->screen.screenHeight) state->ball.y_vel = 0;


    // Right paddle
    if (state->ball.x + BALL_DIMENTIONS > state->screen.screenWidth - PLAYER_X
        && state->ball.x < state->screen.screenWidth - PLAYER_X + PLAYER_WIDTH
        && state->ball.y + BALL_DIMENTIONS > state->player2.y
        && state->ball.y < state->player2.y + state->player2.player_length)
            state->ball.x_vel = 0; // bounce back left

    // Left paddle
    if (state->ball.x < PLAYER_X + PLAYER_WIDTH
        && state->ball.x + BALL_DIMENTIONS > PLAYER_X
        && state->ball.y + BALL_DIMENTIONS > state->player1.y
        && state->ball.y < state->player1.y + state->player1.player_length)
            state->ball.x_vel = 1; // bounce back right


    return 0;
}

void render(void * appstate) {
    Appstate *state = appstate;

    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    // Create Rectangle
    const SDL_FRect Player_1 = { PLAYER_X, state->player1.y, PLAYER_WIDTH, state->player1.player_length};
    const SDL_FRect Player_2 = { state->screen.screenWidth - PLAYER_X, state->player2.y, PLAYER_WIDTH, state->player2.player_length};
    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(state->renderer, &Player_1);
    SDL_RenderFillRect(state->renderer, &Player_2);

    SDL_FRect Ball;
    Ball.x = state->ball.x;
    Ball.y = state->ball.y;
    Ball.w = 50.0f;
    Ball.h = 50.0f;

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

    state->screen.screenWidth = sScreen->w;
    state->screen.screenHeight = sScreen->h;
    state->screen.screenFPS = sScreen->refresh_rate;     // not implementing the use of this yet.

    state->window = SDL_CreateWindow("Ping Pong - window creation", state->screen.screenWidth, state->screen.screenHeight, 
            SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
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

    // Initialize Spirit and Change colour_key
    state->surface = SDL_LoadBMP("./images/ball.bmp");
    // state->surface = SDL_LoadBMP("../images/Garou.bmp");
    if (!state->surface) {
        SDL_Log("Surface creation unsucessfull: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Get colour map for magenta (the one I want to remove)
    Uint32 color_key = SDL_MapSurfaceRGB(state->surface, 255, 0, 255);

    // enable colorkey (transparent pixel)
    if (!SDL_SetSurfaceColorKey(state->surface, true, color_key)) {
        SDL_Log("SDL_SetSurfaceColorKey failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Now create texture
    state->texture = SDL_CreateTextureFromSurface(state->renderer, state->surface);
    if (!state->texture) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    // Initialise Players
    state->player1.y = state->player2.y = state->screen.screenHeight / 2;
    state->player1.dir = state->player2.dir = 0;
    state->player1.player_length = state->player2.player_length = (int)(state->screen.screenHeight / 8);

    // Randomly initialise Ball
    state->ball.x = (SDL_randf() * ((float)(state->screen.screenWidth - 500))) + 200;
    state->ball.y = (SDL_randf() * ((float)(state->screen.screenHeight - 500))) + 200;
    // state->ball.x_vel = 30 * SDL_randf() + 30; // speed in pixels per second, so each 16 miliseconds it will travel 30/16 pixels.
    // state->ball.y_vel = 30 * SDL_randf() + 30;  // let's see if this is enough, or too much, and then change...
    state->ball.x_vel = (SDL_randf() > 0.5f);
    state->ball.y_vel = (SDL_randf() > 0.5f);

    SDL_Log("### \n stats \n ###");
    SDL_Log("Screen Height %d", state->screen.screenHeight);
    SDL_Log("Screen Width %d", state->screen.screenWidth);
    SDL_Log("Player Length %d", state->player1.player_length);


    *appstate = state;

    // SDL_Delay(1000);
    previousTime = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    
    Appstate * state = appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;

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

    const bool * key_pressed = SDL_GetKeyboardState(NULL);

    if (key_pressed[SDL_SCANCODE_ESCAPE])
        return SDL_APP_SUCCESS;
    if (key_pressed[SDL_SCANCODE_W])
        state->player1.y -= PLAYER_SPEED * (state->elapsedTime / 1000.0f);
    if (key_pressed[SDL_SCANCODE_S])
        state->player1.y += PLAYER_SPEED * (state->elapsedTime / 1000.0f);

    if (key_pressed[SDL_SCANCODE_UP])
        state->player2.y -= PLAYER_SPEED * (state->elapsedTime / 1000.0f);
    if (key_pressed[SDL_SCANCODE_DOWN])
        state->player2.y += PLAYER_SPEED * (state->elapsedTime / 1000.0f);

    // 1 second = 1000 miliseconds
    // current framerate = 60 frames per second
    // therefore 1 frame every 1000/60 = 16 miliseconds
    const Uint64 currentTime = SDL_GetTicks();
    state->elapsedTime = currentTime - previousTime; // for movement multiplication
    previousTime = currentTime;

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