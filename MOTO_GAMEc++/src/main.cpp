#include "Game.h"
#include <stdio.h>

int main(int /*argc*/, char* /*argv*/[]) {
    printf("Lancement du jeu (SDL2)...\n");
    Game gameInstance;
    if (!gameInstance.init()) {
        printf("Erreur initialisation jeu!\n");
        return -1;
    }
    printf("Initialisation reussie. Lancement boucle...\n");
    gameInstance.run();
    printf("Boucle terminee. Nettoyage...\n");
    gameInstance.cleanup();
    printf("Sortie.\n");
    return 0;
}    