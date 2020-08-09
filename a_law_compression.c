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
int16_t bytes_to_int16(const unsigned char *buffer);

int8_t a_law_encode(int16_t number);

// **********************************************************************
// GLOBAL VARIABLES
// **********************************************************************
FILE *inputfile;
FILE *outputfile;
char *input_file_name;
char *output_file_name;

// **********************************************************************
// MAIN FUNCTION
// **********************************************************************
int main(int argc, char **argv) {
    char current_directory[1024];
    unsigned char byte_buffer_2[2];
    unsigned char byte_buffer_44[44];
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

    inputfile = fopen(input_file_name, "rb+");
    outputfile = fopen(output_file_name, "wb");

    fread(byte_buffer_44, 44, 1, inputfile);
    fwrite(byte_buffer_44, 44, 1, outputfile);

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
int16_t bytes_to_int16(const unsigned char *buffer) {
    unsigned char bit_one = buffer[0];
    unsigned char bit_two = buffer[1];
    return bit_two << 8 | bit_one;
}

int8_t a_law_encode(int16_t number) {
    uint16_t mask = 0x800;
    uint8_t sign = 0;
    uint8_t position = 11;
    uint8_t lsb = 0;

    if (number < 0) {
        number = -number;
        sign = 0x80;
    }

    if (number > ALAW_MAX) {
        number = ALAW_MAX;
    }

    for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--);
    lsb = (number >> ((position == 4) ? (1) : (position - 4))) & 0x0f;
    return (sign | ((position - 4) << 4) | lsb) ^ 0x55;
}