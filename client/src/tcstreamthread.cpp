#include "tcstreamthread.h"
#include <QtDebug>

tcStreamThread::tcStreamThread(QObject *parent)
    : QThread(parent)
    , mpStream(NULL)
    , mpPollTimer(NULL)
    , mbStarted(false)
{
}

tcStreamThread::~tcStreamThread()
{
	if(mpStream != NULL)
	{
		delete mpStream;
	}

	if(mpPollTimer != NULL)
	{
		mpPollTimer->stop();
		delete mpPollTimer;
	}
}

void tcStreamThread::run()
{
    // Setup the underlying stream
    mpStream = new tcStreamWrapper(msSongData.gsPath);
    connect(this, SIGNAL(s_setPlayState(int)), mpStream, SLOT(setPlayState(int)));
    connect(this, SIGNAL(s_setVolume(int)), mpStream, SLOT(setVolume(int)));
    connect(this, SIGNAL(s_setPosition(qint64)), mpStream, SLOT(setPosition(qint64)));
    //mpStream->setPlayState(msControls.gePlayState);
    //mpStream->setPosition(mnPosition);
    //mpStream->setVolume(msControls.gnVolume);

    // Setup our poll timer
    mpPollTimer = new QTimer();
    mpPollTimer->setSingleShot(false);
    mpPollTimer->setInterval(STREAM_POLL_INTERVAL);
    connect(mpPollTimer, SIGNAL(timeout()), this, SLOT(poll()));
    mpPollTimer->start();

    mbStarted = true;
    // Start this threads event loop
    exec();

    mpPollTimer->stop();
    disconnect(mpPollTimer, SIGNAL(timeout()), this, SLOT(poll()));
    delete mpPollTimer;
    mpPollTimer = NULL;

    disconnect(this, SIGNAL(s_setPlayState(int)), mpStream, SLOT(setPlayState(int)));
    disconnect(this, SIGNAL(s_setVolume(int)), mpStream, SLOT(setVolume(int)));
    disconnect(this, SIGNAL(s_setPosition(qint64)), mpStream, SLOT(setPosition(qint64)));
    delete mpStream;
    mpStream = NULL;
    mbStarted = false;

}

void tcStreamThread::syncData(tsUserData* user_data)
{
    msSongData = user_data->gsSongData;
    msControls = user_data->gsControlData;
    mnPosition = user_data->gnPosition;
}

void tcStreamThread::poll()
{
    if(mpStream == NULL)
    {
        return;
    }
    qint64 position = mpStream->position();
    qint64 length = mpStream->length();
    emit songInfo(position, length);
    qDebug() << position;

    if(mpStream->end_of_stream() /*|| position >= length*/)
    {
        qDebug() << "Song Over";
        quit();
        emit songOver();
        while(mbStarted);
    }
}

void tcStreamThread::checkUser(tsUserData* user_data)
{
    // Check if our song changed
    if(msSongData.gnHash != user_data->gsSongData.gnHash
            || msSongData.gsPath != user_data->gsSongData.gsPath)
    {
        if(isRunning())
        {
            quit();
            while(mbStarted);
        }

        syncData(user_data);
        start();
        while(!mbStarted);
        emit s_setPlayState(user_data->gsControlData.gePlayState);
        emit s_setPosition(user_data->gnPosition);
        emit s_setVolume(user_data->gsControlData.gnVolume);
        return;
    }

    // Check if our play state changed
    if(msControls.gePlayState != user_data->gsControlData.gePlayState)
    {
        if(msControls.gePlayState == eeStop)
        {
            syncData(user_data);
            start();
            while(!mbStarted);
            emit s_setPlayState(user_data->gsControlData.gePlayState);
            emit s_setPosition(user_data->gnPosition);
            emit s_setVolume(user_data->gsControlData.gnVolume);
            return;
        }
        else if(user_data->gsControlData.gePlayState == eeStop)
        {
            quit();
            while(mbStarted);
        }
        else
        {
            emit s_setPlayState(user_data->gsControlData.gePlayState);
        }
    }

    // Check if volume changed
    if(msControls.gnVolume != user_data->gsControlData.gnVolume)
    {
        emit s_setVolume(user_data->gsControlData.gnVolume);
    }

    // Sync our local data to the users
    syncData(user_data);
}
