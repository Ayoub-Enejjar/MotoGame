#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>    // For delta time calculation
#include <algorithm> // For std::min/max
#include <random>    // For random barrier generation

// --- Configuration ---
const int SCREEN_WIDTH = 1100; // User specified width
const int SCREEN_HEIGHT = 720; // User specified height
const char* WINDOW_TITLE = "MotoGame (SDL2)"; // Game title

// Menu Config
const int BUTTON_WIDTH = 150; // Estimated width for click area
const int BUTTON_HEIGHT = 50;  // Estimated height for click area
const int BUTTON_X = 50;       // X position for buttons
const int BUTTON_Y_PLAY = 150;
const int BUTTON_Y_ABOUT = 220;
const int BUTTON_Y_QUIT = 290;
// !!! IMPORTANT: Adjust this to match your actual number of menu background frames (e.g., 4 or 5) !!!
const int MENU_ANIM_FRAMES = 4;
const float MENU_ANIM_SPEED = 0.25f; // Seconds each menu background frame is displayed

// Intro Config
const int INTRO_SLIDE_COUNT = 4; // Number of intro slides/audio files
const Uint32 SLIDE_DEFAULT_DURATION_MS = 22000; // Milliseconds per slide (adjust as needed, fallback/max duration)

// Gameplay Config
const int PLAYER_SQUARE_SIZE = 120;
const float PLAYER_VERT_SPEED = 300.0f; // Pixels per second for vertical movement
// *** UPDATED: Define vertical bounds (Bottom Quarter: From 3/4 down to bottom) ***
const int PLAYER_BOUNDS_TOP = SCREEN_HEIGHT * 3 / 4;
const int PLAYER_BOUNDS_BOTTOM = SCREEN_HEIGHT - PLAYER_SQUARE_SIZE; // Account for square height
const int PLAYER_START_X = 100; // Fixed horizontal position for the player square
const int BARRIER_WIDTH = 40;      
const int BARRIER_HEIGHT = 40;     // Width of barrier squares
const float BARRIER_SPEED = 400.0f;     // Speed of barriers moving towards player
const float BARRIER_SPAWN_INTERVAL = 2.0f; // Seconds between barrier spawns
const int MAX_BARRIERS = 5;             // Maximum number of barriers on screen

// --- Game States ---
enum class GameState {
    MENU,
    INTRO,
    ABOUT,
    PLAYING,
    LOSE,  // New state for lose screen
    EXIT
};

// --- Global Variables ---
// SDL Core
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;

// Assets & Common
TTF_Font* gFont = nullptr;
SDL_Color gTextColor = { 255, 255, 255, 255 };       // Default white text
SDL_Color gButtonHoverColor = { 255, 255, 0, 255 };  // Yellow on hover
SDL_Color gHeaderColor = {255, 200, 0, 255};       // Yellowish for About headers
SDL_Color gAboutTextColor = {220, 220, 220, 255};    // Light grey for About text

// Menu State
SDL_Rect gPlayButtonRect = { BUTTON_X - 10, BUTTON_Y_PLAY - 10, BUTTON_WIDTH, BUTTON_HEIGHT };
SDL_Rect gAboutButtonRect = { BUTTON_X - 10, BUTTON_Y_ABOUT - 10, BUTTON_WIDTH, BUTTON_HEIGHT };
SDL_Rect gQuitButtonRect = { BUTTON_X - 10, BUTTON_Y_QUIT - 10, BUTTON_WIDTH, BUTTON_HEIGHT };
std::vector<SDL_Texture*> gMenuBgFrames;
int gCurrentMenuFrame = 0;
float gMenuAnimTimer = 0.0f;
Mix_Music* gMenuMusic = nullptr;

// Intro State
std::vector<SDL_Texture*> gIntroSlides;
std::vector<Mix_Chunk*> gIntroAudio;
SDL_Texture* gSkipButtonTexture = nullptr;
SDL_Rect gSkipButtonRect;
int gCurrentIntroSlide = 0;
Uint32 gIntroSlideStartTime = 0;
int gIntroAudioChannel = -1; // Track which channel is playing intro audio

// Gameplay State
SDL_Texture* gGameBgFarTexture = nullptr;
SDL_Texture* gPlayerTexture = nullptr;  // New texture for player
SDL_Texture* gBarrierTexture = nullptr;  // New texture for barriers
SDL_Texture* gLoseScreenTexture = nullptr;  // New texture for lose screen
Mix_Chunk* gLoseSound = nullptr;            // New sound for lose screen
float gPlayerY = 0.0f;     // Player square's current vertical position (top edge)
bool gMoveUp = false;      // Is UP key currently held down?
bool gMoveDown = false;    // Is DOWN key currently held down?

// Barrier structure
struct Barrier {
    float x;
    float y;
    bool active;
};

std::vector<Barrier> gBarriers;
float gBarrierSpawnTimer = 0.0f;
std::random_device gRandomDevice;
std::mt19937 gRandomGenerator(gRandomDevice());

// Overall Game State
GameState gCurrentState = GameState::MENU; // Start in the menu

// --- Helper Function: Load Texture (using SDL_image) ---
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
    return newTexture; // Returns nullptr on failure
}


// --- Helper Function: Render Text (using SDL_ttf) ---
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
    return true; // Success
}

// --- Initialization ---
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
    return true; // Success
}

// --- Load Media ---
bool loadMedia() {
     std::cout << "Loading Media..." << std::endl;
    // Load Font
    std::cout << " -> Loading Font: ../assets/fonts/game_font.ttf" << std::endl;
    gFont = TTF_OpenFont("../assets/fonts/game_font.ttf", 28);
    if (gFont == nullptr) { std::cerr << "FATAL ERROR: Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl; return false; }
    std::cout << "    Font loaded." << std::endl;

    // Load Menu Background Frames
    std::cout << " -> Loading Menu Background Frames (" << MENU_ANIM_FRAMES << ")..." << std::endl;
    gMenuBgFrames.resize(MENU_ANIM_FRAMES);
    for (int i = 0; i < MENU_ANIM_FRAMES; ++i) {
        std::string path = "../assets/images/menu_anim/bg_frame_0" + std::to_string(i + 1) + ".png";
         std::cout << "    Loading frame: " << path << std::endl;
        gMenuBgFrames[i] = loadTexture(path, gRenderer);
        if (gMenuBgFrames[i] == nullptr) { std::cerr << "FATAL ERROR: Failed to load background frame " << i+1 << std::endl; return false; }
    }
    std::cout << "    Menu background frames loaded." << std::endl;

    // Load Menu Music
    std::cout << " -> Loading Menu Music: ../assets/audio/music_menu.wav" << std::endl;
    gMenuMusic = Mix_LoadMUS("../assets/audio/music_menu.wav");
    if (gMenuMusic == nullptr) { std::cerr << "FATAL ERROR: Failed to load menu music! SDL_mixer Error: " << Mix_GetError() << std::endl; return false; }
     else { std::cout << "    Menu music loaded." << std::endl; }

    // Load Intro Assets
    std::cout << " -> Loading Intro Assets (" << INTRO_SLIDE_COUNT << ")..." << std::endl;
    gIntroSlides.resize(INTRO_SLIDE_COUNT);
    gIntroAudio.resize(INTRO_SLIDE_COUNT);
    for (int i = 0; i < INTRO_SLIDE_COUNT; ++i) {
        std::string imgPath = "../assets/images/Intro/intro_slide_0" + std::to_string(i + 1) + ".png";
        std::cout << "    Loading slide image: " << imgPath << std::endl;
        gIntroSlides[i] = loadTexture(imgPath, gRenderer);
        if (gIntroSlides[i] == nullptr) { std::cerr << "FATAL ERROR: Failed to load intro slide image " << i+1 << std::endl; return false; }
        std::string wavPath = "../assets/audio/intro_slide_0" + std::to_string(i + 1) + ".wav";
         std::cout << "    Loading slide audio: " << wavPath << std::endl;
        gIntroAudio[i] = Mix_LoadWAV(wavPath.c_str());
        if (gIntroAudio[i] == nullptr) { std::cerr << "WARNING: Failed to load intro slide audio " << i+1 << "! SDL_mixer Error: " << Mix_GetError() << std::endl; }
    }
    std::cout << "    Intro slides and audio loaded." << std::endl;

    // Load Skip Button
    std::cout << " -> Loading Skip Button: ../assets/images/ui/skip_button.png" << std::endl;
    gSkipButtonTexture = loadTexture("../assets/images/ui/skip_button.png", gRenderer);
    if (gSkipButtonTexture == nullptr) { std::cerr << "WARNING: Failed to load skip button texture. Skip button will not be visible." << std::endl; }
     else {
        int skipW, skipH;
        SDL_QueryTexture(gSkipButtonTexture, NULL, NULL, &skipW, &skipH);
        int padding = 20;
        gSkipButtonRect = { SCREEN_WIDTH - skipW - padding, SCREEN_HEIGHT - skipH - padding, skipW, skipH };
         std::cout << "    Skip button loaded and positioned." << std::endl;
    }

    // Load Gameplay Background
    std::cout << " -> Loading Gameplay Background: ../assets/images/background_far.png" << std::endl;
    gGameBgFarTexture = loadTexture("../assets/images/background_far.png", gRenderer);
    if (gGameBgFarTexture == nullptr) { std::cerr << "FATAL ERROR: Failed to load gameplay background!" << std::endl; return false; }
     std::cout << "    Gameplay background loaded." << std::endl;

    // Load Player Texture
    std::cout << " -> Loading Player Texture: ../assets/images/select/player_male.png" << std::endl;
    gPlayerTexture = loadTexture("../assets/images/select/player_male.png", gRenderer);
    if (gPlayerTexture == nullptr) { std::cerr << "FATAL ERROR: Failed to load player texture!" << std::endl; return false; }
    std::cout << "    Player texture loaded." << std::endl;

    // Load Barrier Texture
    std::cout << " -> Loading Barrier Texture: ../assets/images/barrier.png" << std::endl;
    gBarrierTexture = loadTexture("../assets/images/barrier.png", gRenderer);
    if (gBarrierTexture == nullptr) { std::cerr << "WARNING: Failed to load barrier texture. Will use colored rectangle instead." << std::endl; }
    std::cout << "    Barrier texture loaded." << std::endl;

    // Load Lose Screen Assets
    std::cout << " -> Loading Lose Screen Texture: ../assets/images/endscreen/lose_slide.png" << std::endl;
    gLoseScreenTexture = loadTexture("../assets/images/endscreen/lose_slide.png", gRenderer);
    if (gLoseScreenTexture == nullptr) { std::cerr << "WARNING: Failed to load lose screen texture!" << std::endl; }
    std::cout << "    Lose screen texture loaded." << std::endl;

    std::cout << " -> Loading Lose Sound: ../assets/audio/lose_slide.wav" << std::endl;
    gLoseSound = Mix_LoadWAV("../assets/audio/lose_slide.wav");
    if (gLoseSound == nullptr) { std::cerr << "WARNING: Failed to load lose sound! SDL_mixer Error: " << Mix_GetError() << std::endl; }
    std::cout << "    Lose sound loaded." << std::endl;

    // TODO: Load Character Select, Win/Lose assets here

    std::cout << "All Essential Media Loaded Successfully." << std::endl;
    return true; // Success
}


// --- Cleanup ---
void closeSDL() {
     std::cout << "Starting SDL Cleanup..." << std::endl;
    // Free Music & Chunks
    if (gMenuMusic) { Mix_FreeMusic(gMenuMusic); gMenuMusic = nullptr; }
    for (Mix_Chunk* chunk : gIntroAudio) { if (chunk) Mix_FreeChunk(chunk); } gIntroAudio.clear();
    // Free Textures
    for (SDL_Texture* texture : gMenuBgFrames) { if (texture) SDL_DestroyTexture(texture); } gMenuBgFrames.clear();
    for (SDL_Texture* texture : gIntroSlides) { if (texture) SDL_DestroyTexture(texture); } gIntroSlides.clear();
    if (gSkipButtonTexture) { SDL_DestroyTexture(gSkipButtonTexture); gSkipButtonTexture = nullptr; }
    if (gGameBgFarTexture) { SDL_DestroyTexture(gGameBgFarTexture); gGameBgFarTexture = nullptr; }
    if (gPlayerTexture) { SDL_DestroyTexture(gPlayerTexture); gPlayerTexture = nullptr; }
    if (gBarrierTexture) { SDL_DestroyTexture(gBarrierTexture); gBarrierTexture = nullptr; }
    if (gLoseScreenTexture) { SDL_DestroyTexture(gLoseScreenTexture); gLoseScreenTexture = nullptr; }
    if (gLoseSound) { Mix_FreeChunk(gLoseSound); gLoseSound = nullptr; }
    // TODO: Free Character Select, Win/Lose textures here
    // Free Font
    if (gFont) { TTF_CloseFont(gFont); gFont = nullptr; }
    // Destroy Renderer & Window
    if (gRenderer) { SDL_DestroyRenderer(gRenderer); gRenderer = nullptr; }
    if (gWindow) { SDL_DestroyWindow(gWindow); gWindow = nullptr; }
    // Quit SDL Subsystems
    Mix_CloseAudio(); Mix_Quit(); IMG_Quit(); TTF_Quit(); SDL_Quit();
    std::cout << "SDL Cleanup Complete." << std::endl;
}

// --- Utility Function: Play Intro Audio ---
void playCurrentIntroAudio() {
    if (gCurrentIntroSlide < gIntroAudio.size() && gIntroAudio[gCurrentIntroSlide] != nullptr) {
        if (gIntroAudioChannel != -1) { Mix_HaltChannel(gIntroAudioChannel); } // Stop previous if any
        gIntroAudioChannel = Mix_PlayChannel(-1, gIntroAudio[gCurrentIntroSlide], 0); // Play once
        if (gIntroAudioChannel == -1) { std::cerr << "WARNING: Failed to play intro audio chunk " << gCurrentIntroSlide + 1 << "! Error: " << Mix_GetError() << std::endl; }
        else { std::cout << " -> Playing intro audio " << gCurrentIntroSlide + 1 << " on channel " << gIntroAudioChannel << std::endl; }
    } else {
        gIntroAudioChannel = -1; // No audio or audio failed to load
        if (gCurrentIntroSlide < gIntroAudio.size()) { std::cout << " -> No audio loaded for intro slide " << gCurrentIntroSlide + 1 << std::endl; }
    }
    gIntroSlideStartTime = SDL_GetTicks(); // Reset timer when (attempting) to play audio
}


// --- Main Function ---
int main(int argc, char* args[]) {
    std::cout << "Application Starting: " << WINDOW_TITLE << std::endl;
    if (!initializeSDL()) { std::cerr << "Initialization Failed. Exiting." << std::endl; return 1; }
    if (!loadMedia()) { std::cerr << "Media Loading Failed. Exiting." << std::endl; closeSDL(); return 1; }

    // Start Menu Music if loaded
    if (gMenuMusic != nullptr) { if (Mix_PlayMusic(gMenuMusic, -1) == -1) { /* Log Warning */ } }

    std::cout << "\n===== Entering Main Loop =====\n" << std::endl;
    auto lastTime = std::chrono::high_resolution_clock::now();
    int loopCounter = 0;

    // --- Game Loop ---
    while (gCurrentState != GameState::EXIT) {
        loopCounter++;
        // --- Delta Time Calculation ---
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // --- Input Handling ---
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePoint = { mouseX, mouseY };

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            // Global Quit Event
            if (e.type == SDL_QUIT) {
                std::cout << "EVENT: Window close requested (SDL_QUIT)." << std::endl;
                if(Mix_PlayingMusic()) { Mix_HaltMusic(); }
                gCurrentState = GameState::EXIT;
                break; // Exit event loop immediately on quit
            }

            // State Specific Event Handling
            switch(gCurrentState) {
                case GameState::MENU: {
                    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                        if (SDL_PointInRect(&mousePoint, &gPlayButtonRect)) {
                            std::cout << "EVENT: PLAY button clicked!" << std::endl; if(Mix_PlayingMusic()) { Mix_HaltMusic(); }
                            gCurrentState = GameState::INTRO; gCurrentIntroSlide = 0; playCurrentIntroAudio(); std::cout << "STATE CHANGE: -> INTRO" << std::endl;
                        } else if (SDL_PointInRect(&mousePoint, &gAboutButtonRect)) {
                            std::cout << "EVENT: ABOUT button clicked!" << std::endl; gCurrentState = GameState::ABOUT; std::cout << "STATE CHANGE: -> ABOUT" << std::endl;
                        } else if (SDL_PointInRect(&mousePoint, &gQuitButtonRect)) {
                            std::cout << "EVENT: QUIT button clicked!" << std::endl; if(Mix_PlayingMusic()) { Mix_HaltMusic(); } gCurrentState = GameState::EXIT;
                        }
                    }
                } break; // End MENU Events

                case GameState::INTRO: {
                    bool skipTriggered = false;
                    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) { if (gSkipButtonTexture != nullptr && SDL_PointInRect(&mousePoint, &gSkipButtonRect)) { skipTriggered = true; } }
                    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && e.key.repeat == 0) { skipTriggered = true; }
                    if (skipTriggered) {
                        std::cout << "EVENT: Skip triggered in INTRO." << std::endl;
                        if (gIntroAudioChannel != -1) { Mix_HaltChannel(gIntroAudioChannel); gIntroAudioChannel = -1; }
                        gCurrentIntroSlide++;
                        if (gCurrentIntroSlide >= INTRO_SLIDE_COUNT) {
                            std::cout << "Intro sequence skipped/finished." << std::endl; gCurrentState = GameState::PLAYING;
                            // *** UPDATED: Initialize Player Y Position (at new top bound) ***
                            gPlayerY = PLAYER_BOUNDS_TOP;
                            gMoveUp = false; gMoveDown = false; std::cout << "STATE CHANGE: -> PLAYING (Player Y init: " << gPlayerY << ")" << std::endl;
                        } else { std::cout << " -> Skipping to slide " << gCurrentIntroSlide + 1 << std::endl; playCurrentIntroAudio(); }
                    }
                } break; // End INTRO Events

                case GameState::ABOUT: {
                    if ((e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) || (e.type == SDL_KEYDOWN && e.key.repeat == 0)) {
                         std::cout << "EVENT: Back action from About." << std::endl; gCurrentState = GameState::MENU; std::cout << "STATE CHANGE: -> MENU" << std::endl; if (gMenuMusic != nullptr && Mix_PlayingMusic() == 0) { Mix_PlayMusic(gMenuMusic, -1); }
                    }
                } break; // End ABOUT Events

                case GameState::PLAYING: {
                    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                         switch(e.key.keysym.sym) {
                            case SDLK_UP: gMoveUp = true; break;
                            case SDLK_DOWN: gMoveDown = true; break;
                            case SDLK_ESCAPE: std::cout << "EVENT: Escape pressed in Playing state." << std::endl; gCurrentState = GameState::MENU; std::cout << "STATE CHANGE: -> MENU" << std::endl; if (gMenuMusic != nullptr && Mix_PlayingMusic() == 0) { Mix_PlayMusic(gMenuMusic, -1); } break;
                            default: break;
                         }
                    }
                     else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
                          switch(e.key.keysym.sym) {
                            case SDLK_UP: gMoveUp = false; break;
                            case SDLK_DOWN: gMoveDown = false; break;
                            default: break;
                          }
                     }
                } break; // End PLAYING Events

                case GameState::LOSE: {
                    if (e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_KEYDOWN && e.key.repeat == 0)) {
                        std::cout << "EVENT: Returning to menu from lose screen." << std::endl;
                        gCurrentState = GameState::MENU;
                        if (gMenuMusic != nullptr && Mix_PlayingMusic() == 0) {
                            Mix_PlayMusic(gMenuMusic, -1);
                        }
                    }
                } break;

                default: break;
            } // End event switch
             if (gCurrentState == GameState::EXIT) break; // Exit poll loop if state changed to EXIT
        } // End poll event loop

        if (gCurrentState == GameState::EXIT) continue; // Skip update/render if exiting

        // --- Update Game Logic ---
        switch(gCurrentState) {
            case GameState::MENU: {
                gMenuAnimTimer += deltaTime;
                if (gMenuAnimTimer >= MENU_ANIM_SPEED) { gMenuAnimTimer -= MENU_ANIM_SPEED; gCurrentMenuFrame = (gCurrentMenuFrame + 1) % MENU_ANIM_FRAMES; }
            } break;
            case GameState::INTRO: {
                 bool advanceSlide = false;
                 if (gIntroAudioChannel != -1 && Mix_Playing(gIntroAudioChannel) == 0) { gIntroAudioChannel = -1; advanceSlide = true; }
                 Uint32 timeElapsed = SDL_GetTicks() - gIntroSlideStartTime;
                 if (!advanceSlide && timeElapsed > SLIDE_DEFAULT_DURATION_MS) { if (gIntroAudioChannel != -1) { Mix_HaltChannel(gIntroAudioChannel); gIntroAudioChannel = -1; } advanceSlide = true; }
                 if (advanceSlide) {
                      gCurrentIntroSlide++;
                       if (gCurrentIntroSlide >= INTRO_SLIDE_COUNT) {
                           std::cout << "Intro sequence finished automatically." << std::endl;
                           gCurrentState = GameState::PLAYING;
                           // *** UPDATED: Initialize Player Y Position (at new top bound) ***
                           gPlayerY = PLAYER_BOUNDS_TOP;
                           gMoveUp = false; gMoveDown = false;
                           std::cout << "STATE CHANGE: -> PLAYING (Player Y init: " << gPlayerY << ")" << std::endl;
                       } else {
                           std::cout << " -> Advancing to slide " << gCurrentIntroSlide + 1 << std::endl; playCurrentIntroAudio();
                       }
                 }
            } break;
            case GameState::ABOUT: { /* No updates needed */ } break;
            case GameState::PLAYING: {
                 // Update player position
                 float deltaY = 0.0f;
                 if (gMoveUp) { deltaY -= PLAYER_VERT_SPEED * deltaTime; }
                 if (gMoveDown) { deltaY += PLAYER_VERT_SPEED * deltaTime; }
                 gPlayerY += deltaY;
                 gPlayerY = std::max((float)PLAYER_BOUNDS_TOP, std::min(gPlayerY, (float)PLAYER_BOUNDS_BOTTOM));

                 // Update barrier spawn timer
                 gBarrierSpawnTimer += deltaTime;
                 if (gBarrierSpawnTimer >= BARRIER_SPAWN_INTERVAL) {
                     gBarrierSpawnTimer = 0.0f;
                     
                     // Count active barriers
                     int activeCount = 0;
                     for (const auto& barrier : gBarriers) {
                         if (barrier.active) activeCount++;
                     }

                     // Spawn new barrier if below max
                     if (activeCount < MAX_BARRIERS) {
                         // Find inactive barrier or create new one
                         Barrier* newBarrier = nullptr;
                         for (auto& barrier : gBarriers) {
                             if (!barrier.active) {
                                 newBarrier = &barrier;
                                 break;
                             }
                         }
                         if (newBarrier == nullptr) {
                             gBarriers.push_back(Barrier{});
                             newBarrier = &gBarriers.back();
                         }

                         // Initialize new barrier
                         newBarrier->x = SCREEN_WIDTH;
                         std::uniform_int_distribution<> yDist(PLAYER_BOUNDS_TOP, PLAYER_BOUNDS_BOTTOM - BARRIER_HEIGHT);
                         newBarrier->y = yDist(gRandomGenerator);
                         newBarrier->active = true;
                     }
                 }

                 // Update barriers
                 for (auto& barrier : gBarriers) {
                     if (barrier.active) {
                         barrier.x -= BARRIER_SPEED * deltaTime;
                         
                         // Deactivate if off screen
                         if (barrier.x + BARRIER_WIDTH < 0) {
                             barrier.active = false;
                         }

                         // Check collision with player
                         SDL_Rect playerRect = { PLAYER_START_X, (int)gPlayerY, PLAYER_SQUARE_SIZE, PLAYER_SQUARE_SIZE };
                         SDL_Rect barrierRect = { (int)barrier.x, (int)barrier.y, BARRIER_WIDTH, BARRIER_HEIGHT };
                         
                         if (SDL_HasIntersection(&playerRect, &barrierRect)) {
                             std::cout << "Collision detected! Game Over!" << std::endl;
                             // Play lose sound
                             if (gLoseSound != nullptr) {
                                 Mix_PlayChannel(-1, gLoseSound, 0);
                             }
                             // Change to lose state
                             gCurrentState = GameState::LOSE;
                         }
                     }
                 }
            } break;
            default: break;
        } // End update switch

        if (gCurrentState == GameState::EXIT) continue; // Skip render if exiting

        // --- Rendering ---
        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF); 
        SDL_RenderClear(gRenderer);
        
        switch(gCurrentState) {
            case GameState::MENU: {
                if (!gMenuBgFrames.empty() && gCurrentMenuFrame < gMenuBgFrames.size() && gMenuBgFrames[gCurrentMenuFrame] != nullptr) { SDL_RenderCopy(gRenderer, gMenuBgFrames[gCurrentMenuFrame], nullptr, nullptr); }
                 else { SDL_SetRenderDrawColor(gRenderer, 0x22, 0x22, 0x22, 0xFF); SDL_RenderClear(gRenderer); }
                SDL_Color playColor = gTextColor; SDL_Color aboutColor = gTextColor; SDL_Color quitColor = gTextColor;
                if (SDL_PointInRect(&mousePoint, &gPlayButtonRect)) playColor = gButtonHoverColor;
                if (SDL_PointInRect(&mousePoint, &gAboutButtonRect)) aboutColor = gButtonHoverColor;
                if (SDL_PointInRect(&mousePoint, &gQuitButtonRect)) quitColor = gButtonHoverColor;
                renderText("PLAY", BUTTON_X, BUTTON_Y_PLAY, gFont, playColor, gRenderer);
                renderText("ABOUT", BUTTON_X, BUTTON_Y_ABOUT, gFont, aboutColor, gRenderer);
                renderText("QUIT", BUTTON_X, BUTTON_Y_QUIT, gFont, quitColor, gRenderer);
            } break;
            
            case GameState::INTRO: {
                 if (gCurrentIntroSlide < gIntroSlides.size() && gIntroSlides[gCurrentIntroSlide] != nullptr) { SDL_RenderCopy(gRenderer, gIntroSlides[gCurrentIntroSlide], nullptr, nullptr); }
                  else { SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x11, 0xFF); SDL_RenderClear(gRenderer); renderText("Missing Intro Slide!", 100, 100, gFont, gTextColor, gRenderer); }
                 if (gSkipButtonTexture != nullptr) { SDL_RenderCopy(gRenderer, gSkipButtonTexture, nullptr, &gSkipButtonRect); }
            } break;
            
            case GameState::ABOUT: {
                SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x25, 0xFF); SDL_RenderClear(gRenderer);
                int startY = 50; int lineSpacing = 30; int sectionSpacing = 45; int textX = 50;
                renderText("The Story", textX, startY, gFont, gHeaderColor, gRenderer);
                renderText("In a world craving speed, you are a daring rider", textX, startY + lineSpacing, gFont, gAboutTextColor, gRenderer);
                renderText("competing in the legendary Moto Rush challenge.", textX, startY + lineSpacing * 2, gFont, gAboutTextColor, gRenderer);
                renderText("Only the fastest will reach the finish line.", textX, startY + lineSpacing * 3, gFont, gAboutTextColor, gRenderer);
                startY += lineSpacing * 4 + sectionSpacing;
                renderText("Your Goal", textX, startY, gFont, gHeaderColor, gRenderer);
                renderText("Reach the final destination point", textX, startY + lineSpacing, gFont, gAboutTextColor, gRenderer);
                renderText("before the timer runs out!", textX, startY + lineSpacing * 2, gFont, gAboutTextColor, gRenderer);
                startY += lineSpacing * 3 + sectionSpacing;
                renderText("Rules", textX, startY, gFont, gHeaderColor, gRenderer);
                renderText("- You have only 60 seconds to complete the race.", textX, startY + lineSpacing, gFont, gAboutTextColor, gRenderer);
                renderText("- If the timer hits zero before you finish, you lose.", textX, startY + lineSpacing * 2, gFont, gAboutTextColor, gRenderer);
                renderText("- Avoid obstacles (Customize if needed).", textX, startY + lineSpacing * 3, gFont, gAboutTextColor, gRenderer);
                startY += lineSpacing * 4 + sectionSpacing;
                renderText("Controls", textX, startY, gFont, gHeaderColor, gRenderer);
                renderText("- Left/Right Arrows: Select character (selection screen)", textX, startY + lineSpacing, gFont, gAboutTextColor, gRenderer);
                renderText("- Enter: Confirm selection / Start game", textX, startY + lineSpacing * 2, gFont, gAboutTextColor, gRenderer);
                renderText("- Right Arrow (Game): Accelerate / Move Forward", textX, startY + lineSpacing * 3, gFont, gAboutTextColor, gRenderer);
                 renderText("- Up/Down Arrows (Game): Move Vertically", textX, startY + lineSpacing*4, gFont, gAboutTextColor, gRenderer); // Added Up/Down info
                renderText("- ESC (Game): Return to Main Menu", textX, startY + lineSpacing*5, gFont, gAboutTextColor, gRenderer);
                renderText("Press any key or click to return to Menu", 150, SCREEN_HEIGHT - 60, gFont, gButtonHoverColor, gRenderer);
            } break;
            
            case GameState::PLAYING: {
                // Render background
                if (gGameBgFarTexture != nullptr) { 
                    SDL_RenderCopy(gRenderer, gGameBgFarTexture, nullptr, nullptr); 
                } else { 
                    SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x33, 0xFF); 
                    SDL_RenderClear(gRenderer); 
                }

                // Render barriers
                for (const auto& barrier : gBarriers) {
                    if (barrier.active) {
                        SDL_Rect barrierRect = { (int)barrier.x, (int)barrier.y, BARRIER_WIDTH, BARRIER_HEIGHT };
                        if (gBarrierTexture != nullptr) {
                            SDL_RenderCopy(gRenderer, gBarrierTexture, nullptr, &barrierRect);
                        } else {
                            SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
                            SDL_RenderFillRect(gRenderer, &barrierRect);
                        }
                    }
                }

                // Render player
                SDL_Rect playerRect = { PLAYER_START_X, (int)gPlayerY, PLAYER_SQUARE_SIZE, PLAYER_SQUARE_SIZE };
                if (gPlayerTexture != nullptr) {
                    SDL_RenderCopy(gRenderer, gPlayerTexture, nullptr, &playerRect);
                } else {
                    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
                    SDL_RenderFillRect(gRenderer, &playerRect);
                }
            } break;

            case GameState::LOSE: {
                if (gLoseScreenTexture != nullptr) {
                    SDL_RenderCopy(gRenderer, gLoseScreenTexture, nullptr, nullptr);
                } else {
                    SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x11, 0xFF);
                    SDL_RenderClear(gRenderer);
                    renderText("GAME OVER!", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, gFont, gTextColor, gRenderer);
                    renderText("Click or press any key to return to menu", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 50, gFont, gButtonHoverColor, gRenderer);
                }
            } break;

            default: {
                SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255); 
                SDL_RenderClear(gRenderer);
                renderText("UNKNOWN GAME STATE", 100, 100, gFont, gTextColor, gRenderer);
            } break;
        }
        
        // Update Screen
        SDL_RenderPresent(gRenderer);
    }

    // --- Shutdown ---
    std::cout << "\n===== Exiting Main Loop (State = " << static_cast<int>(gCurrentState) << ") =====\n" << std::endl;
    closeSDL();
    std::cout << "Application Exited Gracefully." << std::endl;
    return 0;
}
    