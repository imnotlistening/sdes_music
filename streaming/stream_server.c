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
 * Server stand alone binary.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

static void *play_thread(void *arg);

/* non-zero = playing, 0 = paused. */
int state = 1;

/* Pipeline struct. */
struct music_rtp_pipeline  pipeline;

/*
 * Simple test routine. See if the base API is functioning.
 */
int main(int argc, char **argv){

	char buf[256], cmd[256];
	int err, bytes, convs, vol;
	double vol_normed;
	pthread_t thread;

	/* Init the GST library. */
	gst_init(&argc, &argv);

	/* Only 1 argument, a source to play. */
	if ( argc != 2 ){
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	err = music_rtp_make_pipeline(&pipeline, "test-rtp-pipe", 5000, 5001,
				      "129.21.131.28");
	if ( err ){
		fprintf(stderr, "Could not make pipeline. :(\n");
		return 1;
	}
	music_make_mloop(&pipeline);

	/* Load in the file source. */
	g_object_set(G_OBJECT(pipeline.filesrc), "location", argv[1], NULL);

	/* Start of in the 'play' state. */
	gst_element_set_state(pipeline.pipeline, GST_STATE_PLAYING);

	err = pthread_create(&thread, NULL, play_thread, pipeline.mloop);
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
				gst_element_set_state(pipeline.pipeline,
						      GST_STATE_PAUSED);
			} else {
				printf("Starting.\n");
				gst_element_set_state(pipeline.pipeline,
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

			g_object_set(G_OBJECT(pipeline.volume), "volume",
				     vol_normed, NULL);

		}

	}

	printf("Waiting until the song is over.\n");
	pthread_join(thread, NULL);

	/* And some basic cleanup for when the media is over. */
	gst_element_set_state(pipeline.pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline.pipeline));

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
