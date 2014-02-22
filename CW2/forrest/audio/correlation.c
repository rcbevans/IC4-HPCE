#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

void firDouble (double *coeffs, double *input, double *output, int length, int filterLength)
{

}

int main(int argc, char *argv[])
{	
	const char *file_name_1;
	const char *file_name_2;
	int window_size;
	if (argc < 4)
	{
		printf("Too few args\n");
		return 0;
	}
	else
	{
		file_name_1 = argv[1];
		file_name_2 = argv[2];
		sscanf(argv[3], "%d", &window_size);
	}
	FILE * file_1 = fopen(file_name_1, "r");
	FILE * file_2 = fopen(file_name_2, "r");

	int num_channels = 2, num_samples = 512;
	int16_t samples_input_1[num_channels * num_samples];
	int16_t samples_input_2[num_channels * num_samples];

	unsigned cbBuffer1=sizeof(samples_input_1);
	unsigned cbBuffer2=sizeof(samples_input_2);

	float input_A_mono[num_channels * num_samples];
	float input_B_mono[num_channels * num_samples];

	int array_length;
	if(window_size > 200)
	{
		array_length = 2048;
	}
	else
	{
		array_length = 4096;
	}
	float *Arms = (float *)malloc(sizeof(float)*array_length);
	float *Brms = (float *)malloc(sizeof(float)*array_length);
	float rms_running_total_A=0, rms_running_total_B=0;
	int window_index=0, num_windows=0;

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
			break;

			// exit(1);
		}

		got = fread(samples_input_2, 1, cbBuffer2, file_2);
		if(got<0){
			fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(got==0){
			break;	 // end of file
		}else if(got!=cbBuffer2){
			fprintf(stderr, "%s : Did not receive expected number of bytes.\n", argv[0]);
			break;
			// exit(1);
		}

		//Convert to Mono Signal
		float temp;
		for(int i=0; i < num_channels*num_samples; i+=2)
		{
			temp = (float)(samples_input_1[i] + samples_input_1[i+1])*.5; 
			input_A_mono[i] = temp;
			input_A_mono[i+1] = temp;
			temp = (float)(samples_input_2[i] + samples_input_2[i+1])*.5;
			input_B_mono[i] =temp;
			input_B_mono[i+1] = temp;

		}

		for (int i = 0; i < num_samples; ++i)
		{
			if(window_index < window_size)
			{
				rms_running_total_A += pow(input_A_mono[i], 2);
				rms_running_total_B += pow(input_B_mono[i], 2);
				window_index++;
			}
			else
			{
				if (num_windows >= array_length)
				{
					array_length*=2;
					Arms = (float *)realloc(Arms,sizeof(float)*array_length);
					Brms = (float *)realloc(Brms,sizeof(float)*array_length);
				}
				Arms[num_windows] = sqrt((1/(float)window_size) * rms_running_total_A);
				rms_running_total_A = 0;
				Brms[num_windows] = sqrt((1/(float)window_size) * rms_running_total_B);
				rms_running_total_B = 0;

				window_index=0;
				num_windows++;
			}
		}
	}

	float Arms_sum=0, Brms_sum=0, ABrms_sum=0;
	
	for (int i = 0; i < num_windows; ++i)
	{
		ABrms_sum+=Arms[i]*Brms[i];
		Arms_sum+=pow(Arms[i], 2);
		Brms_sum+=pow(Brms[i], 2);
	}

	float correlation = ABrms_sum / sqrt(Arms_sum*Brms_sum);
	printf("%f\n", correlation);

	return 0;
}
