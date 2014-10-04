#include <assert.h>
#include <stdlib.h>
#include <gst/gst.h>
#include "app.h"

struct VideoSource {
  int n_refs;
  float rate;
  int height, width;
  GstElement *pipe, *sink;
};

VideoSource *video_source_create() {
  gst_init(NULL, NULL);

  GstElement *pipe = gst_pipeline_new("pipe");
  GstElement *gst_source = gst_element_factory_make("autovideosrc", NULL);
  GstElement *adapter = gst_element_factory_make("videoconvert", NULL);
  GstElement *sink = gst_element_factory_make("appsink", NULL);
  GstCaps *min_caps = gst_caps_new_simple("video/x-raw", "format",
					  G_TYPE_STRING, "RGB", NULL);

  g_object_set(sink, "caps", min_caps, NULL);
  g_object_set(sink, "max-buffers", 1, NULL);
  g_object_set(sink, "drop", true, NULL);
  gst_bin_add_many(GST_BIN(pipe), gst_source, adapter, sink, NULL);
  gst_element_link_many(gst_source, adapter, sink, NULL);
  gst_element_set_state(pipe, GST_STATE_PLAYING);
  gst_element_get_state(pipe, NULL, NULL, GST_CLOCK_TIME_NONE);

  GstPad *sink_pad = gst_element_get_static_pad(sink, "sink");
  GstCaps *used_caps = gst_pad_get_current_caps(sink_pad);
  bool available_camera = gst_caps_get_size(used_caps);
  gst_caps_unref(min_caps);
  gst_caps_unref(used_caps);
  gst_object_unref(sink_pad);

  if (!available_camera) {
    error_throw(ERROR_NO_CAMERA);
    return NULL;
  }
  else {
    int height, width, rate_n, rate_d;
    GstStructure *cap_struct = gst_caps_get_structure(used_caps, 0);
    gst_structure_get_int(cap_struct, "height", &height);
    gst_structure_get_int(cap_struct, "width", &width);
    gst_structure_get_fraction(cap_struct, "framerate", &rate_n, &rate_d);
    gst_mini_object_unref(GST_MINI_OBJECT(cap_struct));

    int n_bytes = sizeof(VideoSource);
    VideoSource *source = (VideoSource*) malloc(n_bytes);
    source->n_refs = 1;
    source->rate = ((float) rate_n) / rate_d;
    source->height = height;
    source->width = width;
    source->pipe = pipe;
    source->sink = sink;
    return source;
  }
  return NULL;
}

void video_source_acquire(VideoSource *source) {
  if (source)
    source->n_refs++;
}

void video_source_release(VideoSource *source) {
  if (source)
    source->n_refs--;
  if (source && !source->n_refs) {
    video_source_close(source);
    free(source);
  }
}

void video_source_close(VideoSource *source) {
  if (source->pipe) {
    gst_element_set_state(source->pipe, GST_STATE_NULL);
    gst_object_unref(source->pipe);
    source->pipe = NULL;
  }
}

bool video_source_is_done(VideoSource *source) {
  return source->pipe == NULL;
}

Image *video_source_read(VideoSource *source) {
  assert(!video_source_is_done(source));

  GstMapInfo map_info;
  GstBuffer* buffer;
  GstSample* sample;
  g_signal_emit_by_name(source->sink, "pull-sample", &sample, NULL);
  buffer = gst_sample_get_buffer(sample);
  gst_buffer_map(buffer, &map_info, GST_MAP_READ);

  int n_elements = source->height * source->width * 3;
  Image *frame = image_create(source->height, source->width, 3);
  float *data = image_get_data(frame);

  for (int i = 0; i < n_elements; ++i)
    data[i] = 1.0f/255.0f * (float) map_info.data[i];

  gst_buffer_unmap(buffer, &map_info);
  gst_sample_unref(sample);
  return frame;
}
