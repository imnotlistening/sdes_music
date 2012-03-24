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
 * Streaming API implementation.
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

/*
 * Play a given song. If the pipeline is already playing a song, then that song
 * will be stopped, and the new song will be played instead. This function will
 * not necessarily return immediately. If the pipeline in question is in a
 * transition state, then this function will not return until either an error
 * is encountered or the state of the pipeline settles to a valid state.
 *
 * Returns 0 on success, < 0 on failure.
 */
int music_play_song(struct music_rtp_pipeline *pipe, const char *song_path){

	GstState cur_state;
	GstStateChangeReturn state_change;

	do {

		/* 10 ms timeout. */
		state_change = gst_element_get_state(pipe->pipeline, &cur_state,
						     NULL, 10000000);

	} while ( state_change == GST_STATE_CHANGE_ASYNC );

	/* 
	 * Now we have the state. If the state is playing, then stop the
	 * pipeline before replacing the source file.
	 */
	if ( cur_state == GST_STATE_PLAYING || cur_state == GST_STATE_PAUSED )
		gst_element_set_state(pipe->pipeline, GST_STATE_NULL);

	/* And load the new file. */
	ASSERT_OR_ERROR(song_path != NULL);
	g_object_set(G_OBJECT(pipe->filesrc), "location", song_path, NULL);

	return 0;

}

/*
 * Set the pipeline to the requested state. Valid status are:
 *   GST_STATE_VOID_PENDING
 *   GST_STATE_NULL
 *   GST_STATE_READY
 *   GST_STATE_PAUSED
 *   GST_STATE_PLAYING
 *
 * Returns 0, cause I am lazy. This is really just a convience function.
 */
int music_set_state(struct music_rtp_pipeline *pipe, int state){

	gst_element_set_state(pipe->pipeline, state);

	return 0;

}

/*
 * Set the volume of the pipeline. No error or range checking is performed
 * here; it is assumed that this function is called by music_set_volume() and
 * therefor has a reasonable volume level.
 */
int __music_set_volume(struct music_rtp_pipeline *pipe, double volume){

	g_object_set(G_OBJECT(pipe->volume), "volume", volume, NULL);

	return 0;

}

/*
 * Sets the volume of the pipeline. 
 *
 * Returns 0 on success, -1 on failure.
 */
int music_set_volume(struct music_rtp_pipeline *pipe, int volume){

	ASSERT_OR_ERROR(volume >= 0 && volume <= 100);
	__music_set_volume(pipe, (double)volume / 100.0);

	return 0;

}

/*
 * Returns the volume of the pipeline as an integer on the interval [0, 100].
 */
int music_get_volume(struct music_rtp_pipeline *pipe){

	double volume;
	
	g_object_get(G_OBJECT(pipe->volume), "volume", &volume, NULL);
	return (int)(volume * 100);

}

/*
 * Return the location in the stream. This is only relevant if the stream is
 * playing or paused.
 */
int64_t music_get_time(struct music_rtp_pipeline *pipe){

	int64_t pos = 0;

	

	return pos;

}
