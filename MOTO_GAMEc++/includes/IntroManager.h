#ifndef INTROMANAGER_H
#define INTROMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>   // SDL2 include
#include <vector>
#include <string>

class IntroManager {
public:
    IntroManager();
    ~IntroManager();

    bool load(SDL_Renderer* renderer, TTF_Font* font);
    void unload();
    void start();                 // Démarre la séquence
    bool update(float deltaTime); // Met à jour, retourne true si fini
    void render(SDL_Renderer* renderer);
    void handleEvent(SDL_Event& event); // Gère le clic Skip
    bool wasSkipped() const { return skipRequested; }

private:
    // Helper pour créer texture de texte
    bool createTextTexture(const std::string& text, SDL_Color color, SDL_Texture** outTexture);

    SDL_Renderer* rendererRef;
    TTF_Font* fontRef;

    std::vector<SDL_Texture*> slideTextures;
    std::vector<Mix_Chunk*> slideAudios; // Utilise Mix_Chunk pour SDL_mixer
    std::vector<std::string> slideTexts;

    SDL_Texture* skipButtonTexture;
    SDL_Rect skipButtonRect;
    SDL_Texture* currentTextTexture;

    int currentSlideIndex;
    int audioChannel; // Canal utilisé par Mix_PlayChannel
    bool skipRequested;
    bool isFinished;
};

#endif // INTROMANAGER_H