#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL2_mixer/SDL_mixer.h> // SDL2 include
#include <string>
#include <map> // Pour stocker les sons par nom/ID


class AudioManager {
public:
    AudioManager();
    ~AudioManager(); // Important pour libérer les ressources

    // Initialisation et fermeture (appelées par Game::init et Game::cleanup)
    // Retourne true si l'initialisation de Mixer réussit
    bool initAudio();
    void closeAudio();

    // Chargement des ressources audio
    // Retourne true si le chargement réussit
    bool loadSound(const std::string& id, const std::string& path);
    bool loadMusic(const std::string& path); // Suppose une seule musique de fond

    // Libération des ressources
    void unloadSound(const std::string& id);
    void unloadMusic();
    void unloadAll(); // Libère tout

    // Contrôles de lecture
    // Joue un effet sonore chargé via son ID. loops=0 -> joue une fois, loops=-1 -> infini
    // Retourne le canal utilisé ou -1 si erreur.
    int playSound(const std::string& id, int loops = 0);

    // Joue la musique chargée. loops=-1 -> infini
    bool playMusic(int loops = -1);
    void pauseMusic();
    void resumeMusic();
    void stopMusic();
    bool isMusicPlaying();

    // Contrôle du volume (0 à MIX_MAX_VOLUME=128)
    void setSoundVolume(const std::string& id, int volume); // Volume pour un son spécifique (via son chunk)
    void setAllSoundsVolume(int volume); // Volume pour tous les canaux
    void setMusicVolume(int volume);

private:
    // Stockage des ressources chargées
    std::map<std::string, Mix_Chunk*> soundEffects; // Associe un ID (string) à un son (Mix_Chunk)
    Mix_Music* backgroundMusic;                     // Musique de fond

    bool isInitialized; // Pour savoir si Mixer est prêt
};

#endif // AUDIOMANAGER_H