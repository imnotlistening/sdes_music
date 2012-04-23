/*
 * (C) Copyright 2012
 * Alex Waterman <imNotListening@gmail.com>
 *
 * MUSIC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MUSIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MUSIC.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build a pipeline capable of streaming music of a web server.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

#ifndef _AUDIO_SINK
#  define _AUDIO_SINK  "autoaudiosink"
#endif

static gboolean bus_callb(GstBus *bus, GstMessage *msg, gpointer data);
static void     new_pad_added(GstElement *elem, GstPad *pad, gpointer data);

GMainLoop	*music_mloop;

/*
 * Build a GstRtpBin pipeline. This will attempt to stream on the passed ports.
 */
int music_make_pipeline(struct music_rtp_pipeline *pipe, char *id){

	GstBus *bus;
	GstElement *pipeline;
	GstElement *uridecoder, *audioconv, *volume, *audiosink;

	/* Initialize the pipeline. */
	pipeline      = gst_pipeline_new(id);
	uridecoder    = gst_element_factory_make("uridecodebin", "Decoder");
	audioconv     = gst_element_factory_make("audioconvert", "Converter");
	volume        = gst_element_factory_make("volume", "Volume Control");
	audiosink     = gst_element_factory_make(_AUDIO_SINK, "Audio Sink");

	ASSERT_OR_ERROR(pipeline != NULL);
	ASSERT_OR_ERROR(uridecoder != NULL);
	ASSERT_OR_ERROR(audioconv != NULL);
	ASSERT_OR_ERROR(volume != NULL);
	ASSERT_OR_ERROR(audiosink != NULL);

	/* Set a sane default volume. */
	g_object_set(G_OBJECT(volume), "volume", .2, NULL);
	
	/* Add the elements to the pipeline and link what can be linked. */
	gst_bin_add(GST_BIN(pipeline), uridecoder);
	gst_bin_add(GST_BIN(pipeline), audioconv);
	gst_bin_add(GST_BIN(pipeline), volume);
	gst_bin_add(GST_BIN(pipeline), audiosink);
	gst_element_link_many(audioconv, volume, audiosink, NULL);

	/* Dynamic pad creation for decoder. */
	g_signal_connect(uridecoder, "pad-added", G_CALLBACK(new_pad_added),
			 audioconv);

	pipe->pipeline = pipeline;
	pipe->source = uridecoder;
	pipe->volume = volume;
	pipe->mloop = music_mloop;

	/* Add the bus handler here so that each pipeline created will have
	 * a handler. */
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipe->pipeline));
	gst_bus_add_watch(bus, bus_callb, pipe);
	g_object_unref(bus);

	return 0;

}

/*
 * Build a main loop to run the RTP server in. Part of the initilazation
 * process.
 */
int music_make_mloop(){

	music_mloop = g_main_loop_new(NULL, FALSE);
	return 0;

}

/*
 * Handle changes in state on the bus.
 */
static gboolean bus_callb(GstBus *bus, GstMessage *msg, gpointer data){

	gchar *debug;
	GError *error;
	struct music_rtp_pipeline *pipe = data;

	switch (GST_MESSAGE_TYPE(msg)){

	/*
	 * End of stream.
	 */
	case GST_MESSAGE_EOS:
		if ( pipe->end_of_stream )
			pipe->end_of_stream(pipe);
		break;

	/*
	 * Handle an error on the stream.
	 */
	case GST_MESSAGE_ERROR:
		gst_message_parse_error(msg, &error, &debug);
		g_free(debug);
		g_printerr("Error detected: %s\n", error->message);
		break;

	default:
		break;

	}

	/*
	 * Returning true keeps the call back installed.
	 */
	return TRUE;

}

static void new_pad_added(GstElement *elem, GstPad *pad, gpointer data){

	GstPad *sinkpad;
	GstElement *decoder = (GstElement *) data;

	/*
	 * Sink pad in the volume controller.
	 */
	sinkpad = gst_element_get_static_pad (decoder, "sink");
	gst_pad_link (pad, sinkpad);
	gst_object_unref (sinkpad);

}
