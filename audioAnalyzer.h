//
//  audioAnalyzer.h
//  testAudio
//
//  Created by Steve Myers on 5/3/14.
//  Copyright (c) 2014 Steve Myers. All rights reserved.
//

#ifndef __testAudio__audioAnalyzer__
#define __testAudio__audioAnalyzer__

#include "essentia.h"
#include <iostream>
#include <vector>
#include <string>
#include <audioAnalysis.h>
#include <memory>
#include <deque>
#include <sqlite3.h>
#include "hiberlite.h"

using namespace std;

class AudioAnalyzer
{
public:
    AudioAnalyzer();
    AudioAnalyzer(vector<string>&);
    void nextSplicePoint(float, float, float&, float&);
    void retrieve(vector<string>&);
    void sort();
    void printData();
private:
    class EssentiaInitializer
    {
    public:
        EssentiaInitializer() { essentia::init(); };
        ~EssentiaInitializer() { essentia::shutdown(); };
    };
    void fileInDb(const string&, ifstream::pos_type, bool&, long&);
    void pushBackAnalyzed(bean_ptr<AudioAnalysis>);
    sqlite3* openDb(string fileName);
    void printException(exception&);
    vector<bean_ptr<AudioAnalysis>> analyzed;
    bean_ptr<AudioAnalysis> buildBean(const string&, hiberlite::Database&);
    deque<bean_ptr<AudioAnalysis>> buildVectorToAnalyze(Database&, vector<string>&);
    void analysisThread(deque<bean_ptr<AudioAnalysis>>&, Database&);
    ifstream::pos_type calculateFileSize(const string& filename);
};

#endif /* defined(__testAudio__audioAnalyzer__) */
