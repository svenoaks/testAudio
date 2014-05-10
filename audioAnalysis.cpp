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

using namespace std;
using namespace essentia;
using namespace streaming;
using namespace essentia::scheduler;

typedef TNT::Array2D<Real> array2d;

static const int MIN_NUM_VALID_BEATS = 60;


AudioAnalysis::AudioAnalysis(const string& fileName) : fileName(fileName)
{
    fileSize = calculateFileSize(fileName);
    cout << fileSize << endl;
}

ifstream::pos_type AudioAnalysis::calculateFileSize(const string& filename)
{
    ifstream in;
    ios_base::io_state em = in.exceptions() | ios::failbit | ios::badbit;
    in.exceptions(em);
    in.open(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}
bool AudioAnalysis::isInDb(sqlite3*)
{
    return false;
}

bool AudioAnalysis::hasValidData()
{
    return beatLocations.size() >= MIN_NUM_VALID_BEATS;
}

void AudioAnalysis::analyzeFade()
{
    int sr = 44100;
    int framesize = sr/4;
    int hopsize = 256;
    Real frameRate = Real(sr)/Real(hopsize);
    
    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
    
    standard::Algorithm* audio = factory.create("MonoLoader",
                                      "filename", fileName,
                                      "sampleRate", sr);
    
    standard::Algorithm* frameCutter = factory.create("FrameCutter",
                                            "frameSize", framesize,
                                            "hopSize", hopsize);
    
    standard::Algorithm* rms = factory.create("RMS");
    
    standard::Algorithm* fadeDetect = factory.create("FadeDetection",
                                           "minLength", 3.,
                                           "cutoffHigh", 0.85,
                                           "cutoffLow", 0.20,
                                           "frameRate", frameRate);
    
    // create a pool for fades' storage:
    Pool pool;
    
    // set audio:
    vector<Real> audio_mono;
    audio->output("audio").set(audio_mono);
    
    // set frameCutter:
    vector<Real> frame;
    frameCutter->input("signal").set(audio_mono);
    frameCutter->output("frame").set(frame);
    
    // set rms:
    Real rms_value;
    rms->input("array").set(frame);
    rms->output("rms").set(rms_value);
    
    // we need a vector to store rms values:
    std::vector<Real> rms_vector;
    
    // load audio:
    audio->compute();
    
    // compute and store rms first and will compute fade detection later:
    while (true) {
        frameCutter->compute();
        if (frame.empty())
            break;
        
        rms->compute();
        rms_vector.push_back(rms_value);
    }
    
    // set fade detection:
    array2d fade_in;
    array2d fade_out;
    fadeDetect->input("rms").set(rms_vector);
    fadeDetect->output("fadeIn").set(fade_in);
    fadeDetect->output("fadeOut").set(fade_out);
    
    // compute fade detection:
    fadeDetect->compute();
    
    // Exemplifying how to add/retrieve values from the pool in order to output them into stdout
    cout << "FADE INS: " << fade_in.dim1() << endl;
    cout << "FADE  OUTS: " << fade_out.dim1() << endl;
    
    delete audio;
    delete frameCutter;
    delete rms;
    delete fadeDetect;
}
void AudioAnalysis::analyzeBeats()
{
    
    Pool pool;
    
    int sr = 44100;
    int framesize = sr/4;
    int hopsize = 256;
    Real frameRate = Real(sr)/Real(hopsize);


    
    AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    
    //Algorithms owned by Network n.
    
    Algorithm* audio = factory.create("MonoLoader",
                                      "filename", fileName,
                                      "sampleRate", sr);
    
    
    Algorithm* bt    = factory.create("BeatTrackerMultiFeature");
    /*
    Algorithm* fc = factory.create("FrameCutter",
                                            "frameSize", framesize,
                                            "hopSize", hopsize);
    
    Algorithm* rms = factory.create("RMS");
    
    Algorithm* fd = factory.create("FadeDetection",
                                           "minLength", 3.,
                                           "cutoffHigh", 0.85,
                                           "cutoffLow", 0.20,
                                           "frameRate", frameRate);
    */
    
    
    audio->output("audio")    >>  bt->input("signal");
    bt->output("confidence")  >>  PC(pool, "confidence.bpm");
    bt->output("ticks")       >>  PC(pool, "beats.bpm");
    /*
    audio->output("audio")    >>  fc->input("signal");
    fc->output("frame")       >>  rms->input("array");
    
    rms->output("rms")        >>  NOWHERE;
    fd->output("fadeIn")      >>  PC(pool, "fadein.fde");
    fd->output("fadeOut")     >>  PC(pool, "fadeout.fde");
    */
    Network n(audio);
    n.run();
    
    n.clear();
    
    //fadeInLocations = move(pool.value<TNT::Array2D<Real>>("fadein.fde"));
    //fadeOutLocations = move(pool.value<TNT::Array2D<Real>>("fadeout.fde"));
    beatLocations = move(pool.value<vector<Real>>("beats.bpm"));
    beatConfidence = pool.value<Real>("confidence.bpm");
    
}

static string writePKeyStatement = "insert into Track(fileName, fileSize) values (?, ?);";

void AudioAnalysis::writeToDb(sqlite3* db)
{
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, writePKeyStatement.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, fileName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, to_string(fileSize).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int error = sqlite3_finalize(stmt);
    cout << "ERROR CODE: " << error << endl;
}

void AudioAnalysis::retrieveFromDb(sqlite3* db)
{
    
}


void AudioAnalysis::print()
{
    cout << "FILENAME: " << fileName << endl
    << "FILESIZE: " << fileSize << endl
    //<< "CONFIDENCE: " << beatConfidence << endl
    //<< "SOME DATA : " << beatLocations[0] << "  " << beatLocations[1] << "  "
    //<< beatLocations[2] << "  " << beatLocations[3] << endl
    << "END DATA" << endl;
    
}