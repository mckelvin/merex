/*
   @Author:McKelvin
   @Date:2011-11-01
   */
#include "marsyasbackend.h"

//here OUT_ROWS=6 = 1 prediction + 1 observation label + 4 class
#define OUT_ROWS 6
MarsyasBackend::MarsyasBackend()
{

}

void MarsyasBackend::extract(Collection collection, string outWekaName)
{
    MarSystemManager mng;
    EXModel exm;
    MarSystem* wsink = mng.create("WekaSink", "wsink");
    MarSystem* annotator = mng.create("Annotator", "annotator");
    MarSystem *all = exm.totalExtrator(collection.entry(0));

    //Extract MarSystem I/O variable definition
    realvec in;
    realvec timbreres;
    realvec fullres;
    realvec afullres;
    realvec beat;
    mrs_natural timbre_total_inO = all->getctrl("Series/timbre_total/mrs_natural/inObservations")->to<mrs_natural>(),
                timbre_total_inS = all->getctrl("Series/timbre_total/mrs_natural/inSamples")->to<mrs_natural>(),
                timbre_total_onO = all->getctrl("Series/timbre_total/mrs_natural/onObservations")->to<mrs_natural>(),
                timbre_total_onS = all->getctrl("Series/timbre_total/mrs_natural/onSamples")->to<mrs_natural>();

#ifdef BEAT
    mrs_natural beat_total_inO = all->getctrl("Series/beat_total/mrs_natural/inObservations")->to<mrs_natural>(),
                beat_total_inS = all->getctrl("Series/beat_total/mrs_natural/inSamples")->to<mrs_natural>(),
                beat_total_onO = all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_natural/onObservations")->to<mrs_natural>(),
                beat_total_onS = 0;//note:the last MarSystem isFlowThru. #all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_natural/onSamples")->to<mrs_natural>();
#else
    mrs_natural beat_total_inO = 0,
                beat_total_inS = 0,
                beat_total_onO = 0,
                beat_total_onS = 0;
#endif

    in.create(timbre_total_inO + beat_total_inO,
            timbre_total_inS +  beat_total_inS);
    timbreres.create(timbre_total_onO + beat_total_onO,
            timbre_total_onS +  beat_total_onS);
    beat.create(beat_total_onO,
            beat_total_onS);
    fullres.create(timbre_total_onO + beat_total_onO,
            timbre_total_onS + beat_total_onS  );
    afullres.create(timbre_total_onO + beat_total_onO + 1,
            timbre_total_onS + beat_total_onS );

    //extract MarSystem config
    annotator->updControl("mrs_natural/inObservations", timbre_total_onO
            + beat_total_onO);
    annotator->updControl("mrs_natural/inSamples", all->getctrl("mrs_natural/onSamples"));//FIXME:
    annotator->updControl("mrs_real/israte", all->getctrl("mrs_real/israte"));

    wsink->updControl("mrs_natural/inSamples", annotator->getctrl("mrs_natural/onSamples"));
    wsink->updControl("mrs_natural/inObservations", annotator->getctrl("mrs_natural/onObservations")->to<mrs_natural>());
    wsink->updControl("mrs_real/israte", annotator->getctrl("mrs_real/israte"));

    mrs_natural timbreSize = timbre_total_onO
        + beat_total_onO;

    annotator->updControl("mrs_string/inObsNames", all->getctrl("Series/timbre_total/mrs_string/onObsNames")->to<mrs_string>()
#ifdef BEAT
            + all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_string/onObsNames")->to<mrs_string>()
#endif
            );

    if (outWekaName != EMPTYSTRING)// FIXED:    the action must before wsink->updControl mrs_string/filename
        wsink->updControl("mrs_string/inObsNames", annotator->getctrl("mrs_string/onObsNames"));

#ifndef SVR
    string all_files_in_collection = "nn,np,pn,pp";
    wsink->updControl("mrs_string/labelNames",all_files_in_collection);
    wsink->updControl("mrs_natural/nLabels", 4/*(mrs_natural)collection.getSize()*/);
    wsink->updControl("mrs_string/filename", outWekaName);
    cout << "Writing weka .arff file to :" << outWekaName << endl;
    cout << "------------------------------" << endl;
    cout << "Label names" << endl;
    cout << wsink->getctrl("mrs_string/labelNames")->to<mrs_string>() << endl;
    cout << "------------------------------\n" << endl;
#else
    annotator->updControl("mrs_string/mode","real_label");
    wsink->updControl("mrs_bool/regression",true);
    wsink->updControl("mrs_string/filename", outWekaName);
    cout<<"Try Regression\n";
#endif

    //config done and re-construct "all" again,so here is a delete.it's a system fault/bug fix.
    //but this might be not nessaray,but "delete all" near line 137 is a must. --McKelvin
    delete all;

    cerr<<"Start Extract Work\n";
    stringstream ss;
    for (size_t i=0; i < collection.size(); ++i)
    {
        all = exm.totalExtrator(collection.entry(i));
#ifndef SVR
        int labelnum=-1;
        //order:nn,np,pn,pp
        if(collection.labelEntry(i) =="nn")
            labelnum=0;
        else if(collection.labelEntry(i)=="np")
            labelnum=1;
        else if(collection.labelEntry(i)=="pn")
            labelnum=2;
        else if(collection.labelEntry(i)=="pp")
            labelnum=3;
        if (labelnum!=-1)
            annotator->updControl("mrs_natural/label", labelnum);
#else
        ss<<collection.labelEntry(i);//mA mV stdA stdV
        ss>>mA>>mV;
        ss.str(std::string());
        annotator->updControl("mrs_real/rlabel",mA);
#endif

        all->process(in, timbreres);
#ifdef BEAT
        beat = all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_realvec/processedData")->to<mrs_realvec>();
#endif
        // concatenate timbre and beat vectors
        mrs_natural tempcount = 0;
        for (int t=0; t < timbreSize; t++){
            if(t<timbre_total_onO)
                fullres(t,0) = timbreres(t,0);
            else
                fullres(t,0) = beat(tempcount++,0);
        }
        //cout<<fullres;
        annotator->process(fullres, afullres);
        wsink->process(afullres, afullres);

        cerr << i+1 <<"/"<< collection.size()<<" Processed " << collection.entry(i) << endl;
        delete all;

    }
    delete annotator;
    delete wsink;
}

void MarsyasBackend::extract_single(string sfName, string outWekaName="single_tmp.arff")
{
    cout<<"extract_single_file:"<<sfName<<endl;
    MarSystemManager mng;
    EXModel exm;
    MarSystem* all = exm.totalExtrator(sfName);
    MarSystem* wsink = mng.create("WekaSink", "wsink");

    realvec in;
    realvec timbreres;
    realvec fullres;
    realvec afullres;
    realvec beat;

    mrs_natural timbre_total_inO = all->getctrl("Series/timbre_total/mrs_natural/inObservations")->to<mrs_natural>(),
                timbre_total_inS = all->getctrl("Series/timbre_total/mrs_natural/inSamples")->to<mrs_natural>(),
                timbre_total_onO = all->getctrl("Series/timbre_total/mrs_natural/onObservations")->to<mrs_natural>(),
                timbre_total_onS = all->getctrl("Series/timbre_total/mrs_natural/onSamples")->to<mrs_natural>();

#ifdef BEAT
    mrs_natural beat_total_inO = all->getctrl("Series/beat_total/mrs_natural/inObservations")->to<mrs_natural>(),
                beat_total_inS = all->getctrl("Series/beat_total/mrs_natural/inSamples")->to<mrs_natural>(),
                beat_total_onO = all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_natural/onObservations")->to<mrs_natural>(),
                beat_total_onS = 0;//all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_natural/onSamples")->to<mrs_natural>();
#else
    mrs_natural beat_total_inO = 0,
                beat_total_inS = 0,
                beat_total_onO = 0,
                beat_total_onS = 0;
#endif

    in.create(timbre_total_inO + beat_total_inO,
            timbre_total_inS +  beat_total_inS);

    timbreres.create(timbre_total_onO + beat_total_onO,
            timbre_total_onS +  beat_total_onS);

    beat.create(beat_total_onO,
            beat_total_onS);

    fullres.create(timbre_total_onO + beat_total_onO + 1,
            timbre_total_onS + beat_total_onS  );

    afullres.create(timbre_total_onO + beat_total_onO + 1,
            timbre_total_onS + beat_total_onS );


    wsink->updControl("mrs_natural/inSamples", all->getctrl("mrs_natural/onSamples"));
    wsink->updControl("mrs_natural/inObservations", timbre_total_onO + beat_total_onO + 1);
    wsink->updControl("mrs_real/israte", all->getctrl("mrs_real/israte"));

    mrs_natural timbreSize = timbre_total_onO
        + beat_total_onO;

    if (outWekaName != EMPTYSTRING)
    {
        wsink->updControl("mrs_string/inObsNames",all->getctrl("Series/timbre_total/mrs_string/onObsNames")->to<mrs_string>()
#ifdef BEAT
                + all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_string/onObsNames")->to<mrs_string>()
#endif
                );
    }
    realvec iwin;
#ifndef SVR
    wsink->updControl("mrs_natural/nLabels", 1);
    wsink->updControl("mrs_string/labelNames","unknown");
    wsink->updControl("mrs_string/filename", outWekaName);
#else
    wsink->updControl("mrs_bool/regression",true);
    wsink->updControl("mrs_string/filename", outWekaName);
#endif
    all->process(in, timbreres);

#ifdef BEAT
    beat = all->getctrl("Series/beat_total/FlowThru/tempoInduction/BeatHistoFeatures/bhf/mrs_realvec/processedData")->to<mrs_realvec>();
#endif
    // concatenate timbre and beat vectors
    mrs_natural tempcount = 0;
    int t;
    for (t=0; t < timbreSize; t++){
        if(t<timbre_total_onO)
            fullres(t,0) = timbreres(t,0);
        else
            fullres(t,0) = beat(tempcount++,0);
    }
    fullres(t,0)=0;
    wsink->process(fullres, afullres);

    delete all;
    delete wsink;
}

void MarsyasBackend::train_and_predict(string trainFileName,string testFileName,MOOD_TYPE &predicte_mood)
{
    MarSystemManager mng;
    //start get norm MM values,by Kelvin
    MarSystem* pre = mng.create("WekaSource/pre");
    pre->updctrl("mrs_string/filename", trainFileName);
    pre->updctrl("mrs_natural/inSamples", 1);
    int scount=0,fcount=0;
    realvec indata;
    realvec indatas;

    while (!pre->getctrl("mrs_bool/done")->isTrue()){
        pre->tick();
        indata = pre->getControl("mrs_realvec/processedData")->to<mrs_realvec>();
#ifdef SVR
        //indata has 424 rows(422 features + 1 output + 1 strange *the value of last row is always "-1"* ),1 column, why?
        indata.stretch(423,1);
#endif
        if(fcount == 0){
            fcount=indata.getRows();//fount D. features;
            indatas.stretch(fcount,10);//init. with 10 samples
        }
        indatas.stretchWrite(0,scount,0);
        indatas.setCol(scount,indata);
        scount++;
    }
    //fcount:423(422features+1output) rows | features,scount:60 columns/songs

    indatas.stretch(fcount,scount);
    //linear normalization[0 to fcount-1 rows] exclude the label row

    realvec obsrow(scount);
    realvec mindif;
    mindif.create(fcount-1,2);
    mrs_real min,max,dif;

    for(mrs_natural r=0;r<fcount-1;r++)
    {
        indatas.getRow(r,obsrow);
        min = obsrow.minval();
        max = obsrow.maxval();
        dif = max - min;
        if(dif == 0)
            dif = 1.0;
        for(mrs_natural c=0;c<scount;c++)
        {
            indatas(r,c) -= min;
            indatas(r,c) /= dif;
        }
        mindif(r,0)=min;
        mindif(r,1)=dif;
        //cout<<min<<"\t"<<dif<<endl;
    }

#ifdef SVR
    //try to +10 to labels ;Kelvin ,oct.2nd
    for(mrs_natural c=0;c<scount;c++)
    {
        indatas(fcount-1,c) += 10;
    }
#endif
    MarSystem* cl = mng.create("Classifier", "cl");
    string cl_ = "SVM";
    if (cl_ == "GS")
        cl->updctrl("mrs_string/enableChild", "GaussianClassifier/gaussiancl");
    if (cl_ == "ZEROR")
        cl->updctrl("mrs_string/enableChild", "ZeroRClassifier/zerorcl");
    if (cl_ == "SVM")
        cl->updctrl("mrs_string/enableChild", "SVMClassifier/svmcl");
    cl->updControl("mrs_string/mode", "train");
    // TODO:svr config
#ifdef SVR
    cl->updControl("SVMClassifier/svmcl/mrs_string/svm","NU_SVR");
    cl->updControl("SVMClassifier/svmcl/mrs_natural/cache_size",40000);//libsvm default:100
#else
    cl->updControl("mrs_natural/nClasses",pre->getctrl("mrs_natural/nClasses"));
    vector<string> classNames;
    string s = pre->getctrl("mrs_string/classNames")->to<mrs_string>();
    char *str = (char *)s.c_str();
    char * pch;
    pch = strtok (str,",");
    classNames.push_back(pch);
    while (pch != NULL) {
        pch = strtok (NULL, ",");
        if (pch != NULL)
            classNames.push_back(pch);
    }
#endif

    cl->updControl("mrs_natural/inObservations",pre->getctrl("mrs_natural/onObservations"));
    cl->updControl("mrs_natural/inSamples",pre->getctrl("mrs_natural/onSamples"));
    //config done
    int i=0;
    realvec outdata;
    outdata.create(OUT_ROWS,1);

    for (i = 0; i < scount; i++) {
        indatas.getCol(i,indata);
        cl->process(indata,outdata);
    }


    //cerr<<"train done\n";
    /////////////////////////////////////////
    //  Start Predicting
    //
    pre->updControl("mrs_string/filename", testFileName);
    cl->updControl("mrs_string/mode", "predict");
    cl->updControl("mrs_natural/inObservations",pre->getctrl("mrs_natural/onObservations"));
    cl->updControl("mrs_natural/inSamples",pre->getctrl("mrs_natural/onSamples"));

    realvec predict_data;

    while (!pre->getctrl("mrs_bool/done")->to<mrs_bool>()) {
        pre->tick();
        predict_data = pre->getctrl("mrs_realvec/processedData")->to<mrs_realvec>();
        predict_data.stretch(fcount,1);
        // TODO :norm to predict_data
        for(mrs_natural i=0;i<fcount-1;i++){
            predict_data(i) -= mindif(i,0);
            predict_data(i) /= mindif(i,1);
        }
        realvec final_data;
        final_data.create(OUT_ROWS,1);//test
        cl->process(predict_data,final_data);
        //cerr<<final_data;
#ifdef SVR
        cout<<"Predict as"<<final_data(0,0) << endl;
#else
        predicte_mood = (MOOD_TYPE)final_data(0,0);
        cout<<"Predict as:" <<predicte_mood<<"="<< classNames[predicte_mood] << endl;
#endif
        //  cout << data(0,0) << endl;
    }
}
