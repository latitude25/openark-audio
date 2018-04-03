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
#include <thread>
#include <future>
#include <chrono>
#include <atomic>

using namespace std;
using namespace stk;
using namespace std::literals;

class AudioComponent {

public:

  AudioComponent () {
    HRIR = vector<vector<vector<double>>>(1440, vector<vector<double> >(2, vector<double>(2048)));
  }

  ~AudioComponent () {
    cout << "Destruct AudioComponent\n";
    delete dac;
    input.closeFile();
  }

  void loadHRIR(string fileName) {
    int lineCount = 0;
    string line;

    ifstream infile(fileName);
    while (getline(infile, line)) {
      istringstream iss(line);
      
      for (int i = 0; i < 2048; ++ i){
        iss >> HRIR[lineCount][0][i];
        iss >> HRIR[lineCount][1][i];
      }
      lineCount ++;
    }
  }

  void loadAudio(string audioFile) {
    input.openFile(audioFile, false );

    sampleRate = input.getFileRate();
    channels = input.channelsOut();
    sampleSize = input.getSize();
    
    cout << "Number of Sample points: " << sampleSize << endl;
    cout << "SampleRate: " << sampleRate << endl;
    cout << "channels: " << channels << endl;
    cout << "Time: " << sampleSize/sampleRate << endl;
  }

  void setOutput(string outputFile) {
    try {
      // Define and open the default realtime output device for two-channel playback
      dac = new RtWvOut(2);
    }
    catch ( StkError & ) {
      exit( 1 );
    }
    output.openFile(outputFile, 2, FileWrite::FILE_WAV, Stk::STK_SINT16 );

  }

  void setAngle(int angle) {
    cur_angle = angle;
  }

  bool renderNextFrame() {
    if (input.isFinished())
      return false;
    StkFrames iframes(sampleRate/20, channels);
    StkFrames oframes(sampleRate/20, 2);

    conv_audio_no_crossfade(input, output, iframes, oframes);
    dac->tick(oframes);
    // cout << "Finish" << endl;
  }
  void renderAudio() {
    // std::thread acthread(&conv_audio_no_crossfade, this)
    StkFrames iframes(sampleRate/20, channels);
    StkFrames oframes(sampleRate/20, 2);

    while (!input.isFinished()) {
      conv_audio_no_crossfade(input, output, iframes, oframes);
      dac->tick(oframes);
    }
    conv_audio_no_crossfade(input, output, iframes, oframes);
    dac->tick(oframes);
    cout << "Finish" << endl;
    // dac->stop();
  }


  inline void conv_audio_no_crossfade(FileWvIn& input, FileWvOut& output, StkFrames& iframes, StkFrames& oframes) {
    input.tick(iframes, 0);

    lcur.setCoefficients(HRIR[cur_angle][0]);
    rcur.setCoefficients(HRIR[cur_angle][1]);

    lcur.tick(iframes, oframes, 0, 0);
    rcur.tick(iframes, oframes, 0, 1);
    output.tick(oframes);
    
  }


private:
  unordered_map<int, vector<Fir>> hrtf_set;
  vector<vector<vector<double>>> HRIR;
  StkFloat sampleRate;
  unsigned int channels;
  unsigned long sampleSize;
  int prev_angle;
  int cur_angle;

  Fir lpre;
  Fir rpre;
  Fir lcur;
  Fir rcur;


  FileWvIn input;
  FileWvOut output;
  RtWvOut *dac;

};

unordered_map<int, vector<Fir>> hrtf_set;
vector<vector<vector<double>>> HRIR(1440, vector<vector<double> >(2, vector<double>(2048)));
StkFloat sampleRate;


inline void conv_audio_no_crossfade(FileWvIn& input, FileWvOut& output, Fir& lcur, Fir& rcur, int cur_angle, StkFrames& iframes, StkFrames& oframes) {
  input.tick(iframes, 0);

  lcur.setCoefficients(HRIR[cur_angle][0]);
  rcur.setCoefficients(HRIR[cur_angle][1]);

  lcur.tick(iframes, oframes, 0, 0);
  rcur.tick(iframes, oframes, 0, 1);
  output.tick(oframes);

  // swap(lcur, lpre);
  // swap(rcur, rpre);
  // StkFloat gain1 = hrtf_set[prev_angle][0].getGain();
  // StkFloat gain2 = hrtf_set[cur_angle][0].getGain();
}

void test(AudioComponent *ac) {
  for (int angle = 90; angle < 270; angle += 15) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ac->setAngle(angle);
  }
}


int main() {
  // Set the global sample rate before creating class instances.
  // Stk::setSampleRate( 48000.0 );

  AudioComponent *ac = new AudioComponent();
  ac->loadHRIR("hrir.txt");
  ac->loadAudio("despacito.wav");
  ac->setOutput("despacito_conv.wav");
  std::thread outer(&test, ac);
  ac->renderAudio();
  outer.join();
  // std::thread acthread(&AudioComponent::renderAudio, ac);
  // ac.renderAudio();

  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // ac->setAngle(90);
  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // ac->setAngle(180);
  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // ac->setAngle(270);

  // acthread.join();

  cout << "finish rendering" << endl;

  exit(0);
 //  while (getline(infile, line)) {
 //    istringstream iss(line);
 //    // int a, b;
 //    // if (!(iss >> a >> b)) { break; } // error
    
 //    for (int i = 0; i < 2048; ++ i){
 //      iss >> HRIR[lineCount][0][i];
 //      iss >> HRIR[lineCount][1][i];
 //    }
 //    lineCount ++;
 //  }


 //  FileWvIn input;
 //  FileWvOut output;

 //  input.openFile( "despacito.wav", false );
 //  sampleRate = input.getFileRate();
 //  unsigned int channels = input.channelsOut();
 //  unsigned long sampleSize = input.getSize();
 //  cout << "Number of Sample points: " << sampleSize << endl;
 //  cout << "SampleRate: " << sampleRate << endl;
 //  cout << "channels: " << channels << endl;
 //  cout << "Time: " << sampleSize/sampleRate << endl;

  

 // //  for (int i = 0; i < lineCount; ++i) {
 // //  	vector<StkFloat> rightcoefficients(2048, 0); // create and initialize numerator coefficients
 // //  	vector<StkFloat> leftcoefficients(2048, 0); // create and initialize numerator coefficients
 // //  	rightcoefficients = HRIR[i][1];
 // //  	leftcoefficients = HRIR[i][0];
	// // vector<Fir> nfilter;
	// // nfilter.emplace_back(Fir(leftcoefficients));
	// // nfilter.emplace_back(Fir(rightcoefficients));
	// // hrtf_set[i] = nfilter;
 // //  }


 // //  auto rightcoefficients = HRIR[270][1];
 // //  auto leftcoefficients = HRIR[270][0];
 // //  Fir rfilter(rightcoefficients);
 // //  Fir lfilter(leftcoefficients);

 // //  auto rightcoefficients_ = HRIR[240][1];
 // //  auto leftcoefficients_ = HRIR[240][0];
 // //  Fir rfilter_(rightcoefficients_);
 // //  Fir lfilter_(leftcoefficients_);

  
 //  // Open a 16-bit, two-channel WAV formatted output file
 //  output.openFile( "despacito_conv.wav", 2, FileWrite::FILE_WAV, Stk::STK_SINT16 );
 //  // input.setFrequency( 440.0 );
 //  // Run the oscillator for 40000 samples, writing to the output file

 //  RtWvOut *dac = 0;
 //  try {
 //    // Define and open the default realtime output device for two-channel playback
 //    dac = new RtWvOut( 2 );
 //  }
 //  catch ( StkError & ) {
 //    exit( 1 );
 //  }


 //  StkFrames iframes(sampleRate/20, channels);
 //  StkFrames oframes(sampleRate/20, 2);

 //  Fir lpre(HRIR[0][0]);
 //  Fir rpre(HRIR[0][1]);
 //  Fir lcur;
 //  Fir rcur;

 //  int prev_angle = 0, cur_angle = 0;
 //  // for (cur_angle = 0; cur_angle < 300; cur_angle += 1) {
 //  //   // cout << "cur angle: " << cur_angle << endl;
 //  //   cur_angle = cur_angle%360;
 //  //   conv_audio_no_crossfade(input, output, lcur, rcur, cur_angle, iframes, oframes);

 //  //   dac->tick(oframes);
 //  //   // prev_angle = cur_angle;
    
 //  // }

 //    // Execute lambda asyncronously.
 //  auto f = std::async(std::launch::async, [] {
 //      auto s = ""s;
 //      if (std::cin >> s) return s;
 //  });

 //  // Continue execution in main thread.
  

  
 //  int frame_count = 0;

 //  while (!input.isFinished()) {
 //    // cout << "\nPlaying ... press <enter> to quit.\n";
 //    // while(f.wait_for(2ms) != std::future_status::ready);
 //     // {
 //    //   std::cout << "still waiting..." << std::endl;
 //    // }
 //    cout << frame_count << endl;
 //    frame_count ++;
 //    // std::cout << "Input was: " << f.get() << std::endl;
 //    // cin.get(keyhit);
 //    // switch keyhit:
 //    //   case keyhit == 'a':
 //    //     cur_angle += 10;
 //    //     break;
 //    //   case keyhit == 'd':
 //    //     cur_angle -= 10;
 //    //     break;
 //    //   default:
 //    //     cout << "press only a or d\n"
 //    //     break;

 //    conv_audio_no_crossfade(input, output, lcur, rcur, 0, iframes, oframes);
 //    dac->tick(oframes);
 //  }
  
 //  // cout << "Finish" << endl;
 //  // dac->stop();
  
 //  // cout << "Delete" << endl;
 //  delete dac;

 //  input.closeFile();
  // cout << "HERE" << endl;
 //  for ( int i=0; i<200000; i++ ) {
 //    output.tick( input.tick() );
	// }
}
