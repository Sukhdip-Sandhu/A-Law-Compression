#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// **********************************************************************
// DEFINE STATEMENTS
// **********************************************************************
#define ALAW_MAX 0xFFF

#define bytes_to_int16(byte_one, byte_two) byte_one | byte_two << 8

// **********************************************************************
// FUNCTION DECLARATIONS
// **********************************************************************
int8_t a_law_encode(int16_t sample);

// **********************************************************************
// MAIN FUNCTION
// **********************************************************************
int main(int argc, char **argv) {
    FILE *input_file, *output_file;
    char current_directory[1024];
    char *input_file_name, *output_file_name;
    unsigned char file_header_buffer[44];
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
    strcat(output_file_name, "_tmp.wav");

    input_file = fopen(input_file_name, "rb+");
    output_file = fopen(output_file_name, "wb");

    fread(file_header_buffer, 44, 1, input_file);
    fwrite(file_header_buffer, 44, 1, output_file);

    unsigned int overall_size = file_header_buffer[4]
                                | (file_header_buffer[5] << 8)
                                | (file_header_buffer[6] << 16)
                                | (file_header_buffer[7] << 24);

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