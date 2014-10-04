#include <stddef.h>
#include <stdlib.h>
#include "app.h"

struct Highlighter {
  int n_refs;
  double time;
};

static float get_pixel(Image* image, int y, int x, int c) {
  int x_stride = image->n_channels;
  int y_stride = image->width * image->n_channels;
  return image->data[y * y_stride + x * x_stride + c];
}

static void set_pixel(Image* image, int y, int x, int c, float value) {
  int x_stride = image->n_channels;
  int y_stride = image->width * image->n_channels;
  image->data[y * y_stride + x * x_stride + c] = value;
}

Highlighter *highlighter_create() {
  Highlighter *highlighter = (Highlighter*) malloc(sizeof(Highlighter));
  highlighter->n_refs = 1;
  highlighter->time = 0.0;
  return highlighter;
}

void highlighter_acquire(Highlighter *highlighter) {
  if (highlighter)
    highlighter->n_refs++;
}

void highlighter_release(Highlighter *highlighter) {
  if (highlighter)
    highlighter->n_refs--;
  if (highlighter && !highlighter->n_refs)
    free(highlighter);
}

void highlighter_transform(Highlighter *highlighter, Image **frame) {
  int height = (*frame)->height, width = (*frame)->width;
  int x0 = (int) (width * SCAN_FREQUENCY * highlighter->time);
  float r = ((float) ((HIGHLIGHT_COLOR & 0xff0000) >> 16)) / 255.0f;
  float g = ((float) ((HIGHLIGHT_COLOR & 0x00ff00) >> 8)) / 255.0f;
  float b = ((float) ((HIGHLIGHT_COLOR & 0x0000ff) >> 0)) / 255.0f;
  Image *result = image_create(height, width, 3);

  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
      for (int c = 0; c < 3; ++c)
	set_pixel(result, y, x, c, get_pixel(*frame, y, x, 0));

  for (int y = 0; y < height; ++y)
    for (int x = x0; x < x0 + HIGHLIGHT_WIDTH; ++x) {
      float pixel = get_pixel(*frame, y, x % width, 0);
      set_pixel(result, y, x % width, 0, r * pixel);
      set_pixel(result, y, x % width, 1, g * pixel);
      set_pixel(result, y, x % width, 2, b * pixel);
    }

  image_release(*frame);
  *frame = result;
}

void highlighter_set_time(Highlighter *highlighter, double time) {
  highlighter->time = time;
}
