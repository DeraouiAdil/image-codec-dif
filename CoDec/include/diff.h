#ifndef DIFF_H
#define DIFF_H
#include "image.h"

/*
 * Transforms the raw image pixels into differential values.
 * Step 1: Calculates the difference between neighbor pixels.
 * Step 2: Removes the last bit (division by 2) to reduce noise.
 * Step 3: Converts signed numbers to positive numbers (folding) so they fit in unsigned char.
 *
 * img: pointer to the image structure.
 * layer: number of color layers (1 for Gray, 3 for RGB).
 */
void apply_dif_transform(Img* img, int layer);

/*
 * Reverses the differential transformation to get the original pixels back.
 * Step 1: Unfolds the positive numbers back to signed differences.
 * Step 2: Multiplies by 2 to restore the scale.
 * Step 3: Reconstructs the pixel value by adding the difference to the previous value.
 *
 * img: pointer to the image structure.
 * layer: number of color layers (1 for Gray, 3 for RGB).
 */
void reverse_dif_transform(Img* img, int layer);

#endif