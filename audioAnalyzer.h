//
//  audioAnalyzer.h
//  testAudio
//
//  Created by Steve Myers on 5/3/14.
//  Copyright (c) 2014 Steve Myers. All rights reserved.
//

#ifndef __testAudio__audioAnalyzer__
#define __testAudio__audioAnalyzer__

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
    AudioAnalyzer(deque<string>&);
    void retrieve(deque<string>&);
    void sort();
    void printData();
private:
    void fileInDb(const string&, ifstream::pos_type, bool&, long&);
    sqlite3* openDb(string fileName);
    void printException(exception&);
    vector<bean_ptr<AudioAnalysis>> analyzed;
    bean_ptr<AudioAnalysis> buildBean(const string&, hiberlite::Database&);
    void analysisThread(deque<bean_ptr<AudioAnalysis>>&, Database&);
    ifstream::pos_type calculateFileSize(const string& filename);
};

#endif /* defined(__testAudio__audioAnalyzer__) */
