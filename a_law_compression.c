#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// **********************************************************************
// DEFINE STATEMENTS
// **********************************************************************
#define ALAW_MAX 0xFFF

// **********************************************************************
// FUNCTION DECLARATIONS
// **********************************************************************

int8_t a_law_encode(int16_t sample);

// **********************************************************************
// GLOBAL VARIABLES
// **********************************************************************
FILE *input_file;
FILE *output_file;
char *input_file_name;
char *output_file_name;

// **********************************************************************
// MAIN FUNCTION
// **********************************************************************
int main(int argc, char **argv) {
    char current_directory[1024];
    unsigned char *file_data_buffer;
    unsigned char file_header_buffer[44];
    int16_t input_data;
    int8_t codeword;

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

    file_data_buffer = malloc(overall_size * sizeof(char));
    fread(file_data_buffer, overall_size, 1, input_file);

    for (int i = 0; i < overall_size; i = i + 2) {
        input_data = file_data_buffer[i + 1] << 8 | file_data_buffer[i];
        codeword = a_law_encode(input_data);
        fwrite(&codeword, 1, 1, output_file);
    }

    fclose(input_file);
    fclose(output_file);
    free(input_file_name);
    free(output_file_name);
    free(file_data_buffer);
    return 0;
}

// **********************************************************************
// HELPER FUNCTIONS
// **********************************************************************

int8_t a_law_encode(int16_t sample) {
    uint16_t mask = 0x800;
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

    for (msb_position = 11; msb_position >= 5; msb_position--) {
        if ((sample & mask)) {
            break;
        } else {
            mask >>= 1;
        }
    }

    if (msb_position == 4) {
        step = (sample >> 1) & 0x0F;
    } else {
        step = (sample >> (msb_position - 4)) & 0x0F;
    }

    chord = ((msb_position - 4) << 4);

    return (sign | chord | step) ^ 0x55;
}