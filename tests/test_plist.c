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
 * Just some simple tests for the pipe list code. Boring stuff.
 */

#include <music.h>

#include <stdio.h>
#include <stdlib.h>

struct music_rtp_plist plist;

int main(){

	int i;

	printf("Testing pipe list functionality:\n");
	printf("Initing the plist structure:\n");
	ASSERT_OR_ERROR(music_plist_init(&plist, 1) == 0);

	printf("Adding a bunch of pipes:\n");
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	music_plist_print(&plist, 1);

	ASSERT_OR_ERROR(music_plist_del(&plist, 3) != 0);
	ASSERT_OR_ERROR(music_plist_del(&plist, 3) == 0);
	music_plist_print(&plist, 1);
	ASSERT_OR_ERROR(plist.length == 7);

	ASSERT_OR_ERROR(music_plist_del(&plist, 0) != 0);
	ASSERT_OR_ERROR(music_plist_del(&plist, 2) != 0);
	ASSERT_OR_ERROR(music_plist_del(&plist, 6) != 0);
	music_plist_print(&plist, 1);
	ASSERT_OR_ERROR(plist.length == 4);

	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	ASSERT_OR_ERROR(music_plist_add(&plist, MUSIC_ALLOC_PIPELINE()) == 0);
	music_plist_print(&plist, 1);
	ASSERT_OR_ERROR(plist.length == 9);	

	ASSERT_OR_ERROR(music_plist_del(&plist, 0) != 0);
	ASSERT_OR_ERROR(music_plist_del(&plist, 1) != 0);
	ASSERT_OR_ERROR(music_plist_del(&plist, 3) != 0);
	ASSERT_OR_ERROR(music_plist_del(&plist, 5) != 0);
	ASSERT_OR_ERROR(music_plist_del(&plist, 6) != 0);
	music_plist_print(&plist, 1);

	printf("Iterating through list with music_plist_next()\n");
	for ( i = 0; i < 2; i++ ){
		printf("List element %d: 0x%016lx\n", i, 
		       (unsigned long int)music_plist_next(&plist, 0));
	}
	printf("Reseting list iteration with music_plist_next()\n");
	ASSERT_OR_ERROR(music_plist_next(NULL, 1) == NULL);
	for ( i = 0; i < 6; i++ ){
		printf("List element %d: 0x%016lx\n", i, 
		       (unsigned long int)music_plist_next(&plist, 0));
	}
	
	return 0;

}
