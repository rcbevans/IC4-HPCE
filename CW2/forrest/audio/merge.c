#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

int main(int argc, char *argv[])
{	
	const char *file_name_1;
	const char *file_name_2;
	if (argc < 3)
	{
		printf("Too few args\n");
		return 0;
	}
	else
	{
		file_name_1 = argv[1];
		file_name_2 = argv[2];
		
	}
	FILE * file_1 = fopen(file_name_1, "r");
	FILE * file_2 = fopen(file_name_2, "r");

	int num_channels = 2, num_samples = 512;
	int16_t samples_input_1[num_channels * num_samples];
	int16_t samples_input_2[num_channels * num_samples];
	int16_t sample_output[num_channels * num_samples];
	unsigned cbBuffer1=sizeof(samples_input_1);
	unsigned cbBuffer2=sizeof(samples_input_2);
	unsigned cbBuffer_out=sizeof(sample_output);

	int got;
	while(1){
		got =fread(samples_input_1, 1, cbBuffer1, file_1);
		// got=read(file_1, samples_input_1, cbBuffer1);
		if(got<0){
			fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(got==0){
			break;	 // end of file
		}else if(got!=cbBuffer1){
			fprintf(stderr, "%s : Did not receive expected number of bytes.\n", argv[0]);
			exit(1);
		}

		got = fread(samples_input_2, 1, cbBuffer2, file_2);
		if(got<0){
			fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(got==0){
			break;	 // end of file
		}else if(got!=cbBuffer2){
			fprintf(stderr, "%s : Did not receive expected number of bytes.\n", argv[0]);
			exit(1);
		}

		for (int i = 0; i < num_channels*num_samples; ++i)
		{
			sample_output[i] = (samples_input_1[i] + samples_input_2[i])*.5;

		}
		// Copy one sample to output
		int done=write(STDOUT_FILENO, sample_output, cbBuffer_out);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(done!=cbBuffer_out){
			fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
		}
	}

	return 0;
}
