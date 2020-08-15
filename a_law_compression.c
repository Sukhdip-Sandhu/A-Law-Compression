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

int8_t a_law_encode(int16_t sample);

int16_t a_law_decode(int8_t number);

// **********************************************************************
// GLOBAL VARIABLES
// **********************************************************************
FILE *input_file;
FILE *output_file;
FILE *recovered_file;
char *input_file_name;
char *output_file_name;
char *recovered_file_name;

// **********************************************************************
// MAIN FUNCTION
// **********************************************************************
int16_t compute_diff(int16_t first, int16_t second){
	int16_t result=first-second;
	if(result>=0){
		return result;
	}
	return -result;
}

int main(int argc, char **argv) {
    char current_directory[1024];
    unsigned char byte_buffer_2[2];
    unsigned char byte_buffer_44[44];
    int16_t input_data;
    int8_t codeword;
    int16_t recovered_data;
    int distortion=0;
    int number_of_codeword=0;

    input_file_name = (char *) malloc(sizeof(char) * 1024);
    output_file_name = (char *) malloc(sizeof(char) * 1024);
    recovered_file_name = (char *) malloc(sizeof(char) * 1024);

    if (getcwd(current_directory, sizeof(current_directory)) != NULL) {
        strcpy(input_file_name, current_directory);
        strcat(input_file_name, "/");
        strcat(input_file_name, argv[1]);
    }

    strcpy(output_file_name, argv[1]);
    output_file_name[strlen(output_file_name) - 4] = '\0';
    strcat(output_file_name, "_tmp.wav");
    
    strcpy(recovered_file_name, argv[1]);
    recovered_file_name[strlen(recovered_file_name) - 4] = '\0';
    strcat(recovered_file_name, "_recovered.wav");

    input_file = fopen(input_file_name, "rb+");
    output_file = fopen(output_file_name, "wb");
    recovered_file = fopen(recovered_file_name, "wb");

    fread(byte_buffer_44, 44, 1, input_file);
    fwrite(byte_buffer_44, 44, 1, output_file);
    fwrite(byte_buffer_44, 44, 1, recovered_file);

    while (fread(byte_buffer_2, 1, 2, input_file) == 2) {
        input_data = bytes_to_int16(byte_buffer_2);
        codeword = a_law_encode(input_data);
        fwrite(&codeword, 1, 1, output_file);
        recovered_data=a_law_decode(codeword);
        fwrite(&recovered_data, 1, 2, recovered_file);
        int16_t diff=compute_diff(recovered_data, input_data);
        printf("Original: %7d\tRecovered: %7d\tDifference: %d\n", input_data, recovered_data, diff);
        distortion=distortion+diff;
        number_of_codeword++;
    }

    fclose(input_file);
    fclose(output_file);
    fclose(recovered_file);
    free(input_file_name);
    free(output_file_name);
    free(recovered_file_name);
    printf("Overall Distortion: %d\nTotal number of codeword compressed: %d\nAverage distortion for each codeword: %d", distortion, number_of_codeword, distortion/number_of_codeword);
    return 0;
}

// **********************************************************************
// HELPER FUNCTIONS
// **********************************************************************
int16_t bytes_to_int16(const unsigned char *buffer) {
    unsigned char bit_one = buffer[0];
    unsigned char bit_two = buffer[1];
    return bit_two << 8 | bit_one;
}

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

int16_t a_law_decode(int8_t number) {
	uint8_t sign = 0x00;
	uint8_t position = 0;
	int16_t decoded = 0;
	number ^= 0x55;
	if (number & 0x80) {
		number &= ~(1 << 7);
		sign = -1;
	}
	position = ((number & 0xF0) >> 4) + 4;
	if (position != 4) {
		decoded = ((1 << position) | ((number & 0x0F) << (position - 4))
				| (1 << (position - 5)));
	} else {
		decoded = (number << 1) | 1;
	}
	return (sign == 0) ? (decoded) : (-decoded);
}
