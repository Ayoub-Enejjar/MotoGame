#include "MenuManager.h"
#include "Constants.h"
#include <SDL2_image/SDL_image.h> // SDL2
#include <stdio.h>


MenuManager::MenuManager() : backgroundTexture(nullptr), fontRef(nullptr),
                             playTexture(nullptr), aboutTexture(nullptr), quitTexture(nullptr),
                             mouseX(0), mouseY(0) {
    playRect = {0,0,0,0}; aboutRect = {0,0,0,0}; quitRect = {0,0,0,0};
}

MenuManager::~MenuManager() {
    unload();
}

bool MenuManager::createTextButton(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                                   SDL_Color color, SDL_Texture** texture, SDL_Rect* rect, int yPos) {
    if (!font || !renderer) return false;
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!textSurface) { /* error */ return false; }
    *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!*texture) { /* error */ SDL_FreeSurface(textSurface); return false; } // Utilise SDL_FreeSurface

    rect->w = textSurface->w;
    rect->h = textSurface->h;
    rect->x = (SCREEN_WIDTH - rect->w) / 2;
    rect->y = yPos;

    SDL_FreeSurface(textSurface); // Important en SDL2
    return true;
}

bool MenuManager::load(SDL_Renderer* renderer, TTF_Font* font) {
    fontRef = font;
    if (!fontRef) return false;

    backgroundTexture = IMG_LoadTexture(renderer, MENU_BACKGROUND_PATH.c_str());
    if (!backgroundTexture) { /* error */ return false; }

    int buttonYStart = SCREEN_HEIGHT / 2;
    int buttonSpacing = 60;
    SDL_Color white = {255, 255, 255, 255};

    if (!createTextButton(renderer, fontRef, "Jouer", white, &playTexture, &playRect, buttonYStart)) return false;
    if (!createTextButton(renderer, fontRef, "A Propos", white, &aboutTexture, &aboutRect, buttonYStart + buttonSpacing)) return false;
    if (!createTextButton(renderer, fontRef, "Quitter", white, &quitTexture, &quitRect, buttonYStart + 2 * buttonSpacing)) return false;

    printf("MenuManager loaded.\n");
    return true;
}

void MenuManager::unload() {
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    if (playTexture) SDL_DestroyTexture(playTexture);
    if (aboutTexture) SDL_DestroyTexture(aboutTexture);
    if (quitTexture) SDL_DestroyTexture(quitTexture);
    backgroundTexture = playTexture = aboutTexture = quitTexture = nullptr;
    fontRef = nullptr;
     printf("MenuManager unloaded.\n");
}

EtatJeu MenuManager::handleEvent(SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION) {
        mouseX = event.motion.x;
        mouseY = event.motion.y;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            SDL_Point mousePoint = { event.button.x, event.button.y };
            if (SDL_PointInRect(&mousePoint, &playRect)) return EtatJeu::INTRO;   // Lance l'intro
            if (SDL_PointInRect(&mousePoint, &aboutRect)) return EtatJeu::ABOUT; // Va à A Propos
            if (SDL_PointInRect(&mousePoint, &quitRect)) return EtatJeu::QUIT;    // Demande à quitter
        }
    }
    return EtatJeu::MENU; // Reste dans le menu par défaut
}

void MenuManager::render(SDL_Renderer* renderer) {
    if (!renderer) return;
    if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr); // Utilise SDL_RenderCopy

    // Ajouter la logique pour afficher les textures _hover si la souris est dessus
    if (playTexture) SDL_RenderCopy(renderer, playTexture, nullptr, &playRect);
    if (aboutTexture) SDL_RenderCopy(renderer, aboutTexture, nullptr, &aboutRect);
    if (quitTexture) SDL_RenderCopy(renderer, quitTexture, nullptr, &quitRect);
}