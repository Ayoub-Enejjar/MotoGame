#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>  // SDL2_ttf include
#include <string>
#include "GameState.h"


class MenuManager {
public:
    MenuManager();
    ~MenuManager();

    bool load(SDL_Renderer* renderer, TTF_Font* font);
    void unload();
    EtatJeu handleEvent(SDL_Event& event); // Retourne le nouvel état
    void render(SDL_Renderer* renderer);

private:
    bool createTextButton(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                          SDL_Color color, SDL_Texture** outTexture, SDL_Rect* outRect, int yPos);

    SDL_Texture* backgroundTexture;
    TTF_Font* fontRef;

    SDL_Texture* playTexture;
    SDL_Texture* aboutTexture;
    SDL_Texture* quitTexture;
    SDL_Rect playRect;
    SDL_Rect aboutRect;
    SDL_Rect quitRect;

    int mouseX, mouseY;
    // Ajoutez bool playHover, etc. si vous voulez gérer le survol
};

#endif // MENUMANAGER_H