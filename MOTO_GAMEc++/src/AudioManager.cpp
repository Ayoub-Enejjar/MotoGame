#include "AudioManager.h"
#include <stdio.h> // Pour les messages d'erreur/debug

AudioManager::AudioManager() : backgroundMusic(nullptr), isInitialized(false) {}

AudioManager::~AudioManager() {
    unloadAll(); // Le destructeur s'assure que tout est libéré
    closeAudio(); // Ferme Mixer si ce n'était pas déjà fait
}

bool AudioManager::initAudio() {
    if (isInitialized) {
        printf("AudioManager: Deja initialise.\n");
        return true; // Déjà prêt
    }

    // Init SDL_mixer (Fréquence, Format, Canaux, Taille Chunk)
    // MIX_DEFAULT_FORMAT = AUDIO_S16LSB en SDL2 (Signed 16-bit Little Endian)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("AudioManager Error: SDL_mixer n'a pas pu initialiser! Mix_Error: %s\n", Mix_GetError());
        isInitialized = false;
        return false;
    }

    // Optionnel: Allouer des canaux de mixage si besoin de plus que 8 par défaut
    // Mix_AllocateChannels(16);

    printf("AudioManager: SDL_mixer initialise avec succes.\n");
    isInitialized = true;
    return true;
}

void AudioManager::closeAudio() {
    if (isInitialized) {
        printf("AudioManager: Fermeture de SDL_mixer...\n");
        unloadAll(); // S'assurer que tout est déchargé avant de quitter
        Mix_CloseAudio(); // Ferme le périphérique audio ouvert par OpenAudio
        while(Mix_Init(0)) Mix_Quit(); // Quitte les sous-systèmes (MP3, OGG, etc.)
        isInitialized = false;
    }
}

bool AudioManager::loadSound(const std::string& id, const std::string& path) {
    if (!isInitialized) {
        printf("AudioManager Error: Tentative de charger un son sans initialisation!\n");
        return false;
    }

    // Vérifier si l'ID existe déjà pour éviter les fuites mémoire
    if (soundEffects.count(id)) {
        printf("AudioManager Warning: Remplacement du son avec ID '%s'.\n", id.c_str());
        unloadSound(id); // Décharge l'ancien avant de charger le nouveau
    }

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        printf("AudioManager Error: Erreur chargement son '%s' (%s): %s\n",
               id.c_str(), path.c_str(), Mix_GetError());
        return false;
    }

    soundEffects[id] = chunk; // Stocke le son chargé dans la map
    printf("AudioManager: Son charge '%s' depuis '%s'.\n", id.c_str(), path.c_str());
    return true;
}

bool AudioManager::loadMusic(const std::string& path) {
     if (!isInitialized) return false;

     // Décharge l'ancienne musique s'il y en a une
     unloadMusic();

     backgroundMusic = Mix_LoadMUS(path.c_str()); // Charge formats variés (OGG, MP3, WAV...)
     if (!backgroundMusic) {
         printf("AudioManager Error: Erreur chargement musique '%s': %s\n", path.c_str(), Mix_GetError());
         return false;
     }
     printf("AudioManager: Musique chargee depuis '%s'.\n", path.c_str());
     return true;
}

void AudioManager::unloadSound(const std::string& id) {
    auto it = soundEffects.find(id);
    if (it != soundEffects.end()) {
        if (it->second) { // Vérifie si le pointeur n'est pas null
            Mix_FreeChunk(it->second); // Libère le son
            printf("AudioManager: Son decharge '%s'.\n", id.c_str());
        }
        soundEffects.erase(it); // Retire l'entrée de la map
    }
}

void AudioManager::unloadMusic() {
    if (backgroundMusic) {
        if (Mix_PlayingMusic()) { // Arrêter avant de décharger si elle joue
             Mix_HaltMusic();
        }
        Mix_FreeMusic(backgroundMusic); // Libère la musique
        backgroundMusic = nullptr;
        printf("AudioManager: Musique dechargee.\n");
    }
}

void AudioManager::unloadAll() {
    printf("AudioManager: Dechargement de tous les sons...\n");
    // Décharger la musique
    unloadMusic();

    // Décharger les effets sonores
    // Utilisation d'une boucle propre pour itérer et effacer
    for (auto const& [id, chunk] : soundEffects) {
         if (chunk) Mix_FreeChunk(chunk);
         printf("AudioManager: Son decharge '%s'.\n", id.c_str());
    }
    soundEffects.clear(); // Vide la map
}

int AudioManager::playSound(const std::string& id, int loops) {
    if (!isInitialized) return -1;

    auto it = soundEffects.find(id);
    if (it == soundEffects.end() || it->second == nullptr) {
        printf("AudioManager Warning: Tentative de jouer un son non charge: '%s'\n", id.c_str());
        return -1; // Son non trouvé ou invalide
    }

    // Joue le son sur le premier canal disponible (-1)
    int channel = Mix_PlayChannel(-1, it->second, loops);
    if (channel == -1) {
        printf("AudioManager Error: Impossible de jouer le son '%s': %s\n", id.c_str(), Mix_GetError());
    } else {
        // printf("AudioManager: Son '%s' joue sur le canal %d.\n", id.c_str(), channel); // Optionnel: Debug
    }
    return channel; // Retourne le canal utilisé ou -1
}

bool AudioManager::playMusic(int loops) {
    if (!isInitialized || !backgroundMusic) {
        if(!backgroundMusic) printf("AudioManager Warning: Pas de musique chargee a jouer.\n");
        return false;
    }
    if (Mix_PlayingMusic()) {
         printf("AudioManager: Musique deja en cours, arret avant relance.\n");
         Mix_HaltMusic(); // Arrête avant de relancer si déjà en cours
    }
    if (Mix_PlayMusic(backgroundMusic, loops) == -1) {
        printf("AudioManager Error: Impossible de jouer la musique: %s\n", Mix_GetError());
        return false;
    }
    printf("AudioManager: Lecture musique demarree (loops: %d).\n", loops);
    return true;
}

void AudioManager::pauseMusic() {
    if (Mix_PlayingMusic()) {
        Mix_PauseMusic();
        printf("AudioManager: Musique en pause.\n");
    }
}

void AudioManager::resumeMusic() {
    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
        printf("AudioManager: Reprise musique.\n");
    }
}

void AudioManager::stopMusic() {
    if (Mix_PlayingMusic() || Mix_PausedMusic()) {
        Mix_HaltMusic(); // Arrête complètement
        printf("AudioManager: Musique arretee.\n");
    }
}

bool AudioManager::isMusicPlaying() {
    return Mix_PlayingMusic() == 1; // Retourne 1 si joue, 0 sinon
}

void AudioManager::setSoundVolume(const std::string& id, int volume) {
    auto it = soundEffects.find(id);
    if (it != soundEffects.end() && it->second != nullptr) {
         // Clamp volume
         if (volume < 0) volume = 0;
         if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
         Mix_VolumeChunk(it->second, volume);
          printf("AudioManager: Volume son '%s' mis a %d.\n", id.c_str(), volume);
    } else {
         printf("AudioManager Warning: Tentative de regler volume son non charge: '%s'\n", id.c_str());
    }
}

void AudioManager::setAllSoundsVolume(int volume) {
    if (!isInitialized) return;
     // Clamp volume
     if (volume < 0) volume = 0;
     if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
     // -1 applique à tous les canaux (de 0 à Mix_AllocateChannels()-1)
     Mix_Volume(-1, volume);
     printf("AudioManager: Volume tous effets sonores mis a %d.\n", volume);
}

void AudioManager::setMusicVolume(int volume) {
     if (!isInitialized) return;
      // Clamp volume
     if (volume < 0) volume = 0;
     if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
     Mix_VolumeMusic(volume);
     printf("AudioManager: Volume musique mis a %d.\n", volume);
}