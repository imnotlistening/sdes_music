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
 * Simple implementation of a dymanic list for pipelines.
 */

#include <stdlib.h>
#include <string.h>

#include <music.h>

#define __PLIST_INC_SIZE	(4)

/*
 * Initialize the passed pipeline list. Attempt to make the list have the
 * specified initial capacity. Returns 1 if mempry allocation failes,
 * otherwise 0.
 */
int music_plist_init(struct music_rtp_plist *plist, int capacity){

	memset(plist, 0, sizeof(struct music_rtp_plist));

	plist->pipes = (struct music_rtp_pipeline **)
		malloc(sizeof(struct music_rtp_pipeline *) * capacity);
	if ( ! plist->pipes )
		return -1;

	plist->capacity = capacity;
	return 0;

}

/*
 * Adds the passed pipeline pointer to the list. Returns 0 on success, -1
 * otherwise.
 */
int music_plist_add(struct music_rtp_plist *plist,
		    struct music_rtp_pipeline *pipe){

	int i;
	struct music_rtp_pipeline **pipes;

	/* First determine if we need to grow the list. */
	if ( plist->length >= plist->capacity ){

		pipes = (struct music_rtp_pipeline **)
			realloc(plist->pipes,
				(plist->capacity + __PLIST_INC_SIZE) *
				sizeof(struct music_rtp_pipeline *));
		if ( pipes == NULL )
			return -1; /* Fail :( */

		/* Zero out the new memory. */
		memset(&pipes[plist->capacity], 0,
		       sizeof(struct music_rtp_pipeline *) * __PLIST_INC_SIZE);
		plist->pipes = pipes;
		plist->capacity += __PLIST_INC_SIZE;
	}

	/* Now find the first available slot and store the new pipe pointer. */
	for ( i = 0; i < plist->capacity; i++ ){
		if ( plist->pipes[i] == NULL ){
			plist->pipes[i] = pipe;
			break;
		}
	}

	plist->length++;

	return 0;

}

/*
 * Remove a pointer from the list. It is the callers responsibility to clean
 * that pointer up.
 */
struct music_rtp_pipeline *music_plist_del(struct music_rtp_plist *plist,
					   int offset){

	struct music_rtp_pipeline *pipe;

	if ( offset < 0 || offset >= plist->capacity )
		return NULL;

	if ( plist->pipes[offset] == NULL )
		return NULL;

	pipe = plist->pipes[offset];
	plist->pipes[offset] = NULL;
	plist->length--;
	return pipe;

}

/*
 * Iterate through the list of pipes. Calling this function over and over will
 * return the next valid pointer in the list or NULL if nothing is in the list.
 * Editing the list while making calls to this function is ill advised.
 *
 * If reset is non-zero then this function will reset the position of the next
 * pointer to the start and do nothing else. Yes, there is internal state.
 * Sorry.
 */
struct music_rtp_pipeline *music_plist_next(struct music_rtp_plist *list,
					    int reset){

	int i;

	/*
	 * Start at zero, obviously.
	 */
	static int next = 0;

	if ( reset ){
		next = 0;
		return NULL;
	}

	i = next;
	do {

		if ( list->pipes[i] ){
			next = (i + 1) % list->capacity;
			return list->pipes[i];
		}

		i = (i + 1) % list->capacity;

	} while ( i != next );

	return NULL;

}

/*
 * Returns the specified stream skipping empty streams. Returns NULL if there
 * is an error, e.g: index is out of bounds, or a pointer to the requested
 * element.
 */
struct music_rtp_pipeline *music_plist_get(struct music_rtp_plist *plist,
					   int index){

	int i, count = 0;

	if ( index < 0 )
		return NULL;

	for ( i = 0; i < plist->capacity; i++ ){

		if ( plist->pipes[i] == NULL )
			continue;

		if ( i == count )
			return plist->pipes[i];

		count++;

	}

	return NULL;

}

/*
 * Print the contents of a list. If the passed value is 1, then also indicate
 * where there are empty slots in the list.
 */
void music_plist_print(struct music_rtp_plist *plist, int empty){

	int i;

	printf("Pipes list, capacity = %d  filled = %d\n",
	       plist->capacity, plist->length);
	for ( i = 0; i < plist->capacity; i++ ){
		if ( plist->pipes[i] || empty ){
			printf("  Entry %d: 0x%016lx\n", 
			       i, (unsigned long) plist->pipes[i]);
		}
	}

}
