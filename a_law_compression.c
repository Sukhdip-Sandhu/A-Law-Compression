#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// **********************************************************************
// DEFINE STATEMENTS
// **********************************************************************
#define ALAW_MAX 0xFFF

#define BITS_PER_SAMPLE 8

#define bytes_to_int16(byte_one, byte_two) byte_one | byte_two << 8

// **********************************************************************
// FUNCTION DECLARATIONS
// **********************************************************************
int8_t a_law_encode(int16_t sample);

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
// MAIN FUNCTION
// **********************************************************************
int main(int argc, char **argv) {
    FILE *input_file, *output_file;
    char current_directory[1024];
    char *input_file_name, *output_file_name;
    unsigned char byte_buffer_2[2], byte_buffer_4[4];
    struct wav_header_struct wav_header_struct;
    unsigned char *inputfile_data_buffer, *output_file_data_buffer;
    int16_t input_data1, input_data2, input_data3, input_data4;
    int8_t codeword1, codeword2, codeword3, codeword4;

    input_file_name = (char *) malloc(sizeof(char) * 1024);
    output_file_name = (char *) malloc(sizeof(char) * 1024);

    if (getcwd(current_directory, sizeof(current_directory)) != NULL) {
        strcpy(input_file_name, current_directory);
        strcat(input_file_name, "/");
        strcat(input_file_name, argv[1]);
    }

    strcpy(output_file_name, argv[1]);
    output_file_name[strlen(output_file_name) - 4] = '\0';
    strcat(output_file_name, "_compressed.wav");

    input_file = fopen(input_file_name, "rb+");
    output_file = fopen(output_file_name, "wb");

    // Riff - does not change
    fread(wav_header_struct.riff, sizeof(wav_header_struct.riff), 1, input_file);
    fwrite(wav_header_struct.riff, sizeof(wav_header_struct.riff), 1, output_file);

    // Overall Size (ChunkSize) - the new size must be calculated and written
    fread(wav_header_struct.overall_size, sizeof(wav_header_struct.overall_size), 1, input_file);
    unsigned int overall_size = wav_header_struct.overall_size[0]
                                | (wav_header_struct.overall_size[1] << 8)
                                | (wav_header_struct.overall_size[2] << 16)
                                | (wav_header_struct.overall_size[3] << 24);
    unsigned int new_overall_size = overall_size + 8 - 44;
    new_overall_size = new_overall_size / 2;
    new_overall_size = new_overall_size + 44 - 8;
    byte_buffer_4[0] = (new_overall_size >> 24) & 0xFF;
    byte_buffer_4[1] = (new_overall_size >> 16) & 0xFF;
    byte_buffer_4[2] = (new_overall_size >> 8) & 0xFF;
    byte_buffer_4[3] = new_overall_size & 0xFF;
    fwrite(&byte_buffer_4[3], 1, 1, output_file);
    fwrite(&byte_buffer_4[2], 1, 1, output_file);
    fwrite(&byte_buffer_4[1], 1, 1, output_file);
    fwrite(&byte_buffer_4[0], 1, 1, output_file);

    // Wave (Format) - does not change
    fread(wav_header_struct.wave, sizeof(wav_header_struct.wave), 1, input_file);
    fwrite(wav_header_struct.wave, sizeof(wav_header_struct.wave), 1, output_file);

    // Format Chunk Marker (Subchunk1ID) - does not change
    fread(wav_header_struct.fmt_chunk_marker, sizeof(wav_header_struct.fmt_chunk_marker), 1, input_file);
    fwrite(wav_header_struct.fmt_chunk_marker, sizeof(wav_header_struct.fmt_chunk_marker), 1, output_file);

    // Length of Format (Subchunk1Size) - may need to be changed but for now it is left alone
    fread(wav_header_struct.length_of_fmt, sizeof(wav_header_struct.length_of_fmt), 1, input_file);
    fwrite(wav_header_struct.length_of_fmt, sizeof(wav_header_struct.length_of_fmt), 1, output_file);

    // Audio Format Type: 1=PCM; 6=ALAW; 7=MuLaw
    fread(wav_header_struct.format_type, sizeof(wav_header_struct.format_type), 1, input_file);
    byte_buffer_2[0] = 6;
    byte_buffer_2[1] = '\0';
    fwrite(&byte_buffer_2[0], 1, 1, output_file);
    fwrite(&byte_buffer_2[1], 1, 1, output_file);

    // Number of Channels - record the value but do not change it
    fread(wav_header_struct.num_channels, sizeof(wav_header_struct.num_channels), 1, input_file);
    unsigned int num_channels = wav_header_struct.num_channels[0]
                                | (wav_header_struct.num_channels[1] << 8);
    fwrite(wav_header_struct.num_channels, sizeof(wav_header_struct.num_channels), 1, output_file);

    // Sample Rate - Record the value but do not change it
    fread(wav_header_struct.sample_rate, sizeof(wav_header_struct.sample_rate), 1, input_file);
    unsigned int sample_rate = wav_header_struct.sample_rate[0]
                               | (wav_header_struct.sample_rate[1] << 8)
                               | (wav_header_struct.sample_rate[2] << 16)
                               | (wav_header_struct.sample_rate[3] << 24);
    fwrite(wav_header_struct.sample_rate, sizeof(wav_header_struct.sample_rate), 1, output_file);

    // Byte Rate = (SampleRate * NumChannels * BitsPerSample) / 8
    fread(wav_header_struct.byte_rate, sizeof(wav_header_struct.byte_rate), 1, input_file);
    unsigned int byte_rate = (sample_rate * num_channels * BITS_PER_SAMPLE) / 8;
    byte_buffer_4[0] = (byte_rate >> 24) & 0xFF;
    byte_buffer_4[1] = (byte_rate >> 16) & 0xFF;
    byte_buffer_4[2] = (byte_rate >> 8) & 0xFF;
    byte_buffer_4[3] = byte_rate & 0xFF;
    fwrite(&byte_buffer_4[3], 1, 1, output_file);
    fwrite(&byte_buffer_4[2], 1, 1, output_file);
    fwrite(&byte_buffer_4[1], 1, 1, output_file);
    fwrite(&byte_buffer_4[0], 1, 1, output_file);

    // Block Align = (NumChannels * BitsPerSample) / 8
    fread(wav_header_struct.block_align, sizeof(wav_header_struct.block_align), 1, input_file);
    unsigned int block_align = (num_channels * BITS_PER_SAMPLE) / 8;
    byte_buffer_2[0] = (block_align >> 8) & 0xFF;
    byte_buffer_2[1] = block_align & 0xFF;
    fwrite(&byte_buffer_2[1], 1, 1, output_file);
    fwrite(&byte_buffer_2[0], 1, 1, output_file);

    // Bits per sample = 8
    fread(wav_header_struct.bits_per_sample, sizeof(wav_header_struct.bits_per_sample), 1, input_file);
    byte_buffer_2[0] = BITS_PER_SAMPLE;
    byte_buffer_2[1] = '\0';
    fwrite(&byte_buffer_2[0], 1, 1, output_file);
    fwrite(&byte_buffer_2[1], 1, 1, output_file);

    // Data chunk header - does not change
    fread(wav_header_struct.data_chunk_header, sizeof(wav_header_struct.data_chunk_header), 1, input_file);
    fwrite(wav_header_struct.data_chunk_header, sizeof(wav_header_struct.data_chunk_header), 1, output_file);

    // Data size - calculate the size of the remaining input_data
    fread(wav_header_struct.data_size, sizeof(wav_header_struct.data_size), 1, input_file);
    int chunk_header = new_overall_size + 8 - 44;
    byte_buffer_4[0] = (chunk_header >> 24) & 0xFF;
    byte_buffer_4[1] = (chunk_header >> 16) & 0xFF;
    byte_buffer_4[2] = (chunk_header >> 8) & 0xFF;
    byte_buffer_4[3] = chunk_header & 0xFF;
    fwrite(&byte_buffer_4[3], 1, 1, output_file);
    fwrite(&byte_buffer_4[2], 1, 1, output_file);
    fwrite(&byte_buffer_4[1], 1, 1, output_file);
    fwrite(&byte_buffer_4[0], 1, 1, output_file);

    overall_size -= 36;

    inputfile_data_buffer = malloc(overall_size * sizeof(char));
    output_file_data_buffer = malloc((overall_size / 2) * sizeof(char));

    fread(inputfile_data_buffer, overall_size, 1, input_file);

    int i;
    for (i = 0; i < overall_size; i = i + 8) {
        input_data1 = bytes_to_int16(inputfile_data_buffer[i], inputfile_data_buffer[i + 1]);
        input_data2 = bytes_to_int16(inputfile_data_buffer[i + 2], inputfile_data_buffer[i + 3]);
        input_data3 = bytes_to_int16(inputfile_data_buffer[i + 4], inputfile_data_buffer[i + 5]);
        input_data4 = bytes_to_int16(inputfile_data_buffer[i + 6], inputfile_data_buffer[i + 7]);
        codeword1 = a_law_encode(input_data1);
        codeword2 = a_law_encode(input_data2);
        codeword3 = a_law_encode(input_data3);
        codeword4 = a_law_encode(input_data4);
        output_file_data_buffer[i / 2] = codeword1;
        output_file_data_buffer[(i / 2) + 1] = codeword2;
        output_file_data_buffer[(i / 2) + 2] = codeword3;
        output_file_data_buffer[(i / 2) + 3] = codeword4;
    }

    fwrite(output_file_data_buffer, (overall_size / 2), 1, output_file);

    fclose(input_file);
    fclose(output_file);
    free(input_file_name);
    free(output_file_name);
    free(inputfile_data_buffer);
    free(output_file_data_buffer);
    return 0;
}

// **********************************************************************
// HELPER FUNCTIONS
// **********************************************************************
int8_t a_law_encode(int16_t sample) {
    uint8_t sign = 0;
    uint8_t step = 0;
    uint8_t msb_position;
    uint8_t chord;

    if (sample < 0) {
        sample = -sample;
        sign = 0x80;
    }

    if (sample > ALAW_MAX) {
        sample = ALAW_MAX;
    }

    msb_position = 31 - __builtin_clz(sample);

    if (msb_position <= 4) {
        msb_position = 4;
        step = (sample >> 1) & 0x0F;
    } else {
        step = (sample >> (msb_position - 4)) & 0x0F;
    }

    chord = ((msb_position - 4) << 4);

    return (sign | chord | step) ^ 0x55;
}