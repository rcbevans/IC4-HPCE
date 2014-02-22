#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>


#include <unistd.h>

int main(int argc, char const *argv[])
{
    FILE *input_file = NULL;
    if (argc < 2)
    {
        printf("Error, too few args\n");
        return 0;
    }
    else
    {
        input_file = fopen(argv[1], "r");
    }

    double value = 0.0;
    int array_size = 128;
    int coeff_count = 0;
    double *fir_coeffs = (double *)malloc(sizeof(double) * array_size);


    //read in FIR coefficients
    while (fscanf(input_file, "%lf\n", &value) != EOF)
    {
        
        if(coeff_count == array_size){
            array_size*=2;
            fir_coeffs = (double *)realloc(fir_coeffs, array_size*sizeof(double));
        }
        fir_coeffs[coeff_count] = value;
        coeff_count+=1;
    }

    int num_channels = 2, num_samples=512;
    int num_past_arrays = 0;
    int realloc_limit = 8;

     // Buffer containing one sample (left and right, both 16 bit).
    int16_t input_array[num_samples * num_channels];
    int16_t ouptut_array[num_samples * num_channels];
    unsigned buffer_in = sizeof(input_array);
    unsigned buffer_out = sizeof(ouptut_array);

    int16_t *past_arrays = (int16_t *) malloc(realloc_limit*sizeof(int16_t)*num_samples * num_channels);
    
    int got;
    while (1)
    {
       

        // Read one sample from input
        got = read(STDIN_FILENO, input_array, buffer_in);
        if (got < 0)
        {
            fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
            exit(1);
        }
        else if (got == 0)
        {
            break;   // end of file
        }
        else if (got != buffer_in)
        {
            printf("Misread buffer amount\n");
            break;
            // break on EOF buffer in
        }

        double out_value;
        int current_address;
        for (int i = 0; i < num_channels*num_samples; ++i)
        {  
            out_value=0;
            for (int j = 0; j < coeff_count; ++j)
            {
                current_address = i - 2*j;
                if (current_address > 0){
                     out_value += input_array[current_address]*fir_coeffs[j];
                }
                else
                {
                    current_address = num_past_arrays*num_channels*num_samples+current_address;
                    if (current_address >0)
                    {
                        out_value += past_arrays[current_address]*fir_coeffs[j];
                    }
                }
            }
            ouptut_array[i] = out_value;
            past_arrays[i + num_past_arrays*num_channels*num_samples] = input_array[i];
        }
        int done = write(STDOUT_FILENO, ouptut_array, buffer_out);
        if (done < 0)
        {
            fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
            exit(1);
        }
        else if (done != buffer_out)
        {
            fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
        }

        num_past_arrays++;
        if (num_past_arrays == realloc_limit)
        {
            realloc_limit*=2;
            past_arrays = (int16_t *)realloc(past_arrays, (realloc_limit*sizeof(int16_t)*num_samples * num_channels));
        }
    }

    return 0;
}

