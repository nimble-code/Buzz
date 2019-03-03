#include "buzz.h"

// 64-bit MurmurHash3, 2011, Austin Appleby
// https://code.google.com/p/smhasher/wiki/MurmurHash3
// Matt Morgan, CS118, 2015

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{
  // Set up some constants...
  uint64_t outData = 0;
  void *out = &outData;
  uint8_t *data = (uint8_t*)v;
  int nblocks = len / 16;

  uint64_t h1 = seed;
  uint64_t h2 = seed;

  uint64_t c1 = 0x87c37b91114253d5;
  uint64_t c2 = 0x4cf5ad432745937f;

  uint64_t * blocks = (uint64_t *)(data);

  // Do the mixing...
  for(int i = 0; i < nblocks; i++)
  {
    uint64_t k1 = blocks[i*2];
    uint64_t k2 = blocks[i*2+1];

    k1 *= c1;
    k1 = (k1 << 31) | (k1 >> 33);
    k1 *= c2;
    h1 ^= k1;

    h1 = (h1 << 27) | (h1 >> 37);
    h1 += h2;
    h1 = h1 * 5 + 0x52dce729;

    k2 *= c2;
    k2 = (k2 << 33) | (k2 >> 31);
    k2 *= c1;
    h2 ^= k2;

    h2 = (h2 << 31) | (h2 >> 33);
    h2 += h1;
    h2 = h2 * 5 + 0x38495ab5;
  }

  uint8_t * tail = (uint8_t*)(data + (nblocks * 16));

  uint64_t k1 = 0;
  uint64_t k2 = 0;

  switch(len & 15)
  {
    case 15: k2 ^= ((uint64_t)tail[14]) << 48; break;
    case 14: k2 ^= ((uint64_t)tail[13]) << 40; break;
    case 13: k2 ^= ((uint64_t)tail[12]) << 32; break;
    case 12: k2 ^= ((uint64_t)tail[11]) << 24; break;
    case 11: k2 ^= ((uint64_t)tail[10]) << 16; break;
    case 10: k2 ^= ((uint64_t)tail[9]) << 8; break;
    case 9: k2 ^= ((uint64_t)tail[8]) << 0;
            k2 *= c2;
            k2 = (k2 << 33) | (k2 >> 31);
            k2 *= c1;
            h2 ^= k2; break;
    case 8: k1 ^= ((uint64_t)tail[7]) << 56; break;
    case 7: k1 ^= ((uint64_t)tail[6]) << 48; break;
    case 6: k1 ^= ((uint64_t)tail[5]) << 40; break;
    case 5: k1 ^= ((uint64_t)tail[4]) << 32; break;
    case 4: k1 ^= ((uint64_t)tail[3]) << 24; break;
    case 3: k1 ^= ((uint64_t)tail[2]) << 16; break;
    case 2: k1 ^= ((uint64_t)tail[1]) << 8; break;
    case 1: k1 ^= ((uint64_t)tail[0]) << 0;
            k1 *= c1;
            k1 = (k1 << 31) | (k1 >> 33);
            k1 *= c2;
            h1 ^= k1;
  };

  // And finally...finalize...
  h1 ^= len; h2 ^= len;

  h1 += h2;
  h2 += h1;

  h1 ^= h1 >> 33;
  h1 *= 0xff51afd7ed558ccd;
  h1 ^= h1 >> 33;
  h1 *= 0xc4ceb9fe1a85ec53;
  h1 ^= h1 >> 33;

  h2 ^= h2 >> 33;
  h2 *= 0xff51afd7ed558ccd;
  h2 ^= h2 >> 33;
  h2 *= 0xc4ceb9fe1a85ec53;
  h2 ^= h2 >> 33;

  h1 += h2;
  h2 += h1;

  ((uint64_t*)out)[0] = h1;
  ((uint64_t*)out)[1] = h2;
  return *(uint64_t *)out;
}

uint64_t
hash(uchar *v, int len)
{
	return hash_s(v, len, 1);
}
