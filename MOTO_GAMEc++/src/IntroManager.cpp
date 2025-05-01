#include "IntroManager.h"
#include "Constants.h"
#include <SDL2_image/SDL_image.h>  // SDL2
#include <stdio.h>

IntroManager::IntroManager() :
    rendererRef(nullptr), fontRef(nullptr),
    skipButtonTexture(nullptr), currentTextTexture(nullptr),
    currentSlideIndex(-1), audioChannel(-1),
    skipRequested(false), isFinished(false)
{
    skipButtonRect = { SCREEN_WIDTH - 110, SCREEN_HEIGHT - 60, 100, 50 };
}

IntroManager::~IntroManager() {
    unload();
}

bool IntroManager::createTextTexture(const std::string& text, SDL_Color color, SDL_Texture** outTexture) {
     if (currentTextTexture) { SDL_DestroyTexture(currentTextTexture); currentTextTexture = nullptr; }
     if (text.empty() || !fontRef || !rendererRef) { *outTexture = nullptr; return true; }

     SDL_Surface* textSurface = TTF_RenderText_Blended(fontRef, text.c_str(), color);
     if (!textSurface) { /* error */ return false; }
     *outTexture = SDL_CreateTextureFromSurface(rendererRef, textSurface);
     SDL_FreeSurface(textSurface); // Utilise SDL_FreeSurface
     if (!*outTexture) { /* error */ return false; }
     return true;
}


bool IntroManager::load(SDL_Renderer* renderer, TTF_Font* font) {
    rendererRef = renderer;
    fontRef = font;
    if (!rendererRef || !fontRef) return false;

    skipButtonTexture = IMG_LoadTexture(rendererRef, SKIP_BUTTON_PATH.c_str()); // Utilise IMG_LoadTexture
    if (!skipButtonTexture) printf("Warning: Skip button texture not found.\n");

    slideTextures.resize(NUM_INTRO_SLIDES);
    slideAudios.resize(NUM_INTRO_SLIDES);
    slideTexts.resize(NUM_INTRO_SLIDES);

     slideTexts = { // Exemple de textes
        "Il etait une fois, dans un futur poussiereux...",
        "Sa soeur fut enlevee par la terrible Mafia du Temps...",
        "Leur chef lui envoya un message glacial: 'Tu as 90 secondes.'",
        "Il enfourcha sa chrono-moto, une course contre la mort commencait..."
    };

    for (int i = 0; i < NUM_INTRO_SLIDES; ++i) {
        std::string imgPath = INTRO_IMAGE_PATH_PREFIX + std::to_string(i + 1) + ".png";
        slideTextures[i] = IMG_LoadTexture(rendererRef, imgPath.c_str());
        if (!slideTextures[i]) { /* error */ unload(); return false; }

        std::string audioPath = INTRO_AUDIO_PATH_PREFIX + std::to_string(i + 1) + ".wav";
        slideAudios[i] = Mix_LoadWAV(audioPath.c_str()); // Utilise Mix_LoadWAV
        if (!slideAudios[i]) {
             printf("Erreur chargement audio intro %d: %s\n", i + 1, Mix_GetError());
             // Optionnel: Continuer sans son ou échouer?
             // unload(); return false; // Décommenter pour échouer si audio critique
        }
    }
    currentSlideIndex = -1;
    skipRequested = false;
    isFinished = false;
    currentTextTexture = nullptr;
    audioChannel = -1; // Aucun canal utilisé pour l'instant

    printf("IntroManager loaded %d slides.\n", NUM_INTRO_SLIDES);
    return true;
}

void IntroManager::unload() {
    printf("Unloading IntroManager...\n");
    if (audioChannel != -1) { // Arrêter le son avant de libérer
        Mix_HaltChannel(audioChannel);
        audioChannel = -1;
    }
    for (SDL_Texture* tex : slideTextures) { if (tex) SDL_DestroyTexture(tex); }
    slideTextures.clear();
    for (Mix_Chunk* chunk : slideAudios) { if (chunk) Mix_FreeChunk(chunk); } // Utilise Mix_FreeChunk
    slideAudios.clear();
    slideTexts.clear();
    if (skipButtonTexture) SDL_DestroyTexture(skipButtonTexture);
    if (currentTextTexture) SDL_DestroyTexture(currentTextTexture);
    skipButtonTexture = currentTextTexture = nullptr;
    rendererRef = nullptr;
    fontRef = nullptr;
}

void IntroManager::start() {
    printf("IntroManager starting...\n");
    currentSlideIndex = 0;
    skipRequested = false;
    isFinished = false;
    audioChannel = -1; // Reset channel
    createTextTexture(slideTexts[currentSlideIndex], {255, 255, 255, 255}, &currentTextTexture);

    // Joue le premier son
    if (currentSlideIndex < slideAudios.size() && slideAudios[currentSlideIndex] != nullptr) {
        audioChannel = Mix_PlayChannel(-1, slideAudios[currentSlideIndex], 0); // Joue une fois
        if (audioChannel == -1) {
            printf("Erreur Mix_PlayChannel pour slide 0: %s\n", Mix_GetError());
        } else {
             printf("Audio slide 0 playing on channel %d\n", audioChannel);
        }
    }
}

bool IntroManager::update(float deltaTime) {
    if (isFinished || skipRequested || currentSlideIndex < 0) {
        return true; // Intro terminée ou skip
    }

    // Vérifie si le son de la slide actuelle est terminé
    bool audioDone = true; // Supposer terminé si pas de son ou pas de canal
    if (audioChannel != -1) {
        if (Mix_Playing(audioChannel) != 0) { // Mix_Playing retourne 1 si ça joue, 0 sinon
            audioDone = false;
        } else {
            // Le son vient de se terminer sur ce canal
            printf("Audio slide %d finished on channel %d\n", currentSlideIndex + 1, audioChannel);
            audioChannel = -1; // Marquer le canal comme libre
        }
    } else if (currentSlideIndex < slideAudios.size() && slideAudios[currentSlideIndex] != nullptr) {
         // Si on n'avait pas de canal et qu'il y a un son à jouer, on considère que l'audio n'est pas 'fini'
         // Cela peut arriver si le son n'a pas pu être joué au départ.
         // Pour l'instant, on passe quand même (basé sur le fait que Mix_Playing == 0)
         // Une logique plus robuste gérerait un timer de secours.
         audioDone = true;
    }


    if (audioDone) {
        // Passer à la slide suivante
        currentSlideIndex++;
        if (currentSlideIndex >= NUM_INTRO_SLIDES) {
            printf("Intro finished naturally.\n");
            isFinished = true; // Fin de l'intro
            if (currentTextTexture) { SDL_DestroyTexture(currentTextTexture); currentTextTexture = nullptr; }
        } else {
            // Jouer le son de la nouvelle slide
            createTextTexture(slideTexts[currentSlideIndex], {255, 255, 255, 255}, &currentTextTexture);
            if (currentSlideIndex < slideAudios.size() && slideAudios[currentSlideIndex] != nullptr) {
                 audioChannel = Mix_PlayChannel(-1, slideAudios[currentSlideIndex], 0);
                 if (audioChannel == -1) {
                     printf("Erreur Mix_PlayChannel pour slide %d: %s\n", currentSlideIndex + 1, Mix_GetError());
                 } else {
                     printf("Audio slide %d playing on channel %d\n", currentSlideIndex + 1, audioChannel);
                 }
            } else {
                 audioChannel = -1; // Pas de son pour cette slide ou erreur chargement
            }
        }
    }

    return isFinished;
}

void IntroManager::render(SDL_Renderer* renderer) {
    if (!renderer || currentSlideIndex < 0 || currentSlideIndex >= slideTextures.size()) return;

    if (slideTextures[currentSlideIndex]) {
        SDL_RenderCopy(renderer, slideTextures[currentSlideIndex], nullptr, nullptr); // Utilise SDL_RenderCopy
    }

    if (currentTextTexture) {
        SDL_Rect textRect;
        SDL_QueryTexture(currentTextTexture, nullptr, nullptr, &textRect.w, &textRect.h);
        textRect.x = (SCREEN_WIDTH - textRect.w) / 2;
        textRect.y = SCREEN_HEIGHT - textRect.h - 20;
        SDL_RenderCopy(renderer, currentTextTexture, nullptr, &textRect); // Utilise SDL_RenderCopy
    }

    if (skipButtonTexture) {
        SDL_RenderCopy(renderer, skipButtonTexture, nullptr, &skipButtonRect); // Utilise SDL_RenderCopy
    }
}

void IntroManager::handleEvent(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            SDL_Point mousePoint = { event.button.x, event.button.y };
            if (SDL_PointInRect(&mousePoint, &skipButtonRect)) {
                printf("Skip button clicked!\n");
                skipRequested = true;
                isFinished = true; // Marque comme fini
                if (audioChannel != -1) { // Arrête le son en cours si skip
                    Mix_HaltChannel(audioChannel);
                    audioChannel = -1;
                }
            }
        }
    }
     // Gérer aussi la touche Echap pour skip ?
     else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
         printf("Escape pressed during intro - Skipping.\n");
         skipRequested = true;
         isFinished = true;
         if (audioChannel != -1) {
             Mix_HaltChannel(audioChannel);
             audioChannel = -1;
         }
     }
}