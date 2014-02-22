#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#define NUM_CHANNELS 2
#define SAMPLE_COUNT 512
const char * delims = " \r\n";


int fir_order = 0;

FILE * fileOne;
char *buffer;
double * fir_coeffs;

char * read_file_into_buffer(FILE *);
int get_fir_order(char *);
double * get_fir_coeffs(char *, int);
void init_samples(int16_t[], int);
void move_buffer_down(int16_t[], int);

int main(int argc, char *argv[])
{	

	if (argc < 2){
		fprintf(stderr, "%s : Insufficient Parameters Passed\n", argv[0]);
	}

	fileOne = fopen(argv[1] , "r");
	if (fileOne==NULL) {fprintf(stderr, "%s : File error %s\n",argv[0], argv[1]); exit(1);}

	buffer = read_file_into_buffer(fileOne);
	fir_order = get_fir_order(buffer);
	rewind(fileOne);
	buffer = read_file_into_buffer(fileOne);
	// printf("%s\n", buffer);
	fir_coeffs = get_fir_coeffs(buffer, fir_order);

	free(buffer);
	fclose(fileOne);

	// for (int i = 0; i < fir_order; i++){
	// 	printf("%lf\n", fir_coeffs[i]);
	// }

	int sample_count = NUM_CHANNELS * (fir_order + SAMPLE_COUNT);
	int samples_out = NUM_CHANNELS * SAMPLE_COUNT;
	unsigned bufReadWriteSize = samples_out*sizeof(int16_t);
	int16_t samples[sample_count];
	int16_t output[samples_out];

	init_samples(samples, sample_count);

	int latestSample = 0;

	// for (int i = 0; i < sample_count; i++){
	// 	printf("%d\n", samples[i]);
	// }

	int index;

	while(1){

		if(latestSample + samples_out < sample_count){
			int got=read(STDIN_FILENO, &samples[latestSample], bufReadWriteSize);
			if(got<0){
				fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
				exit(1);
			}else if(got==0){
				break;
			}else if(got!=bufReadWriteSize){
				fprintf(stderr, "%s : Did not receive expected number of bytes.\n", argv[0]);
				exit(1);
			}
		} else {
			int got=read(STDIN_FILENO, &samples[latestSample], (sample_count-latestSample)*sizeof(int16_t));
			if(got<0){
				fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
				exit(1);
			}else if(got==0){
				break;
			}else if(got!=(sample_count-latestSample)*sizeof(int16_t)){
				fprintf(stderr, "%s : Did not receive expected number of bytes.\n", argv[0]);
				exit(1);
			};

			got=read(STDIN_FILENO, &samples[0], ((samples_out - (sample_count - latestSample))*sizeof(int16_t)));
			if(got<0){
				fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
				exit(1);
			}else if(got==0){
				break;
			}else if(got!=((samples_out - (sample_count - latestSample))*sizeof(int16_t))){
				fprintf(stderr, "%s : Did not receive expected number of bytes.\n", argv[0]);
				exit(1);
			}
		}
		
		for (int i = 0; i < samples_out; i += NUM_CHANNELS){
			double tmpVal = 0;
			for (int j = 0; j < fir_order; j++){
				index = latestSample + i - (2*j);
				if (index < 0){
					index += sample_count;
				} else if (index >= sample_count) {
					index -= sample_count;
				}
				tmpVal += (((double)samples[index])*fir_coeffs[j]);
			}

			output[i] = (int16_t) tmpVal;
			output[i+1] = (int16_t) tmpVal;
		}

		latestSample += samples_out;
		if (latestSample >= sample_count){
			latestSample -= sample_count;
		}
		
		int done=write(STDOUT_FILENO, &output, bufReadWriteSize);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(done!=bufReadWriteSize){
			fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
		}
	}

	return 0;
}

char * read_file_into_buffer(FILE * file){
	char * buffer;
	long lSize;

	fseek( file , 0L , SEEK_END);
	lSize = ftell( file );
	rewind( file );

	/* allocate memory for entire content */
	buffer = calloc( 1, lSize+1 );
	if( !buffer ) fclose(file),fputs("memory alloc fails",stderr),exit(1);

	/* copy the file into the buffer */
	if( 1!=fread( buffer , lSize, 1 , file) )
		fclose(file),free(buffer),fputs("entire read fails",stderr),exit(1);

	return buffer;
}


int get_fir_order(char * buffer){
	char * pch;
	int order = 0;
	pch = strtok (buffer,delims);
	while (pch != NULL)
	{
		// printf("%.10f\n", strtod(pch, NULL));
		order++;
		pch = strtok (NULL, delims);
	}

	return order;
}

double * get_fir_coeffs(char * buffer, int order){
	double * coeffs = calloc(order, sizeof(double));
	char * pch;
	pch = strtok (buffer,delims);
	for(int i = 0; i < order; i++, pch = strtok (NULL,delims)){
		coeffs[i] = strtod(pch, NULL);
		//printf("%.10f\n",strtod(pch, NULL));
	}

	return coeffs;
}

void init_samples(int16_t samples[], int sample_count){
	for (int i = 0; i < sample_count; i+=NUM_CHANNELS){
		samples[i] = 0;
		samples[i+1] = 0;
	}
}