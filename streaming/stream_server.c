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

static gboolean bus_callb(GstBus *bus, GstMessage *msg, gpointer data);
static void     new_pad_added(GstElement *elem, GstPad *pad, gpointer data);
static void    *play_thread(void *arg);

/*
 * Elements for building our input pipeline.
 */
GstElement *source;
GstElement *decodebin;
GstElement *volume;
GstElement *autoaudiosink;

/* non-zero = playing, 0 = paused. */
int state = 1;

int main(int argc, char **argv){

	char buf[256], cmd[256];
	int err, bytes, convs, vol;
	double vol_normed;
	GstBus *bus;
	GMainLoop *loop;
	GstElement *pipeline;

	pthread_t thread;

	/* Init the GST library. */
	gst_init(&argc, &argv);
	loop = g_main_loop_new(NULL, FALSE);

	/* Only 1 argument, a source to play. */
	if ( argc != 2 ){
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	/* Initialize the pipeline. */
	pipeline      = gst_pipeline_new("Stream server.");
	source        = gst_element_factory_make("filesrc", "file source");
	decodebin     = gst_element_factory_make("decodebin2", "decode");
	volume        = gst_element_factory_make("volume",  "vol-cntl");
	autoaudiosink = gst_element_factory_make("autoaudiosink", "play");

	if ( ! pipeline || ! source || ! decodebin || ! autoaudiosink ){
		printf("Error making one of the elements.\n");
		return 1;
	}

	/* Set the song to play and a default volume. */
	g_object_set(G_OBJECT(source), "location", argv[1], NULL);
	g_object_set(G_OBJECT(volume), "volume", .1, NULL);

	/* This gets a pointer to the bus through which the media flows. */
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_watch(bus, bus_callb, loop);
	g_object_unref(bus);

	/* Add the elements to the pipeline. */
	gst_bin_add_many(GST_BIN(pipeline),
			 source, decodebin, volume, autoaudiosink, NULL);
	gst_element_link(source, decodebin);
	gst_element_link(volume, autoaudiosink);
	g_signal_connect(decodebin, "pad-added", G_CALLBACK(new_pad_added),
			 volume);

	/* Start of in the 'play' state. */
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	err = pthread_create(&thread, NULL, play_thread, loop);
	if ( err ){
		printf("Failed to start play back thread.\n");
		return 1;
	}
	
	/* Loop forver waiting for input from the terminal. */
	printf("Playing %s\n", argv[1]);
	memset(buf, 0, 256);
	while ( 1 ){
		
		printf("> ");
		fflush(stdout);
		bytes = read(0, buf, 256);
		if ( bytes == 0 )
			break;

		buf[strlen(buf) - 1] = 0;

		switch ( buf[0] ){

		case 'p':
			if ( state ){
				printf("Pausing.\n");
				gst_element_set_state(pipeline,
						      GST_STATE_PAUSED);
			} else {
				printf("Starting.\n");
				gst_element_set_state(pipeline,
						      GST_STATE_PLAYING);
			}
			state = !state;
			break;

		case 's':
			printf("Stopping; goodbye.\n");
			exit(0);
			break;

		case 'v':
			convs = sscanf(buf, "%s %d", cmd, &vol);
			if ( convs != 2 ){
				printf("Failed to parse volume control.\n");
				break;
			}

			printf("Setting volume: %d\n", vol);
			if ( vol < 0 || vol > 100 ){
				printf("Volume out of bounds, accepted values"
				       " [0, 100]\n");
				break;
			}

			vol_normed = (double)vol / 100.0;

			g_object_set(G_OBJECT(volume), "volume",
				     vol_normed, NULL);

		}

	}

	printf("Waiting until the song is over.\n");
	pthread_join(thread, NULL);

	/* And some basic cleanup for when the media is over. */
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));

	return 0;

}

static void *play_thread(void *arg){

	GMainLoop *loop = arg;

	/* And start the media player (lol) up. */
	g_main_loop_run(loop);

	/* Just exit; screw cleaning up data structures. */
	exit(0);

	return NULL;

}

/*
 * Handle changes in state on the bus.
 */
static gboolean bus_callb(GstBus *bus, GstMessage *msg, gpointer data){

	gchar *debug;
	GError *error;
	GMainLoop *loop = (GMainLoop *)data;

	switch (GST_MESSAGE_TYPE(msg)){

	/*
	 * End of stream.
	 */
	case GST_MESSAGE_EOS:
		g_print("End of stream detected\n");
		g_main_loop_quit(loop);
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
