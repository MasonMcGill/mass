#include <stddef.h>
#include <stdlib.h>
#include <glib.h>
#include "app.h"

struct Sonifier {
  int n_refs;
  double t, t0, t1;
  float *pixels_t0, *pixels_t1;
  Image *frame;
  GMutex mutex;
};

static double const PI = 3.141592;

static float get_pixel(Image* image, int y, int x, int c) {
  int x_stride = image->n_channels;
  int y_stride = image->width * image->n_channels;
  return image->data[y * y_stride + x * x_stride + c];
}

static void set_sample(Sound *sound, int sample, int channel, double value) {
  sound->data[sample * sound->n_channels + channel] = (float) value;
}

static double interpolate(double a, double b, double mix) {
  return (1 - mix) * a + mix * b;
}

static int round_by(double a, int b) {
  return b * ((int) a / b);
}

static void load_next_column(Sonifier *sonifier) {
  g_mutex_lock(&sonifier->mutex);
  Image *frame = sonifier->frame;
  float *pixels_t0 = sonifier->pixels_t1;
  float *pixels_t1 = sonifier->pixels_t0;
  int x = (int) (fmod(SCAN_FREQUENCY * sonifier->t1, 1.0) * frame->width);
 
  for (int y = 0; y < frame->height; ++y)
    pixels_t1[y] = get_pixel(frame, y, x, 0);

  sonifier->pixels_t0 = pixels_t0;
  sonifier->pixels_t1 = pixels_t1;
  g_mutex_unlock(&sonifier->mutex);
}

Sonifier *sonifier_create() {
  Sonifier *sonifier = (Sonifier*) malloc(sizeof(Sonifier));
  sonifier->n_refs = 1;
  sonifier->t = 0.0;
  sonifier->t0 = 0.0;
  sonifier->t1 = 0.0;
  sonifier->pixels_t0 = NULL;
  sonifier->pixels_t1 = NULL;
  sonifier->frame = NULL;
  g_mutex_init(&sonifier->mutex);
  return sonifier;
}

void sonifier_acquire(Sonifier *sonifier) {
  if (sonifier)
    sonifier->n_refs++;
}

void sonifier_release(Sonifier *sonifier) {
  if (sonifier)
    sonifier->n_refs--;
  if (sonifier && !sonifier->n_refs) {
    free(sonifier->pixels_t0);
    free(sonifier->pixels_t1);
    image_release(sonifier->frame);
    g_mutex_clear(&sonifier->mutex);
    free(sonifier);
  }
}

Sound *sonifier_read(Sonifier *sonifier) {
  if (!sonifier->frame)
    return sound_create(0, 2);

  g_mutex_lock(&sonifier->mutex);
  int height = sonifier->frame->height;
  int width = sonifier->frame->width;
  g_mutex_unlock(&sonifier->mutex);

  int n_samples = round_by(AUDIO_LATENCY * AUDIO_SAMPLE_RATE, 8);
  Sound *sound = sound_create(n_samples, 2);

  for (int i = 0; i < n_samples; ++i) {
    float *pixels_t0 = sonifier->pixels_t0;
    float *pixels_t1 = sonifier->pixels_t1;
    double mix = (sonifier->t - sonifier->t0) / (sonifier->t1 - sonifier->t0);
    double sample = 0.0;

    for (int y = 0; y < height; ++y) {
      double luminance = interpolate(pixels_t0[y], pixels_t1[y], mix);
      double frequency = AUDIO_FREQUENCY((double) y / height);
      double amplitude = AUDIO_AMPLITUDE(luminance, frequency);
      sample += amplitude * sin(frequency * PI * sonifier->t) / height;
    }

    double pan = fmod(SCAN_FREQUENCY * sonifier->t, 1.0);
    double clipped_sample = fmax(-1.0, fmin(1.0, sample));
    set_sample(sound, i, 0, (1 - pan) * clipped_sample);
    set_sample(sound, i, 1, pan * clipped_sample);
    sonifier->t += 1.0 / AUDIO_SAMPLE_RATE;

    if (sonifier->t > sonifier->t1) {
      sonifier->t0 += 1.0 / SCAN_FREQUENCY / width;
      sonifier->t1 += 1.0 / SCAN_FREQUENCY / width;
      load_next_column(sonifier);
    }
  }

  return sound;
}

void sonifier_set_frame(Sonifier *sonifier, Image *frame) {
  int height = frame->height;
  int width = frame->width;
  g_mutex_lock(&sonifier->mutex);
  image_release(sonifier->frame);
  sonifier->frame = frame;
  image_acquire(sonifier->frame);
  g_mutex_unlock(&sonifier->mutex);
  
  if (!sonifier->pixels_t0) {
    sonifier->t1 = 1.0 / SCAN_FREQUENCY / frame->width;
    sonifier->pixels_t0 = (float*) malloc(frame->height * sizeof(float));
    sonifier->pixels_t1 = (float*) malloc(frame->height * sizeof(float));
    load_next_column(sonifier);
    load_next_column(sonifier);
  }
}
