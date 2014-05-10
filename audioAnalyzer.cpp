//
//  audioAnalyzer.cpp
//  testAudio
//
//  Created by Steve Myers on 5/3/14.
//  Copyright (c) 2014 Steve Myers. All rights reserved.
//

#include "audioAnalyzer.h"
#include <sqlite3.h>
#include <future>
#include <mutex>
#include <functional>
#include <deque>
#include <array>


using namespace std;
using namespace std::placeholders;

static const int NUM_THREADS = 8;
static mutex m;
static const string PATH_DATABASE = "/Users/Steve/Code/Git/testAudio/test.db";

AudioAnalyzer::AudioAnalyzer(const vector<string>& fileNames)
{
    for (auto& fn : fileNames)
    {
        try
        {
            data.push_back(shared_ptr<AudioAnalysis>(new AudioAnalysis(fn)));
        }
        catch (ios_base::failure& e)
        {
            cerr << e.what() << endl;
        }
        
    }
}

void AudioAnalyzer::analysisThread(deque<shared_ptr<AudioAnalysis>>& aq, sqlite3* db)
{
    while (true)
    {
        m.lock();
        if (aq.empty())
        {
            m.unlock();
            break;
        }
        auto aa = aq.front();
        aq.pop_front();
        m.unlock();
        
        if(aa->isInDb(db)) aa->retrieveFromDb(db);
        
        if (!aa->hasValidData()) //testing
        {
            try
            {
                //aa->analyzeBeats();
                aa->analyzeFade();
            }
            catch (EssentiaException& e)
            {
                cerr << "THIS IS AN EXCEPTION" << endl;
                cerr << e.what() << endl;
                cerr << "END EXCEPTION" << endl;
                
            }
            aa->writeToDb(db);
        }
    }
}


sqlite3* AudioAnalyzer::openDb(string fileName)
{
    sqlite3* db;
    //sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    int error = sqlite3_open_v2(fileName.c_str(), &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX, nullptr);
    
    if (error)
    {
        //throw
    }
    cout << "SUCCESS? : " << error << endl;
    return db;
}

void AudioAnalyzer::retrieve()
{
    sqlite3* db = openDb(PATH_DATABASE);
    deque<shared_ptr<AudioAnalysis>>tempData(data);
    vector<future<void>> futures;
        for (int i = 0; i < NUM_THREADS; ++i)
    {
        futures.push_back(async
                          (bind
                           (&AudioAnalyzer::analysisThread, this, _1, _2), ref(tempData), db));
    }
    
    for (auto& f : futures)
    {
        f.get();
    }
    sqlite3_close(db);
}



void AudioAnalyzer::printData()
{
    for (auto d : data)
    {
        d->print();
    }
}

