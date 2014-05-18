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
#include "audioAnalyzerError.h"

using namespace std;
using namespace essentia;
using namespace essentia::scheduler;
using namespace hiberlite;

static const int MIN_NUM_VALID_BEATS = 60;
static const int MIN_CONFIDENCE = 1.5;

float AudioAnalysis::beatBeforeFadeOutIfPresent(float timeBeforeEnd)
{
    static const int BEG_OF_FADE_OUT = 0;
    
    if (!hasValidData())
    {
        throw new AudioAnalyzerError("AudioAnalysis does not have valid data");
    }
    
    float lastBeat = beatLocations.at(beatLocations.size() - 1);
    float requiredTime;
    if (fadeOutLocations.empty())
    {
        requiredTime = lastBeat - timeBeforeEnd;
    }
    else
    {
        float fadeOutBegTime = fadeOutLocations.at(fadeOutLocations.size() - 1).at(BEG_OF_FADE_OUT);
        requiredTime = fadeOutBegTime - timeBeforeEnd;
    }
    
    for (vector<Real>::reverse_iterator rit = beatLocations.rbegin(); rit != beatLocations.rend(); ++rit)
    {
        if (*rit < requiredTime)
        {
            return *rit;
        }
    }
    
    return lastBeat;
    
}
float AudioAnalysis::beatAfterFadeInIfPresent(float timeIntoSong)
{
    static const int FIRST_FADE_IN = 0;
    static const int FIRST_BEAT = 0;
    static const int END_OF_FADE_IN = 1;
    
    if (!hasValidData())
    {
        throw new AudioAnalyzerError("AudioAnalysis does not have valid data");
    }
    
    float endFade  = fadeInLocations.empty() ? FIRST_BEAT : fadeInLocations.at(FIRST_FADE_IN).at(END_OF_FADE_IN);
    float time = endFade + timeIntoSong;
    for (float beat : beatLocations)
    {
        if (beat > time)
        {
            return beat;
        }
    }
    return beatLocations.at(FIRST_BEAT);
}

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
                                                     "minLength", 5.0,
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
    << "SOME DATA : " << beatLocations[0] << "  " << beatLocations[1] << "  "
    << beatLocations[2] << "  " << beatLocations[3] << endl
    << "END DATA" << endl;
    
}
