#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#define NUM_CHANNELS 2
#define NUM_SAMPLES 512

int num_samples = NUM_SAMPLES;

int main(int argc, char *argv[])
{	

	if (argc == 2){
		num_samples = atoi(argv[1]);
	}

	while(1){
		int16_t samples[NUM_CHANNELS * num_samples];
		unsigned cbBuffer=sizeof(samples);	// size in bytes of 
		
		int got=read(STDIN_FILENO, samples, cbBuffer);
		if(got<0){
			fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(got==0){
			break;	 // end of file
		}else if(got!=cbBuffer){
			fprintf(stderr, "%s : Did not receive expected number of bytes.\n", argv[0]);
			exit(1);
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
