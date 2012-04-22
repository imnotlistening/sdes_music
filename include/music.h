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

#include <glib.h>
#include <gst/gst.h>

#ifndef _MUSIC_H_
#define _MUSIC_H_

#ifdef __cplusplus
extern "C" {
#endif

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
	 * A function call back for when the end of stream has occured. You
	 * can use this function and pipeline pointer to start playing a new
	 * song.
	 */
	void 		(*end_of_stream)(struct music_rtp_pipeline *pipe);

};

/*
 * A struct for containing a list of pipelines.
 */
struct music_rtp_plist {

	struct music_rtp_pipeline **pipes;
	unsigned int	length;
	unsigned int	capacity;

};

/*
 * Some API functions.
 */
int	music_make_pipeline(struct music_rtp_pipeline *pipe,
			    char *id, int port, char *dest_host);
int	music_make_mloop();
int	music_play_song(struct music_rtp_pipeline *pipe, const char *song_path);
int	music_set_state(struct music_rtp_pipeline *pipe, int state);
int	music_get_state(struct music_rtp_pipeline *pipe);
int64_t	music_get_time_pos(struct music_rtp_pipeline *pipe);
int64_t	music_get_time_len(struct music_rtp_pipeline *pipe);
int	music_set_volume(struct music_rtp_pipeline *pipe, int volume);
int	music_get_volume(struct music_rtp_pipeline *pipe);

/* Pipeline list functions. */
int	music_plist_init(struct music_rtp_plist *plist, int capacity);
int	music_plist_add(struct music_rtp_plist *plist,
			struct music_rtp_pipeline *pipe);
struct music_rtp_pipeline *music_plist_del(struct music_rtp_plist *plist,
					   int offset);
struct music_rtp_pipeline *music_plist_next(struct music_rtp_plist *list,
					    int reset);
struct music_rtp_pipeline *music_plist_get(struct music_rtp_plist *plist,
					   int index);
void	music_plist_print(struct music_rtp_plist *plist, int empty);

/* Pipeline list functions. */
int	music_plist_init(struct music_rtp_plist *plist, int capacity);
int	music_plist_add(struct music_rtp_plist *plist,
			struct music_rtp_pipeline *pipe);
struct music_rtp_pipeline *music_plist_del(struct music_rtp_plist *plist,
					   int offset);
struct music_rtp_pipeline *music_plist_next(struct music_rtp_plist *list,
					    int reset);
struct music_rtp_pipeline *music_plist_get(struct music_rtp_plist *plist,
					   int index);
void	music_plist_print(struct music_rtp_plist *plist, int empty);

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

/* State macros. */
#define MUSIC_IS_PLAYING(state)	(state == MUSIC_STATE_PLAYING)
#define MUSIC_IS_PUASED(state)	(state == MUSIC_STATE_PAUSED)
#define MUSIC_IS_READY(state)	(state == MUSIC_STATE_READY)
#define MUSIC_IS_NOGO(state)						\
	(state != MUSIC_STATE_READY && state != MUSIC_STATE_PAUSED &&	\
	 state != MUSIC_STATE_PLAYING)

/* A macro for getting an element from a list of pipelines. */
#define MUSIC_PLIST_GET(plist_ptr, offset) (plist->pipes[offset])

/* Allocates a rtp pipeline struct. Just a wrapper for malloc(). */
#define MUSIC_ALLOC_PIPELINE()			\
	(struct music_rtp_pipeline *)malloc(sizeof(struct music_rtp_pipeline))

#ifdef __cplusplus
}
#endif

#endif
