#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>

#define INT16_MAX   0x7fff
#define INT16_MIN   (-INT16_MAX - 1)

#define M_PI 3.1415926535897932384626433832795L
#define NUM_CHANNELS 2
#define NUM_SAMPLES 512

void averageInput(int16_t[], int);

int main(int argc, char *argv[])
{
	if (argc < 4){
		fprintf(stderr, "%s : Unable to Merge, insufficient parameters Passed\n", argv[0]);
	}

	FILE * fileOne;
	FILE * fileTwo;

	fileOne = fopen (argv[1] , "rb" );
	if (fileOne==NULL) {fprintf(stderr, "%s : File error %s\n",argv[0], argv[1]); exit(1);}
	fileTwo = fopen (argv[2], "rb");
	if (fileTwo==NULL) {fprintf(stderr, "%s : File error %s\n",argv[0], argv[2]); exit(1);}

	int w = atoi(argv[3]);

	int16_t samplesOne[NUM_CHANNELS * w];
	int16_t samplesTwo[NUM_CHANNELS * w];
	int sample_size = NUM_CHANNELS * w;
	int result, result2;
	unsigned cbBuffer=sizeof(samplesOne);

	double Arms = 0;
	double Brms = 0;

	double streamOneTmp = 0;
	double streamTwoTmp = 0;
	
	double rTop = 0;
	double rBottomL = 0;
	double rBottomR = 0;

	while(1){

		result = fread(samplesOne, cbBuffer, 1, fileOne);
		if (result != 1) {break;}
		result2 = fread(samplesTwo, cbBuffer, 1, fileTwo);
		if (result2 != 1) {break;}

		averageInput(samplesOne, sample_size);
		averageInput(samplesTwo, sample_size);

		streamOneTmp = 0;
		streamTwoTmp = 0;

		for (int i = 0; i < sample_size; i++){
			streamOneTmp += (double) pow(samplesOne[i],2);
			streamTwoTmp += (double) pow(samplesTwo[i],2);
		}

		Arms = sqrt((1/(double)w) * streamOneTmp);
		Brms = sqrt((1/(double)w) * streamTwoTmp);

		rTop += Arms * Brms;
		rBottomL += pow(Arms, 2);
		rBottomR += pow(Brms, 2);

	}

	double correlation = rTop/sqrt(rBottomL * rBottomR);

	printf("%.5lf\n", correlation);

	return 0;
}

int16_t mix_samples(int16_t a, int16_t b){
	// if (a > 0 && b > 0){
	// 	return a + b - ((a * b)/INT16_MAX);
	// } else if (a < 0 && b < 0){
	// 	return a + b - ((a * b)/INT16_MIN);
	// } else {
	// 	return a + b;
	// }
	return (0.5*(a + b));
}

void averageInput(int16_t samples[], int length){
	for (int i = 0; i < length; i+=NUM_CHANNELS){
		samples[i] = 0.5*(samples[i] + samples[i+1]);
		samples[i+1] = samples[i];
	}
}