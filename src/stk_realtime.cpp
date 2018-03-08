// rtsine.cpp STK tutorial program
#include "SineWave.h"
#include "RtWvOut.h"

#include "FileLoop.h"
#include "FileWvIn.h"
#include "FileWvOut.h"
#include "Fir.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
using namespace stk;

int main()
{
  // Set the global sample rate before creating class instances.
  Stk::setSampleRate( 44100.0 );
  Stk::showWarnings( true );
  int nFrames = 8820;
  SineWave sine;
  RtWvOut *dac = 0;
  try {
    // Define and open the default realtime output device for one-channel playback
    dac = new RtWvOut( 2 );
  }
  catch ( StkError & ) {
    exit( 1 );
  }
  sine.setFrequency( 441.0 );


  vector<vector<vector<double> > > HRIR(1440, vector<vector<double> >(2, vector<double>(2048)));  
  ifstream infile("hrir.txt");
  
  int lineCount = 0;
  string line;

  while (getline(infile, line)) {
    istringstream iss(line);
    for (int i = 0; i < 2048; ++ i){
      iss >> HRIR[lineCount][0][i];
      iss >> HRIR[lineCount][1][i];
    }
    lineCount ++;
  }

  vector<StkFloat> rightcoefficients(2048, 0); // create and initialize numerator coefficients
  vector<StkFloat> leftcoefficients(2048, 0); // create and initialize numerator coefficients
  
  rightcoefficients = HRIR[90][1];
  leftcoefficients = HRIR[90][0];

  Fir rfilter(rightcoefficients);
  Fir lfilter(leftcoefficients);


  vector<StkFloat> rightcoefficients_90(2048, 0);
  vector<StkFloat> leftcoefficients_90(2048, 0);
  rightcoefficients_90 = HRIR[270][1];
  leftcoefficients_90 = HRIR[270][0];

  Fir rfilter_90(rightcoefficients_90);
  Fir lfilter_90(leftcoefficients_90);


  // FileWvIn input;
  // FileWvOut output;

  // input.openFile( "despacito.wav", false );
  // StkFloat sampleRate = input.getFileRate();
  // unsigned long sampleSize = input.getSize();
  // cout << "Number of Sample points: " << sampleSize << endl;
  // cout << "SampleRate: " << sampleRate << endl;
  // cout << "Time: " << sampleSize/sampleRate << endl;


  StkFrames iframes( nFrames, 1 );
  StkFrames oframes( nFrames, 2 );

  // Option 1: Use StkFrames
  int sec = 0;
  while (1) {
    sine.tick(iframes);
    
    if (sec % 2 == 0) {
      rfilter.tick(iframes, oframes, 0, 1);
      lfilter.tick(iframes, oframes, 0, 0);
    } else {
      rfilter_90.tick(iframes, oframes, 0, 1);
      lfilter_90.tick(iframes, oframes, 0, 0);
    }

    try {
      dac->tick(oframes);
    }
    catch ( StkError & ) {
      goto cleanup;
    }
    sec ++;
  }
  
  // Option 2: Single-sample computations
  // for ( int i=0; i<nFrames; i++ ) {
  //   try {
  //     dac->tick( sine.tick() );
  //    }
  //   catch ( StkError & ) {
  //     goto cleanup;
  //   }
  // }
 cleanup:
  delete dac;
  return 0;
}