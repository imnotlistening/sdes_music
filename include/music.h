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
 * Defines, functions, structs, etc for the MUSIC RTP server.
 */

#include <inttypes.h>

#ifndef _MUSIC_H_
#define _MUSIC_H_

/* Translation from Gst types to our own types to avoid requiring the GST
 * includes everywhere. I am not completely sure this is a good idea at the
 * moment so if it needs changing, that can be done.
 */
#define MUSIC_STATE_VOID_PENDING	(0)
#define MUSIC_STATE_NULL		(1)
#define MUSIC_STATE_READY		(2)
#define MUSIC_STATE_PAUSED		(3)
#define MUSIC_STATE_PLAYING		(4)

/*
 * A struct for holding some relevant pointers which define an RTP pipeline.
 */
struct music_rtp_pipeline {

	/*
	 * The pipeline itself.
	 */
	GstElement 	*pipeline;
	
	/*
	 * The file source.
	 */
	GstElement	*filesrc;

	/*
	 * Volume controller.
	 */
	GstElement	*volume;

	/*
	 * The RTP bin.
	 */
	GstElement	*rtpbin;

	/*
	 * A main loop for GLib.
	 */
	GMainLoop	*mloop;

	/*
	 * A function call back for when the end of stream has occured. You
	 * can use this function and pipeline pointer to start playing a new
	 * song.
	 */
	void 		(*end_of_stream)(struct music_rtp_pipeline *pipe);

};

/*
 * Some API functions.
 */
int	music_rtp_make_pipeline(struct music_rtp_pipeline *pipe,
			    char *id, int rtp, int rtcp, char *dest_host);
int	music_make_mloop(struct music_rtp_pipeline *pipe);
int	music_play_song(struct music_rtp_pipeline *pipe, const char *song_path);
int	music_set_state(struct music_rtp_pipeline *pipe, int state);
int	music_get_state(struct music_rtp_pipeline *pipe);
int64_t	music_get_time_pos(struct music_rtp_pipeline *pipe);
int64_t	music_get_time_len(struct music_rtp_pipeline *pipe);
int	music_set_volume(struct music_rtp_pipeline *pipe, int volume);
int	music_get_volume(struct music_rtp_pipeline *pipe);

/*
 * Macros.
 */
#define ASSERT_OR_ERROR(expr)						\
	do {								\
		if ( ! (expr) ){					\
			fprintf(stderr, "Assert failed: [%s:%s %d] %s\n", \
				__FILE__, __func__, __LINE__, #expr);	\
			return -1;					\
		}							\
	} while ( 0 )

#define MUSIC_IS_PLAYING(state)	(state == MUSIC_STATE_PLAYING)
#define MUSIC_IS_PUASED(state)	(state == MUSIC_STATE_PAUSED)
#define MUSIC_IS_READY(state)	(state == MUSIC_STATE_READY)
#define MUSIC_IS_NOGO(state)						\
	(state != MUSIC_STATE_READY && state != MUSIC_STATE_PAUSED &&	\
	 state != MUSIC_STATE_PLAYING)

#endif
