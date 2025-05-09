#ifndef CONFIG_H
#define CONFIG_H

// Screen Dimensions & Core Sizes
#define SCREEN_WIDTH 1045
#define SCREEN_HEIGHT 690
#define PLAYER_SQUARE_SIZE 95
#define ROAD_HEIGHT 170

// Road and Player Bounds (Calculated from above defines)
const int ROAD_Y = SCREEN_HEIGHT - ROAD_HEIGHT;
const int PLAYER_BOUNDS_TOP = ROAD_Y;
const int PLAYER_BOUNDS_BOTTOM = ROAD_Y + ROAD_HEIGHT - PLAYER_SQUARE_SIZE;

// Menu Config
const int BUTTON_WIDTH = 250;
const int BUTTON_HEIGHT = 100;
const int BUTTON_X = 50;
const int BUTTON_Y_PLAY = 200;
const int BUTTON_Y_CHARACTER = 320;
const int BUTTON_Y_ABOUT = 440;
const int BUTTON_Y_QUIT = 560;
const int MENU_ANIM_FRAMES = 4;
const float MENU_ANIM_SPEED = 0.25f;

// Intro Config
const int INTRO_SLIDE_COUNT = 4;
const unsigned int SLIDE_DEFAULT_DURATION_MS = 22000; // Using unsigned int for SDL_Ticks consistency

// Gameplay Config
const float PLAYER_VERT_SPEED = 300.0f;
const float PLAYER_HORIZ_SPEED = 100.0f;
const float PLAYER_HORIZ_MOVE_RANGE = 20.0f;
const float BACKGROUND_SCROLL_SPEED = 200.0f;
const int PLAYER_START_X = 100;
const int BARRIER_WIDTH = 50;
const int BARRIER_HEIGHT = 50;
const float BARRIER_SPEED = 400.0f;
const float BARRIER_SPAWN_INTERVAL = 2.0f;
const int MAX_BARRIERS = 5;
const int COIN_WIDTH = BARRIER_WIDTH / 2;
const int COIN_HEIGHT = BARRIER_HEIGHT / 2;
const float WIN_TIME = 40.0f;
const float WIN_DELAY_TIME = 3.0f;
const float COIN_SPAWN_INTERVAL = 1.5f;


// Road Perspective Config
const float ROAD_PERSPECTIVE_FAR_SCALE = 1.0f;
const float ROAD_PERSPECTIVE_NEAR_SCALE = 1.0f;
const float ROAD_TEXTURE_V_POWER = 1.2f;
const float ROAD_TEXTURE_V_START_OFFSET = 0.0f;

#endif // CONFIG_H