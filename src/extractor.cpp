/*
   @Author:McKelvin
   @Date:2011-11-01
   the implement of all the extractor here
   */
#include <extractor.h>
MarSystem * EXModel::totalExtrator(string sfName)
{
    mrs_natural memSize = 40;
    mrs_natural winSize = 512;
    mrs_natural hopSize = 512;
    mrs_real samplingRate_ = 44100.0;
    mrs_natural offset_time = 30;
    mrs_natural last_time = 30;
    mrs_natural accSize_ = last_time * samplingRate_ / winSize;
    mrs_natural offset = offset_time *samplingRate_;

    EXModel ex;

    MarSystem* src = mng.create("SoundFileSource", "src");
    MarSystem* featureNetwork = mng.create("Series", "featureNetwork");
    featureNetwork->addMarSystem(src);

    featureNetwork->addMarSystem(mng.create("Stereo2Mono", "s2m"));
    MarSystem* timbreFeaturesExtractor =ex.TimbreFeaturesExtractor();

    featureNetwork->addMarSystem(timbreFeaturesExtractor);

    featureNetwork->addMarSystem(mng.create("TextureStats", "tStats"));
    featureNetwork->updControl("TextureStats/tStats/mrs_natural/memSize", memSize);

    featureNetwork->updControl("SoundFileSource/src/mrs_string/filename", sfName);
    featureNetwork->updControl("mrs_natural/inSamples", MRS_DEFAULT_SLICE_NSAMPLES);


    MarSystem* acc = mng.create("Accumulator", "acc");
    acc->updControl("mrs_natural/nTimes", accSize_);

    acc->addMarSystem(featureNetwork);//remove ->clone();by Kelvin

    MarSystem* statistics = mng.create("Fanout", "statistics2");
    statistics->addMarSystem(mng.create("Mean", "mn"));
    statistics->addMarSystem(mng.create("StandardDeviation", "std"));

    MarSystem* timbre_total = mng.create("Series", "timbre_total");//Series/timbre_total/
    timbre_total->addMarSystem(acc);
    timbre_total->addMarSystem(statistics);

    timbre_total->updControl("mrs_natural/inSamples", winSize);


#ifdef BEAT
    MarSystem* beat_total = ex.createBeatHistogramFeatureNetwork();
    beat_total->updControl("Series/onset_strength/Accumulator/accum/Series/fluxnet/SoundFileSource/src/mrs_string/filename",sfName );
    beat_total->updControl("Series/onset_strength/Accumulator/accum/Series/fluxnet/SoundFileSource/src/mrs_natural/pos",offset);
#endif

    MarSystem* all = mng.create("Fanout/all");
    all->addMarSystem(timbre_total);

#ifdef BEAT
    all->addMarSystem(beat_total);
#endif

    all->updControl("Series/timbre_total/Accumulator/acc/Series/featureNetwork/SoundFileSource/src/mrs_string/filename", sfName);
    all->updControl("Series/timbre_total/Accumulator/acc/Series/featureNetwork/SoundFileSource/src/mrs_natural/pos",offset);
#ifdef BEAT
    all->updControl("Series/beat_total/Series/onset_strength/Accumulator/accum/Series/fluxnet/SoundFileSource/src/mrs_string/filename",sfName);
    all->updControl("Series/beat_total/Series/onset_strength/Accumulator/accum/Series/fluxnet/SoundFileSource/src/mrs_natural/pos",offset);
#endif

    return all;
}


MarSystem* EXModel::createBeatHistogramFeatureNetwork()//based code from bextract
{
    mrs_natural  bwinSize = 2048*5;//30s
    mrs_natural bhopSize = 128*80;
    mrs_natural cwinSize = 256;
    mrs_natural chopSize = 128;
    MarSystem *beatTracker = mng.create("Series/beat_total");
    MarSystem *onset_strength = mng.create("Series/onset_strength");
    MarSystem *accum = mng.create("Accumulator/accum");
    MarSystem *fluxnet = mng.create("Series/fluxnet");
    fluxnet->addMarSystem(mng.create("SoundFileSource", "src"));
    fluxnet->addMarSystem(mng.create("Stereo2Mono", "s2m"));
    fluxnet->addMarSystem(mng.create("ShiftInput", "si"));
    fluxnet->addMarSystem(mng.create("Windowing", "windowing1"));
    fluxnet->addMarSystem(mng.create("Spectrum", "spk"));
    fluxnet->addMarSystem(mng.create("PowerSpectrum", "pspk"));
    fluxnet->addMarSystem(mng.create("Flux", "flux"));
    accum->addMarSystem(fluxnet);

    onset_strength->addMarSystem(accum);
    onset_strength->addMarSystem(mng.create("ShiftInput/si2"));
    beatTracker->addMarSystem(onset_strength);
    MarSystem *tempoInduction = mng.create("FlowThru/tempoInduction");
    tempoInduction->addMarSystem(mng.create("Filter", "filt1"));
    tempoInduction->addMarSystem(mng.create("Reverse", "reverse"));
    tempoInduction->addMarSystem(mng.create("Filter", "filt2"));
    tempoInduction->addMarSystem(mng.create("Reverse", "reverse"));
    tempoInduction->addMarSystem(mng.create("Windowing", "windowing2"));
    tempoInduction->addMarSystem(mng.create("AutoCorrelation", "acr"));
    tempoInduction->addMarSystem(mng.create("BeatHistogram", "histo"));


    MarSystem* hfanout = mng.create("Fanout", "hfanout");
    hfanout->addMarSystem(mng.create("Gain", "id1"));
    hfanout->addMarSystem(mng.create("TimeStretch", "tsc1"));
    tempoInduction->addMarSystem(hfanout);
    tempoInduction->addMarSystem(mng.create("Sum", "hsum"));
    tempoInduction->addMarSystem(mng.create("BeatHistoFeatures", "bhf"));
    beatTracker->addMarSystem(tempoInduction);

    onset_strength->updControl("Accumulator/accum/mrs_natural/nTimes", bhopSize);
    onset_strength->updControl("ShiftInput/si2/mrs_natural/winSize",bwinSize);


    realvec bcoeffs(1,3);
    bcoeffs(0) = 0.0564;
    bcoeffs(1) = 0.1129;
    bcoeffs(2) = 0.0564;
    tempoInduction->updControl("Filter/filt1/mrs_realvec/ncoeffs", bcoeffs);
    tempoInduction->updControl("Filter/filt2/mrs_realvec/ncoeffs", bcoeffs);
    realvec acoeffs(1,3);
    acoeffs(0) = 1.0000;
    acoeffs(1) = -1.2247;
    acoeffs(2) = 0.4504;
    tempoInduction->updControl("Filter/filt1/mrs_realvec/dcoeffs", acoeffs);
    tempoInduction->updControl("Filter/filt2/mrs_realvec/dcoeffs", acoeffs);

    onset_strength->updControl("Accumulator/accum/Series/fluxnet/PowerSpectrum/pspk/mrs_string/spectrumType", "magnitude");
    onset_strength->updControl("Accumulator/accum/Series/fluxnet/Flux/flux/mrs_string/mode", "DixonDAFX06");

    tempoInduction->updControl("BeatHistogram/histo/mrs_natural/startBin", 0);//0
    tempoInduction->updControl("BeatHistogram/histo/mrs_natural/endBin", 800);//800
    tempoInduction->updControl("BeatHistogram/histo/mrs_real/factor", 16.0);//16.0

    tempoInduction->updControl("Fanout/hfanout/TimeStretch/tsc1/mrs_real/factor", 0.5);
    tempoInduction->updControl("Fanout/hfanout/Gain/id1/mrs_real/gain", 2.0);

    tempoInduction->updControl("AutoCorrelation/acr/mrs_real/magcompress", 0.65);

    onset_strength->updControl("Accumulator/accum/Series/fluxnet/ShiftInput/si/mrs_natural/winSize", cwinSize);

    beatTracker->updControl("mrs_natural/inSamples", chopSize );

    return beatTracker;
}


MarSystem* EXModel::TimbreFeaturesExtractor(){

    mrs_natural mfcc_c = 5;
    realvec ncoeffs(2);
    realvec dcoeffs(1);
    ncoeffs(0) = 1.0;
    ncoeffs(1) = -0.97;
    dcoeffs(0) = 1.0;
    ////////////////////////////////////////////////////////////////////
    // timbre_features prototype
    ////////////////////////////////////////////////////////////////////
    MarSystem* timbre_features_pr = mng.create("Fanout/timbre_features_pr");
    // TD branch
    MarSystem* timeDomainFeatures = mng.create("Series/timeDomain");
    timeDomainFeatures->addMarSystem(mng.create("ShiftInput/si"));
    MarSystem* tdf = mng.create("Fanout/tdf");
    tdf->addMarSystem(mng.create("ZeroCrossings/zcrs"));
    timeDomainFeatures->addMarSystem(tdf);
    timbre_features_pr->addMarSystem(timeDomainFeatures);
    // FFT branch
    MarSystem* spectralShape = mng.create("Series/spectralShape");
    spectralShape->addMarSystem(mng.create("ShiftInput/si"));
    spectralShape->addMarSystem(mng.create("Windowing/hamming"));
    spectralShape->addMarSystem(mng.create("PowerSpectrumNet1/powerSpect1"));


    // STFT_features prototype
    MarSystem* spectrumFeatures = mng.create("Fanout", "spectrumFeatures");
    spectrumFeatures->addMarSystem(mng.create("Centroid", "cntrd"));
    spectrumFeatures->addMarSystem(mng.create("Rolloff", "rlf"));
    spectrumFeatures->addMarSystem(mng.create("Flux", "flux"));
    spectrumFeatures->addMarSystem(mng.create("MFCC", "mfcc"));
    spectrumFeatures->updControl("MFCC/mfcc/mrs_natural/coefficients", mfcc_c);
    MarSystem* chromaPrSeries =  mng.create("Series", "chromaPrSeries");
    chromaPrSeries->addMarSystem(mng.create("Spectrum2Chroma", "chroma"));
    chromaPrSeries->addMarSystem(mng.create("PeakRatio","pr"));

    spectrumFeatures->addMarSystem(chromaPrSeries);
    spectrumFeatures->addMarSystem(mng.create("SCF", "scf"));//Spectral Crest Factor 24c
    spectrumFeatures->addMarSystem(mng.create("SFM", "sfm"));//Spectral Flatness Measure 24c
    spectralShape->addMarSystem(spectrumFeatures);
    timbre_features_pr->addMarSystem(spectralShape);

    //kelvin test: try use LPC instead of LPCC+LSP

    // LPC branch
    MarSystem* lpcFeatures = mng.create("Series", "lpcFeatures");
    lpcFeatures->addMarSystem(mng.create("Filter", "preEmph"));
    lpcFeatures->updControl("Filter/preEmph/mrs_realvec/ncoeffs", ncoeffs);
    lpcFeatures->updControl("Filter/preEmph/mrs_realvec/dcoeffs", dcoeffs);
    lpcFeatures->addMarSystem(mng.create("ShiftInput", "si"));
    lpcFeatures->addMarSystem(mng.create("Windowing", "ham"));
    MarSystem* lpcf = mng.create("Fanout", "lpcf");
    MarSystem* lspbranch = mng.create("Series", "lspbranch");
    MarSystem* lpccbranch = mng.create("Series","lpccbranch");
    lspbranch->addMarSystem(mng.create("LPC", "lpc"));
    lspbranch->updControl("LPC/lpc/mrs_natural/order", 18);//default 18
    lspbranch->addMarSystem(mng.create("LSP", "lsp"));
    lpccbranch->addMarSystem(mng.create("LPC", "lpc"));
    lpccbranch->updControl("LPC/lpc/mrs_natural/order", 12);//default 12
    lpccbranch->addMarSystem(mng.create("LPCC", "lpcc"));
    lpcf->addMarSystem(lspbranch);
    lpcf->addMarSystem(lpccbranch);
    lpcFeatures->addMarSystem(lpcf);



    timbre_features_pr->addMarSystem(lpcFeatures);

    timbre_features_pr->linkControl("Series/timeDomain/ShiftInput/si/mrs_natural/winSize", "mrs_natural/winSize");
    timbre_features_pr->linkControl("Series/spectralShape/ShiftInput/si/mrs_natural/winSize", "mrs_natural/winSize");
    timbre_features_pr->linkControl("Series/lpcFeatures/ShiftInput/si/mrs_natural/winSize", "mrs_natural/winSize");

    timbre_features_pr->linkControl("Series/spectralShape/Fanout/spectrumFeatures/mrs_string/enableChild", "mrs_string/enableSPChild");
    timbre_features_pr->linkControl("Series/spectralShape/Fanout/spectrumFeatures/mrs_string/disableChild","mrs_string/disableSPChild");
    timbre_features_pr->linkControl("Series/timeDomain/Fanout/tdf/mrs_string/enableChild", "mrs_string/enableTDChild");
    timbre_features_pr->linkControl("Series/timeDomain/Fanout/tdf/mrs_string/disableChild", "mrs_string/disableTDChild");
    timbre_features_pr->linkControl("Series/lpcFeatures/Fanout/lpcf/mrs_string/enableChild", "mrs_string/enableLPCChild");
    timbre_features_pr->linkControl("Series/lpcFeatures/Fanout/lpcf/mrs_string/disableChild", "mrs_string/disableLPCChild");

    //timbre_features_pr->updControl("mrs_string/disableSPChild", "all");//spectrumFeatures
    //timbre_features_pr->updControl("mrs_string/disableTDChild", "all");//timeDomain
    //timbre_features_pr->updControl("mrs_string/disableLPCChild", "all");//lpcFeatures

    return timbre_features_pr;
}
