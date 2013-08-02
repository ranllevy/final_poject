#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dspProcess.h"
#define xval 7
#define Fs 8000
#define S 50
#define PI 3.14
short Fc[S] = 	{550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 
		1050, 1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450,
		1500, 1550, 1600, 1650, 1700, 1750, 1800, 1850, 1900, 1950, 2000,
		2050, 2100, 2150, 2200, 2250, 2300, 2350, 2400, 2450, 2500,
		2550, 2600, 2650, 2700, 2750, 2800, 2850, 2900, 2950, 3000};

const short x_table[xval] = { 0, 512, 1024, 2048, 4096, 8192, 0, };



const int sine = 64;
const short sine_table[64] = {0,195,383,556,707,831,924,981,1000,
		    981,924,831,707,556,383,195,
		    0,-195,-383,-556,-707,-831,-924,-981,-1000,
		   -981,-924,-831,-707,-556,-383,-195,
		    };

const short sine_table_a[64] = {100,295,483,656,807,931,1024,1081,1100,
		    1081,1024,931,807,656,483,295,
		    -100,-295,-483,-656,-807,-931,-1024,-1081,-1100,
		    -1081,-1024,-931,-807,-656,-483,-295,
		    };

const int BL = 75;
const short B[75] = {
        6,      9,      9,     -3,    -35,    -91,   -168,   -252,   -318,
     -340,   -296,   -182,    -18,    155,    279,    306,    212,     17,
     -218,   -404,   -454,   -322,    -27,    340,    640,    731,    526,
       38,   -598,  -1153,  -1365,  -1023,    -46,   1471,   3263,   4956,
     6165,   6603,   6165,   4956,   3263,   1471,    -46,  -1023,  -1365,
    -1153,   -598,     38,    526,    731,    640,    340,    -27,   -322,
     -454,   -404,   -218,     17,    212,    306,    279,    155,    -18,
     -182,   -296,   -340,   -318,   -252,   -168,    -91,    -35,     -3,
        9,      9,      6
};
// implements an fir filter using fir coefficients defined in fdacoefs.h
// in fixed-point
/*
damp = 0.05
F1 = 2*sine((pi*fc)/fs)
Q1 = 2*damp
xn = input_signal
yln = lowpass = F1*ybn + yl(n-1) 
yhn = highpass = xn -y(n-1) -Q1*yb(n-1)
ybn = bandpass = F1*yhn + yb(n-1)
*/

short bufferL = NULL, bufferR = NULL;
short fir_filter(buffer *xn){
	int j;
	int yn = 0;
	/*int yna = 0;
	int sn = 0;
	int sna = 0;*/
	// performs the convolution of xn with B
	for(j=0; j < BL; j++){
		yn += readn(xn,j)*B[j];
	}
	yn = (yn >> 15) & 0xffff; //converts from Q30 to Q15 fixed point
	return (short)yn; // must cast to a 16-bit short for output to ALSA
}

// core dsp block processing
int dspBlockProcess(short *outputBuffer, short *inputBuffer, buffer *xnL, buffer *xnR, int samples, int * filter_on, double * volume){
	int i;
	if(*filter_on == 0) {
	for (i=0; i < samples; i+=2){
		memcpy((char *)outputBuffer, (char *)inputBuffer, 2*samples); // passthru
		//outputBuffer[i] = inputBuffer[i]; // filters the input and stores it in the left output channel
		//outputBuffer[i+1] = inputBuffer[i+1]; 
		}
	}
	else if(*filter_on == 1) {
		for (i=0; i < samples; i+=2){
			push(xnL,inputBuffer[i]); // stores the most recent sample in the circular buffer xn
			push(xnR,inputBuffer[i+1]);
			outputBuffer[i] = (short)(*volume*fir_filter(xnL)); // filters the input and stores it in the left output channel
			outputBuffer[i+1] = (short)(*volume*fir_filter(xnR)); // zeros out the right output channel
		}
	}	
	else if(*filter_on == 1) {
		for (i=0; i < samples; i+=2){
				outputBuffer[i] = (short)(*volume*inputBuffer[i] + (.5*PI*8*i)); //16384*sin(2*PI*8*i));
				outputBuffer[i+1] = (short)(*volume*inputBuffer[i] + (.5*PI*8*i)); //16384*sin(2*PI*8*i));
				//outputBUffer[i] = (short)(inputBuffer[i]);
				//outputBuffer[i+1] = (short)(inputBuffer[i+1]);
				outputBuffer[i+2] = (short)(*volume*inputBuffer[i]);
				outputBuffer[i+3] = (short)(*volume*inputBuffer[i+1]);
		}	
	}
	return DSP_PROCESS_SUCCESS;
}
