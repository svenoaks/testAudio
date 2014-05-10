//
//  AudioAnalysis.h
//  testAudio
//
//  Created by Steve Myers on 5/3/14.
//  Copyright (c) 2014 Steve Myers. All rights reserved.
//

#ifndef __testAudio__AudioAnalysis__
#define __testAudio__AudioAnalysis__

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <types.h>
#include <tnt2vector.h>
#include <sqlite3.h>


using namespace std;
using namespace essentia;

typedef TNT::Array2D<Real> array2d;

class AudioAnalysis
{
public:
    AudioAnalysis(const string& fileName);
    bool hasValidData();
    bool isInDb(sqlite3*);
    void retrieveFromDb(sqlite3*);
    void writeToDb(sqlite3*);
    void analyzeBeats();
    void analyzeFade();
    void print();
private:
    string fileName;
    long fileSize;
    bool dataValid;
    
    vector<Real> beatLocations;
    Real beatConfidence;
    array2d fadeInLocations;
    array2d fadeOutLocations;
    
    ifstream::pos_type calculateFileSize(const string& filename);

};


#endif /* defined(__testAudio__AudioAnalysis__) */
