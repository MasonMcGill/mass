#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "app.h"

Image *image_create(int height, int width, int n_channels) {
  Image *image = (Image*) malloc(sizeof(Image));
  image->n_refs = 1;
  image->height = height;
  image->width = width;
  image->n_channels = n_channels;
  image->data = (float*) malloc(height * width * n_channels * sizeof(float));
  return image;
}

void image_acquire(Image *image) {
  if (image)
    image->n_refs++;
}

void image_release(Image *image) {
  if (image)
    image->n_refs--;
  if (image && !image->n_refs) {
    free(image->data);
    free(image);
  }
}

int image_get_height(Image *image) {
  return image->height;
}

int image_get_width(Image *image) {
  return image->width;
}

int image_get_n_channels(Image *image) {
  return image->n_channels;
}

float *image_get_data(Image *image) {
  return image->data;
}
