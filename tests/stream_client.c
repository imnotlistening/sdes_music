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
 * Streaming client. Plays an RTP stream.
 *
 * Usage:
 *   stream_client <host:rtp-port>
 *
 * It is assumed that RTCP is on the port above the RTP port.
 *
 * Much of this code is borrowed from an example with the GST good plugins
 * source code.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

/* 
 * The caps of the sender RTP stream.
 */
/*
#define AUDIO_CAPS "application/x-rtp, media=(string)audio," \
  " clock-rate=(int)90000, encoding-name=(string)MPA"
*/
/*
#define AUDIO_CAPS "application/x-rtp, media=(string)audio, payload=(int)127, clock-rate=(int)44100, encoding-name=(string)AC3"
*/
#define AUDIO_CAPS "application/x-rtp,media=(string)audio,clock-rate=" \
	"(int)8000,encoding-name=(string)PCMA"

/*
 * Elements to use for the receiver pipeline.
 */
#define AUDIO_DEPAY "rtppcmadepay"
#define AUDIO_DEC   "decodebin2"
#define AUDIO_SINK  "autoaudiosink"

/*
 * Some functions for later use.
 */
static void print_source_stats(GObject *source);
static void on_ssrc_active_cb(GstElement *rtpbin, guint sessid, guint ssrc,
			      GstElement *depay);
static void pad_added_cb (GstElement *rtpbin, GstPad *new_pad, 
			  GstElement *depay);
static void on_pad_added(GstElement *depay, GstPad *pad, gpointer user_data);

/**
 * Main function, derr.
 */
int main(int argc, char **argv){

	GstElement *rtpbin, *rtpsrc, *rtcpsrc, *rtcpsink;
	GstElement *audiodepay, *audiodec, *audiores, *audioconv, *audiosink;
	GstElement *pipeline;
	GMainLoop *loop;
	GstCaps *caps;
	gboolean res;
	GstPadLinkReturn lres;
	GstPad *srcpad, *sinkpad;
	int port, convs;
	char *host;
	char *port_str;

	/* Init the GST library. */
	gst_init(&argc, &argv);

	/* Only 1 argument, a source to play. */
	if ( argc != 2 ){
		printf("Usage: %s <host:rtp-port>\n", argv[0]);
		return 1;
	}

	host = argv[1];
	port_str = strchr(host, ':');
	*port_str++ = 0;
	convs = sscanf(port_str, "%d", &port);
	ASSERT_OR_ERROR(convs == 1);
	ASSERT_OR_ERROR(port > 0 && port < 65536);
	printf("Receiving RTP from: %s:%d\n", host, port);
	
	/* The pipeline to hold everything */
	pipeline = gst_pipeline_new(NULL);
	g_assert(pipeline);

	/* The udp src and source we will use for RTP and RTCP */
	rtpsrc = gst_element_factory_make("udpsrc", "rtpsrc");
	g_assert(rtpsrc);
	g_object_set(rtpsrc, "port", port, NULL);

	/* we need to set caps on the udpsrc for the RTP data */
	caps = gst_caps_from_string(AUDIO_CAPS);
	g_object_set(rtpsrc, "caps", caps, NULL);
	gst_caps_unref(caps);

	rtcpsrc = gst_element_factory_make("udpsrc", "rtcpsrc");
	g_assert(rtcpsrc);
	g_object_set(rtcpsrc, "port", port + 1, NULL);

	rtcpsink = gst_element_factory_make("udpsink", "rtcpsink");
	g_assert(rtcpsink);
	g_object_set(rtcpsink, "port", port + 2, "host", host, NULL);

	/* no need for synchronisation or preroll on the RTCP sink */
	g_object_set(rtcpsink, "async", FALSE, "sync", FALSE, NULL);

	gst_bin_add_many(GST_BIN (pipeline), rtpsrc, rtcpsrc, rtcpsink, NULL);

	/* the depayloading and decoding */
	audiodepay = gst_element_factory_make(AUDIO_DEPAY, "audiodepay");
	g_assert(audiodepay);
	audiodec = gst_element_factory_make(AUDIO_DEC, "audiodec");
	g_assert(audiodec);

	/* the audio playback and format conversion */
	audioconv = gst_element_factory_make("audioconvert", "audioconv");
	g_assert(audioconv);
	audiores = gst_element_factory_make("audioresample", "audiores");
	g_assert(audiores);
	audiosink = gst_element_factory_make(AUDIO_SINK, "audiosink");
	g_assert(audiosink);

	/* add depayloading and playback to the pipeline and link */
	gst_bin_add_many(GST_BIN(pipeline), audiodepay, audiodec, audioconv,
			  audiores, audiosink, NULL);

	
	res = gst_element_link_many(audiodepay, audiodec, NULL);
	g_assert(res == TRUE);
	res = gst_element_link_many(audioconv, audiores, audiosink, NULL);
	g_assert(res == TRUE);

	/* the rtpbin element */
	rtpbin = gst_element_factory_make("gstrtpbin", "rtpbin");
	g_assert(rtpbin);

	gst_bin_add(GST_BIN(pipeline), rtpbin);

	/* now link all to the rtpbin, start by getting an RTP sinkpad for 
	   session 0 */
	srcpad = gst_element_get_static_pad(rtpsrc, "src");
	sinkpad = gst_element_get_request_pad(rtpbin, "recv_rtp_sink_0");
	lres = gst_pad_link(srcpad, sinkpad);
	g_assert(lres == GST_PAD_LINK_OK);
	gst_object_unref(srcpad);

	/* get an RTCP sinkpad in session 0 */
	srcpad = gst_element_get_static_pad(rtcpsrc, "src");
	sinkpad = gst_element_get_request_pad(rtpbin, "recv_rtcp_sink_0");
	lres = gst_pad_link(srcpad, sinkpad);
	g_assert(lres == GST_PAD_LINK_OK);
	gst_object_unref(srcpad);
	gst_object_unref(sinkpad);

	/* get an RTCP srcpad for sending RTCP back to the sender */
	srcpad = gst_element_get_request_pad(rtpbin, "send_rtcp_src_0");
	sinkpad = gst_element_get_static_pad(rtcpsink, "sink");
	lres = gst_pad_link(srcpad, sinkpad);
	g_assert(lres == GST_PAD_LINK_OK);
	gst_object_unref(sinkpad);

	/* the RTP pad that we have to connect to the depayloader will be 
	 * created dynamically so we connect to the pad-added signal, pass the
	 * depayloader as user_data so that we can link to it. */
	g_signal_connect(rtpbin, "pad-added", G_CALLBACK(pad_added_cb),
			 audiodepay);

	/*
	 * Link the decodebin and the next pad in the chain.
	 */
	g_signal_connect(audiodec, "pad-added",
			 G_CALLBACK(on_pad_added), audioconv);

	/* give some stats when we receive RTCP */
	g_signal_connect(rtpbin, "on-ssrc-active", 
			 G_CALLBACK(on_ssrc_active_cb), audiodepay);

	/* set the pipeline to playing */
	g_print("Starting receiver pipeline\n");
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	/* we need to run a GLib main loop to get the messages */
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	g_print("Stopping receiver pipeline\n");
	gst_element_set_state(pipeline, GST_STATE_NULL);

	gst_object_unref(pipeline);

	return 0;

}

/**
 * Print the stats of a source.
 */
static void print_source_stats(GObject *source){

	GstStructure *stats;
	gchar *str;

	g_return_if_fail(source != NULL);

	/* get the source stats */
	g_object_get(source, "stats", &stats, NULL);

	/* simply dump the stats structure */
	str = gst_structure_to_string(stats);
	g_print("source stats: %s\n", str);

	gst_structure_free(stats);
	g_free(str);

}

/*
 * Call back signal handler for printing info from received RTCP packets.
 */
static void on_ssrc_active_cb(GstElement *rtpbin, guint sessid, guint ssrc,
			      GstElement *depay){

	GObject *session, *isrc, *osrc;

	g_print("Got RTCP from session %u, SSRC %u\n", sessid, ssrc);

	/* get the right session */
	g_signal_emit_by_name(rtpbin, "get-internal-session", sessid, &session);

	/* get the internal source (the SSRC allocated to us, the receiver */
	g_object_get(session, "internal-source", &isrc, NULL);
	print_source_stats(isrc);

	/* get the remote source that sent us RTCP */
	g_signal_emit_by_name(session, "get-source-by-ssrc", ssrc, &osrc);
	print_source_stats(osrc);

}

/* will be called when rtpbin has validated a payload that we can depayload */
static void pad_added_cb (GstElement *rtpbin, GstPad *new_pad, 
			  GstElement *depay){

	GstPad *sinkpad;
	GstCaps *caps;
	GstPadLinkReturn lres;

	g_print ("new payload on pad: %s\n", GST_PAD_NAME (new_pad));
	caps = gst_pad_get_caps(new_pad);
	g_print ("  Caps are: %s\n", gst_caps_to_string(caps));

	sinkpad = gst_element_get_static_pad (depay, "sink");
	g_assert (sinkpad);

	lres = gst_pad_link (new_pad, sinkpad);
	g_assert (lres == GST_PAD_LINK_OK);
	gst_object_unref (sinkpad);

	printf("Built the RTP decoder.\n");

}

/*
 * Occurs when the decode in needs to be connected to the payloader.
 */
static void on_pad_added(GstElement *depay, GstPad *pad, gpointer user_data){

	GstPad *sinkpad;
	GstElement *decoder = (GstElement *)user_data;

	printf("Linking audio decoder to audio-resampler.\n");

	sinkpad = gst_element_get_static_pad(decoder, "sink");
	gst_pad_link(pad, sinkpad);

	g_object_unref(sinkpad);

}
