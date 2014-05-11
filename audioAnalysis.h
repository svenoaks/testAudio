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
#include "hiberlite.h"


using namespace std;
using namespace essentia;
using namespace hiberlite;

typedef TNT::Array2D<Real> array2d;


class AudioAnalysis
{
public:
    bool hasValidData();
    void setBeenAnalyzed(bool);
    bool hasBeenAnalyzed();
    void setFileName(const string&);
    void setFileSize(long long);
    void analyzeBeats();
    void analyzeFade();
    void print();
private:
    friend class hiberlite::access;
    template<class Archive>
    void hibernate(Archive& ar);
    string fileName;
    long long fileSize;
    int beenAnalyzed;
    vector<Real> beatLocations;
    Real beatConfidence;
    Real bpm;
    array2d fadeInLocations;
    array2d fadeOutLocations;
};

template<class Archive>
void AudioAnalysis::hibernate(Archive& ar)
{
    ar& HIBERLITE_NVP(beatLocations);
    ar& HIBERLITE_NVP(beatConfidence);
    //ar& HIBERLITE_NVP(fadeInLocations);
    //ar& HIBERLITE_NVP(fadeOutLocations);
    ar& HIBERLITE_NVP(fileName);
    ar& HIBERLITE_NVP(fileSize);
    ar& HIBERLITE_NVP(bpm);
    ar& HIBERLITE_NVP(beenAnalyzed);
}
#endif /* defined(__testAudio__AudioAnalysis__) */
