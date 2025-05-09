#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <cmath>

// Project-Specific Headers
#include "config.h"    // Defines and consts
#include "types.h"     // Enums and structs
#include "globals.h"   // Extern global variable declarations
#include "functions.h" // Function prototypes

// --- Global Variable Definitions ---
const char* const WINDOW_TITLE = "BROTHERHOOD"; // Definition

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* gFont = nullptr;
SDL_Color gTextColor = { 0, 0, 0, 255 };
SDL_Color gAboutTextColor = { 255, 255, 255, 255 };
SDL_Color gButtonHoverColor = { 255, 255, 0, 255 };
SDL_Color gHeaderColor = {255, 200, 0, 255};
SDL_Texture* gLogoTexture = nullptr;
SDL_Texture* gLogoTexture2 = nullptr;
SDL_Texture* gLogoTexture3 = nullptr;
SDL_Texture* gLogoTexture4 = nullptr;
SDL_Texture* gLogoTexture5 = nullptr;
SDL_Texture* gCharacter01Texture = nullptr;
SDL_Texture* gCharacter02Texture = nullptr;
SDL_Texture* gPlayerMaleTexture = nullptr;
SDL_Texture* gPlayerFemaleTexture = nullptr;
SDL_Texture* gCoinTexture = nullptr;
int gSelectedCharacter = 0;

SDL_Rect gPlayButtonRect = { BUTTON_X - 10, BUTTON_Y_PLAY - 10, BUTTON_WIDTH, BUTTON_HEIGHT };
SDL_Rect gCharacterButtonRect = { BUTTON_X - 10, BUTTON_Y_CHARACTER - 10, BUTTON_WIDTH, BUTTON_HEIGHT };
SDL_Rect gAboutButtonRect = { BUTTON_X - 10, BUTTON_Y_ABOUT - 10, BUTTON_WIDTH, BUTTON_HEIGHT };
SDL_Rect gQuitButtonRect = { BUTTON_X - 10, BUTTON_Y_QUIT - 10, BUTTON_WIDTH, BUTTON_HEIGHT };
std::vector<SDL_Texture*> gMenuBgFrames;
int gCurrentMenuFrame = 0;
float gMenuAnimTimer = 0.0f;
Mix_Music* gMenuMusic = nullptr;

std::vector<SDL_Texture*> gIntroSlides;
std::vector<Mix_Chunk*> gIntroAudio;
SDL_Texture* gSkipButtonTexture = nullptr;
SDL_Rect gSkipButtonRect; // Will be initialized in loadMedia
int gCurrentIntroSlide = 0;
unsigned int gIntroSlideStartTime = 0;
int gIntroAudioChannel = -1;

SDL_Texture* gGameBgFarTexture = nullptr;
SDL_Texture* gGameBgNearTexture = nullptr;
SDL_Texture* gBarrierTextures[3] = {nullptr, nullptr, nullptr};
SDL_Texture* gLoseScreenTexture = nullptr;
SDL_Texture* gWinScreenTexture = nullptr;
Mix_Chunk* gLoseSound = nullptr;
Mix_Chunk* gWinSound = nullptr;
float gPlayerY = 0.0f;
float gPlayerX = PLAYER_START_X;
bool gMoveUp = false;
bool gMoveDown = false;
bool gMoveLeft = false;
bool gMoveRight = false;
float gGameTimer = 0.0f;
float gWinDelayTimer = 0.0f;
float gBackgroundX = 0.0f;

std::vector<Barrier> gBarriers;
float gBarrierSpawnTimer = 0.0f;
std::vector<Coin> gCoins;
float gCoinSpawnTimer = 0.0f;
int gCoinCounter = 0;

std::random_device gRandomDevice_for_seeding; // Keep this local to main.cpp for seeding
std::mt19937 gRandomGenerator(gRandomDevice_for_seeding());

GameState gCurrentState = GameState::MENU;


// --- Function Definitions ---

// Helper Function: Load Texture (using SDL_image)
SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        std::cerr << "ERROR: Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    } else {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == nullptr) {
            std::cerr << "ERROR: Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

// Helper Function: Render Text (using SDL_ttf)
bool renderText(const std::string& text, int x, int y, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {
    if (!font) { std::cerr << "ERROR: Cannot render text - Font not loaded!" << std::endl; return false; }
    if (!renderer) { std::cerr << "ERROR: Cannot render text - Renderer is null!" << std::endl; return false; }
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (textSurface == nullptr) { std::cerr << "ERROR: Unable to render text surface for \"" << text << "\"! SDL_ttf Error: " << TTF_GetError() << std::endl; return false; }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) { std::cerr << "ERROR: Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl; SDL_FreeSurface(textSurface); return false; }
    SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };
    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(textTexture);
    return true;
}

// Initialization
bool initializeSDL() {
    std::cout << "Initializing SDL..." << std::endl;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { std::cerr << "FATAL ERROR: SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl; return false; }
    std::cout << " -> SDL Core Initialized." << std::endl;
    if (TTF_Init() == -1) { std::cerr << "FATAL ERROR: SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl; SDL_Quit(); return false; }
     std::cout << " -> SDL_ttf Initialized." << std::endl;
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) { std::cerr << "FATAL ERROR: SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl; TTF_Quit(); SDL_Quit(); return false; }
     std::cout << " -> SDL_image Initialized for PNG." << std::endl;
    std::cout << "Initializing SDL_mixer..." << std::endl;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { std::cerr << "FATAL ERROR: SDL_mixer could not initialize audio device! SDL_mixer Error: " << Mix_GetError() << std::endl; IMG_Quit(); TTF_Quit(); SDL_Quit(); return false; }
     else { std::cout << " -> Audio device opened successfully (44100Hz, Stereo)." << std::endl; }
    std::cout << "Creating Window..." << std::endl;
    gWindow = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) { std::cerr << "FATAL ERROR: Window could not be created! SDL_Error: " << SDL_GetError() << std::endl; Mix_CloseAudio(); Mix_Quit(); IMG_Quit(); TTF_Quit(); SDL_Quit(); return false; }
     std::cout << " -> Window created." << std::endl;
    std::cout << "Creating Renderer..." << std::endl;
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == nullptr) { std::cerr << "FATAL ERROR: Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl; SDL_DestroyWindow(gWindow); Mix_CloseAudio(); Mix_Quit(); IMG_Quit(); TTF_Quit(); SDL_Quit(); return false; }
    SDL_SetRenderDrawColor(gRenderer, 0x22, 0x22, 0x22, 0xFF);
     std::cout << " -> Renderer created." << std::endl;
    std::cout << "All SDL Subsystems Initialized Successfully." << std::endl;
    return true;
}

// Game State Reset
void resetGameState() {
    gPlayerY = PLAYER_BOUNDS_TOP + (PLAYER_BOUNDS_BOTTOM - PLAYER_BOUNDS_TOP) / 2; 
    gPlayerX = PLAYER_START_X;
    gMoveUp = false;
    gMoveDown = false;
    gMoveLeft = false;
    gMoveRight = false;
    gGameTimer = 0.0f;
    gWinDelayTimer = 0.0f;
    gBackgroundX = 0.0f; 
    gBarriers.clear();
    gBarrierSpawnTimer = 0.0f;
    gCoins.clear();
    gCoinSpawnTimer = 0.0f;
    gCoinCounter = 0;
}

// Load Media
bool loadMedia() {
     std::cout << "Loading Media..." << std::endl;
    gFont = TTF_OpenFont("../assets/fonts/game_font.ttf", 28);
    if (gFont == nullptr) { std::cerr << "FATAL ERROR: Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl; return false; }

    gMenuBgFrames.resize(MENU_ANIM_FRAMES);
    for (int i = 0; i < MENU_ANIM_FRAMES; ++i) {
        gMenuBgFrames[i] = loadTexture("../assets/images/menu_anim/bg_frame_0" + std::to_string(i + 1) + ".png", gRenderer);
        if (gMenuBgFrames[i] == nullptr) { return false; }
    }

    gIntroSlides.resize(INTRO_SLIDE_COUNT);
    for (int i = 0; i < INTRO_SLIDE_COUNT; ++i) {
        gIntroSlides[i] = loadTexture("../assets/images/intro/intro_slide_0" + std::to_string(i + 1) + ".png", gRenderer);
    }
    gIntroAudio.resize(INTRO_SLIDE_COUNT);
    for (int i = 0; i < INTRO_SLIDE_COUNT; ++i) {
        gIntroAudio[i] = Mix_LoadWAV(("../assets/audio/intro_slide_0" + std::to_string(i + 1) + ".wav").c_str());
        if (gIntroAudio[i] == nullptr) { std::cerr << "WARNING: Failed to load intro audio " << i+1 << " SDL_mixer Error: " << Mix_GetError() << std::endl;}
    }

    gSkipButtonTexture = loadTexture("../assets/images/ui/skip_button.png", gRenderer);
    if (gSkipButtonTexture) {
        int skipW, skipH; SDL_QueryTexture(gSkipButtonTexture, NULL, NULL, &skipW, &skipH);
        gSkipButtonRect = { SCREEN_WIDTH - skipW - 20, SCREEN_HEIGHT - skipH - 20, skipW, skipH };
    }

    gGameBgFarTexture = loadTexture("../assets/images/background_far.png", gRenderer);
    if (!gGameBgFarTexture) return false;
    gGameBgNearTexture = loadTexture("../assets/images/background_near.jpg", gRenderer); 
    if (!gGameBgNearTexture) return false;
    
    gBarrierTextures[0] = loadTexture("../assets/images/barrier_01.png", gRenderer);
    gBarrierTextures[1] = loadTexture("../assets/images/barrier_02.png", gRenderer);
    gBarrierTextures[2] = loadTexture("../assets/images/barrier_03.png", gRenderer);

    gCoinTexture = loadTexture("../assets/images/coins.png", gRenderer);

    gCharacter01Texture = loadTexture("../assets/images/character_01.png", gRenderer);
    gCharacter02Texture = loadTexture("../assets/images/character_02.png", gRenderer);
    gPlayerMaleTexture = loadTexture("../assets/images/select/player_male.png", gRenderer);
    gPlayerFemaleTexture = loadTexture("../assets/images/select/player_female.png", gRenderer);

    gLogoTexture = loadTexture("../assets/images/logo_01.png", gRenderer);
    gLogoTexture2 = loadTexture("../assets/images/logo_02.png", gRenderer);
    gLogoTexture3 = loadTexture("../assets/images/logo_03.png", gRenderer);
    gLogoTexture4 = loadTexture("../assets/images/logo_04.png", gRenderer);
    gLogoTexture5 = loadTexture("../assets/images/logo_05.png", gRenderer);

    std::cout << " -> Loading Lose Screen Texture: ../assets/images/endscreen/lose_slide.png" << std::endl;
    gLoseScreenTexture = loadTexture("../assets/images/endscreen/lose_slide.png", gRenderer);
    if (gLoseScreenTexture == nullptr) { std::cerr << "WARNING: Failed to load lose screen texture!" << std::endl; }

    std::cout << " -> Loading Win Screen Texture: ../assets/images/endscreen/win_slide.png" << std::endl;
    gWinScreenTexture = loadTexture("../assets/images/endscreen/win_slide.png", gRenderer);
    if (gWinScreenTexture == nullptr) { std::cerr << "WARNING: Failed to load win screen texture!" << std::endl; }

    std::cout << " -> Loading Lose Sound: ../assets/audio/lose_audio.wav" << std::endl;
    gLoseSound = Mix_LoadWAV("../assets/audio/lose_audio.wav");
    if (gLoseSound == nullptr) { std::cerr << "WARNING: Failed to load lose sound! SDL_mixer Error: " << Mix_GetError() << std::endl; }

    std::cout << " -> Loading Win Sound: ../assets/audio/win_audio.wav" << std::endl;
    gWinSound = Mix_LoadWAV("../assets/audio/win_audio.wav");
    if (gWinSound == nullptr) { std::cerr << "WARNING: Failed to load win sound! SDL_mixer Error: " << Mix_GetError() << std::endl; }
    
    std::cout << " -> Loading Menu Music: ../assets/audio/music_menu.wav" << std::endl;
    gMenuMusic = Mix_LoadMUS("../assets/audio/music_menu.wav");
    if (gMenuMusic == nullptr) { std::cerr << "WARNING: Failed to load menu music! SDL_mixer Error: " << Mix_GetError() << std::endl; }
    
    std::cout << "Media Loading Complete." << std::endl;
    return true;
}

// SDL Cleanup
void closeSDL() {
    if (gSkipButtonTexture) { SDL_DestroyTexture(gSkipButtonTexture); gSkipButtonTexture = nullptr; }
    if (gGameBgFarTexture) { SDL_DestroyTexture(gGameBgFarTexture); gGameBgFarTexture = nullptr; }
    if (gGameBgNearTexture) { SDL_DestroyTexture(gGameBgNearTexture); gGameBgNearTexture = nullptr; }
    for (int i = 0; i < 3; ++i) { if (gBarrierTextures[i]) { SDL_DestroyTexture(gBarrierTextures[i]); gBarrierTextures[i] = nullptr; } }
    if (gLoseScreenTexture) { SDL_DestroyTexture(gLoseScreenTexture); gLoseScreenTexture = nullptr; }
    if (gWinScreenTexture) { SDL_DestroyTexture(gWinScreenTexture); gWinScreenTexture = nullptr; }
    if (gLogoTexture) { SDL_DestroyTexture(gLogoTexture); gLogoTexture = nullptr; }
    if (gLogoTexture2) { SDL_DestroyTexture(gLogoTexture2); gLogoTexture2 = nullptr; }
    if (gLogoTexture3) { SDL_DestroyTexture(gLogoTexture3); gLogoTexture3 = nullptr; }
    if (gLogoTexture4) { SDL_DestroyTexture(gLogoTexture4); gLogoTexture4 = nullptr; }
    if (gLogoTexture5) { SDL_DestroyTexture(gLogoTexture5); gLogoTexture5 = nullptr; }
    if (gCharacter01Texture) { SDL_DestroyTexture(gCharacter01Texture); gCharacter01Texture = nullptr; }
    if (gCharacter02Texture) { SDL_DestroyTexture(gCharacter02Texture); gCharacter02Texture = nullptr; }
    if (gPlayerMaleTexture) { SDL_DestroyTexture(gPlayerMaleTexture); gPlayerMaleTexture = nullptr; }
    if (gPlayerFemaleTexture) { SDL_DestroyTexture(gPlayerFemaleTexture); gPlayerFemaleTexture = nullptr; }
    if (gCoinTexture) { SDL_DestroyTexture(gCoinTexture); gCoinTexture = nullptr; }

    for(auto& frame : gMenuBgFrames) if(frame) SDL_DestroyTexture(frame);
    gMenuBgFrames.clear();
    for(auto& slide : gIntroSlides) if(slide) SDL_DestroyTexture(slide);
    gIntroSlides.clear();
    for(auto& audio : gIntroAudio) if(audio) Mix_FreeChunk(audio);
    gIntroAudio.clear();

    if (gLoseSound) { Mix_FreeChunk(gLoseSound); gLoseSound = nullptr; }
    if (gWinSound) { Mix_FreeChunk(gWinSound); gWinSound = nullptr; }
    if (gMenuMusic) { Mix_FreeMusic(gMenuMusic); gMenuMusic = nullptr; }

    if (gFont) { TTF_CloseFont(gFont); gFont = nullptr; }
    if (gRenderer) { SDL_DestroyRenderer(gRenderer); gRenderer = nullptr; }
    if (gWindow) { SDL_DestroyWindow(gWindow); gWindow = nullptr; }
    
    Mix_CloseAudio(); Mix_Quit(); IMG_Quit(); TTF_Quit(); SDL_Quit();
    std::cout << "SDL Cleanup Complete." << std::endl;
}

// Utility Function: Play Intro Audio
void playCurrentIntroAudio() {
    if (gCurrentIntroSlide < gIntroAudio.size() && gIntroAudio[gCurrentIntroSlide] != nullptr) {
        if (gIntroAudioChannel != -1) { Mix_HaltChannel(gIntroAudioChannel); }
        gIntroAudioChannel = Mix_PlayChannel(-1, gIntroAudio[gCurrentIntroSlide], 0);
        if (gIntroAudioChannel == -1) { std::cerr << "WARNING: Failed to play intro audio " << gCurrentIntroSlide + 1 << "! Error: " << Mix_GetError() << std::endl; }
    } else {
        gIntroAudioChannel = -1;
    }
    gIntroSlideStartTime = SDL_GetTicks();
}

// Main Function
int main(int argc, char* args[]) {
    std::cout << "Application Starting: " << WINDOW_TITLE << std::endl;
    if (!initializeSDL()) { std::cerr << "Initialization Failed. Exiting." << std::endl; return 1; }
    if (!loadMedia()) { std::cerr << "Media Loading Failed. Exiting." << std::endl; closeSDL(); return 1; }

    if (gMenuMusic != nullptr) { if (Mix_PlayMusic(gMenuMusic, -1) == -1) { std::cerr << "Warning: Could not play menu music! " << Mix_GetError() << std::endl; } }

    std::cout << "\n===== Entering Main Loop =====\n" << std::endl;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (gCurrentState != GameState::EXIT) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePoint = { mouseX, mouseY };

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                if(Mix_PlayingMusic()) { Mix_HaltMusic(); }
                gCurrentState = GameState::EXIT;
                break;
            }

            switch(gCurrentState) {
                case GameState::MENU: {
                    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                        if (SDL_PointInRect(&mousePoint, &gPlayButtonRect)) {
                            if(Mix_PlayingMusic()) { Mix_HaltMusic(); }
                            gCurrentState = GameState::INTRO; gCurrentIntroSlide = 0; playCurrentIntroAudio();
                            gBackgroundX = 0.0f; 
                        } else if (SDL_PointInRect(&mousePoint, &gCharacterButtonRect)) {
                            gCurrentState = GameState::CHARACTER_SELECT;
                        } else if (SDL_PointInRect(&mousePoint, &gAboutButtonRect)) {
                            gCurrentState = GameState::ABOUT;
                        } else if (SDL_PointInRect(&mousePoint, &gQuitButtonRect)) {
                            if(Mix_PlayingMusic()) { Mix_HaltMusic(); } gCurrentState = GameState::EXIT;
                        }
                    }
                } break;

                case GameState::INTRO: {
                    bool skipTriggered = false;
                    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) { if (gSkipButtonTexture != nullptr && SDL_PointInRect(&mousePoint, &gSkipButtonRect)) { skipTriggered = true; } }
                    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && e.key.repeat == 0) { skipTriggered = true; }
                    if (skipTriggered) {
                        if (gIntroAudioChannel != -1) { Mix_HaltChannel(gIntroAudioChannel); gIntroAudioChannel = -1; }
                        gCurrentIntroSlide++;
                        if (gCurrentIntroSlide >= INTRO_SLIDE_COUNT) {
                            gCurrentState = GameState::PLAYING;
                            resetGameState();
                        } else { playCurrentIntroAudio(); }
                    }
                } break;

                case GameState::ABOUT: {
                    if ((e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) || (e.type == SDL_KEYDOWN && e.key.repeat == 0)) {
                         gCurrentState = GameState::MENU; if (gMenuMusic != nullptr && Mix_PlayingMusic() == 0) { Mix_PlayMusic(gMenuMusic, -1); }
                    }
                } break;

                case GameState::PLAYING: {
                    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                         switch(e.key.keysym.sym) {
                            case SDLK_UP: gMoveUp = true; break;
                            case SDLK_DOWN: gMoveDown = true; break;
                            case SDLK_LEFT: gMoveLeft = true; break;
                            case SDLK_RIGHT: gMoveRight = true; break;
                            case SDLK_ESCAPE: gCurrentState = GameState::MENU; if (gMenuMusic != nullptr && Mix_PlayingMusic() == 0) { Mix_PlayMusic(gMenuMusic, -1); } break;
                            default: break;
                         }
                    } else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
                          switch(e.key.keysym.sym) {
                            case SDLK_UP: gMoveUp = false; break;
                            case SDLK_DOWN: gMoveDown = false; break;
                            case SDLK_LEFT: gMoveLeft = false; break;
                            case SDLK_RIGHT: gMoveRight = false; break;
                            default: break;
                          }
                     }
                } break;

                case GameState::LOSE:
                case GameState::WIN: {
                    if (e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_KEYDOWN && e.key.repeat == 0)) {
                        gCurrentState = GameState::MENU;
                        if (gMenuMusic != nullptr && Mix_PlayingMusic() == 0) { Mix_PlayMusic(gMenuMusic, -1); }
                    }
                } break;

                case GameState::CHARACTER_SELECT: {
                    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                        if (e.key.keysym.sym == SDLK_LEFT) { gSelectedCharacter = 0; }
                        else if (e.key.keysym.sym == SDLK_RIGHT) { gSelectedCharacter = 1; }
                        else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
                            gCurrentState = GameState::MENU;
                        }
                    }
                } break;
                default: break;
            }
             if (gCurrentState == GameState::EXIT) break;
        }

        if (gCurrentState == GameState::EXIT) continue;

        // --- UPDATE LOGIC ---
        switch(gCurrentState) {
            case GameState::MENU: {
                gMenuAnimTimer += deltaTime;
                if (gMenuAnimTimer >= MENU_ANIM_SPEED) { gMenuAnimTimer -= MENU_ANIM_SPEED; gCurrentMenuFrame = (gCurrentMenuFrame + 1) % MENU_ANIM_FRAMES; }
            } break;
            case GameState::INTRO: {
                 bool advanceSlide = false;
                 if (gIntroAudioChannel != -1 && Mix_Playing(gIntroAudioChannel) == 0) { gIntroAudioChannel = -1; advanceSlide = true; }
                 unsigned int timeElapsed = SDL_GetTicks() - gIntroSlideStartTime; // Use unsigned int
                 if (!advanceSlide && timeElapsed > SLIDE_DEFAULT_DURATION_MS) { if (gIntroAudioChannel != -1) { Mix_HaltChannel(gIntroAudioChannel); gIntroAudioChannel = -1; } advanceSlide = true; }
                 if (advanceSlide) {
                      gCurrentIntroSlide++;
                       if (gCurrentIntroSlide >= INTRO_SLIDE_COUNT) {
                           gCurrentState = GameState::PLAYING;
                           resetGameState();
                       } else { playCurrentIntroAudio(); }
                 }
            } break;
            case GameState::ABOUT: { /* No updates */ } break;
            case GameState::PLAYING: {
                 gGameTimer += deltaTime;
                 if (gGameTimer >= WIN_TIME) {
                     gCurrentState = GameState::WIN_DELAY;
                     gWinDelayTimer = 0.0f;
                 }

                 float deltaY = 0.0f;
                 if (gMoveUp) { deltaY -= PLAYER_VERT_SPEED * deltaTime; }
                 if (gMoveDown) { deltaY += PLAYER_VERT_SPEED * deltaTime; }
                 gPlayerY += deltaY;
                 gPlayerY = std::max((float)PLAYER_BOUNDS_TOP, std::min(gPlayerY, (float)PLAYER_BOUNDS_BOTTOM));

                 float deltaX = 0.0f;
                 if (gMoveLeft) { deltaX -= PLAYER_HORIZ_SPEED * deltaTime; }
                 if (gMoveRight) { deltaX += PLAYER_HORIZ_SPEED * deltaTime; }
                 gPlayerX += deltaX;
                 gPlayerX = std::max(PLAYER_START_X - PLAYER_HORIZ_MOVE_RANGE, std::min(gPlayerX, PLAYER_START_X + PLAYER_HORIZ_MOVE_RANGE));

                 gBarrierSpawnTimer += deltaTime;
                 if (gBarrierSpawnTimer >= BARRIER_SPAWN_INTERVAL) {
                     gBarrierSpawnTimer = 0.0f;
                     int activeCount = 0; for (const auto& b : gBarriers) if (b.active) activeCount++;
                     if (activeCount < MAX_BARRIERS) {
                         Barrier* newBarrier = nullptr;
                         for (auto& b : gBarriers) if (!b.active) { newBarrier = &b; break; }
                         if (!newBarrier) { gBarriers.push_back({}); newBarrier = &gBarriers.back(); }
                         newBarrier->x = SCREEN_WIDTH;
                         std::uniform_int_distribution<> topOrBottomDist(0, 1);
                         if (topOrBottomDist(gRandomGenerator) == 0) {
                             newBarrier->y = ROAD_Y;
                         } else {
                             newBarrier->y = ROAD_Y + ROAD_HEIGHT - BARRIER_HEIGHT;
                         }
                         std::uniform_int_distribution<> texDist(0, 2);
                         newBarrier->textureIndex = texDist(gRandomGenerator);
                         newBarrier->active = true;
                     }
                 }

                 gCoinSpawnTimer += deltaTime;
                 if (gCoinSpawnTimer >= COIN_SPAWN_INTERVAL) {
                     gCoinSpawnTimer = 0.0f;
                     Coin* newCoin = nullptr;
                     for (auto& c : gCoins) if (!c.active) { newCoin = &c; break; }
                     if (!newCoin) { gCoins.push_back({}); newCoin = &gCoins.back(); }
                     newCoin->x = SCREEN_WIDTH;
                     std::uniform_int_distribution<> topOrBottomCoinDist(0, 1);
                     if (topOrBottomCoinDist(gRandomGenerator) == 0) {
                         newCoin->y = ROAD_Y;
                     } else {
                         newCoin->y = ROAD_Y + ROAD_HEIGHT - COIN_HEIGHT;
                     }
                     newCoin->active = true;
                 }

                 for (auto& barrier : gBarriers) {
                     if (barrier.active) {
                         barrier.x -= BARRIER_SPEED * deltaTime;
                         if (barrier.x + BARRIER_WIDTH < 0) barrier.active = false;
                         SDL_Rect playerRect = { (int)gPlayerX, (int)gPlayerY, PLAYER_SQUARE_SIZE, PLAYER_SQUARE_SIZE };
                         int shrink = 6;
                         SDL_Rect barrierRect = { (int)barrier.x + shrink, (int)barrier.y + shrink, BARRIER_WIDTH - 2*shrink, BARRIER_HEIGHT - 2*shrink };
                         if (SDL_HasIntersection(&playerRect, &barrierRect)) {
                             if (gLoseSound != nullptr) Mix_PlayChannel(-1, gLoseSound, 0);
                             gCurrentState = GameState::LOSE;
                         }
                     }
                 }

                 for (auto& coin : gCoins) {
                     if (coin.active) {
                         coin.x -= BARRIER_SPEED * deltaTime;
                         if (coin.x + COIN_WIDTH < 0) coin.active = false;
                         SDL_Rect playerRect = { (int)gPlayerX, (int)gPlayerY, PLAYER_SQUARE_SIZE, PLAYER_SQUARE_SIZE };
                         SDL_Rect coinRect = { (int)coin.x, (int)coin.y, COIN_WIDTH, COIN_HEIGHT };
                         if (SDL_HasIntersection(&playerRect, &coinRect)) {
                             coin.active = false;
                             gCoinCounter++;
                         }
                     }
                 }

                 gBackgroundX -= BACKGROUND_SCROLL_SPEED * deltaTime; 
                 if (gBackgroundX <= -SCREEN_WIDTH) gBackgroundX += SCREEN_WIDTH;

            } break;
            case GameState::WIN_DELAY: {
                gWinDelayTimer += deltaTime;
                gBackgroundX -= BACKGROUND_SCROLL_SPEED * deltaTime; 
                if (gBackgroundX <= -SCREEN_WIDTH) gBackgroundX += SCREEN_WIDTH;

                if (gWinDelayTimer >= WIN_DELAY_TIME) {
                    if (gWinSound != nullptr) Mix_PlayChannel(-1, gWinSound, 0);
                    gCurrentState = GameState::WIN;
                }
            } break;
            default: break;
        }

        // --- RENDER LOGIC ---
        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF); 
        SDL_RenderClear(gRenderer);
        
        switch(gCurrentState) {
            case GameState::MENU: {
                if (!gMenuBgFrames.empty() && gCurrentMenuFrame < gMenuBgFrames.size() && gMenuBgFrames[gCurrentMenuFrame] != nullptr) { 
                    SDL_RenderCopy(gRenderer, gMenuBgFrames[gCurrentMenuFrame], nullptr, nullptr); 
                } else { SDL_SetRenderDrawColor(gRenderer, 0x22, 0x22, 0x22, 0xFF); SDL_RenderClear(gRenderer); }
                if (gLogoTexture3) { int w,h; SDL_QueryTexture(gLogoTexture3,0,0,&w,&h); SDL_Rect r = {20,20,(int)(w*0.3f),(int)(h*0.3f)}; SDL_RenderCopy(gRenderer,gLogoTexture3,0,&r); }
                if (gLogoTexture2) { int w,h; SDL_QueryTexture(gLogoTexture2,0,0,&w,&h); SDL_Rect r = {SCREEN_WIDTH-(int)(w*0.4f)-20, 20, (int)(w*0.4f),(int)(h*0.4f)}; SDL_RenderCopy(gRenderer,gLogoTexture2,0,&r); }
                renderText("PLAY", BUTTON_X, BUTTON_Y_PLAY, gFont, SDL_PointInRect(&mousePoint, &gPlayButtonRect) ? gButtonHoverColor : gTextColor, gRenderer);
                renderText("CHARACTER", BUTTON_X, BUTTON_Y_CHARACTER, gFont, SDL_PointInRect(&mousePoint, &gCharacterButtonRect) ? gButtonHoverColor : gTextColor, gRenderer);
                renderText("ABOUT", BUTTON_X, BUTTON_Y_ABOUT, gFont, SDL_PointInRect(&mousePoint, &gAboutButtonRect) ? gButtonHoverColor : gTextColor, gRenderer);
                renderText("QUIT", BUTTON_X, BUTTON_Y_QUIT, gFont, SDL_PointInRect(&mousePoint, &gQuitButtonRect) ? gButtonHoverColor : gTextColor, gRenderer);
            } break;
            
            case GameState::INTRO: {
                 if (gCurrentIntroSlide < gIntroSlides.size() && gIntroSlides[gCurrentIntroSlide] != nullptr) { SDL_RenderCopy(gRenderer, gIntroSlides[gCurrentIntroSlide], nullptr, nullptr); }
                  else { SDL_SetRenderDrawColor(gRenderer, 0x11,0x11,0x11,0xFF); SDL_RenderClear(gRenderer); renderText("Missing Intro Slide!",100,100,gFont,gTextColor,gRenderer); }
                 if (gSkipButtonTexture != nullptr) { SDL_RenderCopy(gRenderer, gSkipButtonTexture, nullptr, &gSkipButtonRect); }
            } break;
            
            case GameState::ABOUT: {
                SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x25, 0xFF); SDL_RenderClear(gRenderer);
                int y=50, ls=30, ss=45, tx=50, rsX=SCREEN_WIDTH-400;
                renderText("The Story",tx,y,gFont,gHeaderColor,gRenderer); y+=ls;
                renderText("In a world craving speed, you are a daring rider",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("competing in the legendary Moto Rush challenge.",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("Only the fastest will reach the finish line.",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls+ss;
                renderText("Your Goal",tx,y,gFont,gHeaderColor,gRenderer); y+=ls;
                renderText("Reach the final destination point",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("before the timer runs out!",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls+ss;
                renderText("Rules",tx,y,gFont,gHeaderColor,gRenderer); y+=ls;
                renderText("- You have only 40 seconds to complete the race.",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("- If the timer hits zero before you finish, you lose.",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("- Avoid obstacles.",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls+ss;
                renderText("Controls",tx,y,gFont,gHeaderColor,gRenderer); y+=ls;
                renderText("- Left/Right Arrows: Select character (selection screen)",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("- Enter: Confirm selection / Start game",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("- Left/Right Arrows (Game): Move Horizontally (Slightly)",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls; 
                renderText("- Up/Down Arrows (Game): Move Vertically",tx,y,gFont,gAboutTextColor,gRenderer); y+=ls;
                renderText("- ESC (Game): Return to Main Menu",tx,y,gFont,gAboutTextColor,gRenderer);
                if(gLogoTexture){int w,h;SDL_QueryTexture(gLogoTexture,0,0,&w,&h);float sc=0.8f; int sw=(int)(w*sc),sh=(int)(h*sc);SDL_Rect lr={rsX+(400-sw)/2,(SCREEN_HEIGHT-sh)/2,sw,sh};SDL_RenderCopy(gRenderer,gLogoTexture,0,&lr);}
            } break;
            
            case GameState::PLAYING:
            case GameState::WIN_DELAY: 
            {
                // 1. Render Far Background (Scrolling)
                if (gGameBgFarTexture) {
                    SDL_Rect r1 = {(int)gBackgroundX, -80, SCREEN_WIDTH, SCREEN_HEIGHT};
                    SDL_Rect r2 = {(int)gBackgroundX + SCREEN_WIDTH, -80, SCREEN_WIDTH, SCREEN_HEIGHT};
                    SDL_RenderCopy(gRenderer, gGameBgFarTexture, nullptr, &r1);
                    SDL_RenderCopy(gRenderer, gGameBgFarTexture, nullptr, &r2);
                }

                // 2. Render Road with Static Perspective (Scanline method)
                if (gGameBgNearTexture && ROAD_HEIGHT > 0) {
                    int roadTexW, roadTexH;
                    SDL_QueryTexture(gGameBgNearTexture, nullptr, nullptr, &roadTexW, &roadTexH);

                    if (roadTexW > 0 && roadTexH > 0) { 
                        float world_segment_depth_for_texture_map = (float)roadTexH; 

                        for (int y_iter = 0; y_iter < ROAD_HEIGHT; ++y_iter) {
                            float screen_y_on_road_segment = ROAD_Y + y_iter;
                            float norm_y_on_segment = (float)y_iter / (ROAD_HEIGHT - 1.0f);
                            if (ROAD_HEIGHT <= 1) norm_y_on_segment = 1.0f;

                            float current_width_scale = ROAD_PERSPECTIVE_FAR_SCALE + norm_y_on_segment * (ROAD_PERSPECTIVE_NEAR_SCALE - ROAD_PERSPECTIVE_FAR_SCALE);
                            int scanline_on_screen_width = (int)(SCREEN_WIDTH * current_width_scale);
                            int scanline_on_screen_x = (SCREEN_WIDTH - scanline_on_screen_width) / 2;

                            float texture_v_normalized = powf(norm_y_on_segment, ROAD_TEXTURE_V_POWER);
                            int src_v = (int)(ROAD_TEXTURE_V_START_OFFSET + texture_v_normalized * world_segment_depth_for_texture_map);
                            
                            src_v = std::max(0, std::min(src_v, roadTexH - 1));
                            
                            SDL_Rect srcRectScanline = {0, src_v, roadTexW, 1}; 
                            SDL_Rect destRectScanline = {scanline_on_screen_x, (int)screen_y_on_road_segment, scanline_on_screen_width, 1};
                            
                            if (destRectScanline.w > 0) { 
                                SDL_RenderCopy(gRenderer, gGameBgNearTexture, &srcRectScanline, &destRectScanline);
                            }
                        }
                    }
                }

                // 3. Render Timer Bar
                int barMaxWidth=SCREEN_WIDTH/4, barH=18, barX=20, barY=15; float timeLeft=std::max(0.0f,WIN_TIME-gGameTimer); int barW=(int)(barMaxWidth*(timeLeft/WIN_TIME));
                SDL_Rect tBarBg={barX,barY,barMaxWidth,barH},tBar={barX,barY,barW,barH}; SDL_SetRenderDrawColor(gRenderer,0,0,0,255); SDL_RenderFillRect(gRenderer,&tBarBg); SDL_SetRenderDrawColor(gRenderer,255,215,0,255); SDL_RenderFillRect(gRenderer,&tBar);

                // 4. Render Barriers and Coins
                for (const auto& b : gBarriers) if (b.active) { SDL_Rect br={(int)b.x,(int)b.y,BARRIER_WIDTH,BARRIER_HEIGHT}; if(gBarrierTextures[b.textureIndex]) SDL_RenderCopy(gRenderer,gBarrierTextures[b.textureIndex],0,&br); else {SDL_SetRenderDrawColor(gRenderer,255,0,0,255);SDL_RenderFillRect(gRenderer,&br);}}
                for (const auto& c : gCoins) if (c.active) { SDL_Rect cr={(int)c.x,(int)c.y,COIN_WIDTH,COIN_HEIGHT}; if(gCoinTexture)SDL_RenderCopy(gRenderer,gCoinTexture,0,&cr); else {SDL_SetRenderDrawColor(gRenderer,255,215,0,255); SDL_RenderFillRect(gRenderer, &cr);}}
                
                // 5. Render Player
                SDL_Rect playerR = {(int)gPlayerX,(int)gPlayerY,PLAYER_SQUARE_SIZE,PLAYER_SQUARE_SIZE};
                SDL_Texture* currentPTex = (gSelectedCharacter==0) ? gPlayerFemaleTexture : gPlayerMaleTexture;
                if(currentPTex) SDL_RenderCopy(gRenderer,currentPTex,0,&playerR); else {SDL_SetRenderDrawColor(gRenderer,255,0,0,255);SDL_RenderFillRect(gRenderer,&playerR);}
                
                // 6. Render Coin Counter
                renderText(std::to_string(gCoinCounter),SCREEN_WIDTH-150,20,gFont,gTextColor,gRenderer);

                if (gCurrentState == GameState::WIN_DELAY) {
                    renderText("YOU WIN!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, gFont, gHeaderColor, gRenderer);
                }
            } break;

            case GameState::LOSE: {
                if (gLoseScreenTexture) SDL_RenderCopy(gRenderer, gLoseScreenTexture, nullptr, nullptr);
                else { SDL_SetRenderDrawColor(gRenderer,0x11,0x11,0x11,0xFF); SDL_RenderClear(gRenderer); renderText("GAME OVER!",SCREEN_WIDTH/2-100,SCREEN_HEIGHT/2-50,gFont,gTextColor,gRenderer); renderText("Click to return",SCREEN_WIDTH/2-100,SCREEN_HEIGHT/2+20,gFont,gTextColor,gRenderer); }
            } break;

            case GameState::WIN: {
                if (gWinScreenTexture) SDL_RenderCopy(gRenderer, gWinScreenTexture, nullptr, nullptr);
                else { SDL_SetRenderDrawColor(gRenderer,0x11,0x11,0x11,0xFF); SDL_RenderClear(gRenderer); renderText("YOU WIN!",SCREEN_WIDTH/2-100,SCREEN_HEIGHT/2-50,gFont,gTextColor,gRenderer); renderText("Click to return",SCREEN_WIDTH/2-100,SCREEN_HEIGHT/2+20,gFont,gTextColor,gRenderer); }
            } break;

            case GameState::CHARACTER_SELECT: {
                SDL_SetRenderDrawColor(gRenderer, 20, 30, 60, 255); SDL_RenderClear(gRenderer);
                if (gLogoTexture5) { int w,h; SDL_QueryTexture(gLogoTexture5,0,0,&w,&h); SDL_Rect r = {(SCREEN_WIDTH-(int)(w*0.25f))/2,20,(int)(w*0.25f),(int)(h*0.25f)}; SDL_RenderCopy(gRenderer,gLogoTexture5,0,&r); }
                int charW=180,charH=220,gap=80,baseY=180,char1X=SCREEN_WIDTH/2-charW-gap/2,char2X=SCREEN_WIDTH/2+gap/2;
                SDL_Rect r1={char1X,baseY,charW,charH},r2={char2X,baseY,charW,charH};
                if(gCharacter01Texture) SDL_RenderCopy(gRenderer,gCharacter01Texture,0,&r1);
                if(gCharacter02Texture) SDL_RenderCopy(gRenderer,gCharacter02Texture,0,&r2);
                SDL_SetRenderDrawColor(gRenderer,255,0,0,255);
                if(gSelectedCharacter==0) SDL_RenderDrawRect(gRenderer,&r1); else SDL_RenderDrawRect(gRenderer,&r2);
                renderText("Select Your Character",SCREEN_WIDTH/2-120,baseY+charH+30,gFont,gHeaderColor,gRenderer);
                renderText("Left/Right Arrows | Enter to Confirm | ESC to Cancel",SCREEN_WIDTH/2-250,baseY+charH+70,gFont,gTextColor,gRenderer);
            } break;
            default: break;
        }
        SDL_RenderPresent(gRenderer);
    }

    std::cout << "\n===== Exiting Main Loop =====\n" << std::endl;
    closeSDL();
    std::cout << "Application Exited Gracefully." << std::endl;
    return 0;
}