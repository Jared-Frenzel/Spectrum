#include "arduinoFFT.h"
#include <SPI.h>

#define SAMPLES 64
#define FS 38500
#define NUM_BINS 5
#define WEIGHT 1300.0

const int CHIP_SELECT_PIN = 7;

double vReal[SAMPLES];
double vImag[SAMPLES];
double bins[NUM_BINS];

char printstr[SAMPLES*6+1];
char levels[9] = {-1, -2, -4, -8, -16, -32, -64, -128, 0};
int sum = 0;
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

//Sends a write command to Seven Segment Displays
void write(int chipSelectPin, char * value, int n) {

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  for(int i = 0; i < n; i++) {
     SPI.transfer(value[i]);
  }


  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  ADCSRA = 0b11100101;      // set ADC to free running mode and set pre-scalar to 32 (0xe5) Effective sample rate ~36.9kS/s
  ADMUX = 0b00000000;       // use pin A0 and external voltage reference

    // start the SPI library:
  SPI.begin();

  // initalize the chip select pins:
  pinMode(CHIP_SELECT_PIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
   for(int i=0; i<SAMPLES; i++)
    {
      while(!(ADCSRA & 0x10));        // wait for ADC to complete current conversion ie ADIF bit set
      ADCSRA = 0b11110101 ;               // clear ADIF bit so that ADC can do next operation (0xf5)
      int value = ADC - 512 ;                 // Read from ADC and subtract DC offset caused value
      vReal[i]= value / 8;                      // Copy to bins after compressing
      vImag[i] = 0;                         
    }

    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    
    int bin_size = ceil(((SAMPLES+1) / 2) / (1.0*NUM_BINS));
    for(int i=0; i < NUM_BINS; i++) {
      sum = 0;
      int start = i * bin_size;
      int finish = min(((SAMPLES+1) / 2), start + bin_size);

      for(int j=start; j<finish; j++) {
        sum += vReal[j];
      }

      bins[i] = sum / WEIGHT;
    }

    char bin_levels[NUM_BINS];
    for(int i = 0; i<NUM_BINS; i++) {
      bin_levels[NUM_BINS - 1 - i] = levels[(int)min((int)floor(bins[i] * 8), 8)];
    }
    write(CHIP_SELECT_PIN, bin_levels, NUM_BINS);

}
