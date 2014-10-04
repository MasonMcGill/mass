#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <glib.h>
#include "app.h"

static VideoSource *video_source;
static VideoSink *video_sink;
static AudioSink *audio_sink;
static Resampler *resampler;
static MotionSensor *motion_sensor;
static Accumulator *accumulator;
static Highlighter *highlighter;
static Sonifier *sonifier;
static double current_time;
static bool running;

static void *stream_audio(void *app_data) {
  while (running) {
    Sound *clip = sonifier_read(sonifier);
    audio_sink_write(audio_sink, clip);
    current_time += (double) clip->n_samples / AUDIO_SAMPLE_RATE;
    sound_release(clip);
  }
  return NULL;
}

static void *stream_video(void *app_data) {
  while (running) {
    Image *frame = video_source_read(video_source);
    resampler_transform(resampler, &frame);
    if (USE_MOTION_SENSOR) motion_sensor_transform(motion_sensor, &frame);
    if (USE_ACCUMULATOR) accumulator_set_time(accumulator, current_time);
    if (USE_ACCUMULATOR) accumulator_transform(accumulator, &frame);
    sonifier_set_frame(sonifier, frame);
    if (USE_HIGHLIGHTER) highlighter_set_time(highlighter, current_time);
    if (USE_HIGHLIGHTER) highlighter_transform(highlighter, &frame);
    video_sink_write(video_sink, frame);
    image_release(frame);
  }
  return NULL;
}

int main() {
  video_source = video_source_create();
  video_sink = video_sink_create();
  audio_sink = audio_sink_create();
  resampler = resampler_create();
  motion_sensor = motion_sensor_create();
  accumulator = accumulator_create();
  highlighter = highlighter_create();
  sonifier = sonifier_create();
  current_time = 0.0;
  running = true;

  if (error_catch() == ERROR_NO_CAMERA) {
    printf("No camera is available.");
    goto exit;
  }

  GThread *audio_thread = g_thread_new("", stream_audio, NULL);
  GThread *video_thread = g_thread_new("", stream_video, NULL);
  getc(stdin); running = false;
  g_thread_join(video_thread);
  g_thread_join(audio_thread);

 exit:
  video_source_release(video_source);
  video_sink_release(video_sink);
  audio_sink_release(audio_sink);
  resampler_release(resampler);
  motion_sensor_release(motion_sensor);
  accumulator_release(accumulator);
  highlighter_release(highlighter);
  sonifier_release(sonifier);
  return 0;
}
