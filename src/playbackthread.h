/*
   @Author:McKelvin
   @Date:2011-11-04
   backend thread implements here
   */
#ifndef THREADBACKEND_H
#define THREADBACKEND_H
#include <QThread>
#include "marsyasbackend.h"

class PlaybackThread :public QThread
{
    Q_OBJECT
    public:
        PlaybackThread(QString name="");
        void run();
        void stop();
        QString audioFile;
        string song,song_weka,train_weka;
        MOOD_TYPE getMood();
        static void msleep( unsigned long msecs );
    private:
        MarsyasBackend mb;
        volatile bool stoped;
        QString name;
        MOOD_TYPE result;
signals:
        void predict_result(int mood);
};

#endif // THREADBACKEND_H
