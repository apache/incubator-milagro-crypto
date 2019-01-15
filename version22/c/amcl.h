/*
	Licensed to the Apache Software Foundation (ASF) under one
	or more contributor license agreements.  See the NOTICE file
	distributed with this work for additional information
	regarding copyright ownership.  The ASF licenses this file
	to you under the Apache License, Version 2.0 (the
	"License"); you may not use this file except in compliance
	with the License.  You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing,
	software distributed under the License is distributed on an
	"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
	KIND, either express or implied.  See the License for the
	specific language governing permissions and limitations
	under the License.
*/

/* AMCL header file */
/* Designed for AES-128/192/256 security, 254-521 bit elliptic curves and BN curves for pairings */
/* Each "limb" of a big number occupies at most (n-3) bits of an n-bit computer word. The most significant word must have at least 4 extra unused bits */

/**
 * @file amcl.h
 * @author Mike Scott and Kealan McCusker
 * @date 19th May 2015
 * @brief Main Header File
 *
 * Allows some user configuration
 * defines structures
 * declares functions
 *
 */

/* NOTE: There is only one user configurable section in this header - see below */

#ifndef AMCL_H
#define AMCL_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "arch.h"

#ifdef CMAKE
#define AMCL_VERSION_MAJOR @AMCL_VERSION_MAJOR@ /**< Major version of the library */
#define AMCL_VERSION_MINOR @AMCL_VERSION_MINOR@ /**< Minor version of the library */
#define AMCL_VERSION_PATCH @AMCL_VERSION_PATCH@ /**< Patch version of the library */
#define OS "@OS@"                               /**< Operative system */
#cmakedefine USE_PATENTS   /**< Use Patents */
#cmakedefine USE_ANONYMOUS /**< Use Anonymous Configuration in MPin */
#endif

/* Curve types */

#define WEIERSTRASS 0 /**< Short Weierstrass form curve  */
#define EDWARDS 1     /**< Edwards or Twisted Edwards curve  */
#define MONTGOMERY 2  /**< Montgomery form curve  */

/* Elliptic curves are defined over prime fields */
/* Here are some popular EC prime fields for which we have prepared standard curves. Feel free to specify your own. */

#define NIST256 0    /**< For the NIST 256-bit standard curve - WEIERSTRASS only */
#define C25519 1     /**< Bernstein's Modulus 2^255-19 - EDWARDS or MONTGOMERY only */
#define BRAINPOOL 2  /**< For Brainpool 256-bit curve - WEIERSTRASS only */
#define ANSSI 3      /**< For French 256-bit standard curve - WEIERSTRASS only */
#define MF254 4      /**< For NUMS curves from Bos et al - 254-bit Montgomery friendly modulus - WEIERSTRASS or EDWARDS or MONTGOMERY */
#define MS255 5      /**< For NUMS curve - 255-bit pseudo-mersenne modulus - WEIERSTRASS or EDWARDS or MONTGOMERY */
#define MF256 6      /**< For NUMS curve - 256-bit Montgomery friendly modulus - WEIERSTRASS or EDWARDS or MONTGOMERY */
#define MS256 7      /**< For NUMS curve - 256-bit pseudo-merseene modulus - WEIERSTRASS or EDWARDS or MONTGOMERY */
#define HIFIVE 8     /**< My 336-bit pseudo-mersenne modulus - EDWARDS only */
#define GOLDILOCKS 9 /**< Goldilocks generalized-mersenne modulus - EDWARDS only */
#define NIST384 10   /**< For the NIST 384-bit standard curve - WEIERSTRASS only */
#define C41417 11    /**< Bernstein et al Curve41417 2^414-17 - EDWARDS only */
#define NIST521 12   /**< For the NIST 521-bit standard curve - WEIERSTRASS only */

/* BN Curves */
#define BN_CURVES 100 /**< Barreto-Naehrig curves */
#define BN454 100     /**< New AES-128 security BN curve - Modulus built from -0x10000010000000000000100000001  - WEIERSTRASS only */
#define BN646 101     /**< AES-192 security BN curve -  Modulus built from t=-0x10000000000000000000004000000000000001001 - WEIERSTRASS only */

/* A few 254-bit alternative BN curves */
#define BN254 102 /**< Standard Nogami BN curve - fastest. Modulus built from  t=-0x4080000000000001 - WEIERSTRASS only */

/* GT_STRONG curves */
#define BN254_CX 103 /**< Our CertiVox BN curve. Modulus built from t=-0x4000000003C012B1 - WEIERSTRASS only */
#define BN254_T 104  /**< GT_Strong BN curve. Modulus built from t=-0x4000806000004081 - WEIERSTRASS only */
#define BN254_T2 105 /**< G2 and GT-Strong BN curve.  Modulus built from t=-0x4000020100608205 - WEIERSTRASS only */

/* BLS-12 Curves */
#define BLS_CURVES 200 /**< Barreto-Lynn-Scott curves */
#define BLS455 200     /**< New AES-128 security BLS curve - Modulus built from -0x10002000002000010007  - WEIERSTRASS only */
#define BLS383 201     /**< New AES-128 security BLS curve - Modulus built from -0x1101000000040110  - WEIERSTRASS only */


/*** START OF USER CONFIGURABLE SECTION - set architecture and choose modulus and curve  ***/

#ifdef CMAKE
#define CHOICE @AMCL_CHOICE@ /**< Current choice of Field */
#else
#define CHOICE BN254_CX	     /**< Current choice of Field */
#endif

/* For some moduli only one parameterisation of curve may supported. For others there is a choice of WEIERSTRASS, EDWARDS or MONTGOMERY curves. See above. */
#ifdef CMAKE
#define CURVETYPE @AMCL_CURVETYPE@ /**< Note that not all curve types are supported - see above */
#else
#define CURVETYPE WEIERSTRASS	   /**< Note that not all curve types are supported - see above */
#endif


/* Actual curve parameters associated with these choices can be found in rom.c */

/* These next options only apply for pairings */
#ifdef USE_PATENTS
#define USE_GLV	  /**< Note this method is patented (GLV), so maybe you want to comment this out */
#define USE_GS_G2 /**< Well we didn't patent it :) But may be covered by GLV patent :( */
#endif
#define USE_GS_GT /**< Not patented, so probably safe to always use this */

/* Finite field support - for RSA, DH etc. */
#ifdef CMAKE
#define FFLEN @AMCL_FFLEN@ /**< 2^n multiplier of BIGBITS to specify supported Finite Field size, e.g 2048=256*2^3 where BIGBITS=256 */
#else
#define FFLEN 8 /**< 2^n multiplier of BIGBITS to specify supported Finite Field size, e.g 2048=256*2^3 where BIGBITS=256 */
#endif



/* For debugging Only.*/
#ifdef CMAKE
#cmakedefine DEBUG_REDUCE /**< Print debug message for field reduction */
#cmakedefine DEBUG_NORM   /**< Detect digit overflow */
#cmakedefine GET_STATS    /**< Debug statistics - use with debugger */
#else
//#define DEBUG_REDUCE
//#define DEBUG_NORM
//#define GET_STATS
#endif


// #define UNWOUND

/*** END OF USER CONFIGURABLE SECTION ***/








#define NLEN (1+((MBITS-1)/BASEBITS)) /**< Number of words in BIG. */
#define MODBYTES (1+(MBITS-1)/8)      /**< Number of bytes in Modulus */
#define BIGBITS (MODBYTES*8)	      /**< Number of bits representable in a BIG */
#define FF_BITS (BIGBITS*FFLEN)	      /**< Finite Field Size in bits - must be BIGBITS.2^n */

/* modulus types */

#define NOT_SPECIAL 0	       /**< Modulus of no exploitable form */
#define PSEUDO_MERSENNE 1      /**< Pseudo-mersenne modulus of form $2^n-c$  */
#define MONTGOMERY_FRIENDLY 3  /**< Montgomery Friendly modulus of form $2^a(2^b-c)-1$  */
#define GENERALISED_MERSENNE 2 /**< Generalised-mersenne modulus of form $2^n-2^m-1$, GOLDILOCKS only */

/* Built-in curves defined here */
/* MIRACL check.cpp utility used to determine optimal choice for BASEBITS */

/* Define AES_S if the desired AES-equivalent security is significantly less than the group order */


#if CHOICE==NIST256
#define MBITS 256	             /**< Number of bits in Modulus */
#define MOD8 7	                     /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#define BASEBITS 13                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 32
#define BASEBITS 29                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56                  /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==C25519
#define MBITS 255	             /**< Number of bits in Modulus */
#define MOD8 5		             /**< Modulus mod 8  */
#define MODTYPE PSEUDO_MERSENNE      /**< Modulus type */
#if CURVETYPE==WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#if CURVETYPE==MONTGOMERY
#error Not supported
#else
#define BASEBITS 13		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==BRAINPOOL
#define MBITS 256                    /**< Number of bits in Modulus */
#define MOD8 7                       /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#define BASEBITS 13                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 32
#define BASEBITS 29                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56                  /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==ANSSI
#define MBITS 256	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#define BASEBITS 13                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 32
#define BASEBITS 29                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56                  /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

/**< NUMS curve from Bos et al. paper */

#if CHOICE==MF254
#define MBITS 254                    /**< Number of bits in Modulus */
#define MOD8 7                       /**< Modulus mod 8  */
#define MODTYPE MONTGOMERY_FRIENDLY  /**< Modulus type */
#if CHUNK==16
#error Not Supported
#endif
#if CHUNK == 32
#define BASEBITS 29                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56                  /**< Numbers represented to base 2*BASEBITS */
#endif
#endif

#if CHOICE==MF256
#define MBITS 256                    /**< Number of bits in Modulus */
#define MOD8 7                       /**< Modulus mod 8  */
#define MODTYPE MONTGOMERY_FRIENDLY  /**< Modulus type */
#if CHUNK==16
#error Not Supported
#endif
#if CHUNK == 32
#define BASEBITS 29                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56                  /**< Numbers represented to base 2*BASEBITS */
#endif
#endif

#if CHOICE==MS255
#define MBITS 255	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE PSEUDO_MERSENNE      /**< Modulus type */
#if CHUNK==16
#define BASEBITS 13                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 32
#define BASEBITS 29                  /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56                  /**< Numbers represented to base 2*BASEBITS */
#endif
#endif

#if CHOICE==MS256
#define MBITS 256	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE PSEUDO_MERSENNE      /**< Modulus type */
#if CHUNK==16
#define BASEBITS 13	             /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif

#if CHOICE==HIFIVE
#define MBITS 336	             /**< Number of bits in Modulus */
#define MOD8 5		             /**< Modulus mod 8  */
#define MODTYPE PSEUDO_MERSENNE      /**< Modulus type */
#define AES_S 128                    /**< Desired AES equivalent strength */
#if CURVETYPE!=EDWARDS
#error Not supported
#else
#if CHUNK==16
#error Not Supported
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 60		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==GOLDILOCKS
#define MBITS 448	             /**< Number of bits in Modulus */
#define MOD8 7		             /**< Modulus mod 8  */
#define MODTYPE GENERALISED_MERSENNE /**< Modulus type */
#if CURVETYPE!=EDWARDS
#error Not supported
#else
#if CHUNK==16
#error Not Supported
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 60		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==NIST384
#define MBITS 384	             /**< Number of bits in Modulus */
#define MOD8 7		             /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#error Not supported
#endif
#if CHUNK == 32
#define BASEBITS 28		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==C41417
#define MBITS 414	             /**< Number of bits in Modulus */
#define MOD8 7		             /**< Modulus mod 8  */
#define MODTYPE  PSEUDO_MERSENNE     /**< Modulus type */
#if CURVETYPE!=EDWARDS
#error Not supported
#else
#if CHUNK==16
#error Not supported
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 60		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==NIST521
#define MBITS 521	             /**< Number of bits in Modulus */
#define MOD8 7		             /**< Modulus mod 8  */
#define MODTYPE  PSEUDO_MERSENNE     /**< Modulus type */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#error Not supported
#endif
#if CHUNK == 32
#define BASEBITS 28		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 60		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

/* New BN curve to be used for AES-128 security as response to new DL developments - see Kim & Barbulescu ePrint Archive: Report 2015/1027 */

#if CHOICE==BN454
#define MBITS 454	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#define AES_S 128                    /**< Desired AES equivalent strength */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#error Not supported
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 60		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

/* New BLS curve to be used for AES-128 security as response to new DL developments - see Kim & Barbulescu ePrint Archive: Report 2015/1027 */

#if CHOICE==BLS455
#define MBITS 455	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#define AES_S 128                    /**< Desired AES equivalent strength */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#error Not supported
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 60		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif


#if CHOICE==BLS383
#define MBITS 383	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#error Not supported
#endif
#if CHUNK == 32
#define BASEBITS 28		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE==BN646
#define MBITS 646	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#define AES_S 192                    /**< Desired AES equivalent strength */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#error Not supported
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 60		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif

#if CHOICE<BLS_CURVES

#if CHOICE>=BN254                    /* Its a BN curve */
#define MBITS 254	             /**< Number of bits in Modulus */
#define MOD8 3		             /**< Modulus mod 8  */
#define MODTYPE  NOT_SPECIAL         /**< Modulus type */
#if CURVETYPE!=WEIERSTRASS
#error Not supported
#else
#if CHUNK==16
#define BASEBITS 13		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 32
#define BASEBITS 29		     /**< Numbers represented to base 2*BASEBITS */
#endif
#if CHUNK == 64
#define BASEBITS 56		     /**< Numbers represented to base 2*BASEBITS */
#endif
#endif
#endif


#if CHOICE>BN254
#define GT_STRONG /**< Using a GT-Strong 254-bit BN curve */
#endif
#endif


/* Don't mess with anything below this line */

#ifdef GET_STATS
extern int tsqr,rsqr,tmul,rmul;
extern int tadd,radd,tneg,rneg;
extern int tdadd,rdadd,tdneg,rdneg;
#endif

#define DCHUNK 2*CHUNK	/**< Number of bits in double-length type */
#define DNLEN 2*NLEN	/**< double length required for products of BIGs */
#define HFLEN (FFLEN/2) /**< Useful for half-size RSA private key operations */

#define CHUNK_BITS 8*sizeof(chunk) /**< Number of bits in a chunk */

#ifdef DEBUG_NORM  /* Add an extra location to track chunk extension */
typedef chunk BIG[NLEN+1];   /**< Define type BIG as array of chunks */
typedef chunk DBIG[DNLEN+1]; /**< Define type DBIG as array of chunks */
#else
typedef chunk BIG[NLEN];     /**< Define type BIG as array of chunks */
typedef chunk DBIG[DNLEN];   /**< Define type DBIG as array of chunks */
#endif

#define HBITS (BASEBITS/2)      /**< Number of bits in number base divided by 2 */
#define HBITS1 ((BASEBITS+1)/2) /**< Number of bits in number base plus 1 divided by 2 */
#define HDIFF (HBITS1-HBITS)    /**< Will be either 0 or 1, depending if number of bits in number base is even or odd */

#define BMASK (((chunk)1<<BASEBITS)-1) /**< Mask = 2^BASEBITS-1 */
#define HMASK (((chunk)1<<HBITS)-1)    /**< Mask = 2^HBITS-1 */
#define HMASK1 (((chunk)1<<HBITS1)-1)  /**< Mask = 2^HBITS1-1 */

#define MODBITS MBITS                             /**< Number of bits in Modulus for selected curve */
#define TBITS (MBITS%BASEBITS)                    /**< Number of active bits in top word */
#define TMASK (((chunk)1<<TBITS)-1)               /**< Mask for active bits in top word */
#define NEXCESS (1<<(CHUNK-BASEBITS-1))           /**< 2^(CHUNK-BASEBITS-1) - digit cannot be multiplied by more than this before normalisation */
#define FEXCESS ((chunk)1<<(BASEBITS*NLEN-MBITS)) /**< 2^(BASEBITS*NLEN-MODBITS) - normalised BIG can be multiplied by more than this before reduction */
#define OMASK (-((chunk)(1)<<TBITS))              /**<  for masking out overflow bits */

/* catch field excesses */
#define EXCESS(a) ((a[NLEN-1]&OMASK)>>(TBITS)) /**< Field Excess */


#define P_MBITS (MODBYTES*8)
#define P_TBITS (P_MBITS%BASEBITS)
#define P_EXCESS(a) ((a[NLEN-1])>>(P_TBITS))
#define P_FEXCESS ((chunk)1<<(BASEBITS*NLEN-P_MBITS))



/* Field Params - see rom.c */
extern const BIG Modulus;  /**< Actual Modulus set in rom.c */
extern const chunk MConst; /**< Montgomery only - 1/p mod 2^BASEBITS */

/* Curve Params - see rom.c */
extern const int CURVE_A;     /**< Elliptic curve A parameter */
extern const BIG CURVE_B;     /**< Elliptic curve B parameter */
extern const BIG CURVE_Order; /**< Elliptic curve group order */
extern const BIG CURVE_Cof;   /**< Elliptic curve cofactor */

/* Generator point on G1 */
extern const BIG CURVE_Gx; /**< x-coordinate of generator point in group G1  */
extern const BIG CURVE_Gy; /**< y-coordinate of generator point in group G1  */

/* For Pairings only */

/* Generator point on G2 */
extern const BIG CURVE_Pxa; /**< real part of x-coordinate of generator point in group G2 */
extern const BIG CURVE_Pxb; /**< imaginary part of x-coordinate of generator point in group G2 */
extern const BIG CURVE_Pya; /**< real part of y-coordinate of generator point in group G2 */
extern const BIG CURVE_Pyb; /**< imaginary part of y-coordinate of generator point in group G2 */

extern const BIG CURVE_Bnx; /**< BN curve x parameter */

extern const BIG CURVE_Cru; /**< BN curve Cube Root of Unity */

extern const BIG CURVE_Fra; /**< real part of BN curve Frobenius Constant */
extern const BIG CURVE_Frb; /**< imaginary part of BN curve Frobenius Constant */


extern const BIG CURVE_W[2];	 /**< BN curve constant for GLV decomposition */
extern const BIG CURVE_SB[2][2]; /**< BN curve constant for GLV decomposition */
extern const BIG CURVE_WB[4];	 /**< BN curve constant for GS decomposition */
extern const BIG CURVE_BB[4][4]; /**< BN curve constant for GS decomposition */

/* Structures */

/**
	@brief ECP structure - Elliptic Curve Point over base field
*/

typedef struct
{
#if CURVETYPE!=EDWARDS
    int inf; /**< Infinity Flag - not needed for Edwards representation */
#endif
    BIG x; /**< x-coordinate of point */
#if CURVETYPE!=MONTGOMERY
    BIG y; /**< y-coordinate of point. Not needed for Montgomery representation */
#endif
    BIG z;/**< z-coordinate of point */
} ECP;

/**
	@brief FP2 Structure - quadratic extension field
*/

typedef struct
{
    BIG a; /**< real part of FP2 */
    BIG b; /**< imaginary part of FP2 */
} FP2;

/**
	@brief FP4 Structure - towered over two FP2
*/

typedef struct
{
    FP2 a; /**< real part of FP4 */
    FP2 b; /**< imaginary part of FP4 */
} FP4;

/**
	@brief FP12 Structure - towered over three FP4
*/

typedef struct
{
    FP4 a; /**< first part of FP12 */
    FP4 b; /**< second part of FP12 */
    FP4 c; /**< third part of FP12 */
} FP12;

/**
	@brief ECP2 Structure - Elliptic Curve Point over quadratic extension field
*/

typedef struct
{
    int inf; /**< Infinity Flag */
    FP2 x;   /**< x-coordinate of point */
    FP2 y;   /**< y-coordinate of point */
    FP2 z;   /**< z-coordinate of point */
} ECP2;

/**
 * @brief SHA256 hash function instance */
typedef struct
{
    unsign32 length[2]; /**< 64-bit input length */
    unsign32 h[8];      /**< Internal state */
    unsign32 w[80];	/**< Internal state */
    int hlen;		/**< Hash length in bytes */
} hash256;

/**
 * @brief SHA384-512 hash function instance */
typedef struct
{
    unsign64 length[2]; /**< 64-bit input length */
    unsign64 h[8];      /**< Internal state */
    unsign64 w[80];	/**< Internal state */
    int hlen;           /**< Hash length in bytes */
} hash512;

/**
 * @brief SHA384 hash function instance */
typedef hash512 hash384;

#define SHA256 32 /**< SHA-256 hashing */
#define SHA384 48 /**< SHA-384 hashing */
#define SHA512 64 /**< SHA-512 hashing */

/* Symmetric Encryption AES structure */

#define ECB   0  /**< Electronic Code Book */
#define CBC   1  /**< Cipher Block Chaining */
#define CFB1  2  /**< Cipher Feedback - 1 byte */
#define CFB2  3  /**< Cipher Feedback - 2 bytes */
#define CFB4  5  /**< Cipher Feedback - 4 bytes */
#define OFB1  14 /**< Output Feedback - 1 byte */
#define OFB2  15 /**< Output Feedback - 2 bytes */
#define OFB4  17 /**< Output Feedback - 4 bytes */
#define OFB8  21 /**< Output Feedback - 8 bytes */
#define OFB16 29 /**< Output Feedback - 16 bytes */
#define CTR1  30 /**< Counter Mode - 1 byte */
#define CTR2  31 /**< Counter Mode - 2 bytes */
#define CTR4  33 /**< Counter Mode - 4 bytes */
#define CTR8  37 /**< Counter Mode - 8 bytes */
#define CTR16 45 /**< Counter Mode - 16 bytes */

#define uchar unsigned char  /**<  Unsigned char */

/**
	@brief AES instance
*/


typedef struct
{
    int Nk;            /**< AES Key Length */
    int Nr;            /**< AES Number of rounds */
    int mode;          /**< AES mode of operation */
    unsign32 fkey[60]; /**< subkeys for encrypton */
    unsign32 rkey[60]; /**< subkeys for decrypton */
    char f[16];        /**< buffer for chaining vector */
} amcl_aes;

/* AES-GCM suppport.  */

#define GCM_ACCEPTING_HEADER 0   /**< GCM status */
#define GCM_ACCEPTING_CIPHER 1   /**< GCM status */
#define GCM_NOT_ACCEPTING_MORE 2 /**< GCM status */
#define GCM_FINISHED 3           /**< GCM status */
#define GCM_ENCRYPTING 0         /**< GCM mode */
#define GCM_DECRYPTING 1         /**< GCM mode */


/**
	@brief GCM mode instance, using AES internally
*/

typedef struct
{
    unsign32 table[128][4]; /**< 2k byte table */
    uchar stateX[16];	    /**< GCM Internal State */
    uchar Y_0[16];	    /**< GCM Internal State */
    unsign32 lenA[2];	    /**< GCM 64-bit length of header */
    unsign32 lenC[2];	    /**< GCM 64-bit length of ciphertext */
    int status;		    /**< GCM Status */
    amcl_aes a;		    /**< Internal Instance of AMCL_AES cipher */
} gcm;

/* Marsaglia & Zaman Random number generator constants */

#define NK   21 /**< PRNG constant */
#define NJ   6  /**< PRNG constant */
#define NV   8  /**< PRNG constant */


/**
	@brief Cryptographically secure pseudo-random number generator instance
*/

typedef struct
{
    unsign32 ira[NK]; /**< random number array   */
    int      rndptr;  /**< pointer into array */
    unsign32 borrow;  /**<  borrow as a result of subtraction */
    int pool_ptr;     /**< pointer into random pool */
    char pool[32];    /**< random pool */
} csprng;


/**
	@brief Portable representation of a big positive number
*/

typedef struct
{
    int len;   /**< length in bytes  */
    int max;   /**< max length allowed - enforce truncation  */
    char *val; /**< byte array  */
} octet;

/**
	@brief Integer Factorisation Public Key
*/

typedef struct
{
    sign32 e;     /**< RSA exponent (typically 65537) */
    BIG n[FFLEN]; /**< An array of BIGs to store public key */
} rsa_public_key;

/**
	@brief Integer Factorisation Private Key
*/

typedef struct
{
    BIG p[FFLEN/2];  /**< secret prime p  */
    BIG q[FFLEN/2];  /**< secret prime q  */
    BIG dp[FFLEN/2]; /**< decrypting exponent mod (p-1)  */
    BIG dq[FFLEN/2]; /**< decrypting exponent mod (q-1)  */
    BIG c[FFLEN/2];  /**< 1/p mod q */
} rsa_private_key;

/*

Note that a normalised BIG consists of digits mod 2^BASEBITS
However BIG digits may be "extended" up to 2^(WORDLENGTH-1).

BIGs in extended form may need to be normalised before certain
operations.

A BIG may be "reduced" to be less that the Modulus, or it
may be "unreduced" and allowed to grow greater than the
Modulus.

Normalisation is quite fast. Reduction involves conditional branches,
which can be regarded as significant "speed bumps". We try to
delay reductions as much as possible. Reductions may also involve
side channel leakage, so delaying and batching them
hopefully disguises internal operations.

*/

/* BIG number prototypes */

/**	@brief Calculates a*b+c+*d
 *
	Calculate partial product of a.b, add in carry c, and add total to d
	@param a multiplier
	@param b multiplicand
	@param c carry
	@param d pointer to accumulated bottom half of result
	@return top half of result
 */
extern chunk muladd(chunk a,chunk b,chunk c,chunk *d);
/**	@brief Tests for BIG equal to zero
 *
	@param x a BIG number
	@return 1 if zero, else returns 0
 */
extern int BIG_iszilch(BIG x);
/**	@brief Tests for DBIG equal to zero
 *
	@param x a DBIG number
	@return 1 if zero, else returns 0
 */
extern int BIG_diszilch(DBIG x);
/**	@brief Outputs a BIG number to the console
 *
	@param x a BIG number
 */
extern void BIG_output(BIG x);
/**	@brief Outputs a BIG number to the console in raw form (for debugging)
 *
	@param x a BIG number
 */
extern void BIG_rawoutput(BIG x);
/**	@brief Conditional constant time swap of two BIG numbers
 *
	Conditionally swaps parameters in constant time (without branching)
	@param x a BIG number
	@param y another BIG number
	@param s swap takes place if not equal to 0
 */
extern void BIG_cswap(BIG x,BIG y,int s);
/**	@brief Conditional copy of BIG number
 *
	Conditionally copies second parameter to the first (without branching)
	@param x a BIG number
	@param y another BIG number
	@param s copy takes place if not equal to 0
 */
extern void BIG_cmove(BIG x,BIG y,int s);
/**	@brief Conditional copy of DBIG number
 *
	Conditionally copies second parameter to the first (without branching)
	@param x a DBIG number
	@param y another DBIG number
	@param s copy takes place if not equal to 0
 */
extern void BIG_dcmove(BIG x,BIG y,int s);
/**	@brief Convert from BIG number to byte array
 *
	@param a byte array
	@param x BIG number
 */
extern void BIG_toBytes(char *a,BIG x);
/**	@brief Convert to BIG number from byte array
 *
	@param x BIG number
	@param a byte array
 */
extern void BIG_fromBytes(BIG x,char *a);
/**	@brief Convert to BIG number from byte array of given length
 *
	@param x BIG number
	@param a byte array
	@param s byte array length
 */
extern void BIG_fromBytesLen(BIG x,char *a,int s);
/**@brief Convert to DBIG number from byte array of given length
 *
   @param x DBIG number
   @param a byte array
   @param s byte array length
 */
extern void BIG_dfromBytesLen(DBIG x,char *a,int s);
/**	@brief Outputs a DBIG number to the console
 *
	@param x a DBIG number
 */
extern void BIG_doutput(DBIG x);
/**	@brief Copy BIG from Read-Only Memory to a BIG
 *
	@param x BIG number
	@param y BIG number in ROM
 */
extern void BIG_rcopy(BIG x,const BIG y);
/**	@brief Copy BIG to another BIG
 *
	@param x BIG number
	@param y BIG number to be copied
 */
extern void BIG_copy(BIG x,BIG y);
/**	@brief Copy DBIG to another DBIG
 *
	@param x DBIG number
	@param y DBIG number to be copied
 */
extern void BIG_dcopy(DBIG x,DBIG y);
/**	@brief Copy BIG to upper half of DBIG
 *
	@param x DBIG number
	@param y BIG number to be copied
 */
extern void BIG_dsucopy(DBIG x,BIG y);
/**	@brief Copy BIG to lower half of DBIG
 *
	@param x DBIG number
	@param y BIG number to be copied
 */
extern void BIG_dscopy(DBIG x,BIG y);
/**	@brief Copy lower half of DBIG to a BIG
 *
	@param x BIG number
	@param y DBIG number to be copied
 */
extern void BIG_sdcopy(BIG x,DBIG y);
/**	@brief Copy upper half of DBIG to a BIG
 *
	@param x BIG number
	@param y DBIG number to be copied
 */
extern void BIG_sducopy(BIG x,DBIG y);
/**	@brief Set BIG to zero
 *
	@param x BIG number to be set to zero
 */
extern void BIG_zero(BIG x);
/**	@brief Set DBIG to zero
 *
	@param x DBIG number to be set to zero
 */
extern void BIG_dzero(DBIG x);
/**	@brief Set BIG to one (unity)
 *
	@param x BIG number to be set to one.
 */
extern void BIG_one(BIG x);
/**	@brief Set BIG to inverse mod 2^256
 *
	@param x BIG number to be inverted
 */
extern void BIG_invmod2m(BIG x);
/**	@brief Set BIG to sum of two BIGs - output not normalised
 *
	@param x BIG number, sum of other two
	@param y BIG number
	@param z BIG number
 */
extern void BIG_add(BIG x,BIG y,BIG z);
/**	@brief Increment BIG by a small integer - output not normalised
 *
	@param x BIG number to be incremented
	@param i integer
 */
extern void BIG_inc(BIG x,int i);
/**	@brief Set BIG to difference of two BIGs
 *
	@param x BIG number, difference of other two - output not normalised
	@param y BIG number
	@param z BIG number
 */
extern void BIG_sub(BIG x,BIG y,BIG z);
/**	@brief Decrement BIG by a small integer - output not normalised
 *
	@param x BIG number to be decremented
	@param i integer
 */
extern void BIG_dec(BIG x,int i);
/**	@brief Set DBIG to difference of two DBIGs
 *
	@param x DBIG number, difference of other two - output not normalised
	@param y DBIG number
	@param z DBIG number
 */
extern void BIG_dsub(DBIG x,DBIG y,DBIG z);
/**	@brief Multiply BIG by a small integer - output not normalised
 *
	@param x BIG number, product of other two
	@param y BIG number
	@param i small integer
 */
extern void BIG_imul(BIG x,BIG y,int i);
/**	@brief Multiply BIG by not-so-small small integer - output normalised
 *
	@param x BIG number, product of other two
	@param y BIG number
	@param i small integer
	@return Overflowing bits
 */
extern chunk BIG_pmul(BIG x,BIG y,int i);
/**	@brief Divide BIG by 3 - output normalised
 *
	@param x BIG number
	@return Remainder
 */
extern int BIG_div3(BIG x);
/**	@brief Multiply BIG by even bigger small integer resulting in a DBIG - output normalised
 *
	@param x DBIG number, product of other two
	@param y BIG number
	@param i small integer
 */
extern void BIG_pxmul(DBIG x,BIG y,int i);
/**	@brief Multiply BIG by another BIG resulting in DBIG - inputs normalised and output normalised
 *
	@param x DBIG number, product of other two
	@param y BIG number
	@param z BIG number
 */
extern void BIG_mul(DBIG x,BIG y,BIG z);
/**	@brief Multiply BIG by another BIG resulting in another BIG - inputs normalised and output normalised
 *
	Note that the product must fit into a BIG, and x must be distinct from y and z
	@param x BIG number, product of other two
	@param y BIG number
	@param z BIG number
 */
extern void BIG_smul(BIG x,BIG y,BIG z);
/**	@brief Square BIG resulting in a DBIG - input normalised and output normalised
 *
	@param x DBIG number, square of a BIG
	@param y BIG number to be squared
 */
extern void BIG_sqr(DBIG x,BIG y);

/**	@brief Montgomery reduction of a DBIG to a BIG  - input normalised and output normalised
 *
	@param a BIG number, reduction of a BIG
	@param md BIG number, the modulus
	@param MC the Montgomery Constant
	@param d DBIG number to be reduced
 */
extern void BIG_monty(BIG a,BIG md,chunk MC,DBIG d);

/**	@brief Shifts a BIG left by any number of bits - input must be normalised, output normalised
 *
	@param x BIG number to be shifted
	@param s Number of bits to shift
 */
extern void BIG_shl(BIG x,int s);
/**	@brief Fast shifts a BIG left by a small number of bits - input must be normalised, output will be normalised
 *
	The number of bits to be shifted must be less than BASEBITS
	@param x BIG number to be shifted
	@param s Number of bits to shift
	@return Overflow bits
 */
extern int BIG_fshl(BIG x,int s);
/**	@brief Shifts a DBIG left by any number of bits - input must be normalised, output normalised
 *
	@param x DBIG number to be shifted
	@param s Number of bits to shift
 */
extern void BIG_dshl(DBIG x,int s);
/**	@brief Shifts a BIG right by any number of bits - input must be normalised, output normalised
 *
	@param x BIG number to be shifted
	@param s Number of bits to shift
 */
extern void BIG_shr(BIG x,int s);
/**	@brief Fast shifts a BIG right by a small number of bits - input must be normalised, output will be normalised
 *
	The number of bits to be shifted must be less than BASEBITS
	@param x BIG number to be shifted
	@param s Number of bits to shift
	@return Shifted out bits
 */
extern int BIG_fshr(BIG x,int s);
/**	@brief Shifts a DBIG right by any number of bits - input must be normalised, output normalised
 *
	@param x DBIG number to be shifted
	@param s Number of bits to shift
 */
extern void BIG_dshr(DBIG x,int s);
/**	@brief Splits a DBIG into two BIGs - input must be normalised, outputs normalised
 *
	Internal function. The value of s must be approximately in the middle of the DBIG.
	Typically used to extract z mod 2^MODBITS and z/2^MODBITS
	@param x BIG number, top half of z
	@param y BIG number, bottom half of z
	@param z DBIG number to be split in two.
	@param s Bit position at which to split
	@return carry-out from top half
 */
extern chunk BIG_split(BIG x,BIG y,DBIG z,int s);
/**	@brief Normalizes a BIG number - output normalised
 *
	All digits of the input BIG are reduced mod 2^BASEBITS
	@param x BIG number to be normalised
 */
extern chunk BIG_norm(BIG x);
/**	@brief Normalizes a DBIG number - output normalised
 *
	All digits of the input DBIG are reduced mod 2^BASEBITS
	@param x DBIG number to be normalised
 */
extern void BIG_dnorm(DBIG x);
/**	@brief Compares two BIG numbers. Inputs must be normalised externally
 *
	@param x first BIG number to be compared
	@param y second BIG number to be compared
	@return -1 is x<y, 0 if x=y, 1 if x>y
 */
extern int BIG_comp(BIG x,BIG y);
/**	@brief Compares two DBIG numbers. Inputs must be normalised externally
 *
	@param x first DBIG number to be compared
	@param y second DBIG number to be compared
	@return -1 is x<y, 0 if x=y, 1 if x>y
 */
extern int BIG_dcomp(DBIG x,DBIG y);
/**	@brief Calculate number of bits in a BIG - output normalised
 *
	@param x BIG number
	@return Number of bits in x
 */
extern int BIG_nbits(BIG x);
/**	@brief Calculate number of bits in a DBIG - output normalised
 *
	@param x DBIG number
	@return Number of bits in x
 */
extern int BIG_dnbits(DBIG x);
/**	@brief Reduce x mod n - input and output normalised
 *
	Slow but rarely used
	@param x BIG number to be reduced mod n
	@param n The modulus
 */
extern void BIG_mod(BIG x,BIG n);
/**	@brief Divide x by n - output normalised
 *
	Slow but rarely used
	@param x BIG number to be divided by n
	@param n The Divisor
 */
extern void BIG_sdiv(BIG x,BIG n);
/**	@brief  x=y mod n - output normalised
 *
	Slow but rarely used. y is destroyed.
	@param x BIG number, on exit = y mod n
	@param y DBIG number
	@param n Modulus
 */
extern void BIG_dmod(BIG x,DBIG y,BIG n);
/**	@brief  x=y/n - output normalised
 *
	Slow but rarely used. y is destroyed.
	@param x BIG number, on exit = y/n
	@param y DBIG number
	@param n Modulus
 */
extern void BIG_ddiv(BIG x,DBIG y,BIG n);
/**	@brief  return parity of BIG, that is the least significant bit
 *
	@param x BIG number
	@return 0 or 1
 */
extern int BIG_parity(BIG x);
/**	@brief  return i-th of BIG
 *
	@param x BIG number
	@param i the bit of x to be returned
	@return 0 or 1
 */
extern int BIG_bit(BIG x,int i);
/**	@brief  return least significant bits of a BIG
 *
	@param x BIG number
	@param n number of bits to return. Assumed to be less than BASEBITS.
	@return least significant n bits as an integer
 */
extern int BIG_lastbits(BIG x,int n);
/**	@brief  Create a random BIG from a random number generator
 *
	Assumes that the random number generator has been suitably initialised
	@param x BIG number, on exit a random number
	@param r A pointer to a Cryptographically Secure Random Number Generator
 */
extern void BIG_random(BIG x,csprng *r);
/**	@brief  Create an unbiased random BIG from a random number generator, reduced with respect to a modulus
 *
	Assumes that the random number generator has been suitably initialised
	@param x BIG number, on exit a random number
	@param n The modulus
	@param r A pointer to a Cryptographically Secure Random Number Generator
 */
extern void BIG_randomnum(BIG x,BIG n,csprng *r);
/**	brief  return NAF (Non-Adjacent-Form) value as +/- 1, 3 or 5, inputs must be normalised
 *
	Given x and 3*x extracts NAF value from given bit position, and returns number of bits processed, and number of trailing zeros detected if any
	param x BIG number
	param x3 BIG number, three times x
	param i bit position
	param nbs pointer to integer returning number of bits processed
	param nzs pointer to integer returning number of trailing 0s
	return + or - 1, 3 or 5
*/
//extern int BIG_nafbits(BIG x,BIG x3,int i,int *nbs,int *nzs);

/**	@brief  Calculate x=y*z mod n
 *
	Slow method for modular multiplication
	@param x BIG number, on exit = y*z mod n
	@param y BIG number
	@param z BIG number
	@param n The BIG Modulus
 */
extern void BIG_modmul(BIG x,BIG y,BIG z,BIG n);
/**	@brief  Calculate x=y/z mod n
 *
	Slow method for modular division
	@param x BIG number, on exit = y/z mod n
	@param y BIG number
	@param z BIG number
	@param n The BIG Modulus
 */
extern void BIG_moddiv(BIG x,BIG y,BIG z,BIG n);
/**	@brief  Calculate x=y^2 mod n
 *
	Slow method for modular squaring
	@param x BIG number, on exit = y^2 mod n
	@param y BIG number
	@param n The BIG Modulus
 */
extern void BIG_modsqr(BIG x,BIG y,BIG n);
/**	@brief  Calculate x=-y mod n
 *
	Modular negation
	@param x BIG number, on exit = -y mod n
	@param y BIG number
	@param n The BIG Modulus
 */
extern void BIG_modneg(BIG x,BIG y,BIG n);
/**	@brief  Calculate jacobi Symbol (x/y)
 *
	@param x BIG number
	@param y BIG number
	@return Jacobi symbol, -1,0 or 1
 */
extern int BIG_jacobi(BIG x,BIG y);
/**	@brief  Calculate x=1/y mod n
 *
	Modular Inversion - This is slow. Uses binary method.
	@param x BIG number, on exit = 1/y mod n
	@param y BIG number
	@param n The BIG Modulus
 */
extern void BIG_invmodp(BIG x,BIG y,BIG n);
/** @brief Calculate x=x mod 2^m
 *
	Truncation
	@param x BIG number, on reduced mod 2^m
	@param m new truncated size
*/
extern void BIG_mod2m(BIG x,int m);



/* FP prototypes */

/**	@brief Tests for BIG equal to zero mod Modulus
 *
	@param x BIG number to be tested
	@return 1 if zero, else returns 0
 */
extern int FP_iszilch(BIG x);
/**	@brief Converts from BIG integer to n-residue form mod Modulus
 *
	@param x BIG number to be converted
 */
extern void FP_nres(BIG x);
/**	@brief Converts from n-residue form back to BIG integer form
 *
	@param x BIG number to be converted
 */
extern void FP_redc(BIG x);
/**	@brief Sets BIG to representation of unity in n-residue form
 *
	@param x BIG number to be set equal to unity.
 */
extern void FP_one(BIG x);
/**	@brief Reduces DBIG to BIG exploiting special form of the modulus
 *
	This function comes in different flavours depending on the form of Modulus that is currently in use.
	@param r BIG number, on exit = d mod Modulus
	@param d DBIG number to be reduced
 */
extern void FP_mod(BIG r,DBIG d);
/**	@brief Fast Modular multiplication of two BIGs in n-residue form, mod Modulus
 *
	Uses appropriate fast modular reduction method
	@param x BIG number, on exit the modular product = y*z mod Modulus
	@param y BIG number, the multiplicand
	@param z BIG number, the multiplier
 */
extern void FP_mul(BIG x,BIG y,BIG z);
/**	@brief Fast Modular multiplication of a BIG in n-residue form, by a small integer, mod Modulus
 *
	@param x BIG number, on exit the modular product = y*i mod Modulus
	@param y BIG number, the multiplicand
	@param i a small number, the multiplier
 */
extern void FP_imul(BIG x,BIG y,int i);
/**	@brief Fast Modular squaring of a BIG in n-residue form, mod Modulus
 *
	Uses appropriate fast modular reduction method
	@param x BIG number, on exit the modular product = y^2 mod Modulus
	@param y BIG number, the number to be squared

 */
extern void FP_sqr(BIG x,BIG y);
/**	@brief Modular addition of two BIGs in n-residue form, mod Modulus
 *
	@param x BIG number, on exit the modular sum = y+z mod Modulus
	@param y BIG number
	@param z BIG number
 */
extern void FP_add(BIG x,BIG y,BIG z);
/**	@brief Modular subtraction of two BIGs in n-residue form, mod Modulus
 *
	@param x BIG number, on exit the modular difference = y-z mod Modulus
	@param y BIG number
	@param z BIG number
 */
extern void FP_sub(BIG x,BIG y,BIG z);
/**	@brief Modular division by 2 of a BIG in n-residue form, mod Modulus
 *
	@param x BIG number, on exit =y/2 mod Modulus
	@param y BIG number
 */
extern void FP_div2(BIG x,BIG y);
/**	@brief Fast Modular exponentiation of a BIG in n-residue form, to the power of a BIG, mod Modulus
 *
	@param x BIG number, on exit  = y^z mod Modulus
	@param y BIG number
	@param z Big number exponent
 */
extern void FP_pow(BIG x,BIG y,BIG z);
/**	@brief Fast Modular square root of a BIG in n-residue form, mod Modulus
 *
	@param x BIG number, on exit  = sqrt(y) mod Modulus
	@param y BIG number, the number whose square root is calculated

 */
extern void FP_sqrt(BIG x,BIG y);
/**	@brief Modular negation of a BIG in n-residue form, mod Modulus
 *
	@param x BIG number, on exit = -y mod Modulus
	@param y BIG number
 */
extern void FP_neg(BIG x,BIG y);
/**	@brief Outputs a BIG number that is in n-residue form to the console
 *
	Converts from n-residue form before output
	@param x a BIG number
 */
extern void FP_output(BIG x);
/**	@brief Outputs a BIG number that is in n-residue form to the console, in raw form
 *
	Converts from n-residue form before output
	@param x a BIG number
 */
extern void FP_rawoutput(BIG x);
/**	@brief Reduces possibly unreduced BIG mod Modulus
 *
	@param x BIG number, on exit reduced mod Modulus
 */
extern void FP_reduce(BIG x);
/**	@brief Tests for BIG a quadratic residue mod Modulus
 *
	@param x BIG number to be tested
	@return 1 if quadratic residue, else returns 0 if quadratic non-residue
 */
extern int FP_qr(BIG x);
/**	@brief Modular inverse of a BIG in n-residue form, mod Modulus
 *
	@param x BIG number, on exit = 1/y mod Modulus
	@param y BIG number
 */
extern void FP_inv(BIG x,BIG y);


/* FP2 prototypes */

/**	@brief Tests for FP2 equal to zero
 *
	@param x FP2 number to be tested
	@return 1 if zero, else returns 0
 */
extern int FP2_iszilch(FP2 *x);
/**	@brief Conditional copy of FP2 number
 *
	Conditionally copies second parameter to the first (without branching)
	@param x FP2 instance, set to y if s!=0
	@param y another FP2 instance
	@param s copy only takes place if not equal to 0
 */
extern void FP2_cmove(FP2 *x,FP2 *y,int s);
/**	@brief Tests for FP2 equal to one
 *
	@param x FP2 instance to be tested
	@return 1 if x=1, else returns 0
 */
extern int FP2_isunity(FP2 *x);
/**	@brief Tests for equality of two FP2s
 *
	@param x FP2 instance to be compared
	@param y FP2 instance to be compared
	@return 1 if x=y, else returns 0
 */
extern int FP2_equals(FP2 *x,FP2 *y);
/**	@brief Initialise FP2 from two BIGs in n-residue form
 *
	@param x FP2 instance to be initialised
	@param a BIG to form real part of FP2
	@param b BIG to form imaginary part of FP2
 */
extern void FP2_from_FPs(FP2 *x,BIG a,BIG b);
/**	@brief Initialise FP2 from two BIG integers
 *
	@param x FP2 instance to be initialised
	@param a BIG to form real part of FP2
	@param b BIG to form imaginary part of FP2
 */
extern void FP2_from_BIGs(FP2 *x,BIG a,BIG b);
/**	@brief Initialise FP2 from single BIG in n-residue form
 *
	Imaginary part is set to zero
	@param x FP2 instance to be initialised
	@param a BIG to form real part of FP2
 */
extern void FP2_from_FP(FP2 *x,BIG a);
/**	@brief Initialise FP2 from single BIG
 *
	Imaginary part is set to zero
	@param x FP2 instance to be initialised
	@param a BIG to form real part of FP2
 */
extern void FP2_from_BIG(FP2 *x,BIG a);
/**	@brief Copy FP2 to another FP2
 *
	@param x FP2 instance, on exit = y
	@param y FP2 instance to be copied
 */
extern void FP2_copy(FP2 *x,FP2 *y);
/**	@brief Set FP2 to zero
 *
	@param x FP2 instance to be set to zero
 */
extern void FP2_zero(FP2 *x);
/**	@brief Set FP2 to unity
 *
	@param x FP2 instance to be set to one
 */
extern void FP2_one(FP2 *x);
/**	@brief Negation of FP2
 *
	@param x FP2 instance, on exit = -y
	@param y FP2 instance
 */
extern void FP2_neg(FP2 *x,FP2 *y);
/**	@brief Conjugation of FP2
 *
	If y=(a,b) on exit x=(a,-b)
	@param x FP2 instance, on exit = conj(y)
	@param y FP2 instance
 */
extern void FP2_conj(FP2 *x,FP2 *y);
/**	@brief addition of two FP2s
 *
	@param x FP2 instance, on exit = y+z
	@param y FP2 instance
	@param z FP2 instance
 */
extern void FP2_add(FP2 *x,FP2 *y,FP2 *z);
/**	@brief subtraction of two FP2s
 *
	@param x FP2 instance, on exit = y-z
	@param y FP2 instance
	@param z FP2 instance
 */
extern void FP2_sub(FP2 *x,FP2 *y,FP2 *z);
/**	@brief Multiplication of an FP2 by an n-residue
 *
	@param x FP2 instance, on exit = y*b
	@param y FP2 instance
	@param b BIG n-residue
 */
extern void FP2_pmul(FP2 *x,FP2 *y,BIG b);
/**	@brief Multiplication of an FP2 by a small integer
 *
	@param x FP2 instance, on exit = y*i
	@param y FP2 instance
	@param i an integer
 */
extern void FP2_imul(FP2 *x,FP2 *y,int i);
/**	@brief Squaring an FP2
 *
	@param x FP2 instance, on exit = y^2
	@param y FP2 instance
 */
extern void FP2_sqr(FP2 *x,FP2 *y);
/**	@brief Multiplication of two FP2s
 *
	@param x FP2 instance, on exit = y*z
	@param y FP2 instance
	@param z FP2 instance
 */
extern void FP2_mul(FP2 *x,FP2 *y,FP2 *z);
/**	@brief Formats and outputs an FP2 to the console
 *
	@param x FP2 instance
 */
extern void FP2_output(FP2 *x);
/**	@brief Formats and outputs an FP2 to the console in raw form (for debugging)
 *
	@param x FP2 instance
 */
extern void FP2_rawoutput(FP2 *x);
/**	@brief Inverting an FP2
 *
	@param x FP2 instance, on exit = 1/y
	@param y FP2 instance
 */
extern void FP2_inv(FP2 *x,FP2 *y);
/**	@brief Divide an FP2 by 2
 *
	@param x FP2 instance, on exit = y/2
	@param y FP2 instance
 */
extern void FP2_div2(FP2 *x,FP2 *y);
/**	@brief Multiply an FP2 by (1+sqrt(-1))
 *
	Note that (1+sqrt(-1)) is irreducible for FP4
	@param x FP2 instance, on exit = x*(1+sqrt(-1))
 */
extern void FP2_mul_ip(FP2 *x);
/**	@brief Divide an FP2 by (1+sqrt(-1))
 *
	Note that (1+sqrt(-1)) is irreducible for FP4
	@param x FP2 instance, on exit = x/(1+sqrt(-1))
 */
extern void FP2_div_ip(FP2 *x);
/**	@brief Normalises the components of an FP2
 *
	@param x FP2 instance to be normalised
 */
extern void FP2_norm(FP2 *x);
/**	@brief Reduces all components of possibly unreduced FP2 mod Modulus
 *
	@param x FP2 instance, on exit reduced mod Modulus
 */
extern void FP2_reduce(FP2 *x);
/**	@brief Raises an FP2 to the power of a BIG
 *
	@param x FP2 instance, on exit = y^b
	@param y FP2 instance
	@param b BIG number
 */
extern void FP2_pow(FP2 *x,FP2 *y,BIG b);
/**	@brief Square root of an FP2
 *
	@param x FP2 instance, on exit = sqrt(y)
	@param y FP2 instance
 */
extern int FP2_sqrt(FP2 *x,FP2 *y);



/* ECP E(Fp) prototypes */
/**	@brief Tests for ECP point equal to infinity
 *
	@param P ECP point to be tested
	@return 1 if infinity, else returns 0
 */
extern int ECP_isinf(ECP *P);
/**	@brief Tests for equality of two ECPs
 *
	@param P ECP instance to be compared
	@param Q ECP instance to be compared
	@return 1 if P=Q, else returns 0
 */
extern int ECP_equals(ECP *P,ECP *Q);
/**	@brief Copy ECP point to another ECP point
 *
	@param P ECP instance, on exit = Q
	@param Q ECP instance to be copied
 */
extern void ECP_copy(ECP *P,ECP *Q);
/**	@brief Negation of an ECP point
 *
	@param P ECP instance, on exit = -P
 */
extern void ECP_neg(ECP *P);
/**	@brief Set ECP to point-at-infinity
 *
	@param P ECP instance to be set to infinity
 */
extern void ECP_inf(ECP *P);
/**	@brief Calculate Right Hand Side of curve equation y^2=f(x)
 *
	Function f(x) depends on form of elliptic curve, Weierstrass, Edwards or Montgomery.
	Used internally.
	@param r BIG n-residue value of f(x)
	@param x BIG n-residue x
 */
extern void ECP_rhs(BIG r,BIG x);
/**	@brief Set ECP to point(x,y) given just x and sign of y
 *
	Point P set to infinity if no such point on the curve. If x is on the curve then y is calculated from the curve equation.
	The correct y value (plus or minus) is selected given its sign s.
	@param P ECP instance to be set (x,[y])
	@param x BIG x coordinate of point
	@param s an integer representing the "sign" of y, in fact its least significant bit.
 */
extern int ECP_setx(ECP *P,BIG x,int s);

#if CURVETYPE==MONTGOMERY
/**	@brief Set ECP to point(x,[y]) given x
 *
	Point P set to infinity if no such point on the curve. Note that y coordinate is not needed.
	@param P ECP instance to be set (x,[y])
	@param x BIG x coordinate of point
	@return 1 if point exists, else 0
 */
extern int ECP_set(ECP *P,BIG x);
/**	@brief Extract x coordinate of an ECP point P
 *
	@param x BIG on exit = x coordinate of point
	@param P ECP instance (x,[y])
	@return -1 if P is point-at-infinity, else 0
 */
extern int ECP_get(BIG x,ECP *P);
/**	@brief Adds ECP instance Q to ECP instance P, given difference D=P-Q
 *
	Differential addition of points on a Montgomery curve
	@param P ECP instance, on exit =P+Q
	@param Q ECP instance to be added to P
	@param D Difference between P and Q
 */
extern void ECP_add(ECP *P,ECP *Q,ECP *D);
#else
/**	@brief Set ECP to point(x,y) given x and y
 *
	Point P set to infinity if no such point on the curve.
	@param P ECP instance to be set (x,y)
	@param x BIG x coordinate of point
	@param y BIG y coordinate of point
	@return 1 if point exists, else 0
 */
extern int ECP_set(ECP *P,BIG x,BIG y);
/**	@brief Extract x and y coordinates of an ECP point P
 *
	If x=y, returns only x
	@param x BIG on exit = x coordinate of point
	@param y BIG on exit = y coordinate of point (unless x=y)
	@param P ECP instance (x,y)
	@return sign of y, or -1 if P is point-at-infinity
 */
extern int ECP_get(BIG x,BIG y,ECP *P);
/**	@brief Adds ECP instance Q to ECP instance P
 *
	@param P ECP instance, on exit =P+Q
	@param Q ECP instance to be added to P
 */
extern void ECP_add(ECP *P,ECP *Q);
/**	@brief Subtracts ECP instance Q from ECP instance P
 *
	@param P ECP instance, on exit =P-Q
	@param Q ECP instance to be subtracted from P
 */
extern void ECP_sub(ECP *P,ECP *Q);
#endif
/**	@brief Converts an ECP point from Projective (x,y,z) coordinates to affine (x,y) coordinates
 *
	@param P ECP instance to be converted to affine form
 */
extern void ECP_affine(ECP *P);
/**	@brief Formats and outputs an ECP point to the console, in projective coordinates
 *
	@param P ECP instance to be printed
 */
extern void ECP_outputxyz(ECP *P);
/**	@brief Formats and outputs an ECP point to the console, converted to affine coordinates
 *
	@param P ECP instance to be printed
 */
extern void ECP_output(ECP * P);
/**	@brief Formats and outputs an ECP point to an octet string
 *
	The octet string is created in the standard form 04|x|y, except for Montgomery curve in which case it is 06|x
	Here x (and y) are the x and y coordinates in big-endian base 256 form.
	@param S output octet string
	@param P ECP instance to be converted to an octet string
 */
extern void ECP_toOctet(octet *S,ECP *P);
/**	@brief Creates an ECP point from an octet string
 *
	The octet string is in the standard form 0x04|x|y, except for Montgomery curve in which case it is 0x06|x
	Here x (and y) are the x and y coordinates in left justified big-endian base 256 form.
	@param P ECP instance to be created from the octet string
	@param S input octet string
	return 1 if octet string corresponds to a point on the curve, else 0
 */
extern int ECP_fromOctet(ECP *P,octet *S);
/**	@brief Doubles an ECP instance P
 *
	@param P ECP instance, on exit =2*P
 */
extern void ECP_dbl(ECP *P);
/**	@brief Multiplies an ECP instance P by a small integer, side-channel resistant
 *
	@param P ECP instance, on exit =i*P
	@param i small integer multiplier
	@param b maximum number of bits in multiplier
 */
extern void ECP_pinmul(ECP *P,int i,int b);
/**	@brief Multiplies an ECP instance P by a BIG, side-channel resistant
 *
	Uses Montgomery ladder for Montgomery curves, otherwise fixed sized windows.
	@param P ECP instance, on exit =b*P
	@param b BIG number multiplier

 */
extern void ECP_mul(ECP *P,BIG b);
/**	@brief Calculates double multiplication P=e*P+f*Q, side-channel resistant
 *
	@param P ECP instance, on exit =e*P+f*Q
	@param Q ECP instance
	@param e BIG number multiplier
	@param f BIG number multiplier
 */
extern void ECP_mul2(ECP *P,ECP *Q,BIG e,BIG f);



/* ECP2 E(Fp2) prototypes */
/**	@brief Tests for ECP2 point equal to infinity
 *
	@param P ECP2 point to be tested
	@return 1 if infinity, else returns 0
 */
extern int ECP2_isinf(ECP2 *P);
/**	@brief Copy ECP2 point to another ECP2 point
 *
	@param P ECP2 instance, on exit = Q
	@param Q ECP2 instance to be copied
 */
extern void ECP2_copy(ECP2 *P,ECP2 *Q);
/**	@brief Set ECP2 to point-at-infinity
 *
	@param P ECP2 instance to be set to infinity
 */
extern void ECP2_inf(ECP2 *P);
/**	@brief Tests for equality of two ECP2s
 *
	@param P ECP2 instance to be compared
	@param Q ECP2 instance to be compared
	@return 1 if P=Q, else returns 0
 */
extern int ECP2_equals(ECP2 *P,ECP2 *Q);
/**	@brief Converts an ECP2 point from Projective (x,y,z) coordinates to affine (x,y) coordinates
 *
	@param P ECP2 instance to be converted to affine form
 */
extern void ECP2_affine(ECP2 *P);
/**	@brief Extract x and y coordinates of an ECP2 point P
 *
	If x=y, returns only x
	@param x FP2 on exit = x coordinate of point
	@param y FP2 on exit = y coordinate of point (unless x=y)
	@param P ECP2 instance (x,y)
	@return -1 if P is point-at-infinity, else 0
 */
extern int ECP2_get(FP2 *x,FP2 *y,ECP2 *P);
/**	@brief Formats and outputs an ECP2 point to the console, converted to affine coordinates
 *
	@param P ECP2 instance to be printed
 */
extern void ECP2_output(ECP2 *P);
/**	@brief Formats and outputs an ECP2 point to the console, in projective coordinates
 *
	@param P ECP2 instance to be printed
 */
extern void ECP2_outputxyz(ECP2 *P);
/**	@brief Formats and outputs an ECP2 point to an octet string
 *
	The octet string is created in the form x|y.
	Convert the real and imaginary parts of the x and y coordinates to big-endian base 256 form.
	@param S output octet string
	@param P ECP2 instance to be converted to an octet string
 */
extern void ECP2_toOctet(octet *S,ECP2 *P);
/**	@brief Creates an ECP2 point from an octet string
 *
	The octet string is in the form x|y
	The real and imaginary parts of the x and y coordinates are in big-endian base 256 form.
	@param P ECP2 instance to be created from the octet string
	@param S input octet string
	return 1 if octet string corresponds to a point on the curve, else 0
 */
extern int ECP2_fromOctet(ECP2 *P,octet *S);
/**	@brief Calculate Right Hand Side of curve equation y^2=f(x)
 *
	Function f(x)=x^3+Ax+B
	Used internally.
	@param r FP2 value of f(x)
	@param x FP2 instance
 */
extern void ECP2_rhs(FP2 *r,FP2 *x);
/**	@brief Set ECP2 to point(x,y) given x and y
 *
	Point P set to infinity if no such point on the curve.
	@param P ECP2 instance to be set (x,y)
	@param x FP2 x coordinate of point
	@param y FP2 y coordinate of point
	@return 1 if point exists, else 0
 */
extern int ECP2_set(ECP2 *P,FP2 *x,FP2 *y);
/**	@brief Set ECP to point(x,[y]) given x
 *
	Point P set to infinity if no such point on the curve. Otherwise y coordinate is calculated from x.
	@param P ECP instance to be set (x,[y])
	@param x BIG x coordinate of point
	@return 1 if point exists, else 0
 */
extern int ECP2_setx(ECP2 *P,FP2 *x);
/**	@brief Negation of an ECP2 point
 *
	@param P ECP2 instance, on exit = -P
 */
extern void ECP2_neg(ECP2 *P);
/**	@brief Doubles an ECP2 instance P
 *
	@param P ECP2 instance, on exit =2*P
 */
extern int ECP2_dbl(ECP2 *P);
/**	@brief Adds ECP2 instance Q to ECP2 instance P
 *
	@param P ECP2 instance, on exit =P+Q
	@param Q ECP2 instance to be added to P
 */
extern int ECP2_add(ECP2 *P,ECP2 *Q);
/**	@brief Subtracts ECP instance Q from ECP2 instance P
 *
	@param P ECP2 instance, on exit =P-Q
	@param Q ECP2 instance to be subtracted from P
 */
extern void ECP2_sub(ECP2 *P,ECP2 *Q);
/**	@brief Multiplies an ECP2 instance P by a BIG, side-channel resistant
 *
	Uses fixed sized windows.
	@param P ECP2 instance, on exit =b*P
	@param b BIG number multiplier

 */
extern void ECP2_mul(ECP2 *P,BIG b);
/**	@brief Multiplies an ECP2 instance P by the internal modulus p, using precalculated Frobenius constant f
 *
	Fast point multiplication using Frobenius
	@param P ECP2 instance, on exit = p*P
	@param f FP2 precalculated Frobenius constant

 */
extern void ECP2_frob(ECP2 *P,FP2 *f);
/**	@brief Calculates P=b[0]*Q[0]+b[1]*Q[1]+b[2]*Q[2]+b[3]*Q[3]
 *
	@param P ECP2 instance, on exit = b[0]*Q[0]+b[1]*Q[1]+b[2]*Q[2]+b[3]*Q[3]
	@param Q ECP2 array of 4 points
	@param b BIG array of 4 multipliers
 */
extern void ECP2_mul4(ECP2 *P,ECP2 *Q,BIG *b);



/* FP4 prototypes */
/**	@brief Tests for FP4 equal to zero
 *
	@param x FP4 number to be tested
	@return 1 if zero, else returns 0
 */
extern int FP4_iszilch(FP4 *x);
/**	@brief Tests for FP4 equal to unity
 *
	@param x FP4 number to be tested
	@return 1 if unity, else returns 0
 */
extern int FP4_isunity(FP4 *x);
/**	@brief Tests for equality of two FP4s
 *
	@param x FP4 instance to be compared
	@param y FP4 instance to be compared
	@return 1 if x=y, else returns 0
 */
extern int FP4_equals(FP4 *x,FP4 *y);
/**	@brief Tests for FP4 having only a real part and no imaginary part
 *
	@param x FP4 number to be tested
	@return 1 if real, else returns 0
 */
extern int FP4_isreal(FP4 *x);
/**	@brief Initialise FP4 from two FP2s
 *
	@param x FP4 instance to be initialised
	@param a FP2 to form real part of FP4
	@param b FP2 to form imaginary part of FP4
 */
extern void FP4_from_FP2s(FP4 *x,FP2 *a,FP2 *b);
/**	@brief Initialise FP4 from single FP2
 *
	Imaginary part is set to zero
	@param x FP4 instance to be initialised
	@param a FP2 to form real part of FP4
 */
extern void FP4_from_FP2(FP4 *x,FP2 *a);
/**	@brief Copy FP4 to another FP4
 *
	@param x FP4 instance, on exit = y
	@param y FP4 instance to be copied
 */
extern void FP4_copy(FP4 *x,FP4 *y);
/**	@brief Set FP4 to zero
 *
	@param x FP4 instance to be set to zero
 */
extern void FP4_zero(FP4 *x);
/**	@brief Set FP4 to unity
 *
	@param x FP4 instance to be set to one
 */
extern void FP4_one(FP4 *x);
/**	@brief Negation of FP4
 *
	@param x FP4 instance, on exit = -y
	@param y FP4 instance
 */
extern void FP4_neg(FP4 *x,FP4 *y);
/**	@brief Conjugation of FP4
 *
	If y=(a,b) on exit x=(a,-b)
	@param x FP4 instance, on exit = conj(y)
	@param y FP4 instance
 */
extern void FP4_conj(FP4 *x,FP4 *y);
/**	@brief Negative conjugation of FP4
 *
	If y=(a,b) on exit x=(-a,b)
	@param x FP4 instance, on exit = -conj(y)
	@param y FP4 instance
 */
extern void FP4_nconj(FP4 *x,FP4 *y);
/**	@brief addition of two FP4s
 *
	@param x FP4 instance, on exit = y+z
	@param y FP4 instance
	@param z FP4 instance
 */
extern void FP4_add(FP4 *x,FP4 *y,FP4 *z);
/**	@brief subtraction of two FP4s
 *
	@param x FP4 instance, on exit = y-z
	@param y FP4 instance
	@param z FP4 instance
 */
extern void FP4_sub(FP4 *x,FP4 *y,FP4 *z);
/**	@brief Multiplication of an FP4 by an FP2
 *
	@param x FP4 instance, on exit = y*a
	@param y FP4 instance
	@param a FP2 multiplier
 */
extern void FP4_pmul(FP4 *x,FP4 *y,FP2 *a);
/**	@brief Multiplication of an FP4 by a small integer
 *
	@param x FP4 instance, on exit = y*i
	@param y FP4 instance
	@param i an integer
 */
extern void FP4_imul(FP4 *x,FP4 *y,int i);
/**	@brief Squaring an FP4
 *
	@param x FP4 instance, on exit = y^2
	@param y FP4 instance
 */
extern void FP4_sqr(FP4 *x,FP4 *y);
/**	@brief Multiplication of two FP4s
 *
	@param x FP4 instance, on exit = y*z
	@param y FP4 instance
	@param z FP4 instance
 */
extern void FP4_mul(FP4 *x,FP4 *y,FP4 *z);
/**	@brief Inverting an FP4
 *
	@param x FP4 instance, on exit = 1/y
	@param y FP4 instance
 */
extern void FP4_inv(FP4 *x,FP4 *y);
/**	@brief Formats and outputs an FP4 to the console
 *
	@param x FP4 instance to be printed
 */
extern void FP4_output(FP4 *x);
/**	@brief Formats and outputs an FP4 to the console in raw form (for debugging)
 *
	@param x FP4 instance to be printed
 */
extern void FP4_rawoutput(FP4 *x);
/**	@brief multiplies an FP4 instance by irreducible polynomial sqrt(1+sqrt(-1))
 *
	@param x FP4 instance, on exit = sqrt(1+sqrt(-1)*x
 */
extern void FP4_times_i(FP4 *x);
/**	@brief Normalises the components of an FP4
 *
	@param x FP4 instance to be normalised
 */
extern void FP4_norm(FP4 *x);
/**	@brief Reduces all components of possibly unreduced FP4 mod Modulus
 *
	@param x FP4 instance, on exit reduced mod Modulus
 */
extern void FP4_reduce(FP4 *x);
/**	@brief Raises an FP4 to the power of a BIG
 *
	@param x FP4 instance, on exit = y^b
	@param y FP4 instance
	@param b BIG number
 */
extern void FP4_pow(FP4 *x,FP4 *y,BIG b);
/**	@brief Raises an FP4 to the power of the internal modulus p, using the Frobenius
 *
	@param x FP4 instance, on exit = x^p
	@param f FP2 precalculated Frobenius constant
 */
extern void FP4_frob(FP4 *x,FP2 *f);
/**	@brief Calculates the XTR addition function r=w*x-conj(x)*y+z
 *
	@param r FP4 instance, on exit = w*x-conj(x)*y+z
	@param w FP4 instance
	@param x FP4 instance
	@param y FP4 instance
	@param z FP4 instance
 */
extern void FP4_xtr_A(FP4 *r,FP4 *w,FP4 *x,FP4 *y,FP4 *z);
/**	@brief Calculates the XTR doubling function r=x^2-2*conj(x)
 *
	@param r FP4 instance, on exit = x^2-2*conj(x)
	@param x FP4 instance
 */
extern void FP4_xtr_D(FP4 *r,FP4 *x);
/**	@brief Calculates FP4 trace of an FP12 raised to the power of a BIG number
 *
	XTR single exponentiation
	@param r FP4 instance, on exit = trace(w^b)
	@param x FP4 instance, trace of an FP12 w
	@param b BIG number
 */
extern void FP4_xtr_pow(FP4 *r,FP4 *x,BIG b);
/**	@brief Calculates FP4 trace of c^a.d^b, where c and d are derived from FP4 traces of FP12s
 *
	XTR double exponentiation
	Assumes c=tr(x^m), d=tr(x^n), e=tr(x^(m-n)), f=tr(x^(m-2n))
	@param r FP4 instance, on exit = trace(c^a.d^b)
	@param c FP4 instance, trace of an FP12
	@param d FP4 instance, trace of an FP12
	@param e FP4 instance, trace of an FP12
	@param f FP4 instance, trace of an FP12
	@param a BIG number
	@param b BIG number
 */
extern void FP4_xtr_pow2(FP4 *r,FP4 *c,FP4 *d,FP4 *e,FP4 *f,BIG a,BIG b);



/* FP12 prototypes */
/**	@brief Tests for FP12 equal to zero
 *
	@param x FP12 number to be tested
	@return 1 if zero, else returns 0
 */
extern int FP12_iszilch(FP12 *x);
/**	@brief Tests for FP12 equal to unity
 *
	@param x FP12 number to be tested
	@return 1 if unity, else returns 0
 */
extern int FP12_isunity(FP12 *x);
/**	@brief Copy FP12 to another FP12
 *
	@param x FP12 instance, on exit = y
	@param y FP12 instance to be copied
 */
extern void FP12_copy(FP12 *x,FP12 *y);
/**	@brief Set FP12 to unity
 *
	@param x FP12 instance to be set to one
 */
extern void FP12_one(FP12 *x);
/**	@brief Tests for equality of two FP12s
 *
	@param x FP12 instance to be compared
	@param y FP12 instance to be compared
	@return 1 if x=y, else returns 0
 */
extern int FP12_equals(FP12 *x,FP12 *y);
/**	@brief Conjugation of FP12
 *
	If y=(a,b,c) (where a,b,c are its three FP4 components) on exit x=(conj(a),-conj(b),conj(c))
	@param x FP12 instance, on exit = conj(y)
	@param y FP12 instance
 */
extern void FP12_conj(FP12 *x,FP12 *y);
/**	@brief Initialise FP12 from single FP4
 *
	Sets first FP4 component of an FP12, other components set to zero
	@param x FP12 instance to be initialised
	@param a FP4 to form first part of FP4
 */
extern void FP12_from_FP4(FP12 *x,FP4 *a);
/**	@brief Initialise FP12 from three FP4s
 *
	@param x FP12 instance to be initialised
	@param a FP4 to form first part of FP12
	@param b FP4 to form second part of FP12
	@param c FP4 to form third part of FP12
 */
extern void FP12_from_FP4s(FP12 *x,FP4 *a,FP4* b,FP4 *c);
/**	@brief Fast Squaring of an FP12 in "unitary" form
 *
	@param x FP12 instance, on exit = y^2
	@param y FP4 instance, must be unitary
 */
extern void FP12_usqr(FP12 *x,FP12 *y);
/**	@brief Squaring an FP12
 *
	@param x FP12 instance, on exit = y^2
	@param y FP12 instance
 */
extern void FP12_sqr(FP12 *x,FP12 *y);
/**	@brief Fast multiplication of an FP12 by an FP12 that arises from an ATE pairing line function
 *
	Here the multiplier has a special form that can be exploited
	@param x FP12 instance, on exit = x*y
	@param y FP12 instance, of special form
 */
extern void FP12_smul(FP12 *x,FP12 *y);
/**	@brief Multiplication of two FP12s
 *
	@param x FP12 instance, on exit = x*y
	@param y FP12 instance, the multiplier
 */
extern void FP12_mul(FP12 *x,FP12 *y);
/**	@brief Inverting an FP12
 *
	@param x FP12 instance, on exit = 1/y
	@param y FP12 instance
 */
extern void FP12_inv(FP12 *x,FP12 *y);
/**	@brief Raises an FP12 to the power of a BIG
 *
	@param r FP12 instance, on exit = y^b
	@param x FP12 instance
	@param b BIG number
 */
extern void FP12_pow(FP12 *r,FP12 *x,BIG b);
/**	@brief Raises an FP12 instance x to a small integer power, side-channel resistant
 *
	@param x ECP instance, on exit = x^i
	@param i small integer exponent
	@param b maximum number of bits in exponent
 */
extern void FP12_pinpow(FP12 *x,int i,int b);
/**	@brief Calculate x[0]^b[0].x[1]^b[1].x[2]^b[2].x[3]^b[3], side-channel resistant
 *
	@param r ECP instance, on exit = x[0]^b[0].x[1]^b[1].x[2]^b[2].x[3]^b[3]
	@param x FP12 array with 4 FP12s
	@param b BIG array of 4 exponents
 */
extern void FP12_pow4(FP12 *r,FP12 *x,BIG *b);
/**	@brief Raises an FP12 to the power of the internal modulus p, using the Frobenius
 *
	@param x FP12 instance, on exit = x^p
	@param f FP2 precalculated Frobenius constant
 */
extern void FP12_frob(FP12 *x,FP2 *f);
/**	@brief Reduces all components of possibly unreduced FP12 mod Modulus
 *
	@param x FP12 instance, on exit reduced mod Modulus
 */
extern void FP12_reduce(FP12 *x);
/**	@brief Normalises the components of an FP12
 *
	@param x FP12 instance to be normalised
 */
extern void FP12_norm(FP12 *x);
/**	@brief Formats and outputs an FP12 to the console
 *
	@param x FP12 instance to be printed
 */
extern void FP12_output(FP12 *x);
/**	@brief Formats and outputs an FP12 instance to an octet string
 *
	Serializes the components of an FP12 to big-endian base 256 form.
	@param S output octet string
	@param x FP12 instance to be converted to an octet string
 */
extern void FP12_toOctet(octet *S,FP12 *x);
/**	@brief Creates an FP12 instance from an octet string
 *
	De-serializes the components of an FP12 to create an FP12 from big-endian base 256 components.
	@param x FP12 instance to be created from an octet string
	@param S input octet string

 */
extern void FP12_fromOctet(FP12 *x,octet *S);
/**	@brief Calculate the trace of an FP12
 *
	@param t FP4 trace of x, on exit = tr(x)
	@param x FP12 instance

 */
extern void FP12_trace(FP4 *t,FP12 *x);



/* Pairing function prototypes */
/**	@brief Calculate Miller loop for Optimal ATE pairing e(P,Q)
 *
	@param r FP12 result of the pairing calculation e(P,Q)
	@param P ECP2 instance, an element of G2
	@param Q ECP instance, an element of G1

 */
extern void PAIR_ate(FP12 *r,ECP2 *P,ECP *Q);
/**	@brief Calculate Miller loop for Optimal ATE double-pairing e(P,Q).e(R,S)
 *
	Faster than calculating two separate pairings
	@param r FP12 result of the pairing calculation e(P,Q).e(R,S), an element of GT
	@param P ECP2 instance, an element of G2
	@param Q ECP instance, an element of G1
	@param R ECP2 instance, an element of G2
	@param S ECP instance, an element of G1
 */
extern void PAIR_double_ate(FP12 *r,ECP2 *P,ECP *Q,ECP2 *R,ECP *S);
/**	@brief Final exponentiation of pairing, converts output of Miller loop to element in GT
 *
	Here p is the internal modulus, and r is the group order
	@param x FP12, on exit = x^((p^12-1)/r)
 */
extern void PAIR_fexp(FP12 *x);
/**	@brief Fast point multiplication of a member of the group G1 by a BIG number
 *
	May exploit endomorphism for speed.
	@param Q ECP member of G1.
	@param b BIG multiplier

 */
extern void PAIR_G1mul(ECP *Q,BIG b);
/**	@brief Fast point multiplication of a member of the group G2 by a BIG number
 *
	May exploit endomorphism for speed.
	@param P ECP2 member of G1.
	@param b BIG multiplier

 */
extern void PAIR_G2mul(ECP2 *P,BIG b);
/**	@brief Fast raising of a member of GT to a BIG power
 *
	May exploit endomorphism for speed.
	@param x FP12 member of GT.
	@param b BIG exponent

 */
extern void PAIR_GTpow(FP12 *x,BIG b);
/**	@brief Tests FP12 for membership of GT
 *
	@param x FP12 instance
	@return 1 if x is in GT, else return 0

 */
extern int PAIR_GTmember(FP12 *x);



/* Finite Field Prototypes */
/**	@brief Copy one FF element of given length to another
 *
	@param x FF instance to be copied to, on exit = y
	@param y FF instance to be copied from
	@param n size of FF in BIGs

 */
extern void FF_copy(BIG *x,BIG *y,int n);
/**	@brief Initialize an FF element of given length from a 32-bit integer m
 *
	@param x FF instance to be copied to, on exit = m
	@param m integer
	@param n size of FF in BIGs
 */
extern void FF_init(BIG *x,sign32 m,int n);
/**	@brief Set FF element of given size to zero
 *
	@param x FF instance to be set to zero
	@param n size of FF in BIGs
 */
extern void FF_zero(BIG *x,int n);
/**	@brief Tests for FF element equal to zero
 *
	@param x FF number to be tested
	@param n size of FF in BIGs
	@return 1 if zero, else returns 0
 */
extern int FF_iszilch(BIG *x,int n);
/**	@brief  return parity of an FF, that is the least significant bit
 *
	@param x FF number
	@return 0 or 1
 */
extern int FF_parity(BIG *x);
/**	@brief  return least significant m bits of an FF
 *
	@param x FF number
	@param m number of bits to return. Assumed to be less than BASEBITS.
	@return least significant n bits as an integer
 */
extern int FF_lastbits(BIG *x,int m);
/**	@brief Set FF element of given size to unity
 *
	@param x FF instance to be set to unity
	@param n size of FF in BIGs
 */
extern void FF_one(BIG *x,int n);
/**	@brief Compares two FF numbers. Inputs must be normalised externally
 *
	@param x first FF number to be compared
	@param y second FF number to be compared
	@param n size of FF in BIGs
	@return -1 is x<y, 0 if x=y, 1 if x>y
 */
extern int FF_comp(BIG *x,BIG *y,int n);
/**	@brief addition of two FFs
 *
	@param x FF instance, on exit = y+z
	@param y FF instance
	@param z FF instance
	@param n size of FF in BIGs
 */
extern void FF_add(BIG *x,BIG *y,BIG *z,int n);
/**	@brief subtraction of two FFs
 *
	@param x FF instance, on exit = y-z
	@param y FF instance
	@param z FF instance
	@param n size of FF in BIGs
 */
extern void FF_sub(BIG *x,BIG *y,BIG *z,int n);
/**	@brief increment an FF by an integer,and normalise
 *
	@param x FF instance, on exit = x+m
	@param m an integer to be added to x
	@param n size of FF in BIGs
 */
extern void FF_inc(BIG *x,int m,int n);
/**	@brief Decrement an FF by an integer,and normalise
 *
	@param x FF instance, on exit = x-m
	@param m an integer to be subtracted from x
	@param n size of FF in BIGs
 */
extern void FF_dec(BIG *x,int m,int n);
/**	@brief Normalises the components of an FF
 *
	@param x FF instance to be normalised
	@param n size of FF in BIGs
 */
extern void FF_norm(BIG *x,int n);
/**	@brief Shift left an FF by 1 bit
 *
	@param x FF instance to be shifted left
	@param n size of FF in BIGs
 */
extern void FF_shl(BIG *x,int n);
/**	@brief Shift right an FF by 1 bit
 *
	@param x FF instance to be shifted right
	@param n size of FF in BIGs
 */
extern void FF_shr(BIG *x,int n);
/**	@brief Formats and outputs an FF to the console
 *
	@param x FF instance to be printed
	@param n size of FF in BIGs
 */
extern void FF_output(BIG *x,int n);
/**	@brief Formats and outputs an FF to the console, in raw form
 *
 	@param x FF instance to be printed
 	@param n size of FF in BIGs
 */
extern void FF_rawoutput(BIG *x,int n);
/**	@brief Formats and outputs an FF instance to an octet string
 *
	Converts an FF to big-endian base 256 form.
	@param S output octet string
	@param x FF instance to be converted to an octet string
	@param n size of FF in BIGs
 */
extern void FF_toOctet(octet *S,BIG *x,int n);
/**	@brief Populates an FF instance from an octet string
 *
	Creates FF from big-endian base 256 form.
	@param x FF instance to be created from an octet string
	@param S input octet string
	@param n size of FF in BIGs
 */
extern void FF_fromOctet(BIG *x,octet *S,int n);
/**	@brief Multiplication of two FFs
 *
	Uses Karatsuba method internally
	@param x FF instance, on exit = y*z
	@param y FF instance
	@param z FF instance
	@param n size of FF in BIGs
 */
extern void FF_mul(BIG *x,BIG *y,BIG *z,int n);
/**	@brief Reduce FF mod a modulus
 *
	This is slow
	@param x FF instance to be reduced mod m - on exit = x mod m
	@param m FF modulus
	@param n size of FF in BIGs
 */
extern void FF_mod(BIG *x,BIG *m,int n);
/**	@brief Square an FF
 *
	Uses Karatsuba method internally
	@param x FF instance, on exit = y^2
	@param y FF instance to be squared
	@param n size of FF in BIGs
 */
extern void FF_sqr(BIG *x,BIG *y,int n);
/**	@brief Reduces a double-length FF with respect to a given modulus
 *
	This is slow
	@param x FF instance, on exit = y mod z
	@param y FF instance, of double length 2*n
	@param z FF modulus
	@param n size of FF in BIGs
 */
extern void FF_dmod(BIG *x,BIG *y,BIG *z,int n);
/**	@brief Invert an FF mod a prime modulus
 *
	@param x FF instance, on exit = 1/y mod z
	@param y FF instance
	@param z FF prime modulus
	@param n size of FF in BIGs
 */
extern void FF_invmodp(BIG *x,BIG *y,BIG *z,int n);
/**	@brief Create an FF from a random number generator
 *
	@param x FF instance, on exit x is a random number of length n BIGs with most significant bit a 1
	@param R an instance of a Cryptographically Secure Random Number Generator
	@param n size of FF in BIGs
 */
extern void FF_random(BIG *x,csprng *R,int n);
/**	@brief Create a random FF less than a given modulus from a random number generator
 *
	@param x FF instance, on exit x is a random number < y
	@param y FF instance, the modulus
	@param R an instance of a Cryptographically Secure Random Number Generator
	@param n size of FF in BIGs
 */
extern void FF_randomnum(BIG *x,BIG *y,csprng *R,int n);
/**	@brief Calculate r=x^e mod m, side channel resistant
 *
	@param r FF instance, on exit = x^e mod p
	@param x FF instance
	@param e FF exponent
	@param m FF modulus
	@param n size of FF in BIGs
 */
extern void FF_skpow(BIG *r,BIG *x,BIG * e,BIG *m,int n);
/**	@brief Calculate r=x^e mod m, side channel resistant
 *
	For short BIG exponent
	@param r FF instance, on exit = x^e mod p
	@param x FF instance
	@param e BIG exponent
	@param m FF modulus
	@param n size of FF in BIGs
 */
extern void FF_skspow(BIG *r,BIG *x,BIG e,BIG *m,int n);
/**	@brief Calculate r=x^e mod m
 *
	For very short integer exponent
	@param r FF instance, on exit = x^e mod p
	@param x FF instance
	@param e integer exponent
	@param m FF modulus
	@param n size of FF in BIGs
 */
extern void FF_power(BIG *r,BIG *x,int e,BIG *m,int n);
/**	@brief Calculate r=x^e mod m
 *
	@param r FF instance, on exit = x^e mod p
	@param x FF instance
	@param e FF exponent
	@param m FF modulus
	@param n size of FF in BIGs
 */
extern void FF_pow(BIG *r,BIG *x,BIG *e,BIG *m,int n);
/**	@brief Test if an FF has factor in common with integer s
 *
	@param x FF instance to be tested
	@param s the supplied integer
	@param n size of FF in BIGs
	@return 1 if gcd(x,s)!=1, else return 0
 */
extern int FF_cfactor(BIG *x,sign32 s,int n);
/**	@brief Test if an FF is prime
 *
	Uses Miller-Rabin Method
	@param x FF instance to be tested
	@param R an instance of a Cryptographically Secure Random Number Generator
	@param n size of FF in BIGs
	@return 1 if x is (almost certainly) prime, else return 0
 */
extern int FF_prime(BIG *x,csprng *R,int n);
/**	@brief Calculate r=x^e.y^f mod m
 *
	@param r FF instance, on exit = x^e.y^f mod p
	@param x FF instance
	@param e BIG exponent
	@param y FF instance
	@param f BIG exponent
	@param m FF modulus
	@param n size of FF in BIGs
 */
extern void FF_pow2(BIG *r,BIG *x,BIG e,BIG *y,BIG f,BIG *m,int n);


/* Octet string handlers */
/**	@brief Formats and outputs an octet to the console in hex
 *
	@param O Octet to be output
 */
extern void OCT_output(octet *O);
/**	@brief Formats and outputs an octet to the console as a character string
 *
	@param O Octet to be output
 */
extern void OCT_output_string(octet *O);
/**	@brief Wipe clean an octet
 *
	@param O Octet to be cleaned
 */
extern void OCT_clear(octet *O);
/**	@brief Compare two octets
 *
	@param O first Octet to be compared
	@param P second Octet to be compared
	@return 1 if equal, else 0
 */
extern int  OCT_comp(octet *O,octet *P);
/**	@brief Compare first n bytes of two octets
 *
	@param O first Octet to be compared
	@param P second Octet to be compared
	@param n number of bytes to compare
	@return 1 if equal, else 0
 */
extern int  OCT_ncomp(octet *O,octet *P,int n);
/**	@brief Join from a C string to end of an octet
 *
	Truncates if there is no room
	@param O Octet to be written to
	@param s zero terminated string to be joined to octet
 */
extern void OCT_jstring(octet *O,char *s);
/**	@brief Join bytes to end of an octet
 *
	Truncates if there is no room
	@param O Octet to be written to
	@param s bytes to be joined to end of octet
	@param n number of bytes to join
 */
extern void OCT_jbytes(octet *O,char *s,int n);
/**	@brief Join single byte to end of an octet, repeated n times
 *
	Truncates if there is no room
	@param O Octet to be written to
	@param b byte to be joined to end of octet
	@param n number of times b is to be joined
 */
extern void OCT_jbyte(octet *O,int b,int n);
/**	@brief Join one octet to the end of another
 *
	Truncates if there is no room
	@param O Octet to be written to
	@param P Octet to be joined to the end of O
 */
extern void OCT_joctet(octet *O,octet *P);
/**	@brief XOR common bytes of a pair of Octets
 *
	@param O Octet - on exit = O xor P
	@param P Octet to be xored into O
 */
extern void OCT_xor(octet *O,octet *P);
/**	@brief reset Octet to zero length
 *
	@param O Octet to be emptied
 */
extern void OCT_empty(octet *O);
/**	@brief Pad out an Octet to the given length
 *
	Padding is done by inserting leading zeros, so abcd becomes 00abcd
	@param O Octet to be padded
	@param n new length of Octet
 */
extern int OCT_pad(octet *O,int n);
/**	@brief Convert an Octet to printable base64 number
 *
	@param b zero terminated byte array to take base64 conversion
	@param O Octet to be converted
 */
extern void OCT_tobase64(char *b,octet *O);
/**	@brief Populate an Octet from base64 number
 *
 	@param O Octet to be populated
	@param b zero terminated base64 string

 */
extern void OCT_frombase64(octet *O,char *b);
/**	@brief Copy one Octet into another
 *
 	@param O Octet to be copied to
	@param P Octet to be copied from

 */
extern void OCT_copy(octet *O,octet *P);
/**	@brief XOR every byte of an octet with input m
 *
 	@param O Octet
	@param m byte to be XORed with every byte of O

 */
extern void OCT_xorbyte(octet *O,int m);
/**	@brief Chops Octet into two, leaving first n bytes in O, moving the rest to P
 *
 	@param O Octet to be chopped
	@param P new Octet to be created
	@param n number of bytes to chop off O

 */
extern void OCT_chop(octet *O,octet *P,int n);
/**	@brief Join n bytes of integer m to end of Octet O (big endian)
 *
	Typically n is 4 for a 32-bit integer
 	@param O Octet to be appended to
	@param m integer to be appended to O
	@param n number of bytes in m

 */
extern void OCT_jint(octet *O,int m,int n);
/**	@brief Create an Octet from bytes taken from a random number generator
 *
	Truncates if there is no room
 	@param O Octet to be populated
	@param R an instance of a Cryptographically Secure Random Number Generator
	@param n number of bytes to extracted from R

 */
extern void OCT_rand(octet *O,csprng *R,int n);
/**	@brief Shifts Octet left by n bytes
 *
	Leftmost bytes disappear
 	@param O Octet to be shifted
	@param n number of bytes to shift

 */
extern void OCT_shl(octet *O,int n);
/**	@brief Convert a hex number to an Octet
 *
	@param dst Octet
	@param src Hex string to be converted
 */
extern void OCT_fromHex(octet *dst,char *src);
/**	@brief Convert an Octet to printable hex number
 *
	@param dst hex value
	@param src Octet to be converted
 */
extern void OCT_toHex(octet *src,char *dst);
/**	@brief Convert an Octet to string
 *
	@param dst string value
	@param src Octet to be converted
 */
extern void OCT_toStr(octet *src,char *dst);



/* Hash function */
/**	@brief Initialise an instance of SHA256
 *
	@param H an instance SHA256
 */
extern void HASH256_init(hash256 *H);
/**	@brief Add a byte to the hash
 *
	@param H an instance SHA256
	@param b byte to be included in hash
 */
extern void HASH256_process(hash256 *H,int b);
/**	@brief Generate 32-byte hash
 *
	@param H an instance SHA256
	@param h is the output 32-byte hash
 */
extern void HASH256_hash(hash256 *H,char *h);


/**	@brief Initialise an instance of SHA384
 *
	@param H an instance SHA384
 */
extern void HASH384_init(hash384 *H);
/**	@brief Add a byte to the hash
 *
	@param H an instance SHA384
	@param b byte to be included in hash
 */
extern void HASH384_process(hash384 *H,int b);
/**	@brief Generate 48-byte hash
 *
	@param H an instance SHA384
	@param h is the output 48-byte hash
 */
extern void HASH384_hash(hash384 *H,char *h);


/**	@brief Initialise an instance of SHA512
 *
	@param H an instance SHA512
 */
extern void HASH512_init(hash512 *H);
/**	@brief Add a byte to the hash
 *
	@param H an instance SHA512
	@param b byte to be included in hash
 */
extern void HASH512_process(hash512 *H,int b);
/**	@brief Generate 64-byte hash
 *
	@param H an instance SHA512
	@param h is the output 64-byte hash
 */
extern void HASH512_hash(hash512 *H,char *h);


/* AES functions */
/**	@brief Reset AES mode or IV
 *
	@param A an instance of the AMCL_AES
	@param m is the new active mode of operation (ECB, CBC, OFB, CFB etc)
	@param iv the new Initialisation Vector
 */
extern void AES_reset(amcl_aes *A,int m,char *iv);
/**	@brief Extract chaining vector from AMCL_AES instance
 *
	@param A an instance of the AMCL_AES
	@param f the extracted chaining vector
 */
extern void AES_getreg(amcl_aes *A,char * f);
/**	@brief Initialise an instance of AMCL_AES and its mode of operation
 *
	@param A an instance AMCL_AES
	@param m is the active mode of operation (ECB, CBC, OFB, CFB etc)
	@param n is the key length in bytes, 16, 24 or 32
	@param k the AES key as an array of 16 bytes
	@param iv the Initialisation Vector
	@return 0 for invalid n
 */
extern int AES_init(amcl_aes *A,int m,int n,char *k,char *iv);
/**	@brief Encrypt a single 16 byte block in ECB mode
 *
	@param A an instance of the AMCL_AES
	@param b is an array of 16 plaintext bytes, on exit becomes ciphertext
 */
extern void AES_ecb_encrypt(amcl_aes *A,uchar * b);
/**	@brief Decrypt a single 16 byte block in ECB mode
 *
	@param A an instance of the AMCL_AES
	@param b is an array of 16 cipherext bytes, on exit becomes plaintext
 */
extern void AES_ecb_decrypt(amcl_aes *A,uchar * b);
/**	@brief Encrypt a single 16 byte block in active mode
 *
	@param A an instance of the AMCL_AES
	@param b is an array of 16 plaintext bytes, on exit becomes ciphertext
	@return 0, or overflow bytes from CFB mode
 */
extern unsign32 AES_encrypt(amcl_aes *A,char *b );
/**	@brief Decrypt a single 16 byte block in active mode
 *
	@param A an instance of the AMCL_AES
	@param b is an array of 16 ciphertext bytes, on exit becomes plaintext
	@return 0, or overflow bytes from CFB mode
 */
extern unsign32 AES_decrypt(amcl_aes *A,char *b);
/**	@brief Clean up after application of AES
 *
	@param A an instance of the AMCL_AES
 */
extern void AES_end(amcl_aes *A);


/* AES-GCM functions */
/**	@brief Initialise an instance of AES-GCM mode
 *
	@param G an instance AES-GCM
	@param nk is the key length in bytes, 16, 24 or 32
	@param k the AES key as an array of 16 bytes
	@param n the number of bytes in the Initialisation Vector (IV)
	@param iv the IV
 */
extern void GCM_init(gcm *G,int nk,char *k,int n,char *iv);
/**	@brief Add header (material to be authenticated but not encrypted)
 *
	Note that this function can be called any number of times with n a multiple of 16, and then one last time with any value for n
	@param G an instance AES-GCM
	@param b is the header material to be added
	@param n the number of bytes in the header
 */
extern int GCM_add_header(gcm *G,char *b,int n);
/**	@brief Add plaintext and extract ciphertext
 *
	Note that this function can be called any number of times with n a multiple of 16, and then one last time with any value for n
	@param G an instance AES-GCM
	@param c is the ciphertext generated
	@param p is the plaintext material to be added
	@param n the number of bytes in the plaintext
 */
extern int GCM_add_plain(gcm *G,char *c,char *p,int n);
/**	@brief Add ciphertext and extract plaintext
 *
	Note that this function can be called any number of times with n a multiple of 16, and then one last time with any value for n
	@param G an instance AES-GCM
	@param p is the plaintext generated
	@param c is the ciphertext material to be added
	@param n the number of bytes in the ciphertext
 */
extern int GCM_add_cipher(gcm *G,char *p,char *c,int n);
/**	@brief Finish off and extract authentication tag (HMAC)
 *
	@param G is an active instance AES-GCM
	@param t is the output 16 byte authentication tag
 */
extern void GCM_finish(gcm *G,char *t);



/* random numbers */
/**	@brief Seed a random number generator from an array of bytes
 *
	The provided seed should be truly random
	@param R an instance of a Cryptographically Secure Random Number Generator
	@param n the number of seed bytes provided
	@param b an array of seed bytes

 */
extern void RAND_seed(csprng *R,int n,char *b);
/**	@brief Delete all internal state of a random number generator
 *
	@param R an instance of a Cryptographically Secure Random Number Generator
 */
extern void RAND_clean(csprng *R);
/**	@brief Return a random byte from a random number generator
 *
	@param R an instance of a Cryptographically Secure Random Number Generator
	@return a random byte
 */
extern int RAND_byte(csprng *R);

/**
 * @brief Convert to DBIG number from byte array of given length
 *
 * Convert to DBIG number from byte array of given length.
 *
 * @param x DBIG number
 * @param a byte array
 * @param s byte array length
 */
extern void BIG_dfromBytesLen(DBIG x,char *a,int s);

#endif
