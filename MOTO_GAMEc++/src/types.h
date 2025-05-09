#ifndef TYPES_H
#define TYPES_H

// --- Game States ---
enum class GameState {
    MENU,
    INTRO,
    ABOUT,
    CHARACTER_SELECT,
    PLAYING,
    WIN_DELAY,
    LOSE,
    WIN,
    EXIT
};

// --- Game Object Structs ---
struct Barrier {
    float x;
    float y;
    bool active;
    int textureIndex;
};

struct Coin {
    float x;
    float y;
    bool active;
};

#endif // TYPES_H