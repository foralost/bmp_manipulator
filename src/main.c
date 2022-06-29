/*
 * main.c
 *
 *  Created on: 19 kwi 2022
 *      Author: foralost
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/bmp.h"
#include "args_metadata.h"

// ----

struct agrp_arguments program_state;

FILE* open_file(char *input, char* mode) {

	FILE *to_ret = fopen(input, mode);

	if (!to_ret) {
		perror("fopen");
		return NULL;
	} else {
		return to_ret;
	}

}

void check_files(FILE *checking_file, char *type) {
	if (!checking_file) {
		fprintf(stderr, "Wrong %.32s file.\n", type);
		exit(1);
	}
}

char* load_file(FILE *f) {
	fseek(f, 0, SEEK_END);
	long int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* to_ret = malloc(fsize);

	fread(to_ret, fsize, 1, f);
	return to_ret;
}

int main(int argc, char **argv) {

	// Parse arguments
	// input file, output file, verbose, which option, if threshold, download threshold value

	memset(&program_state, 0, sizeof(program_state));

	argp_parse(&argp, argc, argv, 0, 0, &program_state);

	// Validate arguments
	FILE *in = open_file(program_state.args[0], "rb");

	check_files(in, "input");

	FILE *out = open_file(program_state.output_file, "wb");

	check_files(out, "output");

	uint8_t curr_mode = 0;
	size_t mode_len = strlen(program_state.mode);

	if (!parse_str_cmp_bmp(program_state.mode, mode_len,
	PROGRAM_MODE_GRAYSCALE)) {
		curr_mode = PROGRAM_MODE_GRAYSCALE;
	} else if (!parse_str_cmp_bmp(program_state.mode, mode_len,
	PROGRAM_MODE_NEGATE)) {
		curr_mode = PROGRAM_MODE_NEGATE;
	} else if (!parse_str_cmp_bmp(program_state.mode, mode_len,
	PROGRAM_MODE_THRESHOLD)) {

		if (!program_state.threshold) {
			fprintf(stderr, "Zero threshold value.\n");
			return -1;
		}

		curr_mode = PROGRAM_MODE_THRESHOLD;
	} else {
		fprintf(stderr, "Invalid mode given.\n");
		return -1;
	}

	// Arguments OK
	char *input_memory = load_file(in);
	{
		struct bmp_structure working_bmp;
		if (bmp_load(input_memory, &working_bmp)) {
			fprintf(stderr, "Could not load bitmap into programs memory.\n");
			return -1;
		}

		switch (curr_mode) {
		case PROGRAM_MODE_GRAYSCALE:
			bmp_make_grayscale(&working_bmp);
			break;
		case PROGRAM_MODE_NEGATE:
			bmp_make_negate(&working_bmp);
			break;
		case PROGRAM_MODE_THRESHOLD:
			bmp_make_threshold(&working_bmp, program_state.threshold);
			break;
		}

		bmp_store(&working_bmp, out);

	}
	return 0;
}
