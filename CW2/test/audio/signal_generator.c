#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>

#define M_PI 3.1415926535897932384626433832795L
#define NUM_CHANNELS 2
#define NUM_SAMPLES 512
#define SAMPLE_RATE 44100

double frequency = 500.0;

int main(int argc, char *argv[])
{	
	if (argc >= 2){
		frequency = atof(argv[1]);
	}

	int16_t samples[NUM_CHANNELS * NUM_SAMPLES];
	unsigned cbBuffer=sizeof(samples);
	unsigned counter = 0;

	while(1){
		for (int i = 0; i < NUM_CHANNELS * NUM_SAMPLES; i+=NUM_CHANNELS){
			samples[i] = (int16_t) 3000 * sin(frequency * 2 * M_PI * (double)counter /(double)(SAMPLE_RATE));
			samples[i+1] = samples[i];
			counter++;
		}

		int done=write(STDOUT_FILENO, samples, cbBuffer);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(done!=cbBuffer){
			fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
		}
	}

	return 0;
}
