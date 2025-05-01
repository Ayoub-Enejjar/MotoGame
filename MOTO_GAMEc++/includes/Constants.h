#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// --- Fenêtre ---
const int SCREEN_WIDTH = 12800;
const int SCREEN_HEIGHT = 800;
const std::string WINDOW_TITLE = "Moto Game C++ (SDL2)";

// --- Gameplay ---
const float PLAYER_SPEED = 350.0f;       // Pixels par seconde
const float GAME_DURATION_SECONDS = 90.0f; // Temps limite en secondes pour gagner
const float WIN_CONDITION_X = 7500.0f; // Exemple: Coordonnée X à atteindre pour gagner

// --- Assets Paths ---
// Assurez-vous que ces chemins correspondent exactement à vos fichiers
const std::string FONT_PATH = "assets/fonts/game_font.ttf"; // <<< METTEZ LE VRAI NOM DE VOTRE POLICE
const std::string MENU_BACKGROUND_PATH = "assets/images/menu_background.png";
const std::string SKIP_BUTTON_PATH = "assets/images/ui/skip_button.png";
const std::string GAME_BACKGROUND_NEAR_PATH = "assets/images/background_near.png";
const std::string GAME_BACKGROUND_FAR_PATH = "assets/images/background_far.png";
const std::string MALE_PREVIEW_PATH = "assets/images/select/player_male.png"; // Utilise l'image joueur directement pour preview
const std::string FEMALE_PREVIEW_PATH = "assets/images/select/player_female.png";
const std::string MALE_TEXTURE_PATH = "assets/images/player_male.png";
const std::string FEMALE_TEXTURE_PATH = "assets/images/player_female.png";
const std::string WIN_SLIDE_PATH = "assets/images/endscreen/win_slide.png";
const std::string LOSE_SLIDE_PATH = "assets/images/endscreen/lose_slide.png";
const std::string WIN_AUDIO_PATH = "assets/audio/win_audio.wav";
const std::string LOSE_AUDIO_PATH = "assets/audio/lose_audio.wav";

// Intro (Adaptez le nombre et les chemins)
const int NUM_INTRO_SLIDES = 4;
const std::string INTRO_IMAGE_PATH_PREFIX = "assets/images/intro/slide_"; // ex: slide_1.png ... slide_4.png
const std::string INTRO_AUDIO_PATH_PREFIX = "assets/audio/intro_slide_"; // ex: slide_1.wav ... slide_4.wav

#endif // CONSTANTS_H