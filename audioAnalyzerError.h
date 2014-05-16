    //
//  AudioAnalyzerError.h
//  testAudio
//
//  Created by Steve Myers on 5/14/14.
//  Copyright (c) 2014 Steve Myers. All rights reserved.
//

#ifndef testAudio_AudioAnalyzerError_h
#define testAudio_AudioAnalyzerError_h

class AudioAnalyzerError : public logic_error
{
public:
    AudioAnalyzerError(const string& what_arg0) : logic_error(what_arg0) {}
};

#endif
