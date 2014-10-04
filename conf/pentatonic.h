static double pentatonic_frequency(int y) {
  static double frequencies[] = {69.2957, 77.7817, 92.4986, 103.826, 116.541};
  int pitch = y % 5;
  int octave = y / 5;
  return frequencies[pitch] * (2 << octave);
}

#define AUDIO_FREQUENCY(POSITION) \
  pentatonic_frequency((int) (FRAME_HEIGHT * (1.0 - POSITION)))
