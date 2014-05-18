//
//  audioAnalyzer.cpp
//  testAudio
//
//  Created by Steve Myers on 5/3/14.
//  Copyright (c) 2014 Steve Myers. All rights reserved.
//

#include "audioAnalyzer.h"
#include "audioAnalyzerError.h"
#include "essentia.h"
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
static const string PATH_DATABASE = "/Users/Steve/Code/Git/testAudio/test.db";
static mutex fnMutex, dataMutex, printMutex;


AudioAnalyzer::AudioAnalyzer(vector<string>& fileNames) : AudioAnalyzer()
{
    retrieve(fileNames);
}

AudioAnalyzer::AudioAnalyzer()
{
        
}

ifstream::pos_type AudioAnalyzer::calculateFileSize(const string& filename)
{
    ifstream in;
    ios_base::io_state em = in.exceptions() | ios::failbit | ios::badbit;
    in.exceptions(em);
    in.open(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}

deque<bean_ptr<AudioAnalysis>>AudioAnalyzer::buildVectorToAnalyze(Database& db, vector<string>& fileNames)
{
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
    return toAnalyze;
}

void AudioAnalyzer::retrieve(vector<string>& fileNames)
{
    EssentiaInitializer init{};
    Database db{ PATH_DATABASE };
    db.registerBeanClass<AudioAnalysis>();
    //db.dropModel();
    //db.createModel();
    
    auto toAnalyze = buildVectorToAnalyze(db, fileNames);
    
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

void AudioAnalyzer::nextSplicePoint(float timeEnd, float timeBegin, float& firstSongValue, float& secondSongValue)
{
    static const int MIN_SONGS = 2;
    
    auto firstSong = analyzed.at(0);
    auto secondSong = analyzed.at(1);
    
    if (analyzed.size() < MIN_SONGS)
    {
        throw AudioAnalyzerError("Can't splice with less than two songs");
    }
    if (!firstSong->hasValidData() || !secondSong->hasValidData())
    {
        throw AudioAnalyzerError("One or more of the songs do not have valid data");
    }
    
    firstSongValue = firstSong->beatBeforeFadeOutIfPresent(timeEnd);
    secondSongValue = secondSong->beatAfterFadeInIfPresent(timeBegin);
}

void AudioAnalyzer::fileInDb(const string& fn, ifstream::pos_type fs, bool& inDb, long& id)
{
    static const string sqlIdStatement = "select hiberlite_id from AudioAnalysis where fileName = ? and fileSize = ?;";
    
    sqlite3* rawDb;
    sqlite3_stmt* stmt;
    
    sqlite3_open_v2(PATH_DATABASE.c_str(), &rawDb, SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX,nullptr);
    sqlite3_prepare_v2(rawDb, sqlIdStatement.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, fn.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, to_string(fs).c_str(), -1, SQLITE_TRANSIENT);
    int code = sqlite3_step(stmt);
    if (code == SQLITE_ROW)
    {
        inDb = true;
        id = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(rawDb);
    
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
        pushBackAnalyzed(aa);
    }
}

inline void AudioAnalyzer::pushBackAnalyzed(bean_ptr<AudioAnalysis> aa)
{
    lock_guard<mutex> lock{dataMutex};
    analyzed.push_back(aa);
}
void AudioAnalyzer::printException(exception& e)
{
    lock_guard<mutex> lock{printMutex};
    cerr << "THIS IS AN EXCEPTION" << endl;
    cerr << e.what() << endl;
    cerr << "END EXCEPTION" << endl;
}

bean_ptr<AudioAnalysis> AudioAnalyzer::buildBean(const string& fileName, Database& db)
{
    bean_ptr<AudioAnalysis> aa;
    bool inDb = false;
    long id = 0;
    ifstream::pos_type fileSize = calculateFileSize(fileName);
    
    fileInDb(fileName, fileSize, inDb, id);
    
    if (inDb)
    {
        aa = db.loadBean<AudioAnalysis>(id);
    }
    else
    {
        aa = db.createBean<AudioAnalysis>();
        aa->setFileName(fileName);
        aa->setFileSize(fileSize);
    }
    
    return aa;
}


sqlite3* AudioAnalyzer::openDb(string fileName)
{
    sqlite3* db;
    int error = sqlite3_open_v2(fileName.c_str(), &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX,nullptr);
    
    if (error)
    {
        throw database_error("Database error code: " + to_string(error));
    }
    return db;
}




void AudioAnalyzer::printData()
{
    for (auto d : analyzed)
    {
        d->print();
    }
}
HIBERLITE_EXPORT_CLASS(AudioAnalysis)

