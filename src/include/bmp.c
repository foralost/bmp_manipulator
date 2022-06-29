/*
 * bmp.c
 *
 *  Created on: 19 kwi 2022
 *      Author: foralost
 */

#include "bmp.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define BMP_BITMAP_HEADER_SIZE 0x0A

#define BMP_DIB_HEADER_SIZE 0x28

#define BMP_BMP_STRUCT_SIZE sizeof(char)*2 + sizeof(uint32_t) * 2 + sizeof(uint16_t)*2
#define BMP_DIB_STRUCT_SIZE sizeof(uint32_t) * 9 + sizeof(uint16_t)*2

uint8_t __BMP_THRESHOLD_VALUE = 0xFF;
void __bmp_set_magic_bytes(char *magic_bytes) {
	magic_bytes[0] = 'B';
	magic_bytes[1] = 'M';
}

size_t __bmp_get_offset_row_end(uint32_t width, uint16_t bits_per_pixel) {
	// Each row must be packed with 32 bit values
	// If values are not packed, they are padded
	// Suppose we have a width 615 pixels, that gives us 615*1 bits (if bits_per_pixel = 1)
	// 615 % 32

	uint64_t calculated = width * bits_per_pixel;
	calculated += calculated & 0b11111;
	return calculated;
}

uint32_t __bmp_write_pixel_rgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a,
		uint16_t bits_per_pixel) {
	switch (bits_per_pixel) {
	case BMP_BITS_PER_PIXEL_SIXTEEN:
		return ((a & 0xF) << 12) | ((r & 0xF) << 8) | ((g & 0xF) << 4)
				| (b & 0xF);
	case BMP_BITS_PER_PIXEL_TWENTYFOUR:
		return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
	case BMP_BITS_PER_PIXEL_THIRTYTWO:
	default:
		return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8)
				| (b & 0xFF);

	}
}

uint8_t __bmp_read_pixel_rgb_index_colortable(uint8_t color_byte,
		uint16_t bits_per_pixel, uint8_t pixel_within_byte,
		uint32_t *color_table) {
	uint8_t byte_mask = 1;
	for (uint8_t i = 0; i < bits_per_pixel; i++) {
		byte_mask <<= 1;
		byte_mask++;
	}

	byte_mask <<= pixel_within_byte;

	return color_byte & byte_mask;
}

void __bmp_process_grey_scale_rgb_ct(uint32_t *color_entry) {
	uint32_t color = *color_entry;
	double avg = ((double) (color & 0xFF) + ((color >> 8) & 0xFF)
			+ ((color >> 16) & 0xFF)) / 3;
	uint8_t avg_byte = (uint8_t) avg;
	*color_entry = (avg_byte << 16) | (avg_byte << 8) | avg_byte;
}

void __bmp_process_negate_ct(uint32_t *color_entry) {
	*color_entry ^= -1;
}

void __bmp_process_grey_scale_rgb(uint8_t *pixel_data, size_t byte_offset,
		uint16_t bits_per_pixel) {
	switch (bits_per_pixel) {
	case BMP_BITS_PER_PIXEL_SIXTEEN:
		break; // no imp
	case BMP_BITS_PER_PIXEL_TWENTYFOUR:
	case BMP_BITS_PER_PIXEL_THIRTYTWO:
		double avg = ((double) pixel_data[byte_offset]
				+ pixel_data[byte_offset + 1] + pixel_data[byte_offset + 2])
				/ 3; // memoization possible?
		pixel_data[byte_offset] = (uint8_t) avg;
		pixel_data[byte_offset + 1] = (uint8_t) avg;
		pixel_data[byte_offset + 2] = (uint8_t) avg;
		break;
	default:
		break;
	}
}

void __bmp_process_threshold_rgb(uint8_t *pixel_data, size_t byte_offset,
		uint16_t bits_per_pixel) {

	switch (bits_per_pixel) {
	case BMP_BITS_PER_PIXEL_SIXTEEN:
		break; // no imp
	case BMP_BITS_PER_PIXEL_TWENTYFOUR:
	case BMP_BITS_PER_PIXEL_THIRTYTWO:

		double avg = ((double) pixel_data[byte_offset]
				+ pixel_data[byte_offset + 1] + pixel_data[byte_offset + 2])
				/ 3; // memoization possible?
		if ((uint8_t) avg > __BMP_THRESHOLD_VALUE) {
			pixel_data[byte_offset] = 0xFF;
			pixel_data[byte_offset + 1] = 0xFF;
			pixel_data[byte_offset + 2] = 0xFF;
		} else {
			pixel_data[byte_offset] = (uint8_t) 0;
			pixel_data[byte_offset + 1] = (uint8_t) 0;
			pixel_data[byte_offset + 2] = (uint8_t) 0;
		}

		break;
	default:
		break;
	}
}

void __bmp_process_threshold_rgb_ct(uint32_t *color_entry) {
	uint32_t color = *color_entry;
	double avg = ((double) (color & 0xFF) + ((color >> 8) & 0xFF)
			+ ((color >> 16) & 0xFF)) / 3;

	uint8_t avg_byte = (uint8_t) avg;
	if (avg_byte > __BMP_THRESHOLD_VALUE)
		avg_byte = 0xFF;
	else
		avg_byte = 0;

	*color_entry = (avg_byte << 16) | (avg_byte << 8) | avg_byte;
}

void __bmp_process_negate_rgb(uint8_t *pixel_data, size_t byte_offset,
		uint16_t bits_per_pixel) {
	switch (bits_per_pixel) {
	case BMP_BITS_PER_PIXEL_SIXTEEN:
		break; // no imp
	case BMP_BITS_PER_PIXEL_TWENTYFOUR:
	case BMP_BITS_PER_PIXEL_THIRTYTWO:
		pixel_data[byte_offset] ^= -1;
		pixel_data[byte_offset + 1] ^= -1;
		pixel_data[byte_offset + 2] ^= -1;
		break;
	default:
		break;
	}
}

void __bmp_process_whole_pixel_data_ct(uint16_t bits_per_pixel,
		uint32_t *color_table, void __func_process_ct(uint32_t*)) {

	if (bits_per_pixel != BMP_BITS_PER_PIXEL_ONE
			&& bits_per_pixel != BMP_BITS_PER_PIXEL_TWO
			&& bits_per_pixel != BMP_BITS_PER_PIXEL_FOUR
			&& bits_per_pixel != BMP_BITS_PER_PIXEL_EIGHT)
		return;

	uint32_t color_table_entries = 1 << bits_per_pixel;

	for (uint32_t i = 0; i < color_table_entries; i++) {
		__func_process_ct(color_table + i);
	}

	return;

}
void __bmp_process_whole_pixel_data(uint8_t *pixel_data,
		uint16_t bits_per_pixel, size_t row_size_bits, uint32_t height,
		void __func_process_pixel(uint8_t*, size_t, uint16_t)) {

	if (bits_per_pixel != BMP_BITS_PER_PIXEL_SIXTEEN
			&& bits_per_pixel != BMP_BITS_PER_PIXEL_TWENTYFOUR
			&& bits_per_pixel != BMP_BITS_PER_PIXEL_THIRTYTWO)
		return;

	uint32_t pixels_in_row = 0;
	uint8_t bytes_per_pixel = bits_per_pixel >> 3;
	size_t all_byte_pixels_in_row = row_size_bits / bits_per_pixel;

	for (uint32_t curr_height = 0; curr_height < height; curr_height++) {
		pixels_in_row = 0;
		while (pixels_in_row < all_byte_pixels_in_row) {
			__func_process_pixel(pixel_data,
					((all_byte_pixels_in_row * curr_height) + pixels_in_row)
							* bytes_per_pixel, bits_per_pixel);

			pixels_in_row++;
		}
	}

	return;
}
int bmp_create_from_sample(struct bmp_structure *dest,
		struct bmp_sample *sample_data) {
	memset(dest, 0, sizeof(*dest));
	__bmp_set_magic_bytes(dest->bitmap_header.magic_bytes);

	size_t bits_per_row = __bmp_get_offset_row_end(sample_data->width,
			sample_data->bits_per_pixel);
	size_t image_size_bytes = (bits_per_row * sample_data->height) >> 3;

	dest->dib_header.height = sample_data->height;
	dest->dib_header.width = sample_data->width;
	dest->dib_header.bits_per_pixel = sample_data->bits_per_pixel;
	dest->dib_header.compression = sample_data->compression;
	dest->bitmap_header.pixel_start = 0;

	if ((sample_data->compression == BMP_COMPRESSION_ALPHABITFIELDS
			|| sample_data->compression == BMP_COMPRESSION_BITFIELDS)
			&& sample_data->bits_per_pixel <= 8) {
		uint16_t color_table_size = 1 << sample_data->bits_per_pixel;
		dest->color_table = malloc(
				color_table_size * sizeof(*dest->color_table));
		dest->bitmap_header.pixel_start += color_table_size;
		memcpy(dest->color_table, sample_data->color_table, color_table_size);
	}

	dest->dib_header.image_size = image_size_bytes;

	if (!dest->dib_header.header_size)
		dest->dib_header.header_size = BMP_DIB_HEADER_SIZE;

	dest->dib_header.color_planes = 1;
	dest->bitmap_header.pixel_start +=
	BMP_DIB_STRUCT_SIZE + BMP_BMP_STRUCT_SIZE;

	dest->bitmap_header.file_size = BMP_DIB_STRUCT_SIZE + BMP_BMP_STRUCT_SIZE
			+ image_size_bytes;

	return 0;

}


void __bmp_make_process(struct bmp_structure *src,
		void __bmp_process_ct(uint32_t*),
		void __bmp_process_rgb(uint8_t*, size_t, uint16_t)) {
	size_t row_size = __bmp_get_offset_row_end(src->dib_header.width,
			src->dib_header.bits_per_pixel);

	if (src->dib_header.compression == BMP_COMPRESSION_ALPHABITFIELDS
			|| src->dib_header.compression == BMP_COMPRESSION_BITFIELDS) {
		__bmp_process_whole_pixel_data_ct(src->dib_header.bits_per_pixel,
				src->color_table, __bmp_process_ct);
	} else {
		__bmp_process_whole_pixel_data(src->pixel_data,
				src->dib_header.bits_per_pixel, row_size,
				src->dib_header.height, __bmp_process_rgb);
	}

}

int bmp_make_threshold(struct bmp_structure *src, uint8_t threshold_val) {
	__BMP_THRESHOLD_VALUE = threshold_val;

	__bmp_make_process(src, __bmp_process_threshold_rgb_ct,
			__bmp_process_threshold_rgb);
	return 0;
}
int bmp_make_negate(struct bmp_structure *src) {

	__bmp_make_process(src, __bmp_process_negate_ct, __bmp_process_negate_rgb);

	return 0;
}

int bmp_make_grayscale(struct bmp_structure *src) {

	__bmp_make_process(src, __bmp_process_grey_scale_rgb_ct,
			__bmp_process_grey_scale_rgb);

	return 0;
}

size_t __bmp_get_padding_bits_count(uint32_t used_pixels, size_t row_end_offset,
		uint16_t bits_per_pixel) {
	uint64_t used_bits = used_pixels * bits_per_pixel;

	return row_end_offset - used_bits;
}

size_t __bmp_get_offset_from_coord(uint32_t row, uint32_t column, uint16_t bpp,
		size_t row_size) {

	return (row * row_size) + (column * bpp);
}
size_t __bmp_get_image_size(const struct bmp_structure *src) {
	size_t image_alloc;
	if (!src->dib_header.image_size) {
		image_alloc = src->bitmap_header.file_size - BMP_BITMAP_HEADER_SIZE
				- src->dib_header.header_size;
	} else {
		image_alloc = src->dib_header.image_size;
	}
	return image_alloc;
}
int bmp_load(char *data, struct bmp_structure *dest) {
	struct bmp_structure to_ret;

	if (data[0] != 'B' || data[1] != 'M') {
		fprintf(stderr, "Wrong magic bytes. Received %c %c\n", data[0],
				data[1]);
		return -1;
	}
	to_ret.bitmap_header.magic_bytes[0] = 'B';
	to_ret.bitmap_header.magic_bytes[1] = 'M';

	memcpy(&to_ret.bitmap_header.file_size, data + 2, 12);
	memcpy(&to_ret.dib_header.header_size, data + 0x0e, BMP_DIB_HEADER_SIZE);

	if (to_ret.dib_header.header_size != BMP_DIB_HEADER_SIZE) {
		fprintf(stderr, "Nonstandard header size: %x\n",
				to_ret.dib_header.header_size);

		to_ret.dib_header.size_extra = to_ret.dib_header.header_size
				- BMP_DIB_HEADER_SIZE;

		to_ret.dib_header.extra = malloc(to_ret.dib_header.size_extra);

		memcpy(to_ret.dib_header.extra, data + 0x0e + BMP_DIB_HEADER_SIZE,
				to_ret.dib_header.size_extra);
	} else {
		to_ret.dib_header.size_extra = 0;
	}

	if (to_ret.bitmap_header.pixel_start >= to_ret.bitmap_header.file_size) {
		fprintf(stderr,
				"Pixel array starts away from file. Pixel array start %x, file size %x.\n",
				to_ret.bitmap_header.pixel_start,
				to_ret.bitmap_header.file_size);

		if(to_ret.dib_header.extra)
			free(to_ret.dib_header.extra);
		return -1;
	}

	if (to_ret.dib_header.compression == BMP_COMPRESSION_ALPHABITFIELDS) {
		memcpy(&to_ret.bit_masks,
				data + to_ret.dib_header.header_size + BMP_BITMAP_HEADER_SIZE,
				16);

	} else if (to_ret.dib_header.compression == BMP_COMPRESSION_BITFIELDS) {
		memcpy(&to_ret.bit_masks,
				data + to_ret.dib_header.header_size + BMP_BITMAP_HEADER_SIZE,
				12);
	}

	if (to_ret.dib_header.bits_per_pixel <= 8) {
		to_ret.color_table = malloc(1 << to_ret.dib_header.bits_per_pixel);
		memcpy(&to_ret.color_table,
				data + BMP_BITMAP_HEADER_SIZE + to_ret.dib_header.header_size,
				1 << to_ret.dib_header.bits_per_pixel);
	}

	size_t image_size = __bmp_get_image_size(&to_ret);
	to_ret.pixel_data = malloc(image_size);
	memcpy(to_ret.pixel_data, data + to_ret.bitmap_header.pixel_start,
			image_size);

	*dest = to_ret;

//TODO: Maybe image size comparison?
	return 0;
}

int bmp_store(const struct bmp_structure *src, FILE *dest) {

// BITMAP Header
	fwrite(&src->bitmap_header.magic_bytes, 2, 1, dest);

	fwrite(&src->bitmap_header.file_size, 12, 1, dest);

// DIB Header

	fwrite(&src->dib_header.header_size, BMP_DIB_HEADER_SIZE, 1, dest);

	if (src->dib_header.size_extra)
		fwrite(src->dib_header.extra, src->dib_header.size_extra, 1, dest);

// Bit Masks
	if (src->dib_header.compression == BMP_COMPRESSION_ALPHABITFIELDS) {
		fwrite(src->bit_masks, sizeof(*src->bit_masks) * 4, 1, dest);
	} else if (src->dib_header.compression == BMP_COMPRESSION_BITFIELDS) {
		fwrite(src->bit_masks, sizeof(*src->bit_masks) * 3, 1, dest);
	}
// Color Table
	if (src->color_table && src->dib_header.bits_per_pixel <= 8) {
		fwrite(src->color_table,
				sizeof(*src->color_table) * src->dib_header.bits_per_pixel, 1,
				dest);
	}

// Pixel Array
	fwrite(src->pixel_data, __bmp_get_image_size(src), 1, dest);
	return 0;
}

int bmp_free_structure(struct bmp_structure *src) {
	if (src->dib_header.compression == BMP_COMPRESSION_ALPHABITFIELDS
			|| src->dib_header.compression == BMP_COMPRESSION_BITFIELDS)
		free(src->color_table);

	if (src->dib_header.header_size > BMP_DIB_HEADER_SIZE)
		free(src->dib_header.extra);

	if (src->dib_header.image_size > 0)
		free(src->pixel_data);

	memset(src, 0, sizeof(*src));
	return 0;
}

void bmp_generate_random_pixels(struct bmp_structure *dest) {
	//TODO: ?

	return;
}
void bmp_create_empty_pixels(struct bmp_structure *dest) {
	dest->pixel_data = malloc(dest->dib_header.image_size);
	memset(dest->pixel_data, 0, dest->dib_header.image_size);
}
