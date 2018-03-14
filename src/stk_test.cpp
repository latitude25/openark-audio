// sineosc.cpp
#include "FileLoop.h"
#include "FileWvIn.h"
#include "FileWvOut.h"
#include "RtWvOut.h"
#include "Fir.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;
using namespace stk;

unordered_map<int, vector<Fir> > hrtf_set;

void conv_audio(FileWvIn& input, FileWvOut& output, int angle, StkFrames& iframes, StkFrames& oframes) {
  size_t padding = 1000;
  // iframes.resize(iframes.size() + padding);
  // oframes.resize(oframes.size() + padding, 2);
  // cout << "iframes size: " << iframes.size() << endl;
  input.tick(iframes, 0);
  // cout << "First" << endl;
  hrtf_set[angle][1].tick(iframes, oframes, 0, 1);
  hrtf_set[angle][0].tick(iframes, oframes, 0, 0);
  // oframes.resize(oframes.size() - padding, 2);
  // iframes.resize(iframes.size() - padding);
  // input.addTime(-1 * padding);
  output.tick(oframes);
}


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
  unsigned int channels = input.channelsOut();
  unsigned long sampleSize = input.getSize();
  cout << "Number of Sample points: " << sampleSize << endl;
  cout << "SampleRate: " << sampleRate << endl;
  cout << "channels: " << channels << endl;
  cout << "Time: " << sampleSize/sampleRate << endl;

  

  for (int i = 0; i < lineCount; ++i) {
  	vector<StkFloat> rightcoefficients(2048, 0); // create and initialize numerator coefficients
  	vector<StkFloat> leftcoefficients(2048, 0); // create and initialize numerator coefficients
  	rightcoefficients = HRIR[i][1];
  	leftcoefficients = HRIR[i][0];
	vector<Fir> nfilter;
	nfilter.emplace_back(Fir(leftcoefficients));
	nfilter.emplace_back(Fir(rightcoefficients));
	hrtf_set[i] = nfilter;
  }


  // vector<StkFloat> coefficients(5, 0); // create and initialize numerator coefficients
//  vector<StkFloat> rightcoefficients(2048, 0); // create and initialize numerator coefficients
//  vector<StkFloat> leftcoefficients(2048, 0); // create and initialize numerator coefficients
  auto rightcoefficients = HRIR[270][1];
  auto leftcoefficients = HRIR[270][0];
  Fir rfilter(rightcoefficients);
  Fir lfilter(leftcoefficients);

  //vector<StkFloat> rightcoefficients_(2048, 0); // create and initialize numerator coefficients
  //vector<StkFloat> leftcoefficients_(2048, 0); // create and initialize numerator coefficients
  auto rightcoefficients_ = HRIR[240][1];
  auto leftcoefficients_ = HRIR[240][0];
  Fir rfilter_(rightcoefficients_);
  Fir lfilter_(leftcoefficients_);

  
  // Open a 16-bit, two-channel WAV formatted output file
  output.openFile( "despacito_conv.wav", 2, FileWrite::FILE_WAV, Stk::STK_SINT16 );
  // input.setFrequency( 440.0 );
  // Run the oscillator for 40000 samples, writing to the output file

  RtWvOut *dac = 0;
  try {
    // Define and open the default realtime output device for two-channel playback
    dac = new RtWvOut( 2 );
  }
  catch ( StkError & ) {
    exit( 1 );
  }


  StkFrames iframes(sampleRate/2, channels);
  StkFrames oframes(sampleRate/2, 2);

  for (int angle = 0; angle < 360; angle += 15) {
    conv_audio(input, output, angle, iframes, oframes);
    dac->tick(oframes);
    // conv_audio(input, output, 240, iframes, oframes);
    // conv_audio(input, output, 210, iframes, oframes);
  }
  
  cout << "Finish" << endl;
  dac->stop();
  
  cout << "Delete" << endl;

  input.closeFile();
  cout << "HERE" << endl;
 //  for ( int i=0; i<200000; i++ ) {
 //    output.tick( input.tick() );
	// }
  return 0;
}
