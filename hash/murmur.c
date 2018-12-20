#include "buzz.h"

// variant of Murmur hashing
// Sumanth Dathathri, CS118, 2015

uint64_t
hash_s(uchar *v, int len, uint64_t seed)
{	uint64_t h = seed;
	static const uint64_t m=0xc6a4a7935bd1e995L;
	static const uint64_t r=47;
	int i, length8=len/8;

	for ( i = 0; i < length8; i++ )
	{	int i8=i*8;

		long k =  ((long)((*(v+0+i8))&0xff)) +(((long)(*(v+1+i8))&0xff)<<8) +(((long)(*(v+2+i8))&0xff)<<16) +(((long)(*(v+3+i8))&0xff)<<24)
                    +(((long)(*(v+4+i8))&0xff)<<32) +(((long)(*(v+5+i8))&0xff)<<40)
                    +(((long)(*(v+6+i8))&0xff)<<48) +(((long)(*(v+7+i8))&0xff)<<56);
            
	    k *= m;
            k ^= k >> r;
            k *= m;
            
            h ^= k;
            h *= m; 
	}

   	switch (len%8) {
        case 7: h ^= (long)(*(v+(len&~7)+6)&0xff) << 48;
        case 6: h ^= (long)(*(v+(len&~7)+5)&0xff) << 40;
        case 5: h ^= (long)(*(v+(len&~7)+4)&0xff) << 32;
        case 4: h ^= (long)(*(v+(len&~7)+3)&0xff) << 24;
        case 3: h ^= (long)(*(v+(len&~7)+2)&0xff) << 16;
        case 2: h ^= (long)(*(v+(len&~7)+1)&0xff) << 8;
        case 1: h ^= (long)(*(v+(len&~7))&0xff);
                h *= m;
        };
     
        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
}


uint64_t
hash(uchar *v, int len)
{	uint64_t h = 53390;
	static const uint64_t m=0xc6a4a7935bd1e995L;
	static const uint64_t r=47;
	int i, length8=len/8;

	for ( i = 0; i < length8; i++ )
	{	int i8=i*8;
		long k =  ((long)((*(v+0+i8))&0xff)) +(((long)(*(v+1+i8))&0xff)<<8) +(((long)(*(v+2+i8))&0xff)<<16) +(((long)(*(v+3+i8))&0xff)<<24)
                    +(((long)(*(v+4+i8))&0xff)<<32) +(((long)(*(v+5+i8))&0xff)<<40)
                    +(((long)(*(v+6+i8))&0xff)<<48) +(((long)(*(v+7+i8))&0xff)<<56);
            
	    k *= m;
            k ^= k >> r;
            k *= m;
            
            h ^= k;
            h *= m; 
	}

   	switch (len%8) {
        case 7: h ^= (long)(*(v+(len&~7)+6)&0xff) << 48;
        case 6: h ^= (long)(*(v+(len&~7)+5)&0xff) << 40;
        case 5: h ^= (long)(*(v+(len&~7)+4)&0xff) << 32;
        case 4: h ^= (long)(*(v+(len&~7)+3)&0xff) << 24;
        case 3: h ^= (long)(*(v+(len&~7)+2)&0xff) << 16;
        case 2: h ^= (long)(*(v+(len&~7)+1)&0xff) << 8;
        case 1: h ^= (long)(*(v+(len&~7))&0xff);
                h *= m;
        };
     
        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
}
