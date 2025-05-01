#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h> // SDL2 include
#include <string>

class Player {
public:
    Player(float startX, float startY);
    ~Player();
    bool loadTexture(SDL_Renderer* renderer, const std::string& path);
    void unloadTexture();
    void update(float deltaTime, const Uint8* keyStates); // Utilise Uint8* pour SDL2
    void render(SDL_Renderer* renderer);
    SDL_Rect getRect() const; // Utilise SDL_Rect pour SDL2
    float getX() const { return x; }

private:
    SDL_Texture* texture;
    float x, y;
    float width, height; // Garder float pour logique interne précise
    float speed;
};

#endif // PLAYER_H