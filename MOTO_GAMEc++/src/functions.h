#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

// Helper Functions
SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer);
bool renderText(const std::string& text, int x, int y, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer);

// Core Game Functions
bool initializeSDL();
void resetGameState();
bool loadMedia();
void closeSDL();
void playCurrentIntroAudio();

#endif // FUNCTIONS_H