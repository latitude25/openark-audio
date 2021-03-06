#pragma once 

#include "FileWvIn.h"
#include "FileWvOut.h"
#include "RtAudio.h"
#include "Fir.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <complex>

#include <string>
#include <cmath>
#include <vector>
#include <valarray>
#include <unordered_map>

using namespace std;
// using namespace stk;

class AudioComponent {
public:
  AudioComponent ();
  ~AudioComponent ();
  void loadHRIR(string fileName);
  void loadAudio(string audioFile);
  void minimumPhase(double r);


  void setOutput(std::string outputFile);
  void setRealTime();
  void startRealTime();
  void setAngle(int angle);
  void setDistance(int distance);
  double estimate_itd(double r, double theta);

  friend int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, 
                    double streamTime, RtAudioStreamStatus status, void *dataPointer);

private:
  unordered_map<int, vector<stk::Fir>> hrtf_set;
  vector<vector<vector<stk::StkFloat>>> HRIR;
  vector<vector<vector<stk::StkFloat>>> HRIR_minimumPhase;
  vector<double> itd;
  stk::StkFloat sampleRate;
  unsigned int channels;
  unsigned long sampleSize;

  int pre_angle;
  int cur_angle;
  int distance;

  stk::Fir lpre;
  stk::Fir rpre;
  stk::Fir lcur;
  stk::Fir rcur;


  stk::FileWvIn linput;
  stk::FileWvIn rinput;
  stk::FileWvOut output;
  bool isOpen;
  RtAudio dac;

};

typedef std::complex<double> Complex;
typedef std::valarray<Complex> ComplexArray;

int tick( void *, void *, unsigned int , double , RtAudioStreamStatus, void *);
void fft(ComplexArray&);
void ifft(ComplexArray&);

