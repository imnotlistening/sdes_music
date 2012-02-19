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

#ifndef _MUSIC_H_
#define _MUSIC_H_

/*
 * A struct for holding some relevant pointers into a RTP pipeline.
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

};

/*
 * Some API functions.
 */
int music_rtp_make_pipeline(struct music_rtp_pipeline *pipe,
			    char *id, int rtp, int rtcp, char *dest_host);
int music_make_mloop(struct music_rtp_pipeline *pipe);

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

#endif
