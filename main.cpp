
#include <iostream>

#include "audioAnalyzer.h"
#include <vector>
#include <ctime>
using namespace std;


int main(int argc, char* argv[])
{
    vector<string> fns = {"/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/01 Scrood Bi U.mp3",
                            
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/02 Lover Boy _ Lover Girl.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/03 Rover Take Over.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/05 Slave To Love.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/06 Sex Bomb.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/07 Take Off.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/08 Stripper.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/09 Lucy's Fucking Sky.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/10 (A Treatise On The Practical Methods Whereby One Can) Worship The Lords.mp3",
         "/Users/Steve/Music/iTunes/iTunes Media/Music/Lords of Acid/Farstucker/11 A Ride With Satans Little Helpers.mp3",
    };
    
    time_t start, end;
    time(&start);
    
    
    AudioAnalyzer a;
    
    a.retrieve(fns);
    
    a.printData();
    
    time(&end);
    cout << difftime(end, start) << " seconds" << std::endl;
    float f, s;
    a.nextSplicePoint(10.0f, 10.0f, f, s);
    cout << "FIRST: " << f << " SECOND: " << s << endl;
    return 0;
}

