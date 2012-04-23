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
#include <pthread.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

static void *play_thread(void *arg);
static void stream_end(struct music_rtp_pipeline *pipe);

/* non-zero = playing, 0 = paused. */
int state = 1;

/*
 * Main function, derr.
 */
int main(int argc, char **argv){

	long int offset;
	int err, bytes, convs, vol;
	char buf[256], cmd[256], *uri;
	pthread_t thread;
	struct music_rtp_pipeline pipeline;
	
	/* Init the GST library. */
	gst_init(&argc, &argv);

	/* Only 1 argument, a source to play. */
	if ( argc != 2 ){
		printf("Usage: %s <URI>\n", argv[0]);
		return 1;
	}

	printf("Receiving URI: %s\n", argv[1]);
	
	/* Init the main loop for the pipeline. This does not start the
	 * main loop, thats done later. */
	music_make_mloop();

	/* The pipeline to hold everything */
	err = music_make_pipeline(&pipeline, "test-pipe");
	if ( err ){
		fprintf(stderr, "Could not make pipeline. :(\n");
		return 1;
	}
	pipeline.end_of_stream = stream_end;

	/* Now play a song. */
	err = music_play_song(&pipeline, argv[1]);
	if ( err ){
		fprintf(stderr, "Failed to load song: %s\n", argv[1]);
		return 1;
	}
	
	/* And now set the pipeline's state to playing. Still not gonna start
	 * making music quite yet, only when we start the mainloop will that
	 * happen. */
	music_set_state(&pipeline, GST_STATE_PLAYING);

	/* Run the main loop in a different thread. */
	err = pthread_create(&thread, NULL, play_thread, pipeline.mloop);
	if ( err ){
		printf("Failed to start play back thread.\n");
		return 1;
	}

	/* Loop forver waiting for input from the terminal. */
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

			music_set_volume(&pipeline, vol);
			break;

		case 'f': /* Forward 10 seconds. */
		case 'b': /* Backwards 10 seconds. */
			printf("Not implemented.\n");
			break;

		case 'k':
			convs = sscanf(buf, "%s %ld", cmd, &offset);
			if ( convs != 2 ){
				printf("Failed to parse seek time.\n");
				break;
			}

			/* Scale this input number by a factor of 1^9 */
			offset *= 1e9;
			printf("Seeking to %ld ns\n", offset);

			if (!gst_element_seek_simple(pipeline.pipeline,
						     GST_FORMAT_TIME,
						     GST_SEEK_FLAG_FLUSH |
						     GST_SEEK_FLAG_KEY_UNIT,
						     offset))
				g_print ("Seek failed!\n");
			break;

		case 'n':
			uri = strstr(buf, " ");
			uri++;

			printf("Playing new URL: %s\n", uri);
			music_play_song(&pipeline, uri);
			music_set_state(&pipeline, GST_STATE_PLAYING);

		}

	}

	pthread_join(thread, NULL);

	/* And some basic cleanup for when the media is over. */
	music_set_state(&pipeline, GST_STATE_NULL);
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

static void stream_end(struct music_rtp_pipeline *pipe){

	printf("Stream is done. Yay. What should we do now?\n");

}
