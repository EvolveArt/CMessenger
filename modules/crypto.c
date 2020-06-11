#include "pse.h"

void hashToString(char *output, const unsigned char *hash)
{
  char buffer[3];
  char hex_hash[HASH_HEX_SIZE] = {0};

  for(int i = 0; i < HASH_SIZE; i++)
  {
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"%02x", hash[i]);
    strcat(hex_hash, buffer);
  }

  strcpy(output,hex_hash);

  output[HASH_HEX_SIZE] = 0;
}