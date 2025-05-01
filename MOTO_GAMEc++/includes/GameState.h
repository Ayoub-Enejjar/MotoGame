#ifndef GAMESTATE_H
#define GAMESTATE_H

enum class EtatJeu {
    INIT,
    MENU,
    INTRO,
    CHAR_SELECT,
    PLAYING,
    GAME_WON,
    GAME_LOST,
    ABOUT, // Gardons-le, même si non implémenté pour l'instant
    QUIT
};

#endif // GAMESTATE_H