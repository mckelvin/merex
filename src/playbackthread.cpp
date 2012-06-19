/*
   @Author:McKelvin
   @Date:2011-11-01
  */
#include "playbackthread.h"

PlaybackThread::PlaybackThread(QString name)
{
    stoped = false;
    this->name = name;
}

void PlaybackThread::run()
{
    if(this->name=="PLAY"){

        stoped = false;
        MarSystemManager mng;
        //Playback
        MarSystem *playbacknet = mng.create("Series","playbacknet");
        //SoundFileSource
        playbacknet->addMarSystem(mng.create("SoundFileSource","src"));
        //Gain
        playbacknet->addMarSystem(mng.create("Gain","gt"));
        //AudioSink
        playbacknet->addMarSystem(mng.create("AudioSink","dest"));

        std::string sfName = this->audioFile.toUtf8().data();
        playbacknet->updctrl("SoundFileSource/src/mrs_string/filename",sfName);
        playbacknet->updctrl("SoundFileSource/src/mrs_natural/pos",30*44100);//start at 30s;
        playbacknet->updctrl("AudioSink/dest/mrs_bool/initAudio",true);

        if(sfName!="")
            while(playbacknet->getctrl("SoundFileSource/src/mrs_bool/hasData")->isTrue()&&!stoped){
                playbacknet->tick();
            }
        delete playbacknet;
    }else if(this->name=="PREDICT"){
        mb.extract_single(song,song_weka);
        mb.train_and_predict(train_weka,song_weka,result);
        emit predict_result(this->result);
    }
}


void PlaybackThread::stop()
{
    stoped=true;
}

MOOD_TYPE PlaybackThread::getMood()
{
    return this->result;
}

void PlaybackThread::msleep( unsigned long msecs )
{
    QThread::msleep(msecs);
}
