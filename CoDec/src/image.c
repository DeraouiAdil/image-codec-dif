#include "../include/image.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/** Checks the type of a file using an external command
 * @param filepath path to the file
 * @param category expected category (e.g., "image")
 * @param format expected format (e.g., "x-portable-pixmap")
 * @param encoding expected encoding (e.g., "binary")
 * @return 1 if it matches, 0 otherwise
 */
int check_file(const char *filepath, char* category, char* format, char* encoding){
  char cmd[256];
  char out[256];
  char check_category[32];
  char check_format[32];
  char check_encoding[32];

  snprintf(cmd, sizeof(cmd), "file -ib '%s'", filepath);
  FILE *fp = popen(cmd, "r");
  if (fp == NULL) {
    perror("popen");
    return 0;
  }

  if(fgets(out, sizeof(out), fp) == NULL){
    perror("fgets");
    pclose(fp);
    return 0;
  }
  pclose(fp);

  if(sscanf(out, "%31[^/]/%31[^;]; charset=%31s", check_category, check_format, check_encoding) != 3){
    fprintf(stderr, "erreur: sscanf\n");
    return 0;
  }

  // printf("Category : %s\n", check_category);
  // printf("Format   : %s\n", check_format);
  // printf("Encoding : %s\n", check_encoding);
  
  if(category != NULL){
    if(strcmp(category, check_category) != 0){
      // printf("a\n");
      return 0;
    }
  }
  if(format != NULL){
    if(strcmp(format, check_format) != 0){
      // printf("b\n");
      return 0;
    }
  }
  if(encoding != NULL){
    // printf("e %s\n", encoding);
    // printf("ce %s\n", check_encoding);
    if(strcmp(encoding, check_encoding) != 0){
      // printf("c\n");
      return 0;
    }
  }
  // printf("1\n");
  return 1;
}

/** Reads a DIF file structure
 * Reads the header (magic number, size, quantifier) and the compressed data.
 * @param img structure to fill
 * @param path_to_file path to the DIF file
 * @param layer pointer to store the number of layers
 * @param max pointer to store the max value
 * @return 1 if success, 0 if error
 */
static int read_dif_file(Img* img, const char *path_to_file, unsigned short* layer, unsigned short* max){
  if(!check_file(path_to_file, NULL, NULL, "binary")){
    return 0;
  }
  FILE* f = fopen(path_to_file, "rb");
  if(!f){
    perror("fopen");
    exit(1);
  }
  unsigned short magic_number;
  
  if(fread(&magic_number, sizeof(unsigned short), 1, f) != 1){
      fclose(f); return 0;
  }

  if(magic_number == 0xD1FF || magic_number == 0xD3FF){
      img->header.magic_number = magic_number;
      *layer = (magic_number == 0xD1FF) ? 1 : 3;
      *max = 255;

      if(fread(&img->header.width, sizeof(unsigned short), 1, f) != 1 ||
         fread(&img->header.height, sizeof(unsigned short), 1, f) != 1){
        fclose(f); return 0;
      }
      
      if(img->header.width == 0 || img->header.height == 0) {
          fclose(f); return 0;
      }

      if(fread(&img->header.q.n_levels, sizeof(unsigned char), 1, f) != 1){
        fclose(f); return 0;
      }

      if(img->header.q.n_levels != 4) {
          fprintf(stderr, "Erreur: Fichier DIF invalide (n_levels=%d != 4)\n", img->header.q.n_levels);
          fclose(f); return 0;
      }

      if(fread(img->header.q.bit_counts, sizeof(unsigned char), img->header.q.n_levels, f) != img->header.q.n_levels){
        fclose(f); return 0;
      }

      for(int i=0; i<4; i++) {
          if(img->header.q.bit_counts[i] > 16) {
             fprintf(stderr, "Erreur: Quantificateur aberrant (bits > 16)\n");
             fclose(f); return 0;
          }
      }

      long current_pos = ftell(f);
      fseek(f, 0, SEEK_END);
      long end_pos = ftell(f);
      fseek(f, current_pos, SEEK_SET);

      size_t size = end_pos - current_pos;
      //verif si fichier vide
      if(size == 0 && (img->header.width * img->header.height > 1)) {
          fclose(f); return 0;
      }

      img->pixmap = malloc(size);
      if(!img->pixmap){
        perror("malloc");
        fclose(f);
        return 0;
      }

      size_t nb_read = fread(img->pixmap, sizeof(unsigned char), size, f);
      if(nb_read != size){
        free(img->pixmap);
        fclose(f);
        return 0;
      }
      
  } else {
      //magic number inconnu
      fclose(f);
      return 0;
  }

  fclose(f);
  return 1;
}

/** Reads an image into memory
 * Supports DIF files directly, and converts other formats to PNM.
 * @param img structure to fill
 * @param path_to_file path to the file
 * @param layer pointer to store the number of layers
 * @param max pointer to store the max value
 * @return 1 if success, 0 if error
 */
int read_image(Img* img, const char *path_to_file, unsigned short* layer, unsigned short* max){
  if(read_dif_file(img, path_to_file, layer, max)){
    // printf("ok\n");
    return 1;
  }
  if(!check_file(path_to_file, "image", NULL, NULL)){//autre magic number refusé plus tard
    // printf("ici");
    return 0;
  }

  FILE* fp;
  char cmd[64];
  if(check_file(path_to_file, "image", "x-portable-greymap","binary") || check_file(path_to_file, "image", "x-portable-pixmap","binary")){//PNM PGM
    snprintf(cmd, sizeof(cmd), "cat %s", path_to_file);
    fp = popen(cmd, "r");
    if(!fp){ 
      // printf("pa\n");
      perror("popen");return 0; 
    }
  }else{
    snprintf(cmd, sizeof(cmd), "convert %s pnm:-", path_to_file);
    fp = popen(cmd, "r");
    if(!fp){ 
      // printf("pb\n");
      perror("popen");return 0; 
    }
  }


  char tmp_magic[3];
  if(fscanf(fp, "%2s", tmp_magic) != 1){ pclose(fp); return 0;}

  if(strcmp(tmp_magic, "P5") == 0){
    img->header.magic_number = 0xD1FF;
    *layer = 1;
  }else if(strcmp(tmp_magic, "P6") == 0){
    img->header.magic_number = 0xD3FF;
    *layer = 3;
  }else{
    fprintf(stderr, "format non supporté");
    pclose(fp);
    return 0;
  }
  // printf("%s\n", tmp_magic);
  int c = fgetc(fp);
  while (c != EOF) {
    if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {//avant #
      c = fgetc(fp);
      continue;
    }
    if (c == '#') {
      while (c != '\n' && c != EOF) {
        c = fgetc(fp);
      }
      continue; 
    }
    ungetc(c, fp);
    break;
  }

  if(fscanf(fp, "%hu %hu %hu", &img->header.width, &img->header.height, max)!=3){
    pclose(fp);
    return 0;
  }
  fgetc(fp);
  // // printf("%hu %hu %hu\n", img->header.width, img->header.height, *max);

  size_t nb_pixels = img->header.width * img->header.height * (*layer);
  img->pixmap = malloc(nb_pixels);
  if(!img->pixmap){
      perror("malloc pixmap");
      pclose(fp);
      return 0;
  }

  size_t nb_read = fread(img->pixmap, sizeof(unsigned char), nb_pixels, fp);
  if(nb_read != nb_pixels){
    free(img->pixmap);
    pclose(fp);
    return 0;
  }
  pclose(fp);
  img->header.q.n_levels = 0;
  return 1;
  // printf("1read\n");
  // return 1;
  // if(fscanf(fp, "%hu", &img->header.magic_number) != 1){
  //   pclose(fp);
  //   return 0;
  // }
  
  // P5
  // # une image en niveaux de gris
  // # -> 1 pixel = 1 octet
  // 512 512 255
  // Â70VÂW>"FMZÂÂiIÂÂ²uOYGXc@EwvG7Â...


}

/** Writes the image structure to a file
 * It writes the header (DIF format) and the raw data.
 * @param img the image to write
 * @param path_to_file output filename
 * @param max max value for pixels
 * @param layer number of channels
 * @return 1 if success, 0 if error
 */
int write_image(Img* img, const char *path_to_file, const unsigned short max, const unsigned short layer){
  FILE* f = fopen(path_to_file, "wb");
  if(!f){
    perror("fopen");
    return 0;
  }
  // if(fprintf(f, "%hu\n", img->header.magic_number) < 0){
  //   fclose(f);
  //   return 0;
  // }
  // if(fprintf(f, "%hu %hu\n", img->header.width, img->header.height) < 0){
  //   fclose(f);
  //   return 0;
  // }
  // if(fprintf(f, "%hu\n", max) < 0){
  //   fclose(f);
  //   return 0;
  // }

  if(fwrite(&img->header.magic_number, sizeof(unsigned short), 1, f) != 1){
    fclose(f);
    return 0;
  }
  if(fwrite(&img->header.width, sizeof(unsigned short), 1, f) != 1){
    fclose(f);
    return 0;
  }
  if(fwrite(&img->header.height, sizeof(unsigned short), 1, f) != 1){
    fclose(f);
    return 0;
  }
  if(fwrite(&img->header.q.n_levels, sizeof(unsigned char), 1, f) != 1){
    fclose(f);
    return 0;
  }
  if(fwrite(img->header.q.bit_counts, sizeof(unsigned char), img->header.q.n_levels, f) != img->header.q.n_levels){
    fclose(f);
    return 0;
  }
  size_t nb_pixels = img->header.width * img->header.height * layer;
  size_t nb_written = fwrite(img->pixmap, sizeof(unsigned char), nb_pixels, f);
  fclose(f);

  if(nb_pixels != nb_written){
    fprintf(stderr, "error: ecriture incomplet");
    return 0;
  }
  return 1;
}

/** Inverts the colors of the image
 * @param img the image to modify
 * @param layer number of channels
 */
void negative_image(Img *img, int layer) {
  int max = 1;
  size_t npixels = img->header.width * img->header.height * layer;
  unsigned char maxv = (max <= 255) ? (unsigned char)max : 255;

  for(size_t i = 0; i < npixels; i++){
    unsigned char old = img->pixmap[i];
    img->pixmap[i] = maxv - old;
  }
}

/** Frees the memory allocated for the image
 * @param img the image structure to clean
 */
void free_image(Img *img){
  if(img->pixmap){
    free(img->pixmap);
    img->pixmap = NULL;
  }
}