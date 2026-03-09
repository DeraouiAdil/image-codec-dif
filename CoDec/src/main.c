#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/stat.h>
#include "../include/CoDec.h"

// Print usage and help message
void help(char *prog_name) {
  printf("Usage: %s [OPTIONS] <input_file> <output_file>\n\n", prog_name);
  printf("Options:\n");
  printf("  -e, --encode   Encode image (PNM -> DIF)\n");
  printf("  -d, --decode   Decode image (DIF -> PNM)\n");
  printf("  -s, --show     Show image after decoding (Visualizer)\n");
  printf("  -v, --verbose  Verbose mode\n");
  printf("  -t, --time     Measure execution time\n");
  printf("  -h, --help     Show this help\n");
}

// Get file size from disk
long get_file_size(const char *filename) {
  struct stat st;
  if (stat(filename, &st) == 0)
    return st.st_size;
  return -1;
}

// Calculate raw image size from DIF header
// Used to compute compression ratio based on raw data
long get_raw_image_size_from_dif(const char *dif_file) {
  FILE *f = fopen(dif_file, "rb");
  if (!f) return -1;

  unsigned short magic, w, h;
  // Read Magic, Width, Height
  if (fread(&magic, 2, 1, f) != 1) { fclose(f); return -1; }
  if (fread(&w, 2, 1, f) != 1) { fclose(f); return -1; }
  if (fread(&h, 2, 1, f) != 1) { fclose(f); return -1; }
  
  fclose(f);

  int layer = (magic == 0xD1FF) ? 1 : 3; // Gray=1, RGB=3
  return (long)w * (long)h * (long)layer;
}

int main(int argc, char *argv[]) {
  int opt;
  int option_index = 0;
  int mode = 0; 
  int time_mode = 0;
  int show_mode = 0;
  int verbose_mode = (getenv("CODEC_VERBOSE") != NULL);

  static struct option long_options[] = {
    {"encode",  no_argument, 0, 'e'},
    {"decode",  no_argument, 0, 'd'},
    {"show",    no_argument, 0, 's'},
    {"verbose", no_argument, 0, 'v'},
    {"time",    no_argument, 0, 't'},
    {"help",    no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  // Parse arguments
  while((opt = getopt_long(argc, argv, "edsvth", long_options, &option_index)) != -1){
    switch (opt) {
      case 'e': mode = 1; break;
      case 'd': mode = 2; break;
      case 's': show_mode = 1; break;
      case 'v': verbose_mode = 1; setenv("CODEC_VERBOSE", "1", 1); break;
      case 't': time_mode = 1; break;
      case 'h': help(argv[0]); return 0;
      default: help(argv[0]); return 1;
    }
  }

  // Check required files
  if(optind + 2 != argc){
    fprintf(stderr, "Erreur : il manque le fichier d'entrée ou de sortie.\n");
    help(argv[0]);
    return 1;
  }

  char *input_file = argv[optind];
  char *output_file = argv[optind + 1];

  clock_t start, end;
  double cpu_time_used;
  int res = 1;

  if (verbose_mode) {
    printf("[VERBOSE] Fichier d'entrée : %s\n", input_file);
    printf("[VERBOSE] Fichier de sortie : %s\n", output_file);
  }

  //ENCODE
  if (mode == 1) { 
    if (verbose_mode) printf("[VERBOSE] Démarrage de l'encodage...\n");
    
    if (time_mode) start = clock();
    res = pnmtodif(input_file, output_file);
    if (time_mode) end = clock();

    if (res == 0) {
      printf("Succès : Encodage terminé.\n");

      // Calculate compression stats
      long compressed_size = get_file_size(output_file);
      long raw_size = get_raw_image_size_from_dif(output_file);

      if (raw_size > 0 && compressed_size > 0) {
        double ratio = ((double)compressed_size / (double)raw_size) * 100.0;
        printf(" -> Données Brutes (Raw) : %ld octets\n", raw_size);
        printf(" -> Données Compressées  : %ld octets\n", compressed_size);
        printf(" -> Taux de compression  : %.2f%%\n", ratio);
        
        if(ratio > 100.0) 
          printf("    (Note : Le fichier est plus grand que les données brutes)\n");
        else
          printf("    (Gain de place : %.2f%%)\n", 100.0 - ratio);
      }

      // Warn user if -s was used with encode
      if (show_mode) {
        printf("\n[INFO] L'option --show a été ignorée.\n");
        printf("       Impossible d'afficher directement un fichier .dif.\n");
        printf("       Utilisez -d et -s pour décoder et voir le résultat.\n");
      }
    } else {
      fprintf(stderr, "Échec de l'encodage.\n");
    }

  //DECODE
  } else if (mode == 2) { 
    if (verbose_mode) printf("[VERBOSE] Démarrage du décodage...\n");

    if (time_mode) start = clock();
    res = diftopnm(input_file, output_file);
    if (time_mode) end = clock();

    if (res == 0) {
      printf("Succès : Décodage terminé vers %s\n", output_file);
      
      // Handle Visualizer
      if (show_mode) {
        char cmd[512];
        
        // Check for custom viewer in env vars
        char *user_viewer = getenv("CODEC_VIEWER");
        
        if (user_viewer != NULL) {
          if (verbose_mode) printf("[VERBOSE] Custom viewer: %s\n", user_viewer);
          snprintf(cmd, sizeof(cmd), "%s %s &", user_viewer, output_file);
        } else {
          // Default: use 'display' (ImageMagick) to avoid 'eog' issues
          if (verbose_mode) printf("[VERBOSE] Default viewer (display)\n");
          snprintf(cmd, sizeof(cmd), "display %s &", output_file);
        }
        
        // Execute command
        int ret = system(cmd);
        if (ret != 0) {
          fprintf(stderr, "Erreur : Impossible de lancer le visualiseur.\n");
          fprintf(stderr, "Vérifiez que ImageMagick est installé (sudo apt install imagemagick)\n");
        }
      }
    } else {
      fprintf(stderr, "Échec du décodage.\n");
    }

  } else {
    // No valid mode selected
    help(argv[0]);
    return 1;
  }

  // Print execution time if requested
  if (time_mode && res == 0) {
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("[TIME] Temps CPU écoulé : %f secondes\n", cpu_time_used);
  }

  return res;
}