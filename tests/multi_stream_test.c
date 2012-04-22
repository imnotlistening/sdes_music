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

#define TERM_SAVE_CURSOR(TERM)	(write(TERM, "\0337", 2))
#define TERM_RESET_CURSOR(TERM)	(write(TERM, "\0338", 2))
#define TERM_WIPE_DOWN(TERM)	(write(TERM, "\033[J", 3))

extern GMainLoop *music_mloop;

static void *play_thread(void *arg);

/* non-zero = playing, 0 = paused. */
int state = 1;

/* Pipeline struct. */
struct music_rtp_plist plist;

/*
 * Simple test routine. See if the base API is functioning.
 */
int main(int argc, char **argv){

	int err;
	int64_t pos, len;
	pthread_t thread;
	GstFormat fmt = GST_FORMAT_TIME;
	struct music_rtp_pipeline *pline;

	/* Init the GST library. */
	gst_init(&argc, &argv);

	/* Only 1 argument, a source to play. */
	if ( argc != 3 ){
		printf("Usage: %s <file> <file>\n", argv[0]);
		return 1;
	}

	/* Init the pipeline list. */
	ASSERT_OR_ERROR(music_plist_init(&plist, 1) == 0);
	
	/* Make a couple of pipelines. Requires an IP address to send to :(. */
	pline = MUSIC_ALLOC_PIPELINE();
	ASSERT_OR_ERROR(pline != NULL);
	err = music_make_pipeline(pline, "test-udp-pipe1", 5000, "127.0.0.1");
	if ( err ){
		fprintf(stderr, "Could not make pipeline. :(\n");
		return 1;
	}
	err = music_play_song(pline, argv[1]);
	if ( err ){
		fprintf(stderr, "Failed to load song: %s\n", argv[1]);
		return 1;
	}
	music_set_state(pline, GST_STATE_PLAYING);
	ASSERT_OR_ERROR(music_plist_add(&plist, pline) == 0);
	
	/* And the second. */
	pline = MUSIC_ALLOC_PIPELINE();
	ASSERT_OR_ERROR(pline != NULL);
	err = music_make_pipeline(pline, "test-udp-pipe2", 5000, "127.0.0.2");
	if ( err ){
		fprintf(stderr, "Could not make pipeline. :(\n");
		return 1;
	}
	err = music_play_song(pline, argv[2]);
	if ( err ){
		fprintf(stderr, "Failed to load song: %s\n", argv[2]);
		return 1;
	}
	music_set_state(pline, GST_STATE_PLAYING);
	ASSERT_OR_ERROR(music_plist_add(&plist, pline) == 0);

	/* And init the main loop for the pipeline. This does not start the
	 * main loop, thats done later. */
	music_make_mloop();
	
	/* Run the main loop in a different thread. */
	err = pthread_create(&thread, NULL, play_thread, music_mloop);
	if ( err ){
		printf("Failed to start play back thread.\n");
		return 1;
	}

	/* Terminal magic. ESC 7: Saves cursor position. */
	while ( TERM_SAVE_CURSOR(0) > 0 );

	/* Now that we have the music streaming, monitor the stream indexes. */
	while ( 1 ){

		/* Display the progress of these two streams. */
		pline = music_plist_get(&plist, 0);
		ASSERT_OR_ERROR(pline != NULL);
		gst_element_query_position (pline->pipeline, &fmt, &pos);
		gst_element_query_duration (pline->pipeline, &fmt, &len);
		g_print ("S0: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\n",
			 GST_TIME_ARGS (pos), GST_TIME_ARGS (len));

		/* The second stream. */
		pline = music_plist_get(&plist, 1);
		ASSERT_OR_ERROR(pline != NULL);
		gst_element_query_position (pline->pipeline, &fmt, &pos);
		gst_element_query_duration (pline->pipeline, &fmt, &len);
		g_print ("S0: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\n",
			 GST_TIME_ARGS (pos), GST_TIME_ARGS (len));

		while ( TERM_RESET_CURSOR(0) > 0 );
		while ( TERM_WIPE_DOWN(0) > 0 );
		
	}

	pthread_join(thread, NULL);

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
