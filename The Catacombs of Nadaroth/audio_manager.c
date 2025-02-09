#include "audio_manager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_AUDIO_CHANNEL 4

static Mix_Music* current_music = NULL;

static Mix_Chunk* sound_effects[AUDIO_COUNT] = {0};

void play_bgm(const char* filename, int loop) {
    if (current_music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(current_music);
        current_music = NULL;
    }

    current_music = Mix_LoadMUS(filename);
    if (current_music == NULL) exit(EXIT_FAILURE);

    int loopCount = loop ? -1 : 0;
    if (Mix_PlayMusic(current_music, loopCount) == -1) exit(EXIT_FAILURE);
}

void load_track(const char* filename, AudioID audio_id) {
    if (audio_id < 0 || audio_id >= AUDIO_COUNT) exit(EXIT_FAILURE);
    if (sound_effects[audio_id] != NULL) return;

    Mix_Chunk* chunk = Mix_LoadWAV(filename);

    if (!chunk) exit(EXIT_FAILURE);

    sound_effects[audio_id] = chunk;
}

void play_sound_effect_by_id(AudioID audio_id) {
    if (audio_id < 0 || audio_id >= AUDIO_COUNT) return;
    Mix_Chunk* effect = sound_effects[audio_id];
    if (!effect) return;

    // Check if this effect is already playing on any channel.
    for (int i = 0; i < MAX_AUDIO_CHANNEL; i++) {
        if (Mix_Playing(i)) {
            Mix_Chunk* playing = Mix_GetChunk(i);
            if (playing == effect) return;
        }
    }

    // Try to play the effect on a free channel.
    int channel = Mix_PlayChannel(-1, effect, 0);
    if (channel == -1) {
        // No free channel available; override one (choose channel 0 for example).
        Mix_HaltChannel(0);
        channel = Mix_PlayChannel(0, effect, 0);
        if (channel == -1) {
            fprintf(stderr, "Failed to play sound effect %d even after overriding a channel: %s\n", audio_id, Mix_GetError());
        }
    }
}

void stop_audio() {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
    if (current_music != NULL) {
        Mix_FreeMusic(current_music);
        current_music = NULL;
    }
}

/**
 * Free all sound effects stored in the cache.
 */
void free_sound_effects() {
    for (int i = 0; i < AUDIO_COUNT; i++) {
        if (sound_effects[i]) {
            Mix_FreeChunk(sound_effects[i]);
            sound_effects[i] = NULL;
        }
    }
}

void audio_close() {
    stop_audio();
    free_sound_effects();
    Mix_CloseAudio();
    SDL_Quit();
}

int init_audio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) return 1;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 1;

    Mix_AllocateChannels(MAX_AUDIO_CHANNEL);

    load_track("assets/audio/footstep1.wav", AUDIO_FOOTSTEP1);
    load_track("assets/audio/footstep2.wav", AUDIO_FOOTSTEP2);

    return 0;
}