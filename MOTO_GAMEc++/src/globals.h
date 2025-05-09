#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>
#include <random>
#include "types.h" // For GameState, Barrier, Coin

// Window Title (Actual definition in main.cpp)
extern const char* const WINDOW_TITLE;

// SDL Objects
extern SDL_Window* gWindow;
extern SDL_Renderer* gRenderer;
extern TTF_Font* gFont;

// Colors
extern SDL_Color gTextColor;
extern SDL_Color gAboutTextColor;
extern SDL_Color gButtonHoverColor;
extern SDL_Color gHeaderColor;

// Textures
extern SDL_Texture* gLogoTexture;
extern SDL_Texture* gLogoTexture2;
extern SDL_Texture* gLogoTexture3;
extern SDL_Texture* gLogoTexture4;
extern SDL_Texture* gLogoTexture5;
extern SDL_Texture* gCharacter01Texture;
extern SDL_Texture* gCharacter02Texture;
extern SDL_Texture* gPlayerMaleTexture;
extern SDL_Texture* gPlayerFemaleTexture;
extern SDL_Texture* gCoinTexture;
extern std::vector<SDL_Texture*> gMenuBgFrames;
extern std::vector<SDL_Texture*> gIntroSlides;
extern SDL_Texture* gSkipButtonTexture;
extern SDL_Texture* gGameBgFarTexture;
extern SDL_Texture* gGameBgNearTexture;
extern SDL_Texture* gBarrierTextures[3]; // Declaration for array of textures
extern SDL_Texture* gLoseScreenTexture;
extern SDL_Texture* gWinScreenTexture;


// Game State & UI Variables
extern int gSelectedCharacter;
extern SDL_Rect gPlayButtonRect;
extern SDL_Rect gCharacterButtonRect;
extern SDL_Rect gAboutButtonRect;
extern SDL_Rect gQuitButtonRect;
extern SDL_Rect gSkipButtonRect;

// Menu Animation
extern int gCurrentMenuFrame;
extern float gMenuAnimTimer;

// Intro State
extern int gCurrentIntroSlide;
extern unsigned int gIntroSlideStartTime;
extern int gIntroAudioChannel;

// Gameplay Variables
extern float gPlayerY;
extern float gPlayerX;
extern bool gMoveUp;
extern bool gMoveDown;
extern bool gMoveLeft;
extern bool gMoveRight;
extern float gGameTimer;
extern float gWinDelayTimer;
extern float gBackgroundX;

// Barriers & Coins
extern std::vector<Barrier> gBarriers;
extern float gBarrierSpawnTimer;
extern std::vector<Coin> gCoins;
extern float gCoinSpawnTimer;
extern int gCoinCounter;

// Random Number Generation
extern std::mt19937 gRandomGenerator;

// Game State
extern GameState gCurrentState;

// Sounds & Music
extern Mix_Music* gMenuMusic;
extern std::vector<Mix_Chunk*> gIntroAudio;
extern Mix_Chunk* gLoseSound;
extern Mix_Chunk* gWinSound;


#endif // GLOBALS_H