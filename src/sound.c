#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "app.h"

Sound *sound_create(int n_samples, int n_channels) {
  Sound *sound = (Sound*) malloc(sizeof(Sound));
  sound->n_refs = 1;
  sound->n_samples = n_samples;
  sound->n_channels = n_channels;
  sound->data = (float*) malloc(n_samples * n_channels * sizeof(float));
  return sound;
}

void sound_acquire(Sound *sound) {
  if (sound)
    sound->n_refs++;
}

void sound_release(Sound *sound) {
  if (sound)
    sound->n_refs--;
  if (sound && !sound->n_refs) {
    free(sound->data);
    free(sound);
  }
}

int sound_get_n_samples(Sound *sound) {
  return sound->n_samples;
}

int sound_get_n_channels(Sound *sound) {
  return sound->n_channels;
}

float *sound_get_data(Sound *sound) {
  return sound->data;
}
