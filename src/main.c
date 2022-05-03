/*
 * main.c
 *
 *  Created on: 19 kwi 2022
 *      Author: foralost
 */

#include <stdlib.h>
#include <stdio.h>

#include "include/bmp.h"
// ----
#include "args_metadata.c"
// ----

char* load_file(char *src) {
	FILE *fp = fopen(src, "rb");
	fseek(fp, 0, SEEK_END);

	size_t filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *to_ret = malloc(filesize);

	fread(to_ret, filesize, 1, fp);
	fclose(fp);

	return to_ret;
}

int main(int argc, char **argv) {
	struct agrp_arguments def;
	memset(&def, 0, sizeof(def));


	argp_parse(&argp, argc, argv, 0, 0, &def);

	return 0;
}
