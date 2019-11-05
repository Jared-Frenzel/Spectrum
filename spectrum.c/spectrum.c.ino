#include "arduinoFFT.h"

#define SAMPLES 64
#define FS 38500

int vReal[SAMPLES];
int vImag[SAMPLES];

char printstr[SAMPLES*6+1];
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

void setup() {
  // put your setup code here, to run once:
  ADCSRA = 0b11100101;      // set ADC to free running mode and set pre-scalar to 32 (0xe5) Effective sample rate ~36.9kS/s
  ADMUX = 0b00000000;       // use pin A0 and external voltage reference

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
   for(int i=0; i<SAMPLES; i++)
    {
      while(!(ADCSRA & 0x10));        // wait for ADC to complete current conversion ie ADIF bit set
      ADCSRA = 0b11110101 ;               // clear ADIF bit so that ADC can do next operation (0xf5)
      int value = ADC - 512 ;                 // Read from ADC and subtract DC offset caused value
      vReal[i]= value;                      // Copy to bins after compressing
      vImag[i] = 0;                         
    }
    
    for(int i=0; i<SAMPLES; i++)
    {
      sprintf(printstr,"%s%05d,",printstr, vReal[i]);
    }
    printstr[SAMPLES*6] = "\n";
    //Serial.print(printstr);
    printstr[0] = '\0';

    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    double x = FFT.MajorPeak(vReal, SAMPLES, FS);

    Serial.print(x);
    
    delay(10000);
}
