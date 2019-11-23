#include "crypto/blowfish/cipher.hpp"

#include "crypto/blowfish/core.hpp"
#include "crypto/blowfish/s_box.hpp"

#include "datum/bytes.hpp"

using namespace WarGrey::SCADA;

# define n2ln(c,l1,l2,n) { \
                        c+=n; \
                        l1=l2=0; \
                        switch (n) { \
                        case 8: l2 =((unsigned long)(*(--(c))))    ; \
                        /* fall thru */                              \
                        case 7: l2|=((unsigned long)(*(--(c))))<< 8; \
                        /* fall thru */                              \
                        case 6: l2|=((unsigned long)(*(--(c))))<<16; \
                        /* fall thru */                              \
                        case 5: l2|=((unsigned long)(*(--(c))))<<24; \
                        /* fall thru */                              \
                        case 4: l1 =((unsigned long)(*(--(c))))    ; \
                        /* fall thru */                              \
                        case 3: l1|=((unsigned long)(*(--(c))))<< 8; \
                        /* fall thru */                              \
                        case 2: l1|=((unsigned long)(*(--(c))))<<16; \
                        /* fall thru */                              \
                        case 1: l1|=((unsigned long)(*(--(c))))<<24; \
                                } \
                        }

/* NOTE - c is not incremented as per l2n */
# define l2nn(l1,l2,c,n) { \
                        c+=n; \
                        switch (n) { \
                        case 8: *(--(c))=(unsigned char)(((l2)    )&0xff); \
                        /* fall thru */                                    \
                        case 7: *(--(c))=(unsigned char)(((l2)>> 8)&0xff); \
                        /* fall thru */                                    \
                        case 6: *(--(c))=(unsigned char)(((l2)>>16)&0xff); \
                        /* fall thru */                                    \
                        case 5: *(--(c))=(unsigned char)(((l2)>>24)&0xff); \
                        /* fall thru */                                    \
                        case 4: *(--(c))=(unsigned char)(((l1)    )&0xff); \
                        /* fall thru */                                    \
                        case 3: *(--(c))=(unsigned char)(((l1)>> 8)&0xff); \
                        /* fall thru */                                    \
                        case 2: *(--(c))=(unsigned char)(((l1)>>16)&0xff); \
                        /* fall thru */                                    \
                        case 1: *(--(c))=(unsigned char)(((l1)>>24)&0xff); \
                                } \
                        }

static const size_t bf_block_size = 8;

static size_t inline bf_block_encrypt(const uint8* plainblock, size_t pstart, uint8* cipherblock, size_t cstart, BFBox* box) {
	uint32 L = bigendian_uint32_ref(plainblock, pstart);
	uint32 R = bigendian_uint32_ref(plainblock, pstart + 4);

	bf_encrypt(L, R, box->parray, box->sbox, &L, &R);
	bigendian_uint32_set(cipherblock, cstart + 0, L);
	bigendian_uint32_set(cipherblock, cstart + 4, R);

	return pstart + bf_block_size;
}

static size_t inline bf_block_encrypt(const uint8* plainblock, size_t pstart, uint8* cipherblock, size_t cstart, BFBox* box, uint32* cbcL, uint32* cbcR) {
	uint32 pL = bigendian_uint32_ref(plainblock, pstart);
	uint32 pR = bigendian_uint32_ref(plainblock, pstart + 4);

	bf_encrypt(pL ^ (*cbcL), pR ^ (*cbcR), box->parray, box->sbox, cbcL, cbcR);
	bigendian_uint32_set(cipherblock, cstart + 0, (*cbcL));
	bigendian_uint32_set(cipherblock, cstart + 4, (*cbcR));

	return pstart + bf_block_size;
}

static size_t inline bf_block_decrypt(const uint8* cipherblock, size_t cstart, uint8* plainblock, size_t pstart, BFBox* box) {
	uint32 L = bigendian_uint32_ref(cipherblock, cstart);
	uint32 R = bigendian_uint32_ref(cipherblock, cstart + 4);

	bf_decrypt(L, R, box->parray, box->sbox, &L, &R);
	bigendian_uint32_set(plainblock, pstart + 0, L);
	bigendian_uint32_set(plainblock, pstart + 4, R);

	return pstart + bf_block_size;
}

static size_t inline bf_block_decrypt(const uint8* cipherblock, size_t cstart, uint8* plainblock, size_t pstart, BFBox* box, uint32* cbcL, uint32* cbcR) {
	uint32 L = bigendian_uint32_ref(cipherblock, cstart);
	uint32 R = bigendian_uint32_ref(cipherblock, cstart + 4);

	bf_decrypt(L, R, box->parray, box->sbox, &L, &R);
	bigendian_uint32_set(plainblock, pstart + 0, L ^ (*cbcL));
	bigendian_uint32_set(plainblock, pstart + 4, R ^ (*cbcR));

	(*cbcL) = L;
	(*cbcR) = R;

	return pstart + bf_block_size;
}

/*************************************************************************************************/
BlowfishCipher::BlowfishCipher(const uint8* key, size_t ksize) {
	blowfish_initiate_boxes(key, ksize, &this->box);
}

size_t BlowfishCipher::encrypt(const uint8* plaintext, size_t pstart, size_t pend, uint8* ciphertext, size_t cstart, size_t cend) {
	while (pstart < pend) {
		pstart = bf_block_encrypt(plaintext, pstart, ciphertext, cstart, &this->box);
		cstart += bf_block_size;
	}

	return cstart + bf_block_size;
}

size_t BlowfishCipher::decrypt(const uint8* ciphertext, size_t cstart, size_t cend, uint8* plaintext, size_t pstart, size_t pend) {
	while (cstart < cend) {
		cstart = bf_block_decrypt(ciphertext, cstart, plaintext, pstart, &this->box);
		pstart += bf_block_size;
	}

	return pstart + bf_block_size;
}

/*************************************************************************************************/
BlowfishCipherCBC::BlowfishCipherCBC(const uint8* IV, const uint8* key, size_t ksize) {
	blowfish_initiate_boxes(key, ksize, &this->box);
	this->cbcL = bigendian_uint32_ref(IV, 0);
	this->cbcR = bigendian_uint32_ref(IV, 4);
}

size_t BlowfishCipherCBC::encrypt(const uint8* plaintext, size_t pstart, size_t pend, uint8* ciphertext, size_t cstart, size_t cend) {
	while (pstart < pend) {
		pstart = bf_block_encrypt(plaintext, pstart, ciphertext, cstart, &this->box, &this->cbcL, &this->cbcR);
		cstart += bf_block_size;
	}

	return cstart + bf_block_size;
}

size_t BlowfishCipherCBC::decrypt(const uint8* ciphertext, size_t cstart, size_t cend, uint8* plaintext, size_t pstart, size_t pend) {
	while (cstart < cend) {
		cstart = bf_block_decrypt(ciphertext, cstart, plaintext, pstart, &this->box, &this->cbcL, &this->cbcR);
		pstart += bf_block_size;
	}
	
	return pstart + bf_block_size;
}
