#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
        
      
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    else{
        printf("SDL initialized successfully.\n");
    }
     
    // Create window
    window = SDL_CreateWindow(
        "SDL3 Tutorial",            
        SCREEN_WIDTH,               
        SCREEN_HEIGHT,             
        0                           
    );

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }


    SDL_Delay(5000);

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL subsystems
    SDL_Quit();

    printf("SDL3 Program finished successfully.\n");
    return 0;
}
