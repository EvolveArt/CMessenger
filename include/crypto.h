#ifndef CRYPTO_H
#define CRYPTO_H

/////
// Constantes
/////

#define HASH_SIZE 32
#define HASH_HEX_SIZE 65

/////
// Fonctions
/////

// Crypto
void hashToString(char *output, const unsigned char *hash);

#endif