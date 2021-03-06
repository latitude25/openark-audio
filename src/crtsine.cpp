#include "audioComponent.h"

using namespace stk;
using namespace std;

void fft(ComplexArray &x)
{
  // DFT
  unsigned int N = x.size(), k = N, n;
  double thetaT = 3.14159265358979323846264338328L / N;
  Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
  while (k > 1)
  {
    n = k;
    k >>= 1;
    phiT = phiT * phiT;
    T = 1.0L;
    for (unsigned int l = 0; l < k; l++)
    {
      for (unsigned int a = l; a < N; a += n)
      {
        unsigned int b = a + k;
        Complex t = x[a] - x[b];
        x[a] += x[b];
        x[b] = t * T;
      }
      T *= phiT;
    }
  }
  // Decimate
  unsigned int m = (unsigned int)log2(N);
  for (unsigned int a = 0; a < N; a++)
  {
    unsigned int b = a;
    // Reverse bits
    b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
    b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
    b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
    b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
    b = ((b >> 16) | (b << 16)) >> (32 - m);
    if (b > a)
    {
      Complex t = x[a];
      x[a] = x[b];
      x[b] = t;
    }
  }
  //// Normalize (This section make it not working correctly)
  //Complex f = 1.0 / sqrt(N);
  //for (unsigned int i = 0; i < N; i++)
  //  x[i] *= f;
}
 
// inverse fft (in-place)
void ifft(ComplexArray& x)
{
    // conjugate the complex numbers
    x = x.apply(std::conj);
    // forward fft
    fft( x );
    // conjugate the complex numbers again
    x = x.apply(std::conj);
    // scale the numbers
    x /= x.size();
}

// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
  {
    AudioComponent* ac = (AudioComponent*) dataPointer;
    FileWvIn *input = &ac->input;
    int angle = ac->cur_angle; 
    
    register StkFloat *samples = (StkFloat *) outputBuffer;

    if (input->isFinished()) {
      cout << "Audio finished\n";
      return 1;
    }

    ac->lcur.setCoefficients(ac->HRIR[angle][0]);
    ac->rcur.setCoefficients(ac->HRIR[angle][1]);

    StkFloat tmp = 0.;
    for ( unsigned int i=0; i<nBufferFrames; i++ ) {
      tmp = input->tick();
      
      *samples++ = ac->lcur.tick(tmp);
      *samples++ = ac->rcur.tick(tmp);
    }

    return 0;
  }

AudioComponent::AudioComponent () {
  // HRIR = vector<vector<vector<StkFloat>>>(1440, vector<vector<StkFloat> >(2, vector<StkFloat>(2048)));
  HRIR = vector<vector<vector<stk::StkFloat>>>(1440, vector<vector<stk::StkFloat> >(2, vector<stk::StkFloat>(2048)));
  HRIR_minimumPhase = vector<vector<vector<stk::StkFloat>>>(1440, vector<vector<stk::StkFloat> >(2, vector<stk::StkFloat>(2048)));
  itd.resize(360);
}

AudioComponent::~AudioComponent () {
  cout << "Destruct AudioComponent\n";
  try {
    dac.closeStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
  }
  input.closeFile();
}


void AudioComponent::loadHRIR(string fileName) {
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

  cout << "Finish loading HRIR\n";
}

void AudioComponent::loadAudio(string audioFile) {
  input.openFile(audioFile, false );

  sampleRate = input.getFileRate();
  channels = input.channelsOut();
  sampleSize = input.getSize();
  
  cout << "Audio Information:\n";
  cout << "Number of Sample points: " << sampleSize << endl;
  cout << "SampleRate: " << sampleRate << endl;
  cout << "channels: " << channels << endl;
  cout << "Time: " << sampleSize/sampleRate << endl;
}

void AudioComponent::setOutput(string outputFile) {
  try {
    output.openFile(outputFile, 2, FileWrite::FILE_WAV, Stk::STK_SINT16 );
  }
  catch ( StkError & ) {
    exit( 1 );
  }
  isOpen = true;
  
}

void AudioComponent::setRealTime() {
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = 2;
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  unsigned int bufferFrames = RT_BUFFER_SIZE;

  try {
    dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)this );
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    // goto cleanup;
    exit(1);
  }
}

void AudioComponent::startRealTime() {
  try {
    dac.startStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    exit(1);
  }
}

void AudioComponent::setAngle(int angle) {
  cur_angle = angle;
}

void AudioComponent::setDistance(int dist) {
  distance = dist;
}

void AudioComponent::minimumPhase(double r) {
  ComplexArray window(2048), y(2048);

  for (int i = 0; i < 1025; i ++) window[i] = {1, 0};
  
  size_t i, j, k;
  for (i = 0; i < HRIR.size(); i ++) {
    for (j = 0; j < HRIR[i].size(); j ++) {
      for (k = 0; k < y.size(); k ++) y[k] = {HRIR[i][j][k], 0};

      fft(y); abs(y);
      for (k = 0; k < y.size(); k ++) y[k] = log(y[k]);
      // log(y);
      ifft(y);

      for (k = 0; k < y.size(); k ++) y[k] = y[k].real();
      for (k = 0; k < y.size(); k ++) y[k] *= window[k];
      fft(y);
      // exp(y);
      for (k = 0; k < y.size(); k ++) y[k] = exp(y[k]); 
      ifft(y);

      for (k = 0; k < 2048; k++) HRIR_minimumPhase[i][j][k] = y[k].real();
    }
  }
  for (i = 0; i < itd.size(); i++)
    itd[i] = estimate_itd(r, double(i));
}

double AudioComponent::estimate_itd(double r, double theta) {
   if (theta > 180) {
     theta = 360 - theta;
   }
   double PI = 3.14159265358979323846264338328L;
   theta = theta / 180. * PI;
   double c = 343.;

   double ret;
   if (theta > PI / 2.) {
     ret = r / c * (PI - theta + sin(theta));
   }
   else {
     ret = r / c * (theta + sin(theta));
   }
   return ret;
}

void test(AudioComponent *ac) {
  for (int angle = 0; angle < 360; angle += 2) {
    std::this_thread::sleep_for(std::chrono::milliseconds(60000/360));
    ac->setAngle(angle);
  }
}


int main() {
  // Set the global sample rate before creating class instances.
  // Stk::setSampleRate( 48000.0 );

  AudioComponent *ac = new AudioComponent();
  ac->loadHRIR("hrir.txt");
  ac->loadAudio("happyhouse.wav");
  ac->minimumPhase(0.2);
  // ac->setOutput("Wallpaper_conv.wav");
  ac->setRealTime();
  std::thread outer(&test, ac);
  ac->startRealTime();
  // ac->renderAudio();
  outer.join();
  delete ac;

  cout << "finish rendering" << endl;

  exit(0);
 
}