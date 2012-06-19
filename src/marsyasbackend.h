/*
   @Author:McKelvin
   @Date:2011-11-04

   */
#ifndef MARSYASBACKEND_H
#define MARSYASBACKEND_H

#include <fstream>
#include <sstream>
#include <marsyas/Collection.h>
#include <marsyas/MarSystemManager.h>
#include "config.h"
#include "extractor.h"

using namespace std;
using namespace Marsyas;
class MarsyasBackend
{
    public:
        MarsyasBackend();
        void extract(Collection collection, string outWekaName);
        void extract_single(string sfName, string outWekaName);
        void train_and_predict(string trainFileName,string testFileName,MOOD_TYPE &predicte_mood);
        void playback(string audio_file);
};

#endif // MARSYASBACKEND_H
