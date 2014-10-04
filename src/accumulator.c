#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "app.h"

struct Accumulator {
  int n_refs;
  double time;
  Image *frame;
};

static void set_pixel(Image* image, int y, int x, int c, float value) {
  int x_stride = image->n_channels;
  int y_stride = image->width * image->n_channels;
  image->data[y * y_stride + x * x_stride + c] = value;
}

Accumulator *accumulator_create() {
  int n_bytes = sizeof(Accumulator);
  Accumulator *accumulator = (Accumulator*) malloc(n_bytes);
  accumulator->n_refs = 1;
  accumulator->time = 0.0;
  accumulator->frame = NULL;
  return accumulator;
}

void accumulator_acquire(Accumulator *accumulator) {
  if (accumulator)
    accumulator->n_refs++;
}

void accumulator_release(Accumulator *accumulator) {
  if (accumulator)
    accumulator->n_refs--;
  if (accumulator && !accumulator->n_refs) {
    image_release(accumulator->frame);
    free(accumulator);
  }
}

void accumulator_transform(Accumulator *accumulator, Image **frame) {
  int height = (*frame)->height;
  int width = (*frame)->width;
  int n_channels = (*frame)->n_channels;
  int n_elements = height * width * n_channels;

  if (!accumulator->frame) {
    accumulator->frame = image_create(height, width, n_channels);

    for (int i = 0; i < n_elements; ++i)
      accumulator->frame->data[i] = 0.0f;
  }

  float *outputs = accumulator->frame->data;
  float *inputs = (*frame)->data;

  for (int i = 0; i < n_elements; ++i) {
    float input = inputs[i] >= ACCUMULATOR_THRESHOLD ? inputs[i] : 0.0f;
    outputs[i] = fmin(1.0, ACCUMULATOR_DECAY * outputs[i] + input);
  }

  image_release(*frame);
  image_acquire(accumulator->frame);
  *frame = accumulator->frame;
}

void accumulator_set_time(Accumulator *accumulator, double time) {
  if (accumulator->frame) {
    Image *frame = accumulator->frame;
    int x0 = (int) (frame->width * SCAN_FREQUENCY * accumulator->time);
    int x1 = (int) (frame->width * SCAN_FREQUENCY * time);
    
    for (int y = 0; y < frame->height; ++y)
      for (int x = x0; x < x1; ++x)
	set_pixel(frame, y, x % frame->width, 0, 0.0f);
  }

  accumulator->time = time;
}
