#include <stddef.h>
#include <stdlib.h>
#include "app.h"

struct Resampler {
  int n_refs;
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

static float get_average(Image *image, int y0, int x0, int y1, int x1) {
  float sum = 0.0f;

  for (int y = y0; y < y1; ++y)
    for (int x = x0; x < x1; ++x)
      for (int c = 0; c < image->n_channels; ++c)
	sum += get_pixel(image, y, x, c);

  return sum / ((y1 - y0) * (x1 - x0) * image->n_channels);
}

Resampler *resampler_create() {
  Resampler *resampler = (Resampler*) malloc(sizeof(Resampler));
  resampler->n_refs = 1;
  return resampler;
}

void resampler_acquire(Resampler *resampler) {
  if (resampler)
    resampler->n_refs++;
}

void resampler_release(Resampler *resampler) {
  if (resampler)
    resampler->n_refs--;
  if (resampler && !resampler->n_refs)
    free(resampler);
}

void resampler_transform(Resampler *resampler, Image **frame) {
  Image *result = image_create(FRAME_HEIGHT, FRAME_WIDTH, 1);
  int y_zoom = (*frame)->height / result->height;
  int x_zoom = (*frame)->width / result->width;
 
  for (int y = 0; y < result->height; ++y)
    for (int x = 0; x < result->width; ++x) {
      int iy0 = y_zoom * y, iy1 = y_zoom * (y + 1);
      int ix0 = x_zoom * x, ix1 = x_zoom * (x + 1);
      float pixel = get_average(*frame, iy0, ix0, iy1, ix1);
      set_pixel(result, y, x, 0, pixel);
    }

  image_release(*frame);
  *frame = result;
}
