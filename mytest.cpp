// sineosc.cpp
#include "FileLoop.h"
#include "FileWvIn.h"
#include "FileWvOut.h"
#include "Fir.h"
#include <cstdio>
#include <iostream>

using namespace std;
using namespace stk;

int main()
{
  // Set the global sample rate before creating class instances.
  // Stk::setSampleRate( 48000.0 );
  FileWvIn input;
  FileWvOut output;

  input.openFile( "despacito.wav", false );
  StkFloat sampleRate = input.getFileRate();
  unsigned long sampleSize = input.getSize();
  cout << "Number of Sample points: " << sampleSize << endl;
  cout << "SampleRate: " << sampleRate << endl;
  cout << "Time: " << sampleSize/sampleRate << endl;

  // Load the sine wave file.
  
  // Open a 16-bit, one-channel WAV formatted output file
  output.openFile( "hellosine.wav", 2, FileWrite::FILE_WAV, Stk::STK_SINT16 );
  // input.setFrequency( 440.0 );
  // Run the oscillator for 40000 samples, writing to the output file
  StkFrames frames(96000, 2);

  // for (int i = 0; i < )
  input.tick(frames, 1);
  // input.tick(frames, 0);
  // input.tick(frames, 1);
  output.tick(frames);

  for ( int i=0; i<200000; i++ )
    output.tick( input.tick() );
  return 0;
}
