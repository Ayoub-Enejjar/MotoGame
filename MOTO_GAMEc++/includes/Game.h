#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>        // SDL2
#include <SDL2_image/SDL_image.h>  // SDL2
#include <SDL2_ttf/SDL_ttf.h>    // SDL2
#include <SDL2_mixer/SDL_mixer.h>  // SDL2
#include <string>
#include <vector>
#include "GameState.h"
#include "Player.h"
#include "MenuManager.h"
#include "IntroManager.h"
#include "AudioManager.h" // <<< Inclure le gestionnaire audio

class Game {
public:
    Game();
    ~Game();
    bool init();
    void run();
    void cleanup();

private:
    void handleEvents();
    void update(float deltaTime);
    void render();

    // Resource Management
    bool loadResources();
    void unloadResources();
    SDL_Texture* loadTexture(const std::string& path);
    // Mix_Chunk* loadSound(const std::string& path); // Déplacé dans AudioManager
    bool loadFonts();

    // State Specific Logic
    void updateMenu(float dt);      void renderMenu();     void handleMenuEvent(SDL_Event& e);
    void updateIntro(float dt);     void renderIntro();    void handleIntroEvent(SDL_Event& e);
    void updateCharSelect(float dt);void renderCharSelect();void handleCharSelectEvent(SDL_Event& e);
    void updatePlaying(float dt);   void renderPlaying();  void handlePlayingEvent(SDL_Event& e);
    void updateEndScreen(float dt); void renderEndScreen();void handleEndScreenEvent(SDL_Event& e);
    // void updateAbout(float dt);  void renderAbout();    void handleAboutEvent(SDL_Event& e);

    // Helpers
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void transitionState(EtatJeu newState);

    // Core SDL Components
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
    EtatJeu etatActuel;

    // Timing & Input
    Uint64 lastFrameTime_SDL2;
    const Uint8* currentKeyStates;

    // Fonts & Audio Manager <<< MODIFIÉ
    TTF_Font* mainFont;
    AudioManager audioManager; // Instance du gestionnaire

    // State Managers
    MenuManager menuManager;
    IntroManager introManager;

    // Game Objects & State
    Player* player;

    // --- Textures de Fond (Gameplay) ---           <<< MODIFIÉ / AJOUTÉ
    SDL_Texture* bgTextureFar;          // Fond éloigné
    SDL_Texture* bgTextureNear;         // Fond proche
    float bgFarScrollX;                 // Position de défilement X du fond éloigné
    float bgNearScrollX;                // Position de défilement X du fond proche
    int bgTextureWidth;                 // Largeur des textures de fond (supposée identique)

    // --- Autres Textures ---
    SDL_Texture* winSlideTexture;
    SDL_Texture* loseSlideTexture;
    SDL_Texture* malePreviewTexture;
    SDL_Texture* femalePreviewTexture;

    int selectedCharIndex;
    bool playerIsMale;

    // Gameplay State
    float gameTimer;
};

#endif // GAME_H