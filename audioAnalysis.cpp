//
//  AudioAnalysis.cpp
//  testAudio
//
//  Created by Steve Myers on 5/3/14.
//  Copyright (c) 2014 Steve Myers. All rights reserved.
//

#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include "audioAnalysis.h"
#include "algorithmfactory.h"
#include "poolstorage.h"
#include "tnt2vector.h"
#include "network.h"
#include "hiberlite.h"

using namespace std;
using namespace essentia;
using namespace essentia::scheduler;
using namespace hiberlite;

static const int MIN_NUM_VALID_BEATS = 60;
static const int MIN_CONFIDENCE = 1.5;


void AudioAnalysis::setFileName(const string& fileName)
{
    this->fileName = fileName;
}

void AudioAnalysis::setFileSize(long long fileSize)
{
    this->fileSize = fileSize;
}

void AudioAnalysis::setBeenAnalyzed(bool value)
{
    beenAnalyzed = value;
}

bool AudioAnalysis::hasBeenAnalyzed()
{
    return beenAnalyzed;
}
bool AudioAnalysis::hasValidData()
{
    return beatLocations.size() >= MIN_NUM_VALID_BEATS &&
            beatConfidence >= MIN_CONFIDENCE;
}

void AudioAnalysis::analyzeFade()
{
    using namespace standard;
    
    int sr = 44100;
    int framesize = sr/4;
    int hopsize = 256;
    Real frameRate = Real(sr)/Real(hopsize);
    
    
    AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
    
    unique_ptr<Algorithm> audio{factory.create("MonoLoader",
                                               "filename", fileName,
                                               "sampleRate", sr)};
    
    unique_ptr<Algorithm> frameCutter{factory.create("FrameCutter",
                                                     "frameSize", framesize,
                                                     "hopSize", hopsize)};
    
    unique_ptr<Algorithm> rms{factory.create("RMS")};
    
    unique_ptr<Algorithm> fadeDetect {factory.create("FadeDetection",
                                                     "minLength", 3.,
                                                     "cutoffHigh", 0.85,
                                                     "cutoffLow", 0.20,
                                                     "frameRate", frameRate)};
    
    
    vector<Real> audio_mono;
    audio->output("audio").set(audio_mono);
    
    vector<Real> frame;
    frameCutter->input("signal").set(audio_mono);
    frameCutter->output("frame").set(frame);
    
    Real rms_value;
    rms->input("array").set(frame);
    rms->output("rms").set(rms_value);
    
    vector<Real> rms_vector;
    
    audio->compute();
    
    while (true) {
        frameCutter->compute();
        if (frame.empty())
            break;
        
        rms->compute();
        rms_vector.push_back(rms_value);
    }
    
    array2d fade_in;
    array2d fade_out;
    fadeDetect->input("rms").set(rms_vector);
    fadeDetect->output("fadeIn").set(fade_in);
    fadeDetect->output("fadeOut").set(fade_out);
    
    fadeDetect->compute();
    
    if (fade_in.dim1() > 0) fadeInLocations = array2DToVecvec(fade_in);
    if (fade_out.dim1() > 0) fadeOutLocations = array2DToVecvec(fade_out);
}
void AudioAnalysis::analyzeBeats()
{
    using namespace streaming;
    
    Pool pool;
    
    int sr = 44100;

    AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    
    //Algorithms owned by Network n.
    
    Algorithm* audio = factory.create("MonoLoader",
                                      "filename", fileName,
                                      "sampleRate", sr);
    
    Algorithm* bt    = factory.create("RhythmExtractor2013");
    
    audio->output("audio")     >>  bt->input("signal");
    bt->output("confidence")   >>  PC(pool, "confidence.bpm");
    bt->output("ticks")        >>  PC(pool, "beats.bpm");
    bt->output("bpm")          >>  PC(pool, "bpm.bpm");
    bt->output("estimates")    >>  NOWHERE;
    bt->output("bpmIntervals") >>  NOWHERE;
    
    Network n(audio);
    n.run();
    
    n.clear();
    
    beatLocations = move(pool.value<vector<Real>>("beats.bpm"));
    beatConfidence = pool.value<Real>("confidence.bpm");
    bpm = pool.value<Real>("bpm.bpm");
}

void AudioAnalysis::print()
{
    cout << "FILENAME: " << fileName << endl
    << "FILESIZE: " << fileSize << endl
    << "CONFIDENCE: " << beatConfidence << endl
    << "bpm : " << bpm << endl
    << "SOME DATA : " << beatLocations.at(0) << "  " << beatLocations.at(1) << "  "
    << beatLocations.at(2) << "  " << beatLocations.at(3) << endl
    << "END DATA" << endl;
    
}
