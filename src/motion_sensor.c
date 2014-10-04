#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "app.h"

struct MotionSensor {
  int n_refs;
  Image *last_frame;
};

MotionSensor *motion_sensor_create() {
  MotionSensor *sensor = (MotionSensor*) malloc(sizeof(MotionSensor));
  sensor->n_refs = 1;
  sensor->last_frame = NULL;
  return sensor;
}

void motion_sensor_acquire(MotionSensor *sensor) {
  if (sensor)
    sensor->n_refs++;
}

void motion_sensor_release(MotionSensor *sensor) {
  if (sensor)
    sensor->n_refs--;
  if (sensor && !sensor->n_refs) {
    image_release(sensor->last_frame);
    free(sensor);
  }
}

void motion_sensor_transform(MotionSensor *sensor, Image **frame) {
  int height = (*frame)->height;
  int width = (*frame)->width;
  int n_channels = (*frame)->n_channels;
  int n_elements = height * width * n_channels;
  Image *result = image_create(height, width, n_channels);

  if (sensor->last_frame)
    for (int i = 0; i < n_elements; ++i)
      result->data[i] = fabsf((*frame)->data[i] - sensor->last_frame->data[i]);
  else
    for (int i = 0; i < n_elements; ++i)
      result->data[i] = 0.0f;

  image_release(sensor->last_frame);
  sensor->last_frame = *frame;
  *frame = result;
}
