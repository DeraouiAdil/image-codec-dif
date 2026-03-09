#ifndef IMAGE_H
#define IMAGE_H
#define N 4

#include <stdlib.h>

//5octets
/*
 * Structure for the Quantifier.
 * It defines how we split the values for compression.
 * n_levels: number of intervals (usually 4).
 * bit_counts: array containing the number of bits used for each level.
 */
typedef struct{
  unsigned char n_levels;
  unsigned char bit_counts[N];
}Quantifier;

//11octets
/*
 * Structure for the DIF File Header.
 * This is written at the very beginning of the compressed file.
 * width, height: dimensions of the image.
 * magic_number: ID of the file type (0xD1FF for Gray, 0xD3FF for RGB).
 * q: the quantifier settings used for this file.
 */
typedef struct {
  unsigned short width, height;
  unsigned short magic_number;
  Quantifier q;
}Header;

/*
 * Main Image structure.
 * Contains the header info and the actual pixel data.
 */
typedef struct{
  Header header;
  unsigned char* pixmap;
}Img;
//width*height*layer (size buffer)

/*
 * Checks if a file exists and verifies its type/extension.
 * Useful to avoid crashing if the user gives a bad filename.
 */
int check_file(const char *filepath, char* category, char* format, char* encoding);

/*
 * Reads a PNM image from a file and loads it into the Img structure.
 * path_to_file: path to the .pgm or .ppm file.
 * layer: pointer to store the number of channels (1 or 3).
 * max: pointer to store the max color value (usually 255).
 */
int read_image(Img* img, const char *path_to_file, unsigned short* layer, unsigned short* max);

/*
 * Saves the Img structure into a PNM file.
 * path_to_file: destination path.
 * max: max color value.
 * layer: number of channels.
 */
int write_image(Img* img, const char *path_to_file, const unsigned short max, const unsigned short layer);

/*
 * Helper function to invert image colors (useful for tests or visual check).
 */
void negative_image(Img *img, int layer);

/*
 * Frees the memory allocated for the image (pixmap).
 */
void free_image(Img *img);

#endif
