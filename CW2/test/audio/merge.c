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

int num_samples = 512;

int sgn(int);
int16_t mix_samples(int16_t, int16_t);

int main(int argc, char *argv[])
{
	if (argc < 3){
		fprintf(stderr, "%s : Unable to Merge, insufficient parameters Passed\n", argv[0]);
	}

	FILE * fileOne;
	FILE * fileTwo;

	fileOne = fopen (argv[1] , "rb" );
	if (fileOne==NULL) {fprintf(stderr, "%s : File error %s\n",argv[0], argv[1]); exit(1);}
	fileTwo = fopen (argv[2], "rb");
	if (fileTwo==NULL) {fprintf(stderr, "%s : File error %s\n",argv[0], argv[2]); exit(1);}

	int16_t samplesOne[NUM_CHANNELS * num_samples];
	int16_t samplesTwo[NUM_CHANNELS * num_samples];
	int result, result2;
	unsigned cbBuffer=sizeof(samplesOne);

	while(1){

		result = fread(samplesOne, sizeof(samplesOne), 1, fileOne);
		if (result != 1) {fprintf(stderr, "%s : Reading Error %s %d\n", argv[0], argv[1], result); fclose(fileOne); fclose(fileTwo); exit(3);}
		result2 = fread(samplesTwo, sizeof(samplesTwo), 1, fileTwo);
		if (result2 != 1) {fprintf(stderr, "%s : Reading Error %s %d\n", argv[0], argv[2], result2); fclose(fileOne); fclose(fileTwo); exit(3);}

		for (int i = 0; i < NUM_CHANNELS*num_samples; i++){
			samplesOne[i] = mix_samples(samplesOne[i], samplesTwo[i]);
		}
		
		int done=write(STDOUT_FILENO, samplesOne, cbBuffer);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.\n", argv[0], strerror(errno));
			fclose(fileOne); fclose(fileTwo);
			exit(1);
		}else if(done!=cbBuffer){
			fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
			fclose(fileOne); fclose(fileTwo);
			exit(1);
		}
	}

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