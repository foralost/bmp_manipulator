/*
 * bmp.h
 *
 *  Created on: 19 kwi 2022
 *      Author: foralost
 */

#ifndef INCLUDE_BMP_H_
#define INCLUDE_BMP_H_
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define BMP_BITMAP_BITMASKS_SIZE 		4
#define BMP_BITS_PER_PIXEL_ONE 			1
#define BMP_BITS_PER_PIXEL_TWO 			2
#define BMP_BITS_PER_PIXEL_FOUR 		4
#define BMP_BITS_PER_PIXEL_EIGHT 		8
#define BMP_BITS_PER_PIXEL_SIXTEEN 		16
#define BMP_BITS_PER_PIXEL_TWENTYFOUR 	24
#define BMP_BITS_PER_PIXEL_THIRTYTWO 	32

#define BMP_COMPRESSION_RGB 			0
#define BMP_COMPRESSION_RLE8 			1
#define BMP_COMPRESSION_RLE4 			2
#define BMP_COMPRESSION_BITFIELDS		3
#define BMP_COMPRESSION_ALPHABITFIELDS 	6

struct dib_header {
	uint32_t header_size;
	uint32_t width;
	uint32_t height;
	uint16_t color_planes;
	uint16_t bits_per_pixel;
	uint32_t compression;
	uint32_t image_size;
	uint32_t horizontal_res;
	uint32_t vertical_res;
	uint32_t no_colors_palette;
	uint32_t important_colors;
	uint8_t  size_extra;
	uint8_t* extra;
};

enum BMP_COLOR {
	RED, BLUE, GREEN, ALPHA
};

enum BMP_RANDOM_TYPE{
	GRADIENT, RGB_PALETTE
};

struct bmp_header {
	char magic_bytes[2];
	uint32_t file_size;
	uint16_t reserved[2];
	uint32_t pixel_start;
};


struct bmp_sample{
	uint16_t compression;
	uint32_t width;
	uint32_t height;
	uint16_t bits_per_pixel;
	uint32_t bit_masks[BMP_BITMAP_BITMASKS_SIZE];
	uint32_t* color_table;
};

struct bmp_structure {
	struct bmp_header bitmap_header;
	struct dib_header dib_header;
	uint32_t bit_masks[BMP_BITMAP_BITMASKS_SIZE];
	uint32_t *color_table;
	uint8_t* pixel_data;
	uint8_t* extra_data;
};
int bmp_create_from_sample(struct bmp_structure *dest, struct bmp_sample *sample_data);

int bmp_make_grayscale(struct bmp_structure *src);

int bmp_make_negate(struct bmp_structure *src);

int bmp_load(char *data, struct bmp_structure *dest);

int bmp_store(const struct bmp_structure *src, FILE* fd);

int bmp_free_structure(struct bmp_structure *src);

int bmp_make_threshold(struct bmp_structure *src, uint8_t threshold_val);

void bmp_generate_random_pixels(struct bmp_structure *dest );


void bmp_create_empty_pixels(struct bmp_structure *dest);
#endif /* INCLUDE_BMP_H_ */
