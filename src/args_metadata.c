/*
 * args_metadata.c
 *
 *  Created on: 30 kwi 2022
 *      Author: foralost
 */

#include <argp.h>
/*
 * Arguments: input_bmp, mode = {THRESHOLD, NEGATE, GRAYSCALE}
 * 						 if mode == THRESHOLD:
 * 						 		additional argument (threshold value)
 * 						 we could also do verbose?
 */

const static char argp_args_doc[] = "INPUT_FILE OUTPUT_FILE MODE";

const static char argp_doc[] =
		"BMP Manipulator - a simple program created for negating, \
grayscaling, and thresholding BMP images.";

//const static char argp_program_bug_address[] = "szwajkamarek@gmail.com";

#define ARG_COUNT 6
struct agrp_arguments {
	char *args[ARG_COUNT]; /* ARG1 and ARG2 */
	char verbose; /* The -v flag */
	char mode; /* Argument for -o */
	char threshold;
	char *argv[2]; /* Arguments for -a and -b */
};

static struct argp_option options[] = {
		{ "verbose", 'v', 0, OPTION_ARG_OPTIONAL,
		"Produce verbose output" },
		{ "inputfile", 'i', "INPUT_FILE", 0,
		"Input bitmap name" },
		{ "outputfile", 'o', "OUTPUT_FILE", 0,
		"Output bitmap name" },
		{ "mode", 'm', "MODE", 0,
		"Type what BMP conversion method you want to use:\n"
				"GREYSCALE \t - create a grayscale BMP from INPUT_FILE image\n"
				"NAGATE \t - perform negation from INPUT_FO:E image\n"
				"THRESHOLD \t - perform a threshold operation. An argument "
				"THRESHOLD_VALUE (-t, --threshold_value) must be given within "
				"range. Its value will be used to determine the threshold." },
				{ "threshold_value", 't', "THRESHOLD_VAL", OPTION_ARG_OPTIONAL,
		"Threshold value for threshold operation. Mandatory for"
				"THRESHOLD mode. Range within 0-255." }, { 0 } };

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct agrp_arguments *arguments = state->input;

	switch (key) {
	case 'v':
	case 'i':
	case 'o':
	case 'm':
		arguments->verbose = 1;
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num >= ARG_COUNT) {
			argp_usage(state);
			break;
		}
		arguments->args[state->arg_num] = arg;
		break;
	case ARGP_KEY_END:
		if (state->arg_num < ARG_COUNT) {
			argp_usage(state);
			break;
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, argp_args_doc, argp_doc };
