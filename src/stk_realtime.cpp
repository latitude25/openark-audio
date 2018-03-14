// rtsine.cpp STK tutorial program
#include "SineWave.h"
#include "RtWvOut.h"

#include "FileLoop.h"
#include "FileWvIn.h"
#include "FileWvOut.h"
#include "Filter.h"
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
  rightcoefficients = HRIR[0][1];
  leftcoefficients = HRIR[0][0];

  vector<StkFloat> rightcoefficients_90(2048, 0);
  vector<StkFloat> leftcoefficients_90(2048, 0);
  rightcoefficients_90 = HRIR[60][1];
  leftcoefficients_90 = HRIR[60][0];
  
  vector<StkFloat> rightcoefficients_180(2048, 0);
  vector<StkFloat> leftcoefficients_180(2048, 0);
  rightcoefficients_180 = HRIR[120][1];
  leftcoefficients_180 = HRIR[120][0];
  
  vector<StkFloat> rightcoefficients_270(2048, 0);
  vector<StkFloat> leftcoefficients_270(2048, 0);
  rightcoefficients_270 = HRIR[180][1];
  leftcoefficients_270 = HRIR[180][0];
  
  Fir rfilter(rightcoefficients);
  Fir lfilter(leftcoefficients);
  rfilter.setGain(0.5);
  lfilter.setGain(0.5);

  Fir rfilter_90(rightcoefficients_90);
  Fir lfilter_90(leftcoefficients_90);
  rfilter_90.setGain(0.5);
  lfilter_90.setGain(0.5);
  
  Fir rfilter_180(rightcoefficients_180);
  Fir lfilter_180(leftcoefficients_180);
  rfilter_180.setGain(0.5);
  lfilter_180.setGain(0.5);

  Fir rfilter_270(rightcoefficients_270);
  Fir lfilter_270(leftcoefficients_270);
  rfilter_270.setGain(0.5);
  lfilter_270.setGain(0.5);
  // FileWvIn input;
  // FileWvOut output;

  // input.openFile( "despacito.wav", false );
  // StkFloat sampleRate = input.getFileRate();
  // unsigned long sampleSize = input.getSize();
  // cout << "Number of Sample points: " << sampleSize << endl;
  // cout << "SampleRate: " << sampleRate << endl;
  // cout << "Time: " << sampleSize/sampleRate << endl;


  int cross_size = 44100/40; //crossfade of 25ms
  StkFrames currframes( nFrames, 1 );
  StkFrames nextframes(cross_size, 1);
  StkFrames prevframes(cross_size, 1);

  StkFrames convframes(nFrames + cross_size, 1);
  StkFrames oframes( nFrames + cross_size, 2 );


  // Option 1: Use StkFrames
  int sec = 0, i;
  while (1) {
  	
	if (sec == 0) {
    	sine.tick(currframes);
		sine.tick(nextframes);
	
		for (i = 0; i < nFrames; ++ i) {
			convframes[i] = currframes[i];
		}
		for (i = 0; i < cross_size; ++ i) {
			convframes[i + nFrames] = nextframes[i];
		}
	} else {
		for (i = 0; i < cross_size; ++i) {
			convframes[i] = prevframes[i];
		}
		sine.tick(currframes);
		sine.tick(nextframes);
		for (i = 0; i < nFrames - cross_size; ++i) {
			convframes[i + cross_size] = currframes[i];
		}
		for (i = 0; i < cross_size; ++i) {
			convframes[i + nFrames] = nextframes[i];
		}
	}
	
	//sine.tick(convframes);
	//sine.addTime(-1 * cross_size);
	//iframes.resize(nFrames + cross_size);
  	/*
	for (i = 0; i < cross_size; ++i) {
		iframes(i + nFrames, 0) = nextframes(i, 0);
		iframes(i + nFrames, 1) = nextframes(i, 1);
	}
	*/
	
	//corssframes();  
    if (sec % 4 == 0) {
      rfilter.tick(convframes, oframes, 0, 1);
      lfilter.tick(convframes, oframes, 0, 0);
    } else if (sec % 4 == 1) {
      rfilter_90.tick(convframes, oframes, 0, 1);
      lfilter_90.tick(convframes, oframes, 0, 0);
    
    } else if (sec % 4 == 2) {
      rfilter_180.tick(convframes, oframes, 0, 1);
      lfilter_180.tick(convframes, oframes, 0, 0);
    
    } else {
      rfilter_270.tick(convframes, oframes, 0, 1);
      lfilter_270.tick(convframes, oframes, 0, 0);
    }

    try {
	  oframes.resize(nFrames, 2);
      dac->tick(oframes);
	  oframes.resize(nFrames + cross_size, 2);
    }
    catch ( StkError & ) {
      goto cleanup;
    }
	if (sec == 0) {
		currframes.resize(nFrames - cross_size);
		prevframes = nextframes;
	}
	else {
		prevframes = nextframes;
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
