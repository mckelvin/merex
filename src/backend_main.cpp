/*
   @Author:McKelvin
   @Date:2011-11-01
   test marsyas backend
   */
#include <config.h>

#ifdef TEST
#include <marsyasbackend.h>
using namespace Marsyas;

    int
__not_main(int argc, const char **argv)
{
    MarsyasBackend mb;
    string location="/media/Media/mirex_music/mc_fix/mc_fix_";

#ifdef TRAIN
    Collection collection;
    collection.read(location+"all.mf");
    mb.extract(collection,location +"tmp.arff");
#else
    mb.extract_single("/media/Media/Music/Just Jack/All Night Cinema/03-THE DAY I DIED.mp3","single__tmp.arff");
    MOOD_TYPE t;
    mb.train_and_predict(location +"tmp.arff","single__tmp.arff",t);
    cout<<"tt:"<<t;
#endif
}
#endif

