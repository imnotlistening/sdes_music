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
 * Streaming client. Plays a UDP stream.
 *
 * Usage:
 *   stream_client <udp-port>
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

#define AUDIO_DEC   "flacdec"
#define AUDIO_SINK  "alsasink"

#ifdef	_MUSIC_USE_TCP
#  define NET_SRC   "tcpserversrc"
#else
#  define NET_SRC   "udpsrc"
#endif

/*
 * Some functions for later use.
 */
static void on_pad_added(GstElement *depay, GstPad *pad, gpointer user_data);

/**
 * Main function, derr.
 */
int main(int argc, char **argv){

	GstElement *netsrc;
	GstElement *audiodec, *audioconv, *audiosink;
	GstElement *pipeline;
	GMainLoop *loop;
	gboolean res;
	int port, convs;

	/* Init the GST library. */
	gst_init(&argc, &argv);

	/* Only 1 argument, a source to play. */
	if ( argc != 2 ){
		printf("Usage: %s <port>\n", argv[0]);
		return 1;
	}

	convs = sscanf(argv[1], "%d", &port);
	ASSERT_OR_ERROR(convs == 1);
	ASSERT_OR_ERROR(port > 0 && port < 65536);
	printf("Receiving " NET_SRC " stream on port %d\n", port);
	
	/* The pipeline to hold everything */
	pipeline = gst_pipeline_new(NULL);
	g_assert(pipeline);

	netsrc = gst_element_factory_make(NET_SRC, "UDP source");
	audiodec = gst_element_factory_make(AUDIO_DEC, "audiodec");
	audioconv = gst_element_factory_make("audioconvert", "audioconv");
	audiosink = gst_element_factory_make(AUDIO_SINK, "audiosink");

	g_assert(netsrc);
	g_assert(audiodec);
	g_assert(audioconv);
	g_assert(audiosink);

	g_object_set(netsrc, "port", port, NULL);

#ifdef _MUSIC_USE_TCP
	g_object_set(netsrc, "host", "129.21.131.148", NULL);
	//g_object_set(netsrc, "protocol", 1, NULL);
#endif

	g_object_set(audiosink, "sync", FALSE, NULL);

	gst_bin_add_many(GST_BIN(pipeline), netsrc, audiodec, audioconv,
		    audiosink, NULL);

	res = gst_element_link_many(netsrc, audiodec, audioconv,
				    audiosink, NULL);
	g_assert(res == TRUE);

	/*
	 * Link the decodebin and the next pad in the chain.
	 */
	g_signal_connect(audiodec, "pad-added",
			 G_CALLBACK(on_pad_added), audioconv);

	/* set the pipeline to playing */
	g_print("Starting receiver pipeline\n");
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	/* we need to run a GLib main loop to get the messages */
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	g_print("Stopping receiver pipeline\n");
	gst_element_set_state(pipeline, GST_STATE_NULL);

	gst_object_unref(pipeline);

	return 0;

}

/*
 * Occurs when the decode in needs to be connected to the payloader.
 */
static void on_pad_added(GstElement *depay, GstPad *pad, gpointer user_data){

	GstPad *sinkpad;
	GstElement *decoder = (GstElement *)user_data;

	printf("Linking audio decoder to audio-resampler.\n");

	sinkpad = gst_element_get_static_pad(decoder, "sink");
	gst_pad_link(pad, sinkpad);

	g_object_unref(sinkpad);

}
