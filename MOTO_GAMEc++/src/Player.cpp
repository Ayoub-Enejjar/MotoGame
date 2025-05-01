#include "Player.h"
#include "Constants.h"
#include <SDL2_image/SDL_image.h> // SDL2 include
#include <stdio.h>
#include <cmath> // Pour static_cast et roundf
 

Player::Player(float startX, float startY)
    : texture(nullptr), x(startX), y(startY), width(50.0f), height(50.0f), speed(PLAYER_SPEED) {
    printf("Player cree a (%.2f, %.2f)\n", x, y);
}

Player::~Player() {
    unloadTexture();
    printf("Player detruit.\n");
}

bool Player::loadTexture(SDL_Renderer* renderer, const std::string& path) {
    unloadTexture();
    printf("Chargement texture joueur: %s\n", path.c_str());
    // Utilise IMG_LoadTexture de SDL2_image
    texture = IMG_LoadTexture(renderer, path.c_str());
    if (texture == nullptr) {
        printf("Erreur chargement texture joueur %s! IMG_Error: %s\n", path.c_str(), IMG_GetError());
        return false;
    }
    // SDL_QueryTexture prend int* en SDL2
    int texW = 0, texH = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH) == 0) {
        width = (float)texW;
        height = (float)texH;
        printf("Texture joueur chargee (%dx%d).\n", texW, texH);
        x -= width / 2.0f; // Centrer après avoir eu la largeur
    } else {
        printf("Avertissement: Impossible de recuperer dimensions texture joueur %s. SDL_Error: %s\n", path.c_str(), SDL_GetError());
    }
    return true;
}

void Player::unloadTexture() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        printf("Texture joueur dechargee.\n");
    }
}

void Player::update(float deltaTime, const Uint8* keyStates) { // Utilise Uint8*
    if (!keyStates) return;
    float moveX = 0.0f;
    float moveY = 0.0f;

    // Vérifie les touches (SDL2 utilise Uint8, 1 si pressé)
    if (keyStates[SDL_SCANCODE_LEFT] || keyStates[SDL_SCANCODE_A]) moveX -= 1.0f;
    if (keyStates[SDL_SCANCODE_RIGHT] || keyStates[SDL_SCANCODE_D]) moveX += 1.0f;
    // Activer si besoin
    // if (keyStates[SDL_SCANCODE_UP] || keyStates[SDL_SCANCODE_W]) moveY -= 1.0f;
    // if (keyStates[SDL_SCANCODE_DOWN] || keyStates[SDL_SCANCODE_S]) moveY += 1.0f;

    x += moveX * speed * deltaTime;
    y += moveY * speed * deltaTime;

    // Boundary checks
    if (x < 0.0f) x = 0.0f;
    // Pas de limite à droite pour un monde qui défile
    // if (x + width > SCREEN_WIDTH) x = SCREEN_WIDTH - width;
    if (y < 0.0f) y = 0.0f;
    if (y + height > SCREEN_HEIGHT) y = SCREEN_HEIGHT - height;
}

void Player::render(SDL_Renderer* renderer) {
    if (texture == nullptr || renderer == nullptr) return;
    // Crée un SDL_Rect en arrondissant les floats pour SDL_RenderCopy
    SDL_Rect destRect = {
        static_cast<int>(std::roundf(x)),
        static_cast<int>(std::roundf(y)),
        static_cast<int>(std::roundf(width)),
        static_cast<int>(std::roundf(height))
    };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect); // Utilise SDL_RenderCopy
}

SDL_Rect Player::getRect() const {
    return {
        static_cast<int>(std::roundf(x)),
        static_cast<int>(std::roundf(y)),
        static_cast<int>(std::roundf(width)),
        static_cast<int>(std::roundf(height))
    };
}