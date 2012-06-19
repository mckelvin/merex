/*
   @Author:McKelvin
   @Date:2011-11-01
   all feature extractors here
   */
#ifndef EXTRACTOR_H
#define EXTRACTOR_H
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <config.h>
#include <marsyas/Collection.h>
#include <marsyas/MarSystemManager.h>
#include <marsyas/CommandLineOptions.h>
using namespace Marsyas;
using namespace std;

class  EXModel{
    MarSystemManager mng;
    public:
    EXModel(){}

    MarSystem * totalExtrator(string sfName=EMPTYSTRING);

    MarSystem* createBeatHistogramFeatureNetwork();

    MarSystem* TimbreFeaturesExtractor();
};

#endif //EXTRACTOR_H
