#define USE_MOTION_SENSOR (false)
#define USE_ACCUMULATOR (false)

#define AUDIO_AMPLITUDE(LUMINANCE, FREQUENCY) \
  (400.0 * LUMINANCE * LUMINANCE / FREQUENCY)
