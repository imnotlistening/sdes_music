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
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <glib.h>
#include <gst/gst.h>

#include <music.h>

#define ENCODER		"alawenc"
#define PAYLOADER	"rtppcmapay"

static gboolean bus_callb(GstBus *bus, GstMessage *msg, gpointer data);
static void     new_pad_added(GstElement *elem, GstPad *pad, gpointer data);

/**
 * Build a GstRtpBin pipeline. This will attempt to stream on the passed ports.
 */
int music_rtp_make_pipeline(struct music_rtp_pipeline *pipe,
			    char *id, int rtp, int rtcp, char *dest_host){

	GstPad *rtp_src, *rtp_sink, *rtcp_src, *rtcp_sink, *tmp;
	GstElement *pipeline;
	GstElement *source;
	GstElement *decodebin;
	GstElement *volume, *convert, *resample;
	GstElement *encoder, *payloader;
	GstElement *rtp_sender;
	GstElement *udp_rtp_sink, *udp_rtcp_sink, *udp_rtcp_src;

	/* Initialize the pipeline. */
	pipeline      = gst_pipeline_new("Stream server.");
	source        = gst_element_factory_make("filesrc", "file source");
	decodebin     = gst_element_factory_make("decodebin2", "decode");
	convert       = gst_element_factory_make("audioconvert", "converter");
	volume        = gst_element_factory_make("volume", "vol-cntl");
	resample      = gst_element_factory_make("audioresample", "resampler");
	encoder       = gst_element_factory_make(ENCODER, "encoder");
	payloader     = gst_element_factory_make(PAYLOADER, "payloader");
	rtp_sender    = gst_element_factory_make("gstrtpbin", id);
	udp_rtp_sink  = gst_element_factory_make("udpsink", "rtp-output");
	udp_rtcp_sink = gst_element_factory_make("udpsink", "rtcp-output");
	udp_rtcp_src  = gst_element_factory_make("udpsrc", "rtcp-input");

	ASSERT_OR_ERROR(pipeline != NULL);
	ASSERT_OR_ERROR(source != NULL);
	ASSERT_OR_ERROR(decodebin != NULL);
	ASSERT_OR_ERROR(convert != NULL);
	ASSERT_OR_ERROR(volume != NULL);
	ASSERT_OR_ERROR(resample != NULL);
	ASSERT_OR_ERROR(encoder != NULL);
	ASSERT_OR_ERROR(payloader != NULL);
	ASSERT_OR_ERROR(rtp_sender != NULL);
	ASSERT_OR_ERROR(udp_rtp_sink != NULL);
	ASSERT_OR_ERROR(udp_rtcp_sink != NULL);
	ASSERT_OR_ERROR(udp_rtcp_src != NULL);

	/* Set a sane default volume. */
	g_object_set(G_OBJECT(volume), "volume", .1, NULL);
	
	/* Set the RTP UDP port. */
	g_object_set(G_OBJECT(udp_rtp_sink), "port", rtp,
		     "host", dest_host, NULL);
	g_object_set(G_OBJECT(udp_rtcp_sink), "port", rtcp,
		     "host", dest_host, NULL);
	g_object_set(G_OBJECT(udp_rtcp_src), "port", rtcp + 1, NULL);

	/* Add the elements to the pipeline and link what can be linked. */
	gst_bin_add_many(GST_BIN(pipeline),
			 source, decodebin, volume, convert, resample, encoder,
			 payloader, rtp_sender, udp_rtp_sink, NULL);
	gst_element_link(source, decodebin);
	gst_element_link_many(convert, volume, resample, encoder,
			      payloader, NULL);

	/* Make required requests for pads in the rtpbin. */
	rtp_sink  = gst_element_get_request_pad(rtp_sender, "send_rtp_sink_0");
	rtp_src   = gst_element_get_static_pad(rtp_sender, "send_rtp_src_0");
	rtcp_sink = gst_element_get_request_pad(rtp_sender, "recv_rtcp_sink_0");
	rtcp_src  = gst_element_get_request_pad(rtp_sender, "send_rtcp_src_0");

	/* Link in the RT(C)P stuff here; link the pads directly. */
	tmp = gst_element_get_static_pad(payloader, "src");
	gst_pad_link(tmp, rtp_sink);
	gst_object_unref(tmp);
	tmp = gst_element_get_static_pad(udp_rtp_sink, "sink");
	gst_pad_link(rtp_src, tmp);
	gst_object_unref(tmp);
	tmp = gst_element_get_static_pad(udp_rtcp_sink, "sink");
	gst_pad_link(rtcp_src, tmp);
	gst_object_unref(tmp);
	tmp = gst_element_get_static_pad(udp_rtcp_src, "src");
	gst_pad_link(tmp, rtcp_sink);
	gst_object_unref(tmp);
	gst_object_unref(rtp_sink);
	gst_object_unref(rtp_src);

	/* Dynamic pad creation for decoder. */
	g_signal_connect(decodebin, "pad-added", G_CALLBACK(new_pad_added),
			 convert);

	pipe->pipeline = pipeline;
	pipe->filesrc = source;
	pipe->volume = volume;
	pipe->rtpbin = rtp_sender;

	return 0;

}

/**
 * Build a main loop to run the RTP server in.
 */
int music_make_mloop(struct music_rtp_pipeline *pipe){

	GstBus *bus;

	pipe->mloop = g_main_loop_new(NULL, FALSE);
	
	/* Hook up callbacks for the bus/loop. */
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipe->pipeline));
	gst_bus_add_watch(bus, bus_callb, pipe->mloop);
	g_object_unref(bus);

	return 0;

}

/*
 * Handle changes in state on the bus.
 */
static gboolean bus_callb(GstBus *bus, GstMessage *msg, gpointer data){

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

static void new_pad_added(GstElement *elem, GstPad *pad, gpointer data){

	GstPad *sinkpad;
	GstElement *decoder = (GstElement *) data;

	/*
	 * Sink pad in the volume controller.
	 */
	sinkpad = gst_element_get_static_pad (decoder, "sink");
	gst_pad_link (pad, sinkpad);
	gst_object_unref (sinkpad);

}
