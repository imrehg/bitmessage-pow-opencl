#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <openssl/sha.h>

#include <iostream>
#include <climits>

#include "bmpow.hh"

#include <endian.h>

#define BITMESSAGE_NONCEBYTES 8


// convert ulonglong to big endian
// http://stackoverflow.com/a/27770821/171237
void write64be(char out[8], unsigned long long in)
{
    out[0] = in >> 56 & 0xff;
    out[1] = in >> 48 & 0xff;
    out[2] = in >> 40 & 0xff;
    out[3] = in >> 32 & 0xff;
    out[4] = in >> 24 & 0xff;
    out[5] = in >> 16 & 0xff;
    out[6] = in >>  8 & 0xff;
    out[7] = in >>  0 & 0xff;
}

// Digest to trial value
unsigned long long trialFromDigest(unsigned char digest[])
{
  unsigned long long trial = 0;
  trial = ((unsigned long long)digest[0] << 56) +
          ((unsigned long long)digest[1] << 48) +
          ((unsigned long long)digest[2] << 40) +
          ((unsigned long long)digest[3] << 32) +
          ((unsigned long long)digest[4] << 24) +
          ((unsigned long long)digest[5] << 16) +
          ((unsigned long long)digest[6] <<  8) +
          ((unsigned long long)digest[7]);
  return trial;
}


unsigned long long pow(unsigned long long target, char* string) {
  unsigned char digest[SHA512_DIGEST_LENGTH];
  unsigned long long nonce = 0;
  char noncebe[8];
  char str[72];
  SHA512_CTX ctx;
  unsigned long long trialValue = ULLONG_MAX;

  while (trialValue > target) {
    nonce++;
    write64be(noncebe, nonce);
    memcpy(&str[0], noncebe, 8);
    memcpy(&str[8], string, 64);

    SHA512_Init(&ctx);
    SHA512_Update(&ctx, str, sizeof(str));
    SHA512_Final(digest, &ctx);

    SHA512_Init(&ctx);
    SHA512_Update(&ctx, digest, 64);
    SHA512_Final(digest, &ctx);

    // char mdString[SHA512_DIGEST_LENGTH*2+1];
    // for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
    //     sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

    // printf("SHA512 digest: %s\n", mdString);

    trialValue = trialFromDigest(digest);
    // char correct[8];
    // write64be(correct, (unsigned long long)result);
  }
  return nonce;
}
