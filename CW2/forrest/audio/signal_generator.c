#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#define _USE_MATH_DEFINES

int main(int argc, char *argv[])
{
    int frequency;
    if (argc < 2)
    {
    	//frequency too low to hear on many laptop speakers
        frequency = 50;
    }
    else
    {
        sscanf (argv[1], "%d", &frequency);
    }

    int time_counter = 0, num_channels = 2, num_samples = 512, sample_rate = 44100;
    // Buffer containing five hundred and twelve samples (left and right, both 16 bit).
    int16_t samples[num_channels * num_samples];
    unsigned cbBuffer = sizeof(samples); // size in bytes of
    while (1)
    {
        // Copy five hundred and twelve samples to output
        for (int t = 0; t < num_samples * num_channels; t += num_channels)
        {
            //for every set of channels, ensure same value
            for (int i = 0; i < num_channels; ++i)
            {
                samples[t + i] = 30000 * sin(time_counter * 2 * M_PI * frequency / sample_rate);
            }
            time_counter++;
        }

        int done = write(STDOUT_FILENO, samples, cbBuffer);
        if (done < 0)
        {
            fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
            exit(1);
        }
        else if (done != cbBuffer)
        {
            fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
        }
    }

    return 0;
}
