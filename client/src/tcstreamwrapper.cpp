#include "tcstreamwrapper.h"

#include <QtDebug>
#include <gst/gst.h>
#include <glib.h>

//extern GMainLoop *music_mloop;
tcStreamWrapper::tcStreamWrapper(QString song, QObject *parent)
	: QObject(parent)
{
	/* The pipeline to hold everything */
	mpPLine = MUSIC_ALLOC_PIPELINE();
    int err = music_make_pipeline(mpPLine, "test-pipe");
    if ( err ){
        fprintf(stderr, "Could not make pipeline. :(\n");
    }
    else
    {
        // start the song
        err = music_play_song(mpPLine, (char*)song.toAscii().data());
        if(err)
        {
            qDebug() << QString("ERROR: Failed to play song %1").arg(song);
        }
    }
}

tcStreamWrapper::~tcStreamWrapper()
{
    music_set_state(mpPLine, GST_STATE_NULL);
	music_destroy_pipeline(mpPLine);
    delete mpPLine;
}

qint64 tcStreamWrapper::position()
{
    if(mpPLine == NULL)
    {
        return -1;
    }

    return music_get_time_pos(mpPLine);
}

qint64 tcStreamWrapper::length()
{
    if(mpPLine == NULL)
    {
        return -1;
    }

    return music_get_time_len(mpPLine);
}

bool tcStreamWrapper::end_of_stream()
{
    if(mpPLine == NULL)
        return false;
#ifndef WIN32
	return mpPLine->endofstream;
#else
	// Windows Qt doesn't hook into the gst main loop
	// so we need to poll it and see if there is an
	// end of line message posted
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(mpPLine->pipeline));
	return gst_bus_pop_filtered(bus, GST_MESSAGE_EOS) != NULL;
#endif
}

void tcStreamWrapper::setVolume(int vol)
{
    if(vol >= 0  && vol <= 100)
    {
        music_set_volume(mpPLine, vol);
    }
}

void tcStreamWrapper::setPlayState(int play_state)
{
    switch((tePlayState)play_state)
    {
        case eePlay:
            music_set_state(mpPLine, GST_STATE_PLAYING);
            while(music_get_state(mpPLine) != GST_STATE_PLAYING);
            break;
        case eePause:
            music_set_state(mpPLine, GST_STATE_PAUSED);
            break;
    }
}

void tcStreamWrapper::setPosition(qint64 pos)
{
    if(pos > 0)
    {
        if(!music_seek(mpPLine, pos))
        {
            qDebug() << QString("ERROR: Skip Failed");
        }
    }
}
