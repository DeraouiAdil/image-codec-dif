#include "../include/diff.h"
#include <stdio.h>
#include <string.h>

/** Transforms a signed difference into a positive number
 * @param x the signed value [-127, 127]
 * @return the folded unsigned value [0, 255]
 */
static unsigned char fold_diff(int x){
  if(x < 0){
    return (unsigned char)(-2*x-1);
  }else{
    return (unsigned char)(2*x);
  }
}

/** Restores the signed difference from a positive number
 * @param y the folded value [0, 255]
 * @return the unfolded signed value [-127, 127]
 */
static unsigned char unfold_diff(int y){
  if((y%2) == 1){
    return (unsigned char) (-(y+1)/2);
  }else{
    return (unsigned char)(y/2);
  }
}

/** Applies the differential transformation to the image
 * It removes the last bit, calculates differences, and folds the result.
 * @param img the image to transform
 * @param layer number of channels (1 for gray, 3 for RGB)
 */
void apply_dif_transform(Img* img, int layer){
  size_t size = img->header.width * img->header.height * layer;
  unsigned char* current_pix_map = img->pixmap;
  //suppr le bit de poids faible
  for(size_t i = 0; i < size; i++){
    current_pix_map[i] = current_pix_map[i]>>1;
  }
  //format unsigned
  for(size_t i = size - 1; i >= (size_t)layer; i--){
    int current_value = (int)current_pix_map[i];
    int before_value = (int)current_pix_map[i-layer];
    int dif = current_value - before_value;
    current_pix_map[i] = fold_diff(dif);
  }
}

/** Reverses the differential transformation
 * It unfolds the values, adds the previous pixel, and restores the scale.
 * @param img the image to reconstruct
 * @param layer number of channels (1 for gray, 3 for RGB)
 */
void reverse_dif_transform(Img* img, int layer){
  size_t size = img->header.width * img->header.height * layer;
  unsigned char* current_pix_map = img->pixmap;

  for(size_t i = (size_t)layer; i < size; i++){
    unsigned char folded = current_pix_map[i];
    int diff = unfold_diff(folded);
    int before = current_pix_map[i-layer];
    
    int build = before+diff;
    current_pix_map[i] = build;
  }
  //mult par 2
  for(size_t i = 0; i < size; i++){
    current_pix_map[i] = current_pix_map[i]<<1;
  }
}