/**
@file main.c
@brief test driver for libcompress
@todo RLE encoding of a block should only occur if it shrinks the size of a block.
**/
#include "libcompress.h"
#include <assert.h>
#include <string.h>

static uint8_t input_to_rle[4096];
static uint8_t rle_to_lzss[8192];

int main(int argc, char **argv)
{
#if 0
	/* form a pipe: Input -> RLE -> LZSS -> output */
	io_t *output = io_file(stdout);

	size_t readin = 0;

	while((readin = fread(input_to_rle, 1, sizeof(input_to_rle), stdin))) {
		io_t *rle_out, *rle_in, *lzss_in;
		size_t rle_chars_output;

		fprintf(stderr, "read: %zu\n", readin);

		rle_in  = io_string_external(IO_READ,  readin, input_to_rle);
		rle_out = io_string_external(IO_WRITE, sizeof(rle_to_lzss), rle_to_lzss);
		
		if(run_length_encode(rle_in, rle_out) < 0) {
			fprintf(stderr, "RLE encoding failure\n");
			return -1;
		}

		rle_chars_output = io_get_chars_written(rle_out);
		assert(rle_chars_output < sizeof(rle_to_lzss));

		fprintf(stderr, "rle write: %zu\n", rle_chars_output);

		io_free(rle_in);
		io_free(rle_out);

		lzss_in = io_string_external(IO_READ, rle_chars_output, rle_to_lzss);

		if(lzss_encode(lzss_in, output) < 0) {
			fprintf(stderr, "RLE encoding failure\n");
			return -1;
		}
		fprintf(stderr, "lzss (total) write: %zu\n", io_get_chars_written(output));

		io_free(lzss_in);
	}

	return 0;
#endif
}

