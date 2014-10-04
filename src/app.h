#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <math.h>

#define QUOTE(X) #X
#define INCLUDE_PATH(X) QUOTE(X.h)

#ifdef CONFIG
  #include INCLUDE_PATH(CONFIG)
#endif

#undef QUOTE
#undef INCLUDE_PATH

#ifndef FRAME_HEIGHT
  #define FRAME_HEIGHT (36)
#endif
#ifndef FRAME_WIDTH
  #define FRAME_WIDTH (64)
#endif

#ifndef SCAN_FREQUENCY
  #define SCAN_FREQUENCY (1.0)
#endif

#ifndef USE_MOTION_SENSOR
  #define USE_MOTION_SENSOR (true)
#endif
#ifndef USE_ACCUMULATOR
  #define USE_ACCUMULATOR (true)
#endif
#ifndef ACCUMULATOR_THRESHOLD
  #define ACCUMULATOR_THRESHOLD (0.01)
#endif
#ifndef ACCUMULATOR_DECAY
  #define ACCUMULATOR_DECAY (0.95)
#endif

#ifndef USE_HIGHLIGHTER
  #define USE_HIGHLIGHTER (true)
#endif
#ifndef HIGHLIGHT_WIDTH
  #define HIGHLIGHT_WIDTH (4)
#endif
#ifndef HIGHLIGHT_COLOR
  #define HIGHLIGHT_COLOR (0x00bbff)
#endif
#ifndef AUDIO_SAMPLE_RATE
  #define AUDIO_SAMPLE_RATE (44100)
#endif
#ifndef AUDIO_LATENCY
  #define AUDIO_LATENCY (0.005)
#endif

#ifndef AUDIO_FREQUENCY
  #define AUDIO_FREQUENCY(POSITION) \
    exp(POSITION * log(160.0) + (1.0 - POSITION) * log(10240.0))
#endif
#ifndef AUDIO_AMPLITUDE
  #define AUDIO_AMPLITUDE(LUMINANCE, FREQUENCY) \
    4000.0 * LUMINANCE * LUMINANCE / FREQUENCY
#endif

typedef enum {ERROR_NONE, ERROR_NO_CAMERA} Error;
void error_throw(Error error);
Error error_catch();
char const *error_get_message(Error error);

typedef struct {int n_refs; int height, width, n_channels; float *data;} Image;
Image *image_create(int width, int height, int n_channels);
void image_acquire(Image *image);
void image_release(Image *image);
int image_get_height(Image *image);
int image_get_width(Image *image);
int image_get_n_channels(Image *image);
float *image_get_data(Image *image);

typedef struct {int n_refs; int n_samples, n_channels; float *data;} Sound;
Sound *sound_create(int n_samples, int n_channels);
void sound_acquire(Sound *sound);
void sound_release(Sound *sound);
int sound_get_n_samples(Sound *sound);
int sound_get_n_channels(Sound *sound);
float *sound_get_data(Sound *sound);

typedef struct VideoSource VideoSource;
VideoSource *video_source_create();
void video_source_acquire(VideoSource *source);
void video_source_release(VideoSource *source);
void video_source_close(VideoSource *source);
bool video_source_is_done(VideoSource *source);
Image *video_source_read(VideoSource *source);

typedef struct VideoSink VideoSink;
VideoSink *video_sink_create();
void video_sink_acquire(VideoSink *sink);
void video_sink_release(VideoSink *sink);
void video_sink_close(VideoSink *sink);
void video_sink_write(VideoSink *sink, Image *frame);

typedef struct AudioSink AudioSink;
AudioSink *audio_sink_create();
void audio_sink_acquire(AudioSink *sink);
void audio_sink_release(AudioSink *sink);
void audio_sink_close(AudioSink *sink);
void audio_sink_write(AudioSink *sink, Sound *clip);

typedef struct Resampler Resampler;
Resampler *resampler_create();
void resampler_acquire(Resampler *resampler);
void resampler_release(Resampler *resampler);
void resampler_transform(Resampler *resampler, Image **frame);

typedef struct MotionSensor MotionSensor;
MotionSensor *motion_sensor_create();
void motion_sensor_acquire(MotionSensor *sensor);
void motion_sensor_release(MotionSensor *sensor);
void motion_sensor_transform(MotionSensor *sensor, Image **frame);

typedef struct Accumulator Accumulator;
Accumulator *accumulator_create();
void accumulator_acquire(Accumulator *accumulator);
void accumulator_release(Accumulator *accumulator);
void accumulator_transform(Accumulator *accumulator, Image **frame);
void accumulator_set_time(Accumulator *accumulator, double time);

typedef struct Highlighter Highlighter;
Highlighter *highlighter_create();
void highlighter_acquire(Highlighter *highlighter);
void highlighter_release(Highlighter *highlighter);
void highlighter_transform(Highlighter *highlighter, Image **frame);
void highlighter_set_time(Highlighter *highlighter, double time);

typedef struct Sonifier Sonifier;
Sonifier *sonifier_create();
void sonifier_acquire(Sonifier *sonifier);
void sonifier_release(Sonifier *sonifier);
Sound *sonifier_read(Sonifier *sonifier);
void sonifier_set_frame(Sonifier *sonifier, Image *frame);

#endif
