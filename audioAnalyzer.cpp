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
using namespace hiberlite;

static const int MAX_THREADS = 8;
static mutex fnMutex, dataMutex, printMutex;
static const string PATH_DATABASE = "/Users/Steve/Code/Git/testAudio/test.db";

AudioAnalyzer::AudioAnalyzer(deque<string>& toAnalyze)
{
    retrieve(toAnalyze);
}

AudioAnalyzer::AudioAnalyzer()
{
    
}

void AudioAnalyzer::retrieve(deque<string>& fileNames)
{
    Database db(PATH_DATABASE);
    db.registerBeanClass<AudioAnalysis>();
    db.dropModel();
    db.createModel();
    
    deque<bean_ptr<AudioAnalysis>> toAnalyze;
    for (auto fn : fileNames)
    {
        try
        {
            auto aa = buildBean(fn, db);
            if (aa->hasBeenAnalyzed())
            {
                analyzed.push_back(aa);
            }
            else
            {
               toAnalyze.push_back(aa);
            }
        }
        catch (ios_base::failure& e)
        {
            printException(e);
        }
        catch (database_error& e)
        {
            printException(e);
        }
    }
    
    vector<future<void>> futures;
    for (int i = 0; i < MAX_THREADS; ++i)
    {
        futures.push_back(async
                          (bind
                           (&AudioAnalyzer::analysisThread, this, _1, _2), ref(toAnalyze), ref(db)));
    }
    
    
    for (auto& f : futures)
    {
        f.get();
    }
    
}

void AudioAnalyzer::fileInDb(const string& fn, bool& inDb, long& id)
{
    
}
void AudioAnalyzer::analysisThread(deque<bean_ptr<AudioAnalysis>>& toAnalyze, Database& db)
{
    while (true)
    {
        fnMutex.lock();
        if (toAnalyze.empty())
        {
            fnMutex.unlock();
            break;
        }
        auto aa = toAnalyze.front();
        toAnalyze.pop_front();
        fnMutex.unlock();
        try
        {
            aa->analyzeBeats();
            aa->analyzeFade();
        }
        catch (EssentiaException& e)
        {
            printException(e);
        }
        aa->setBeenAnalyzed(true);
        dataMutex.lock();
        analyzed.push_back(aa);
        dataMutex.unlock();
    }
}

void AudioAnalyzer::printException(exception& e)
{
    printMutex.lock();
    cerr << "THIS IS AN EXCEPTION" << endl;
    cerr << e.what() << endl;
    cerr << "END EXCEPTION" << endl;
     printMutex.unlock();
}
bean_ptr<AudioAnalysis> AudioAnalyzer::buildBean(const string& fileName, Database& db)
{
    bean_ptr<AudioAnalysis> aa;
    bool inDb = false;
    long id = 0;
    
    fileInDb(fileName, inDb, id);
    
    if (inDb)
    {
        aa = db.loadBean<AudioAnalysis>(id);
    }
    else
    {
        aa = db.createBean<AudioAnalysis>();
        aa->setFileNameAndSize(fileName);
    }
    
    return aa;
}

/*
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
*/



void AudioAnalyzer::printData()
{
    for (auto d : analyzed)
    {
        d->print();
    }
}
HIBERLITE_EXPORT_CLASS(AudioAnalysis)

