#include <stdio.h>
#include "../include/diff.h"
#include "../include/bitstream.h"
#include "../include/image.h"
#include "../include/CoDec.h"

/** Encodes a PNM image into a DIF file
 * It manages the whole process: read, transform, compress, write.
 * @param pnminput path to the source image
 * @param difoutput path to the destination file
 * @return 0 if success, 1 if error
 */
int pnmtodif(const char* pnminput, const char* difoutput){
  Img img;
  unsigned short layer, max;
  if(!read_image(&img, pnminput, &layer, &max)){
    return 1;
  } 
  
  img.header.q.n_levels = 4;
  img.header.q.bit_counts[0] = 1;
  img.header.q.bit_counts[1] = 2;
  img.header.q.bit_counts[2] = 4;
  img.header.q.bit_counts[3] = 8;
  
  apply_dif_transform(&img, layer);

  size_t raw_size = img.header.width * img.header.height * layer;
  size_t buffer_size = raw_size * 2;
  unsigned char* compressed_data = calloc(buffer_size, 1);

  for(int i=0; i<layer; i++){
    compressed_data[i] = img.pixmap[i];
  } 

  BitStream bs;
  bs.ptr = compressed_data + layer;
  bs.capa = 8;
  
  encode(&bs, img.pixmap + layer, raw_size - layer, img.header.q);

  size_t compressed_len = (bs.ptr - compressed_data) + 1;
  FILE* f = fopen(difoutput, "wb");
  if(f){
      fwrite(&img.header.magic_number, 2, 1, f);
      fwrite(&img.header.width, 2, 1, f);
      fwrite(&img.header.height, 2, 1, f);
      fwrite(&img.header.q.n_levels, 1, 1, f);
      fwrite(img.header.q.bit_counts, 1, img.header.q.n_levels, f);
      fwrite(compressed_data, 1, compressed_len, f);
      fclose(f);
  }

  free(compressed_data);
  free_image(&img);
  return 0;
}

/** Decodes a DIF file into a PNM image
 * It manages the whole process: read header, decompress, reverse transform, save.
 * @param difinput path to the compressed file
 * @param pnmoutput path to the output image
 * @return 0 if success, 1 if error
 */
int diftopnm(const char* difinput, const char* pnmoutput){
  Img img_in;
  unsigned short layer, max;
  if(check_file(difinput, "image", NULL, NULL)) {
    fprintf(stderr, "erreur: ce fichier est déjà une image\n");
    return 1;
  }

  if(!read_image(&img_in, difinput, &layer, &max)){
    fprintf(stderr, "Erreur lecture DIF ou fichier invalide\n");
    return 1; 
  }

  Img img_out;
  img_out.header = img_in.header;
  size_t nb_pixels_total = img_in.header.width * img_in.header.height * layer;
  
  img_out.pixmap = malloc(nb_pixels_total * sizeof(unsigned char));

  if(!img_out.pixmap){
    perror("malloc img_out");
    free_image(&img_in);
    return 1;
  }

  for(int i = 0; i < layer; i++){
    img_out.pixmap[i] = img_in.pixmap[i];
  }

  BitStream bs;
  bs.ptr = img_in.pixmap + layer; 
  bs.capa = 8; 

  decode(&bs, img_out.pixmap + layer, nb_pixels_total - layer, img_in.header.q);

  reverse_dif_transform(&img_out, layer);

  FILE *f_out = fopen(pnmoutput, "wb");
  if(!f_out){
    perror("fopen output");
    free_image(&img_in);
    free_image(&img_out);
    return 1;
  }

  if(layer == 1) fprintf(f_out, "P5\n");
  else           fprintf(f_out, "P6\n");
  
  fprintf(f_out, "%d %d\n255\n", img_out.header.width, img_out.header.height);
  fwrite(img_out.pixmap, sizeof(unsigned char), nb_pixels_total, f_out);
  
  fclose(f_out);
  free_image(&img_in);
  free_image(&img_out);

  return 0;
}