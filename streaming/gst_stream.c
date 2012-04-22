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
 * Build an RTP stream from a URL (local, youtube vid, etc) and allow incoming
 * clients to react to the stream.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

#define ENCODER		"flacenc"
#ifdef _MUSIC_USE_TCP
#  define NET_SINK	"tcpclientsink"
#else
#  define NET_SINK	"udpsink"
#endif

static gboolean bus_callb(GstBus *bus, GstMessage *msg, gpointer data);
static void     new_pad_added(GstElement *elem, GstPad *pad, gpointer data);

GMainLoop	*music_mloop;

/*
 * Build a GstRtpBin pipeline. This will attempt to stream on the passed ports.
 */
int music_make_pipeline(struct music_rtp_pipeline *pipe,
			char *id, int port, char *dest_host){

	GstBus *bus;
	GstElement *pipeline;
	GstElement *source, *decodebin;
	GstElement *volume, *convert, *resample, *encoder, *netsink;

	/* Initialize the pipeline. */
	pipeline      = gst_pipeline_new("Stream server");
	source        = gst_element_factory_make("filesrc", "file source");
	decodebin     = gst_element_factory_make("decodebin2", "decode");
	convert       = gst_element_factory_make("audioconvert", "converter");
	volume        = gst_element_factory_make("volume", "vol-cntl");
	resample      = gst_element_factory_make("audioresample", "resampler");
	encoder       = gst_element_factory_make(ENCODER, "stream encoder");
	netsink       = gst_element_factory_make(NET_SINK, "Net Sink");

	ASSERT_OR_ERROR(pipeline != NULL);
	ASSERT_OR_ERROR(source != NULL);
	ASSERT_OR_ERROR(decodebin != NULL);
	ASSERT_OR_ERROR(convert != NULL);
	ASSERT_OR_ERROR(volume != NULL);
	ASSERT_OR_ERROR(resample != NULL);
	ASSERT_OR_ERROR(encoder != NULL);
	ASSERT_OR_ERROR(netsink != NULL);

	/* Set a sane default volume. */
	g_object_set(G_OBJECT(volume), "volume", 1.0, NULL);
	
	/* Set the port. */
	g_object_set(G_OBJECT(netsink), "port", port,
		     "host", dest_host, NULL);
#ifdef _MUSIC_USE_TCP
	/* 
	 * Lets use some sort of protocol to help manage the stream. For now
	 * this has to be commented out since my instalation of gstreamer's
	 * base plugins is missing the TCP headers. Yay. Thankfully it shouldn't
	 * be necessary...
	 */
	/*
	g_object_set(G_OBJECT(netsink), "protocol", GST_TCP_PROTOCOL_GDP, NULL);
	*/
#endif

	/* Add the elements to the pipeline and link what can be linked. */
	gst_bin_add_many(GST_BIN(pipeline), source, decodebin, volume, convert,
			 encoder, resample, netsink, NULL);
	gst_element_link(source, decodebin);
	gst_element_link_many(convert, volume, resample, 
			      encoder, netsink, NULL);

	/* Dynamic pad creation for decoder. */
	g_signal_connect(decodebin, "pad-added", G_CALLBACK(new_pad_added),
			 convert);

	pipe->pipeline = pipeline;
	pipe->filesrc = source;
	pipe->volume = volume;

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
		g_print("End of stream detected\n");
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
