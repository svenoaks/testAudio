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

using namespace std;



class AudioAnalyzer
{
public:
    AudioAnalyzer(const vector<string>&);
    enum class dataValidity { VALID, INVALID_ANALYSIS_FAILED, INVALID_NOT_ENOUGH_DATA };
    void retrieve();
    void sort();
    void printData();
private:
    deque<shared_ptr<AudioAnalysis>>data;
    void analysisThread(deque<shared_ptr<AudioAnalysis>>& analysisQueue, sqlite3*);
    sqlite3* openDb(string);
};

#endif /* defined(__testAudio__audioAnalyzer__) */
