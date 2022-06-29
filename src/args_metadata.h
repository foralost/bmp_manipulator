/*
 * args_metadata.c
 *
 *  Created on: 30 kwi 2022
 *      Author: foralost
 */

#include <argp.h>
#define PROGRAM_MODE_NEGATE 	0
#define PROGRAM_MODE_GRAYSCALE	1
#define PROGRAM_MODE_THRESHOLD	2

const static char *PROGRAM_MODES_STRS[] = { "NEGATE", "GRAYSCALE", "THRESHOLD" };

const static unsigned char PROGRAM_MODES_LEN[] = { 6, 9, 9 };

const static char argp_args_doc[] = "INPUT_FILE";

const static char argp_doc[] =
		"BMP Manipulator - a simple program created for negating, \
grayscaling, and thresholding BMP images.";

const char *argp_program_bug_address = "szwajkamarek@gmail.com";

struct agrp_arguments {
	char *args[1];
	unsigned char verbose;
	char *mode;
	unsigned char threshold;
	char *output_file;
};

unsigned char parse_str_cmp_bmp(char *input, unsigned long long length,
		unsigned long long mode) {
	if (length != PROGRAM_MODES_LEN[mode])
		return 2; // Wrong length...

	for (size_t i = 0; i < length; i++) {
		if ((input[i] != PROGRAM_MODES_STRS[mode][i])
				&& ((input[i] ^ (1 << 5)) != PROGRAM_MODES_STRS[mode][i]))
			return 1;
	}
	return 0;
}
static struct argp_option options[] =
		{ { "verbose", 'v', 0,
		OPTION_ARG_OPTIONAL, "Produce verbose output" }, { "outputfile", 'o', "OUTPUT", 0,
				"Output bitmap name" },
				{ "mode", 'm', "MODE", 0,
						"Type what BMP conversion method you want to use:\n"
								"GREYSCALE \t - create a grayscale BMP from INPUT_FILE image\n"
								"NAGATE \t - perform negation from INPUT_FILE image\n"
								"THRESHOLD \t - perform a threshold operation. An argument "
								"TH_VALUE (-t, --th_value) must be given within "
								"range. Its value will be used to determine the threshold." },
				{ "th_value", 't', "VALUE", 0,
						"Threshold value for threshold operation. Mandatory for"
								"THRESHOLD mode. Range within 1-255." }, { 0 } };

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct agrp_arguments *arguments = state->input;

	switch (key) {
	case 'v':
		arguments->verbose = 1;
		break;
	case 'o':
		arguments->output_file = arg;
		break;
	case 'm':
		arguments->mode = arg;
		break;
	case 't':
		arguments->threshold = strtol(arg, (char**) NULL, 0);
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num > 1) {
			argp_usage(state);
		} else {
			arguments->args[state->arg_num] = arg;
		}
		break;
	case ARGP_KEY_END:
		if (state->arg_num < 1) {
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, argp_args_doc, argp_doc };
