// sineosc.cpp
#include "FileLoop.h"
#include "FileWvIn.h"
#include "FileWvOut.h"
#include "Fir.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
using namespace stk;




int main()
{
  // Set the global sample rate before creating class instances.
  // Stk::setSampleRate( 48000.0 );
  vector<vector<vector<double> > > HRIR(1440, vector<vector<double> >(2, vector<double>(2048)));  
  ifstream infile("hrir.txt");
  
  int lineCount = 0;
  string line;

  while (getline(infile, line)) {
    istringstream iss(line);
    // int a, b;
    // if (!(iss >> a >> b)) { break; } // error
    
    for (int i = 0; i < 2048; ++ i){
      iss >> HRIR[lineCount][0][i];
      iss >> HRIR[lineCount][1][i];
    }
    lineCount ++;
  }


  FileWvIn input;
  FileWvOut output;

  input.openFile( "despacito.wav", false );
  StkFloat sampleRate = input.getFileRate();
  unsigned long sampleSize = input.getSize();
  cout << "Number of Sample points: " << sampleSize << endl;
  cout << "SampleRate: " << sampleRate << endl;
  cout << "Time: " << sampleSize/sampleRate << endl;


  // vector<StkFloat> coefficients(5, 0); // create and initialize numerator coefficients
  vector<StkFloat> rightcoefficients(2048, 0); // create and initialize numerator coefficients
  vector<StkFloat> leftcoefficients(2048, 0); // create and initialize numerator coefficients
  // vector<StkFloat> denominator;         // create empty denominator coefficients
  // denominator.push_back( 1.0 );              // populate our denomintor values
  // denominator.push_back( 0.3 );
  // denominator.push_back( -0.5 );
  
  // for (int i = 0; i < 2048; i ++) {
    rightcoefficients = HRIR[270][1];
    leftcoefficients = HRIR[270][0];
  // }


  Fir rfilter(rightcoefficients);
  Fir lfilter(leftcoefficients);


  
  // Open a 16-bit, one-channel WAV formatted output file
  output.openFile( "hellosine.wav", 2, FileWrite::FILE_WAV, Stk::STK_SINT16 );
  // input.setFrequency( 440.0 );
  // Run the oscillator for 40000 samples, writing to the output file
  StkFrames iframes(2*sampleRate, 1);
  StkFrames oframes(2*sampleRate, 2);



  input.tick(iframes, 0);
  
  rfilter.tick(iframes, oframes, 0, 1);
  lfilter.tick(iframes, oframes, 0, 0);  
  output.tick(oframes);

  for ( int i=0; i<200000; i++ )
    output.tick( input.tick() );
  return 0;
}
