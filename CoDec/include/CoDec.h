#ifndef CODEC_H
#define CODEC_H

/*
 * Encodes a PNM image (PGM or PPM) into a compressed DIF file.
 * It reads the input file, applies the differential transform,
 * and compresses it using the VLC format.
 *
 * pnminput: path to the source image (PNM).
 * difoutput: path where the compressed file (DIF) will be saved.
 *
 * Returns 0 if it works, or a positive number if there is an error.
 */
int pnmtodif(const char* pnminput, const char* difoutput);

/*
 * Decodes a compressed DIF file back into a PNM image.
 * It reads the DIF header and data, uncompresses the buffer,
 * reverses the differential transform, and saves the image.
 *
 * difinput: path to the compressed file (DIF).
 * pnmoutput: path where the reconstructed image (PNM) will be saved.
 *
 * Returns 0 if it works, or a positive number if there is an error.
 */
int diftopnm(const char* difinput, const char* pnmoutput);

#endif