#ifndef BITSTREAM_H
#define BITSTREAM_H
#include <stdlib.h>

typedef unsigned char uchar;

/*
 * Structure for the Bit Buffer.
 * ptr: pointer to the data array.
 * capa: total capacity of the buffer in bytes.
 */
typedef struct{
  uchar* ptr;
  size_t capa;
}BitStream;

/*
 * Compresses the image data into the bitstream.
 * It takes the normalized differential values and writes the VLC codes.
 *
 * bs: the output buffer.
 * src: array of input values (from the image).
 * len: number of pixels to encode.
 * q: the quantifier table to use for encoding.
 * Returns the number of bytes written.
 */
int encode(BitStream* bs, uchar* src, size_t len, Quantifier q);

/*
 * Decompresses data from the bitstream.
 * It reads the VLC codes and reconstructs the normalized differential values.
 *
 * bs: the input buffer (filled with file data).
 * dest: array where we write the decoded values.
 * len: number of pixels we expect to find.
 * q: the quantifier table to use for decoding.
 * Returns 0 on success.
 */
int decode(BitStream* bs, uchar* dest, size_t len, Quantifier q);


/*
 * Compares two data arrays to check if they are identical.
 * Useful for debugging to see if Input == Output.
 */
int compare(uchar* sig_in, uchar* sig_out, size_t len);

#endif