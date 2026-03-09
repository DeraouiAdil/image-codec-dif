#define CHAR_BIT 8
#define N 4
#include "../include/image.h"
#include "../include/bitstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/** Reads a specific bit from a byte
 * @param byte the byte to read
 * @param b index of the bit to read
 * @return 0 or 1
 */
static uchar getbit(uchar byte, size_t b){
  return byte >> b & 1;
}

/** Sets a specific bit in a byte
 * @param byte pointer to the byte
 * @param b index of the bit
 * @param valb value to set (0 or 1)
 */
static void setbit(uchar* byte, size_t b, uchar valb){
  if(valb){
    *byte |= 1 << b;
  }else{
    *byte &= ~(1<<b);
  }
}

/** Prints the bits of a byte for debugging
 * @param byte the byte to display
 */
void printbits(uchar byte){
  for(int i = CHAR_BIT-1; i >= 0; i--){
    printf("%d", (byte >> i) & 1);
  }
}

/** Writes bits into the bitstream
 * @param curr the bitstream buffer
 * @param src byte containing bits to write
 * @param nbit number of bits to write
 * @return number of bits written
 */
size_t pushbits(BitStream* curr, uchar src, size_t nbit){
  for(size_t i = 0; i < nbit; i++){
    if(curr->capa == 0){
      curr->ptr++;
      *(curr->ptr)=0;
      curr->capa = CHAR_BIT;
    }
    uchar bit = getbit(src, nbit - 1 - i);
    setbit(curr->ptr, curr->capa - 1, bit);
    curr->capa--;
  }
  return nbit;
}

/** Reads bits from the bitstream
 * @param curr the bitstream buffer
 * @param dest pointer to store the read bits
 * @param nbit number of bits to read
 * @return number of bits read
 */
size_t pullbits(BitStream* curr, uchar* dest, size_t nbit){
  *dest=0;
  for(int i = 0; i < nbit; i++){
    if(curr->capa == 0){
      curr->ptr++;
      curr->capa=CHAR_BIT;
    }
    uchar bit = getbit(*curr->ptr, curr->capa-1);
    setbit(dest, nbit-i-1, bit);
    curr->capa--;
  }
  return nbit;
}


/** Encodes data using the quantifier intervals
 * It writes a prefix (level) and a suffix (offset) for each value.
 * @param bs the bitstream to write to
 * @param src array of values to encode
 * @param len number of values
 * @param q the quantifier settings
 * @return 0
 */
int encode(BitStream* bs, uchar* src, size_t len, Quantifier q) {
  for(size_t current_byte = 0; current_byte < len; current_byte++){
    uchar current_value = src[current_byte];
    int b_inf = 0;
    for(int level = 0; level < q.n_levels; level++){//je cherche le bon interval
      int n_bit = q.bit_counts[level];
      int interval_size;
      if(level == q.n_levels -1){//le dernier c le reste
        interval_size = 256 - b_inf;
      }else{
        interval_size = 1 << n_bit;
      }

      int b_sup = b_inf + interval_size;
      if(current_value>= b_inf && current_value < b_sup){
        // printf("value = %d b_inf = %d b_sup = %d\n", current_value, b_inf, b_sup);
        //prefixe
        for(int p = 0; p < level; p++){
          pushbits(bs, 1, 1);
        }
        if(level < q.n_levels - 1){//pour le dernier pas de 1 à la fin
          pushbits(bs, 0, 1);
        }
        //suffixe
        uchar suffix = current_value - b_inf;
        pushbits(bs, suffix, n_bit);
        break;
      }
      b_inf = b_sup;
    }
  }
  return 0;
}

/** Decodes data using the quantifier intervals
 * It reads the prefix to find the level, then the suffix to get the value.
 * @param bs the bitstream to read from
 * @param dest array to store decoded values
 * @param len number of values to decode
 * @param q the quantifier settings
 * @return 0
 */
int decode(BitStream* bs, uchar* dest, size_t len, Quantifier q) {
  
  for(size_t i = 0; i < len; i++){
    int e = 0;
    while(e < q.n_levels - 1){//lecture prefix
      uchar bit;
      pullbits(bs, &bit, 1);
      if(bit == 0) break;
      e++;
    }
    int current_offset = 0;
    for(int j = 0; j < e; j++){//calcul b_inf
      current_offset += (1 << q.bit_counts[j]); 
    }
    //lecture suffix
    int n_bits = q.bit_counts[e];
    uchar y = 0;
    pullbits(bs, &y, n_bits);
    
    dest[i] = current_offset + y;
  }
  return 0;
}

/** Compares two signal arrays
 * @param sig_in first array
 * @param sig_out second array
 * @param len length to compare
 * @return 1 if identical, 0 otherwise
 */
int compare(uchar* sig_in, uchar* sig_out, size_t len){
  for(size_t i = 0; i < len; i++){
    if(sig_in[i] != sig_out[i]) return 0;
  }
  return 1;
}