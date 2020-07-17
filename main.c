#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// **********************************************************************
// DEFINE STATEMENTS
// **********************************************************************
#define BITS_PER_SAMPLE 8
#define MULAW_MAX 0x1FFF
#define MULAW_BIAS 33
#define ALAW_MAX 0xFFF

// **********************************************************************
// FUNCTION DECLARATIONS
// **********************************************************************
int16_t bytes_to_int16(unsigned char *buffer);

int8_t u_law_encode(int16_t number);
int8_t a_law_encode(int16_t number);

char codeword_compression(unsigned int sample_magnitude, int sign);

// **********************************************************************
// STRUCT
// **********************************************************************
struct wav_header_struct {
    unsigned char riff[4];                        // RIFF string
    unsigned char overall_size[4];                // overall size of file in bytes
    unsigned char wave[4];                        // WAVE string
    unsigned char fmt_chunk_marker[4];            // fmt string with trailing null char
    unsigned char length_of_fmt[4];               // length of the format data
    unsigned char format_type[2];                 // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    unsigned char num_channels[2];                // no.of channels
    unsigned char sample_rate[4];                 // sampling rate (blocks per second)
    unsigned char byte_rate[4];                   // SampleRate * NumChannels * BitsPerSample/8
    unsigned char block_align[2];                 // NumChannels * BitsPerSample/8
    unsigned char bits_per_sample[2];             // bits per sample, 8- 8bits, 16- 16 bits etc
    unsigned char data_chunk_header[4];           // DATA string or FLLR string
    unsigned char data_size[4];                   // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

// **********************************************************************
// GLOBAL VARIABLES
// **********************************************************************
FILE *inputfile;
FILE *outputfile;
char *input_file_name;
char *output_file_name;

struct wav_header_struct wavHeaderStruct;

// **********************************************************************
// MAIN FUNCTION
// **********************************************************************
int main(int argc, char **argv) {
    char current_directory[1024];
    unsigned char byte_buffer_2[2];
    unsigned char byte_buffer_4[4];
    int input_data;
    int8_t codeword;

    input_file_name = (char *) malloc(sizeof(char) * 1024);
    output_file_name = (char *) malloc(sizeof(char) * 1024);

    if (input_file_name == NULL) {
        printf("Error in malloc\n");
        exit(1);
    }

    if (getcwd(current_directory, sizeof(current_directory)) != NULL) {
        strcpy(input_file_name, current_directory);
        if (argc < 2) {
            printf("No input file specified!\n");
            exit(1);
        }
        strcat(input_file_name, "/");
        strcat(input_file_name, argv[1]);
    }

    strcpy(output_file_name, argv[1]);
    output_file_name[strlen(output_file_name) - 4] = '\0';
    strcat(output_file_name, "_output.wav");

    inputfile = fopen(input_file_name, "rb+");
    if (inputfile == NULL) {
        printf("Error opening input file!\n");
        exit(1);
    }

    outputfile = fopen(output_file_name, "wb");
    if (outputfile == NULL) {
        printf("Error writing output file!\n");
        exit(1);
    }


    // Riff - does not change
    fread(wavHeaderStruct.riff, sizeof(wavHeaderStruct.riff), 1, inputfile);
    fwrite(wavHeaderStruct.riff, sizeof(wavHeaderStruct.riff), 1, outputfile);

    // Overall Size (ChunkSize) - the new size must be calculated and written
    fread(wavHeaderStruct.overall_size, sizeof(wavHeaderStruct.overall_size), 1, inputfile);
    unsigned int overall_size = wavHeaderStruct.overall_size[0]
                                | (wavHeaderStruct.overall_size[1] << 8)
                                | (wavHeaderStruct.overall_size[2] << 16)
                                | (wavHeaderStruct.overall_size[3] << 24);
    unsigned int new_overall_size = overall_size + 8 - 44;
    new_overall_size = new_overall_size / 2;
    new_overall_size = new_overall_size + 44 - 8;
    byte_buffer_4[0] = (new_overall_size >> 24) & 0xFF;
    byte_buffer_4[1] = (new_overall_size >> 16) & 0xFF;
    byte_buffer_4[2] = (new_overall_size >> 8) & 0xFF;
    byte_buffer_4[3] = new_overall_size & 0xFF;
    fwrite(&byte_buffer_4[3], 1, 1, outputfile);
    fwrite(&byte_buffer_4[2], 1, 1, outputfile);
    fwrite(&byte_buffer_4[1], 1, 1, outputfile);
    fwrite(&byte_buffer_4[0], 1, 1, outputfile);

    // Wave (Format) - does not change
    fread(wavHeaderStruct.wave, sizeof(wavHeaderStruct.wave), 1, inputfile);
    fwrite(wavHeaderStruct.wave, sizeof(wavHeaderStruct.wave), 1, outputfile);

    // Format Chunk Marker (Subchunk1ID) - does not change
    fread(wavHeaderStruct.fmt_chunk_marker, sizeof(wavHeaderStruct.fmt_chunk_marker), 1, inputfile);
    fwrite(wavHeaderStruct.fmt_chunk_marker, sizeof(wavHeaderStruct.fmt_chunk_marker), 1, outputfile);

    // Length of Format (Subchunk1Size) - may need to be changed but for now it is left alone
    fread(wavHeaderStruct.length_of_fmt, sizeof(wavHeaderStruct.length_of_fmt), 1, inputfile);
/*    byte_buffer_4[0] = 16; byte_buffer_4[1] = '\0';
    byte_buffer_4[2] = '\0'; byte_buffer_4[3] = '\0';
    fwrite(&byte_buffer_4[0], 1, 1, outputfile); fwrite(&byte_buffer_4[1], 1, 1, outputfile);
    fwrite(&byte_buffer_4[2], 1, 1, outputfile); fwrite(&byte_buffer_4[3], 1, 1, outputfile);*/
    fwrite(wavHeaderStruct.length_of_fmt, sizeof(wavHeaderStruct.length_of_fmt), 1, outputfile);

    // Audio Format Type: 1=PCM; 6=ALAW; 7=MuLaw
    fread(wavHeaderStruct.format_type, sizeof(wavHeaderStruct.format_type), 1, inputfile);
    byte_buffer_2[0] = 7;
    byte_buffer_2[1] = '\0';
    fwrite(&byte_buffer_2[0], 1, 1, outputfile);
    fwrite(&byte_buffer_2[1], 1, 1, outputfile);

    // Number of Channels - record the value but do not change it
    fread(wavHeaderStruct.num_channels, sizeof(wavHeaderStruct.num_channels), 1, inputfile);
    unsigned int num_channels = wavHeaderStruct.num_channels[0]
                                | (wavHeaderStruct.num_channels[1] << 8);
    fwrite(wavHeaderStruct.num_channels, sizeof(wavHeaderStruct.num_channels), 1, outputfile);

    // Sample Rate - Record the value but do not change it
    fread(wavHeaderStruct.sample_rate, sizeof(wavHeaderStruct.sample_rate), 1, inputfile);
    unsigned int sample_rate = wavHeaderStruct.sample_rate[0]
                               | (wavHeaderStruct.sample_rate[1] << 8)
                               | (wavHeaderStruct.sample_rate[2] << 16)
                               | (wavHeaderStruct.sample_rate[3] << 24);
    fwrite(wavHeaderStruct.sample_rate, sizeof(wavHeaderStruct.sample_rate), 1, outputfile);

    // Byte Rate = (SampleRate * NumChannels * BitsPerSample) / 8
    fread(wavHeaderStruct.byte_rate, sizeof(wavHeaderStruct.byte_rate), 1, inputfile);
    unsigned int byte_rate = (sample_rate * num_channels * BITS_PER_SAMPLE) / 8;
    byte_buffer_4[0] = (byte_rate >> 24) & 0xFF;
    byte_buffer_4[1] = (byte_rate >> 16) & 0xFF;
    byte_buffer_4[2] = (byte_rate >> 8) & 0xFF;
    byte_buffer_4[3] = byte_rate & 0xFF;
    fwrite(&byte_buffer_4[3], 1, 1, outputfile);
    fwrite(&byte_buffer_4[2], 1, 1, outputfile);
    fwrite(&byte_buffer_4[1], 1, 1, outputfile);
    fwrite(&byte_buffer_4[0], 1, 1, outputfile);

    // Block Align = (NumChannels * BitsPerSample) / 8
    fread(wavHeaderStruct.block_align, sizeof(wavHeaderStruct.block_align), 1, inputfile);
    unsigned int block_align = (num_channels * BITS_PER_SAMPLE) / 8;
    byte_buffer_2[0] = (block_align >> 8) & 0xFF;
    byte_buffer_2[1] = block_align & 0xFF;
    fwrite(&byte_buffer_2[1], 1, 1, outputfile);
    fwrite(&byte_buffer_2[0], 1, 1, outputfile);

    // Bits per sample = 8
    fread(wavHeaderStruct.bits_per_sample, sizeof(wavHeaderStruct.bits_per_sample), 1, inputfile);
    byte_buffer_2[0] = BITS_PER_SAMPLE;
    byte_buffer_2[1] = '\0';
    fwrite(&byte_buffer_2[0], 1, 1, outputfile);
    fwrite(&byte_buffer_2[1], 1, 1, outputfile);

    // Data chunk header - does not change
    fread(wavHeaderStruct.data_chunk_header, sizeof(wavHeaderStruct.data_chunk_header), 1, inputfile);
    fwrite(wavHeaderStruct.data_chunk_header, sizeof(wavHeaderStruct.data_chunk_header), 1, outputfile);

    // Data size - calcuate the size of the remaining input_data
    fread(wavHeaderStruct.data_size, sizeof(wavHeaderStruct.data_size), 1, inputfile);
    int chunk_header = new_overall_size + 8 - 44;
    byte_buffer_4[0] = (chunk_header >> 24) & 0xFF;
    byte_buffer_4[1] = (chunk_header >> 16) & 0xFF;
    byte_buffer_4[2] = (chunk_header >> 8) & 0xFF;
    byte_buffer_4[3] = chunk_header & 0xFF;
    fwrite(&byte_buffer_4[3], 1, 1, outputfile);
    fwrite(&byte_buffer_4[2], 1, 1, outputfile);
    fwrite(&byte_buffer_4[1], 1, 1, outputfile);
    fwrite(&byte_buffer_4[0], 1, 1, outputfile);

    while (fread(byte_buffer_2, 1, 2, inputfile) == 2) {
        input_data = bytes_to_int16(byte_buffer_2);
        codeword = a_law_encode(input_data);
        fwrite(&codeword, 1, 1, outputfile);
    }

    fclose(inputfile);
    fclose(outputfile);
    free(input_file_name);
    free(output_file_name);
    return 0;
}

// HELPER FUNCTIONS
int16_t bytes_to_int16(unsigned char *buffer) {
    unsigned char bit_one = buffer[0];
    unsigned char bit_two = buffer[1];
    int16_t number = ((int) bit_two << 8) | (int) bit_one;
    return number;
}

int8_t u_law_encode(int16_t number) {
    uint16_t mask = 0x1000;
    uint8_t sign = 0;
    uint8_t position = 12;
    uint8_t lsb = 0;
    if (number < 0) {
        number = -number;
        sign = 0x80;
    }
    number += MULAW_BIAS;
    if (number > MULAW_MAX) {
        number = MULAW_MAX;
    }
    for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--);
    lsb = (number >> (position - 4)) & 0x0f;
    return (~(sign | ((position - 5) << 4) | lsb));
}

int8_t a_law_encode(int16_t number){
   uint16_t mask = 0x800;
   uint8_t sign = 0;
   uint8_t position = 11;
   uint8_t lsb = 0;
   if (number < 0)
   {
      number = -number;
      sign = 0x80;
   }
   if (number > ALAW_MAX)
   {
      number = ALAW_MAX;
   }
   for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--);
   lsb = (number >> ((position == 4) ? (1) : (position - 4))) & 0x0f;
   return (sign | ((position - 4) << 4) | lsb) ^ 0x55;
}

/*
char codeword_compression(unsigned int sample_magnitude,
                          int sign) {
    int chord, step;
    int codeword_tmp;
    if (sample_magnitude & (1 << 12)) {
        chord = 0x7;
        step = (sample_magnitude >> 8) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    if (sample_magnitude & (1 << 11)) {
        chord = 0x6;
        step = (sample_magnitude >> 7) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    if (sample_magnitude & (1 << 10)) {
        chord = 0x5;
        step = (sample_magnitude >> 6) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    if (sample_magnitude & (1 << 9)) {
        chord = 0x4;
        step = (sample_magnitude >> 5) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    if (sample_magnitude & (1 << 8)) {
        chord = 0x3;
        step = (sample_magnitude >> 4) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    if (sample_magnitude & (1 << 7)) {
        chord = 0x2;
        step = (sample_magnitude >> 3) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    if (sample_magnitude & (1 << 6)) {
        chord = 0x1;
        step = (sample_magnitude >> 2) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    if (sample_magnitude & (1 << 5)) {
        chord = 0x0;
        step = (sample_magnitude >> 1) & 0xF;
        codeword_tmp = (sign << 7) | (chord << 4) | step;
        return ((char) codeword_tmp);
    }
    return '\0';
}*/
