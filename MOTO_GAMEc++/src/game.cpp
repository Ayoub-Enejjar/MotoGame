#include "Game.h"
#include "Constants.h"
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>  
#include <SDL2_mixer/SDL_mixer.h> 
#include <stdio.h>
#include <string>
#include <cmath> // Pour ceil et roundf

// --- Constructeur & Destructeur ---
Game::Game() :
    window(nullptr), renderer(nullptr), isRunning(false), etatActuel(EtatJeu::INIT),
    lastFrameTime_SDL2(0), currentKeyStates(nullptr), mainFont(nullptr),
    // audioManager est construit par défaut
    // menuManager et introManager sont construits par défaut
    player(nullptr),
    bgTextureFar(nullptr),              // <<< AJOUT Initialisation
    bgTextureNear(nullptr),             // <<< AJOUT Initialisation
    bgFarScrollX(0.0f),                 // <<< AJOUT Initialisation
    bgNearScrollX(0.0f),                // <<< AJOUT Initialisation
    bgTextureWidth(0),                  // <<< AJOUT Initialisation
    winSlideTexture(nullptr), loseSlideTexture(nullptr),
    malePreviewTexture(nullptr), femalePreviewTexture(nullptr),
    selectedCharIndex(0), playerIsMale(true), gameTimer(0.0f)
{}

Game::~Game() {
    cleanup();
}

// --- Initialisation ---
bool Game::init() {
    printf("Initialisation SDL2...\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { /* error */ return false; }
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) { /* error */ SDL_Quit(); return false; }
    if (TTF_Init() == -1) { /* error */ IMG_Quit(); SDL_Quit(); return false; }

    // Initialise l'AudioManager (qui initialise Mixer)
    if (!audioManager.initAudio()) {
         printf("Echec initialisation AudioManager/SDL_mixer!\n");
         TTF_Quit(); IMG_Quit(); SDL_Quit(); return false;
    }

    printf("Creation fenetre...\n");
    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) { /* error */ audioManager.closeAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit(); return false; }

    printf("Creation renderer...\n");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { /* error */ SDL_DestroyWindow(window); audioManager.closeAudio(); TTF_Quit(); IMG_Quit(); SDL_Quit(); return false; }

    printf("Chargement ressources...\n");
    if (!loadResources()) { /* error */ cleanup(); return false; }

    etatActuel = EtatJeu::MENU;
    isRunning = true;
    lastFrameTime_SDL2 = SDL_GetPerformanceCounter();
    printf("Initialisation reussie.\n");
    return true;
}

// --- Chargement Ressources ---
bool Game::loadFonts() {
     mainFont = TTF_OpenFont(FONT_PATH.c_str(), 28);
     if (!mainFont) { /* error handling */ return false; }
     printf("Police chargee: %s\n", FONT_PATH.c_str());
     return true;
}

bool Game::loadResources() {
    if (!loadFonts()) return false;

    // Les managers chargent leurs propres ressources
    if (!menuManager.load(renderer, mainFont)) return false;
    // Passe une référence à audioManager à IntroManager pour jouer les sons
    if (!introManager.load(renderer, mainFont)) {
         printf("Erreur chargement IntroManager\n");
         return false;
    }

    // Charger les textures globales gérées par Game
    bgTextureFar = loadTexture(GAME_BACKGROUND_FAR_PATH);      // <<< MODIFIÉ
    bgTextureNear = loadTexture(GAME_BACKGROUND_NEAR_PATH);     // <<< MODIFIÉ
    winSlideTexture = loadTexture(WIN_SLIDE_PATH);
    loseSlideTexture = loadTexture(LOSE_SLIDE_PATH);
    malePreviewTexture = loadTexture(MALE_PREVIEW_PATH);
    femalePreviewTexture = loadTexture(FEMALE_PREVIEW_PATH);

    // Vérifier si TOUTES les textures essentielles sont chargées
    if (!bgTextureFar || !bgTextureNear || !winSlideTexture || !loseSlideTexture ||
        !malePreviewTexture || !femalePreviewTexture) {
        printf("Erreur: Au moins une texture globale n'a pas pu etre chargee.\n");
        return false;
    }

    // Récupérer largeur fond (utilise near, suppose far identique)
    int texW = 0, texH = 0;
    if (SDL_QueryTexture(bgTextureNear, nullptr, nullptr, &texW, &texH) == 0) {
        bgTextureWidth = texW;
    } else {
        bgTextureWidth = SCREEN_WIDTH * 2; // Fallback
        printf("Warning: Could not query near background texture width: %s\n", SDL_GetError());
    }
    bgNearScrollX = bgFarScrollX = 0.0f; // Initialise scroll

    // Charger les sons globaux (Win/Lose) via AudioManager
    if (!audioManager.loadSound("win", WIN_AUDIO_PATH)) { /* Optionnel: gérer erreur */ }
    if (!audioManager.loadSound("lose", LOSE_AUDIO_PATH)) { /* Optionnel: gérer erreur */ }

    printf("Ressources principales chargees.\n");
    return true;
}

SDL_Texture* Game::loadTexture(const std::string& path) {
    printf("Chargement texture: %s\n", path.c_str());
    SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
    if (!tex) printf("Erreur chargement texture %s: %s\n", path.c_str(), IMG_GetError());
    return tex;
}

// --- Déchargement ---
void Game::unloadResources() {
    printf("Dechargement ressources...\n");
    if (player) player->unloadTexture();
    menuManager.unload();
    introManager.unload();
    audioManager.unloadAll(); // Demande à AudioManager de libérer tous ses sons

    // Libérer les textures gérées par Game
    if (bgTextureFar) SDL_DestroyTexture(bgTextureFar);
    if (bgTextureNear) SDL_DestroyTexture(bgTextureNear); // <<< AJOUT
    if (winSlideTexture) SDL_DestroyTexture(winSlideTexture);
    if (loseSlideTexture) SDL_DestroyTexture(loseSlideTexture);
    if (malePreviewTexture) SDL_DestroyTexture(malePreviewTexture);
    if (femalePreviewTexture) SDL_DestroyTexture(femalePreviewTexture);
    bgTextureFar=bgTextureNear=winSlideTexture=loseSlideTexture=malePreviewTexture=femalePreviewTexture=nullptr;
}

void Game::cleanup() {
    printf("Nettoyage jeu...\n");
    unloadResources(); // Libère textures, sons via managers

    if (player) { delete player; player = nullptr; }
    if (mainFont) { TTF_CloseFont(mainFont); mainFont = nullptr; }

    audioManager.closeAudio(); // Demande à AudioManager de fermer Mixer

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    renderer = nullptr; window = nullptr;

    // SDL_mixer, TTF, Image sont quittés par AudioManager et les appels directs ici
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    printf("Nettoyage termine.\n");
}


// --- Boucle Principale & Gestion États --- (Identique à avant)
void Game::run() { /* ... comme avant ... */ }
void Game::handleEvents() { /* ... comme avant ... */ }
void Game::update(float deltaTime) { /* ... comme avant ... */ }
void Game::render() { /* ... comme avant ... */ }

// --- Implémentation Logique Spécifique aux États ---

// Menu (Identique à avant)
void Game::updateMenu(float dt) { /* ... */ }
void Game::renderMenu() { menuManager.render(renderer); }
void Game::handleMenuEvent(SDL_Event& e) {
    EtatJeu nextState = menuManager.handleEvent(e);
    if (nextState != EtatJeu::MENU) {
        if (nextState == EtatJeu::QUIT) { isRunning = false; }
        else if (nextState == EtatJeu::INTRO) {
             introManager.start(); // L'intro jouera les sons elle-même via AudioManager
             transitionState(EtatJeu::INTRO);
        } else { transitionState(nextState); } // ex: ABOUT
    }
}

// Intro (Identique à avant - utilise maintenant AudioManager via IntroManager)
void Game::updateIntro(float dt) {
    if (introManager.update(dt) || introManager.wasSkipped()) {
        transitionState(EtatJeu::CHAR_SELECT);
    }
}
void Game::renderIntro() { introManager.render(renderer); }
void Game::handleIntroEvent(SDL_Event& e) { introManager.handleEvent(e); }

// Character Select (Identique à avant)
void Game::updateCharSelect(float dt) { /* ... */ }
void Game::renderCharSelect() { /* ... */ }
void Game::handleCharSelectEvent(SDL_Event& e) { /* ... */ }

// Playing (Logique de scroll mise à jour)
void Game::updatePlaying(float dt) {
    if (!player || !currentKeyStates) return;

    float oldPlayerX = player->getX();
    player->update(dt, currentKeyStates);
    float playerDeltaX = player->getX() - oldPlayerX;

    // --- Défilement Parallaxe ---
    bgNearScrollX -= playerDeltaX;        // Vitesse normale
    bgFarScrollX -= playerDeltaX * 0.5f; // Vitesse moitié

    // Boucle (Looping) - Utilise fmodf pour garder dans [0, width) puis ajuste
    // nécessite #include <cmath>
    if (bgTextureWidth > 0) {
        bgNearScrollX = fmodf(bgNearScrollX, (float)bgTextureWidth);
        bgFarScrollX = fmodf(bgFarScrollX, (float)bgTextureWidth);
        // fmod peut retourner un négatif proche de 0, ajustons si besoin
        // mais pour le rendu double, on peut le laisser négatif.
    }


    // --- Update Timer & Check End Conditions ---
    gameTimer -= dt;
    if (gameTimer <= 0.0f) {
        audioManager.playSound("lose"); // Joue son via AudioManager
        transitionState(EtatJeu::GAME_LOST);
    } else if (player->getX() >= WIN_CONDITION_X) {
        audioManager.playSound("win"); // Joue son via AudioManager
        transitionState(EtatJeu::GAME_WON);
    }
}

// Playing (Rendu des deux fonds)
void Game::renderPlaying() {
    // --- Dessin du Fond Éloigné (deux fois pour la boucle) ---
    if (bgTextureFar) {
        // Position de la première copie (peut être négative)
        SDL_Rect destFar1 = { static_cast<int>(roundf(bgFarScrollX)), 0, bgTextureWidth, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, bgTextureFar, nullptr, &destFar1);

        // Position de la deuxième copie pour remplir l'espace
        SDL_Rect destFar2 = { static_cast<int>(roundf(bgFarScrollX)) + bgTextureWidth, 0, bgTextureWidth, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, bgTextureFar, nullptr, &destFar2);
    }

    // --- Dessin du Fond Proche (deux fois pour la boucle) ---
    if (bgTextureNear) {
        SDL_Rect destNear1 = { static_cast<int>(roundf(bgNearScrollX)), 0, bgTextureWidth, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, bgTextureNear, nullptr, &destNear1);
        SDL_Rect destNear2 = { static_cast<int>(roundf(bgNearScrollX)) + bgTextureWidth, 0, bgTextureWidth, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, bgTextureNear, nullptr, &destNear2);
    }

    // --- Dessin du Joueur ---
    if (player) { player->render(renderer); }

    // --- Affichage du Timer ---
    if (mainFont) {
        std::string timerText = "Temps: " + std::to_string(static_cast<int>(ceil(gameTimer)));
        renderText(timerText, 10, 10, {255, 255, 0, 255}); // Jaune
    }
}

void Game::handlePlayingEvent(SDL_Event& e) { /* Ex: Pause? */ }

// End Screens (Identique à avant)
void Game::updateEndScreen(float dt) { /* Rien */ }
void Game::renderEndScreen() { // Renommé pour utiliser etatActuel
    SDL_Texture* slide = (etatActuel == EtatJeu::GAME_WON) ? winSlideTexture : loseSlideTexture;
     if (slide) SDL_RenderCopy(renderer, slide, nullptr, nullptr);
     renderText("Appuyez sur Entree pour retourner au Menu", 50, SCREEN_HEIGHT - 50, {255, 255, 255, 255});
}
void Game::handleEndScreenEvent(SDL_Event& e) {
     if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)) {
        transitionState(EtatJeu::MENU);
    }
}

// --- Helper Texte (Identique à avant) ---
void Game::renderText(const std::string& text, int x, int y, SDL_Color color) { /* ... */ }

// --- Helper Changement d'État (Identique à avant) ---
void Game::transitionState(EtatJeu newState) { /* ... */ }