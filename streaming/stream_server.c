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

#include <glib.h>
#include <gst/gst.h>

/*
 * Handle changes in state on the bus.
 */
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data){

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

