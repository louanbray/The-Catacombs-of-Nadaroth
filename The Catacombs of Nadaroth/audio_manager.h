#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

typedef enum AudioID {
    AUDIO_FOOTSTEP1,
    AUDIO_FOOTSTEP2,
    AUDIO_COUNT
} AudioID;

/**
 * Initialize SDL audio and SDL_mixer.
 * Returns 0 on success, or non-zero on error.
 */
int init_audio();

/**
 * Play background music from the specified file.
 * If loop is non-zero, the music will loop indefinitely;
 * otherwise, it will play once.
 * Any existing background music is stopped and freed first.
 */
void play_bgm(const char* filename, int loop);

/**
 * @brief Plays a sound effect based on the provided audio ID.
 *
 * This function triggers the playback of a sound effect corresponding to the
 * given AudioID. The AudioID should be a valid identifier for a sound effect
 * that has been loaded into the audio manager.
 *
 * @param audio_id The identifier of the sound effect to be played.
 */
void play_sound_effect_by_id(AudioID audio_id);

/**
 * Stop any currently playing background music.
 */
void stop_audio();

/**
 * Clean up and close the audio system.
 * This stops any playing audio, frees background music and cached sound effects,
 * closes the audio device, and quits SDL.
 */
void audio_close();

#endif