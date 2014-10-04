#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <gst/gst.h>
#include "app.h"

typedef enum {STATE_UNOPENED, STATE_OPEN, STATE_CLOSED} State;

struct VideoSink {
  int n_refs;
  GstElement *pipe, *source;
  int rate, height, width, n_channels;
  State state;
};

static void open(VideoSink *sink) {
  char const *formats[] = {"GRAY8", "", "RGB", "RGBA"};
  char const *format = formats[sink->n_channels - 1];
  GstElement *pipe = gst_pipeline_new("pipe");
  GstElement *source = gst_element_factory_make("appsrc", NULL);
  GstElement *adapter = gst_element_factory_make("videoconvert", NULL);
  GstElement *gst_sink = gst_element_factory_make("autovideosink", NULL);
  GstCaps *caps = gst_caps_new_simple(
    "video/x-raw",
    "width", G_TYPE_INT, sink->width,
    "height", G_TYPE_INT, sink->height,
    "framerate", GST_TYPE_FRACTION, sink->rate, 1000,
    "format", G_TYPE_STRING, format, NULL
  );

  g_object_set(source, "caps", caps, NULL);
  g_object_set(source, "block", true, NULL);
  g_object_set(source, "max-bytes", 1, NULL);
  g_object_set(source, "format", GST_FORMAT_TIME, NULL);
  gst_bin_add_many(GST_BIN(pipe), source, adapter, gst_sink, NULL);
  gst_element_link_many(source, adapter, gst_sink, NULL);
  gst_element_set_state(pipe, GST_STATE_PLAYING);
  gst_caps_unref(caps);

  sink->source = source;
  sink->pipe = pipe;
}

VideoSink *video_sink_create() {
  gst_init(NULL, NULL);
  int n_bytes = sizeof(VideoSink);
  VideoSink *sink = (VideoSink*) malloc(n_bytes);
  sink->n_refs = 1;
  sink->rate = 25;
  sink->state = STATE_UNOPENED;
  sink->source = NULL;
  sink->pipe = NULL;
  return sink;
}

void video_sink_acquire(VideoSink *sink) {
  if (sink)
    sink->n_refs++;
}

void video_sink_release(VideoSink *sink) {
  if (sink)
    sink->n_refs--;
  if (sink && !sink->n_refs) {
    video_sink_close(sink);
    free(sink);
  }
}

void video_sink_close(VideoSink *sink) {
  if (sink->state == STATE_OPEN) {
    sink->state = STATE_CLOSED;
    gst_element_set_state(sink->pipe, GST_STATE_NULL);
    gst_object_unref(sink->pipe);
  }
}

void video_sink_write(VideoSink *sink, Image *frame) {
  int height = image_get_height(frame);
  int width = image_get_width(frame);
  int n_chans = image_get_n_channels(frame);

  if (sink->state == STATE_UNOPENED) {
    assert(n_chans == 1 || n_chans == 3 || n_chans == 4);
    sink->state = STATE_OPEN;
    sink->height = height;
    sink->width = width;
    sink->n_channels = n_chans;
    open(sink);
  }
  else {
    assert(sink->state == STATE_OPEN);
    assert(height == sink->height);
    assert(width == sink->width);
    assert(n_chans == sink->n_channels);
  }

  typedef unsigned char uchar;
  int n_elements = height * width * n_chans;
  float *i_data = image_get_data(frame);
  uchar *o_data = (uchar*) malloc(width * height * n_chans);

  for (int i = 0; i < n_elements; ++i)
    o_data[i] = (uchar) lrint(255.0f * i_data[i]);

  GstBuffer* buffer;
  GstFlowReturn flow_result;
  buffer = gst_buffer_new_wrapped(o_data, height * width * n_chans);
  g_signal_emit_by_name(sink->source, "push-buffer", buffer, &flow_result);
  gst_buffer_unref(buffer);
}
