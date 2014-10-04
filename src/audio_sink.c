#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>
#include "app.h"

typedef enum {STATE_UNOPENED, STATE_OPEN, STATE_CLOSED} State;

struct AudioSink {
  int n_refs;
  GstElement *pipe, *source;
  State state;
  int n_channels;
};

static void open(AudioSink *sink) {
  unsigned int endianness_test_int = 1;
  bool big_endian = ((char*) &endianness_test_int)[0] == 0;
  char const *format = big_endian ? "F32BE" : "F32LE";
  char const *layout = "interleaved";

  GstElement *pipe = gst_pipeline_new("pipe");
  GstElement *source = gst_element_factory_make("appsrc", NULL);
  GstElement *resampler = gst_element_factory_make("audioresample", NULL);
  GstElement *adapter = gst_element_factory_make("audioconvert", NULL);
  GstElement *gst_sink = gst_element_factory_make("autoaudiosink", NULL);
  GstCaps *caps = gst_caps_new_simple("audio/x-raw",
				      "rate", G_TYPE_INT, AUDIO_SAMPLE_RATE,
				      "channels", G_TYPE_INT, sink->n_channels,
				      "format", G_TYPE_STRING, format,
				      "layout", G_TYPE_STRING, layout,
				      NULL);

  g_object_set(source, "caps", caps, NULL);
  g_object_set(source, "block", true, NULL);
  g_object_set(source, "max-bytes", 1, NULL);
  g_object_set(source, "format", GST_FORMAT_TIME, NULL);
  gst_bin_add_many(GST_BIN(pipe), source, resampler, adapter, gst_sink, NULL);
  gst_element_link_many(source, resampler, adapter, gst_sink, NULL);
  gst_element_set_state(pipe, GST_STATE_PLAYING);
  gst_caps_unref(caps);

  sink->source = source;
  sink->pipe = pipe;
}

AudioSink *audio_sink_create() {
  gst_init(NULL, NULL);
  int n_bytes = sizeof(AudioSink);
  AudioSink *sink = (AudioSink*) malloc(n_bytes);
  sink->n_refs = 1;
  sink->state = STATE_UNOPENED;
  sink->source = NULL;
  sink->pipe = NULL;
  sink->n_channels = 0;
  return sink;
 }

void audio_sink_acquire(AudioSink *sink) {
  if (sink)
    sink->n_refs++;
}

void audio_sink_release(AudioSink *sink) {
  if (sink)
    sink->n_refs--;
  if (sink && !sink->n_refs) {
    audio_sink_close(sink);
    free(sink);
  }
}

void audio_sink_close(AudioSink *sink) {
  if (sink->state == STATE_OPEN) {
    sink->state = STATE_CLOSED;
    gst_element_set_state(sink->pipe, GST_STATE_NULL);
    gst_object_unref(sink->pipe);
  }
}

void audio_sink_write(AudioSink *sink, Sound *clip) {
  int n_samples = sound_get_n_samples(clip);
  int n_channels = sound_get_n_channels(clip);

  if (sink->state == STATE_UNOPENED) {
    assert(n_channels == 1 || n_channels == 2);
    sink->n_channels = n_channels;
    sink->state = STATE_OPEN;
    open(sink);
  }
  else {
    assert(sink->state == STATE_OPEN);
    assert(n_channels == sink->n_channels);
  }

  int n_bytes = n_samples * n_channels * sizeof(float);
  float *i_data = sound_get_data(clip);
  float *o_data = (float*) malloc(n_bytes);
  memcpy(o_data, i_data, n_bytes);

  GstBuffer *buffer;
  GstFlowReturn flow_result;
  buffer = gst_buffer_new_wrapped(o_data, n_bytes);
  g_signal_emit_by_name(sink->source, "push-buffer", buffer, &flow_result);
  gst_buffer_unref(buffer);
}
