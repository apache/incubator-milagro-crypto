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

pub type Chunk=i64;
//pub type DChunk=i128;
pub const CHUNK:usize=64;

pub const NOT_SPECIAL:usize =0;
pub const PSEUDO_MERSENNE:usize=1;
pub const MONTGOMERY_FRIENDLY:usize=2;
pub const GENERALISED_MERSENNE:usize=3;
pub const WEIERSTRASS:usize=0;
pub const EDWARDS:usize=1;
pub const MONTGOMERY:usize=2;
pub const BN_CURVE: usize=0;
pub const BLS_CURVE: usize=1;


// Curve 25519
#[cfg(feature = "Ed25519")]
pub const MODBITS: usize = 255;
#[cfg(feature = "Ed25519")]
pub const MOD8: usize = 5;
#[cfg(feature = "Ed25519")]
pub const BASEBITS: usize = 56;
#[cfg(feature = "Ed25519")]
pub const AES_S: usize=0;

// GOLDILOCKS
#[cfg(feature = "GOLDILOCKS")]
pub const MODBITS: usize=448;
#[cfg(feature = "GOLDILOCKS")]
pub const MOD8: usize=7;
#[cfg(feature = "GOLDILOCKS")]
pub const BASEBITS: usize=60;
#[cfg(feature = "GOLDILOCKS")]
pub const AES_S: usize= 0;


// BN254 Curve
#[cfg(feature = "BN254")]
pub const MODBITS:usize = 254; /* Number of bits in Modulus */
#[cfg(feature = "BN254")]
pub const MOD8:usize = 3;   /* Modulus mod 8 */
#[cfg(feature = "BN254")]
pub const BASEBITS:usize = 56;
#[cfg(feature = "BN254")]
pub const AES_S:usize=0;

// BLS383 Curve
#[cfg(feature = "BLS383")]
pub const MODBITS:usize = 383; /* Number of bits in Modulus */
#[cfg(feature = "BLS383")]
pub const MOD8: usize = 3;  /* Modulus mod 8 */
#[cfg(feature = "BLS383")]
pub const BASEBITS:usize = 56;
#[cfg(feature = "BLS383")]
pub const AES_S: usize= 0;

// BLS455 Curve
#[cfg(feature = "BLS455")]
pub const MODBITS:usize = 455; /* Number of bits in Modulus */
#[cfg(feature = "BLS455")]
pub const MOD8: usize = 3;  /* Modulus mod 8 */
#[cfg(feature = "BLS455")]
pub const BASEBITS:usize = 60;
#[cfg(feature = "BLS455")]
pub const AES_S: usize= 128;

//---------------

/* RSA/DH modulus length as multiple of BIGBITS */
pub const FFLEN:usize=8;

pub const NLEN: usize = (1+((MODBITS-1)/BASEBITS));
pub const BIG_HEX_STRING_LEN:usize = NLEN * 16 + NLEN - 1;
pub const DNLEN: usize = 2*NLEN;
pub const BMASK: Chunk= ((1<<BASEBITS)-1);
pub const MODBYTES: usize = 1+(MODBITS-1)/8;
pub const NEXCESS:isize = (1<<((CHUNK)-BASEBITS-1));
pub const FEXCESS:Chunk = ((1 as Chunk)<<(BASEBITS*(NLEN)-MODBITS));
pub const OMASK:Chunk = (-1)<<(MODBITS%BASEBITS);
pub const TBITS:usize=MODBITS%BASEBITS; // Number of active bits in top word
pub const TMASK:Chunk=(1<<TBITS)-1;
pub const BIGBITS:usize = (MODBYTES*8);
pub const HBITS: usize=(BASEBITS/2);
pub const HMASK: Chunk= ((1<<HBITS)-1);

/* Finite field support - for RSA, DH etc. */
pub const FF_BITS:usize=(BIGBITS*FFLEN); /* Finite Field Size in bits - must be 256.2^n */
pub const HFLEN:usize=(FFLEN/2);  /* Useful for half-size RSA private key operations */

pub const P_MBITS:usize=(MODBYTES as usize)*8;
pub const P_MB: usize=(P_MBITS%BASEBITS);
pub const P_OMASK:Chunk=((-1)<<(P_MBITS%BASEBITS));
pub const P_FEXCESS: Chunk=(1<<(BASEBITS*NLEN-P_MBITS));
pub const P_TBITS: usize=(P_MBITS%BASEBITS);


// Curve25519 Modulus 
#[cfg(feature = "Ed25519")]
pub const MODTYPE:usize=PSEUDO_MERSENNE;
#[cfg(feature = "Ed25519")]
pub const MODULUS:[Chunk;NLEN]=[0xFFFFFFFFFFFFED,0xFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFF,0x7FFFFFFF];
#[cfg(feature = "Ed25519")]
pub const MCONST:Chunk=19;

//GOLDILOCKS
#[cfg(feature = "GOLDILOCKS")]
pub const MODTYPE: usize=GENERALISED_MERSENNE;
#[cfg(feature = "GOLDILOCKS")]
pub const MODULUS: [Chunk;NLEN]= [0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFEFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFF];
#[cfg(feature = "GOLDILOCKS")]
pub const MCONST: Chunk=0x1;

// BN254 Curve Modulus
#[cfg(feature = "BN254")]
pub const MODTYPE:usize = NOT_SPECIAL;
#[cfg(feature = "BN254")]
pub const MODULUS:[Chunk;NLEN] = [0x13,0x13A7,0x80000000086121,0x40000001BA344D,0x25236482];
#[cfg(feature = "BN254")]
pub const MCONST:Chunk=0x435E50D79435E5;

// BLS383 Curve
#[cfg(feature = "BLS383")]
pub const MODTYPE:usize = NOT_SPECIAL;
#[cfg(feature = "BLS383")]
pub const MODULUS:[Chunk;NLEN] = [0xACAAB52AAD556B,0x1BB01475F75D7A,0xCF73083D5D7520,0x531820F99EB16,0x2C01355A68EA32,0x5C6105C552A785,0x7AC52080A9F7];
#[cfg(feature = "BLS383")]
pub const MCONST:Chunk=0xA59AB3B123D0BD;


// BLS455 Curve
#[cfg(feature = "BLS455")]
pub const MODTYPE:usize = NOT_SPECIAL;
#[cfg(feature = "BLS455")]
pub const MODULUS:[Chunk;NLEN] = [0xAA00001800002AB,0xC589556B2AA956A,0xB9994ACE86D1BA6,0x3954FCB314B8B3D,0xE3A5B1D56234BD9,0x95B49203003F665,0x57955572AA00E0F,0x555559555];
#[cfg(feature = "BLS455")]
pub const MCONST:Chunk=0xB3EF8137F4017FD;


// Ed25519 Curve 
#[cfg(feature = "Ed25519")]
pub const CURVETYPE:usize=EDWARDS;
#[cfg(feature = "Ed25519")]
pub const CURVE_A:isize = -1;
#[cfg(feature = "Ed25519")]
pub const CURVE_B:[Chunk;NLEN]=[0xEB4DCA135978A3,0xA4D4141D8AB75,0x797779E8980070,0x2B6FFE738CC740,0x52036CEE];
#[cfg(feature = "Ed25519")]
pub const CURVE_ORDER:[Chunk;NLEN]=[0x12631A5CF5D3ED,0xF9DEA2F79CD658,0x14DE,0x0,0x10000000];
#[cfg(feature = "Ed25519")]
pub const CURVE_GX:[Chunk;NLEN]=[0x562D608F25D51A,0xC7609525A7B2C9,0x31FDD6DC5C692C,0xCD6E53FEC0A4E2,0x216936D3];
#[cfg(feature = "Ed25519")]
pub const CURVE_GY:[Chunk;NLEN]=[0x66666666666658,0x66666666666666,0x66666666666666,0x66666666666666,0x66666666];

// GOLDILOCKS
#[cfg(feature = "GOLDILOCKS")]
pub const CURVETYPE: usize= EDWARDS;
#[cfg(feature = "GOLDILOCKS")]
pub const CURVE_A: isize = 1;
#[cfg(feature = "GOLDILOCKS")]
pub const CURVE_B: [Chunk;NLEN]=[0xFFFFFFFFFFF6756,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFEFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFF];
#[cfg(feature = "GOLDILOCKS")]
pub const CURVE_ORDER: [Chunk;NLEN]=[0x378C292AB5844F3,0x6CC2728DC58F552,0xEDB49AED6369021,0xFFFF7CCA23E9C44,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF,0x3FFFFFF];
#[cfg(feature = "GOLDILOCKS")]
pub const CURVE_GX: [Chunk;NLEN]=[0x555555555555555,0x555555555555555,0x555555555555555,0xAAA955555555555,0xAAAAAAAAAAAAAAA,0xAAAAAAAAAAAAAAA,0xAAAAAAAAAAAAAAA,0xAAAAAAA];
#[cfg(feature = "GOLDILOCKS")]
pub const CURVE_GY: [Chunk;NLEN]=[0xAEAFBCDEA9386ED,0xBCB2BED1CDA06BD,0x565833A2A3098BB,0x6D728AD8C4B80D6,0x7A035884DD7B7E3,0x205086C2B0036ED,0x34AD7048DB359D6,0xAE05E96];


// BN254 Curve
#[cfg(feature = "BN254")]
pub const CURVETYPE:usize = WEIERSTRASS;
#[cfg(feature = "BN254")]
pub const CURVE_PAIRING_TYPE:usize = BN_CURVE;
#[cfg(feature = "BN254")]
pub const CURVE_A:isize = 0;
#[cfg(feature = "BN254")]
pub const CURVE_B:[Chunk;NLEN]=[0x2,0x0,0x0,0x0,0x0];
#[cfg(feature = "BN254")]
pub const CURVE_ORDER:[Chunk;NLEN]=[0xD,0x800000000010A1,0x8000000007FF9F,0x40000001BA344D,0x25236482];
#[cfg(feature = "BN254")]
pub const CURVE_GX:[Chunk;NLEN]=[0x12,0x13A7,0x80000000086121,0x40000001BA344D,0x25236482];
#[cfg(feature = "BN254")]
pub const CURVE_GY:[Chunk;NLEN]=[0x1,0x0,0x0,0x0,0x0];

#[cfg(feature = "BN254")]
pub const CURVE_FRA:[Chunk;NLEN]=[0x7DE6C06F2A6DE9,0x74924D3F77C2E1,0x50A846953F8509,0x212E7C8CB6499B,0x1B377619];
#[cfg(feature = "BN254")]
pub const CURVE_FRB:[Chunk;NLEN]=[0x82193F90D5922A,0x8B6DB2C08850C5,0x2F57B96AC8DC17,0x1ED1837503EAB2,0x9EBEE69];
#[cfg(feature = "BN254")]
pub const CURVE_PXA:[Chunk;NLEN]=[0xEE4224C803FB2B,0x8BBB4898BF0D91,0x7E8C61EDB6A464,0x519EB62FEB8D8C,0x61A10BB];
#[cfg(feature = "BN254")]
pub const CURVE_PXB:[Chunk;NLEN]=[0x8C34C1E7D54CF3,0x746BAE3784B70D,0x8C5982AA5B1F4D,0xBA737833310AA7,0x516AAF9];
#[cfg(feature = "BN254")]
pub const CURVE_PYA:[Chunk;NLEN]=[0xF0E07891CD2B9A,0xAE6BDBE09BD19,0x96698C822329BD,0x6BAF93439A90E0,0x21897A0];
#[cfg(feature = "BN254")]
pub const CURVE_PYB:[Chunk;NLEN]=[0x2D1AEC6B3ACE9B,0x6FFD739C9578A,0x56F5F38D37B090,0x7C8B15268F6D44,0xEBB2B0E];
#[cfg(feature = "BN254")]
pub const CURVE_BNX:[Chunk;NLEN]=[0x80000000000001,0x40,0x0,0x0,0x0];
#[cfg(feature = "BN254")]
pub const CURVE_COF:[Chunk;NLEN]=[0x1,0x0,0x0,0x0,0x0];
#[cfg(feature = "BN254")]
pub const CURVE_CRU:[Chunk;NLEN]=[0x80000000000007,0x6CD,0x40000000024909,0x49B362,0x0];
#[cfg(feature = "BN254")]
pub const CURVE_W:[[Chunk;NLEN];2]=[[0x3,0x80000000000204,0x6181,0x0,0x0],[0x1,0x81,0x0,0x0,0x0]];
#[cfg(feature = "BN254")]
pub const CURVE_SB:[[[Chunk;NLEN];2];2]=[[[0x4,0x80000000000285,0x6181,0x0,0x0],[0x1,0x81,0x0,0x0,0x0]],[[0x1,0x81,0x0,0x0,0x0],[0xA,0xE9D,0x80000000079E1E,0x40000001BA344D,0x25236482]]];
#[cfg(feature = "BN254")]
pub const CURVE_WB:[[Chunk;NLEN];4]=[[0x80000000000000,0x80000000000040,0x2080,0x0,0x0],[0x80000000000005,0x54A,0x8000000001C707,0x312241,0x0],[0x80000000000003,0x800000000002C5,0xC000000000E383,0x189120,0x0],[0x80000000000001,0x800000000000C1,0x2080,0x0,0x0]];
#[cfg(feature = "BN254")]
pub const CURVE_BB:[[[Chunk;NLEN];4];4]=[[[0x8000000000000D,0x80000000001060,0x8000000007FF9F,0x40000001BA344D,0x25236482],[0x8000000000000C,0x80000000001060,0x8000000007FF9F,0x40000001BA344D,0x25236482],[0x8000000000000C,0x80000000001060,0x8000000007FF9F,0x40000001BA344D,0x25236482],[0x2,0x81,0x0,0x0,0x0]],[[0x1,0x81,0x0,0x0,0x0],[0x8000000000000C,0x80000000001060,0x8000000007FF9F,0x40000001BA344D,0x25236482],[0x8000000000000D,0x80000000001060,0x8000000007FF9F,0x40000001BA344D,0x25236482],[0x8000000000000C,0x80000000001060,0x8000000007FF9F,0x40000001BA344D,0x25236482]],[[0x2,0x81,0x0,0x0,0x0],[0x1,0x81,0x0,0x0,0x0],[0x1,0x81,0x0,0x0,0x0],[0x1,0x81,0x0,0x0,0x0]],[[0x80000000000002,0x40,0x0,0x0,0x0],[0x2,0x102,0x0,0x0,0x0],[0xA,0x80000000001020,0x8000000007FF9F,0x40000001BA344D,0x25236482],[0x80000000000002,0x40,0x0,0x0,0x0]]];

#[cfg(feature = "BN254")]
pub const USE_GLV:bool = true;
#[cfg(feature = "BN254")]
pub const USE_GS_G2:bool = true;
#[cfg(feature = "BN254")]
pub const USE_GS_GT:bool = true;
#[cfg(feature = "BN254")]
pub const GT_STRONG:bool = false;

// BLS383 Curve
#[cfg(feature = "BLS383")]
pub const CURVETYPE:usize = WEIERSTRASS;
#[cfg(feature = "BLS383")]
pub const CURVE_PAIRING_TYPE:usize = BLS_CURVE;
#[cfg(feature = "BLS383")]
pub const CURVE_A:isize = 0;

#[cfg(feature = "BLS383")]
pub const CURVE_ORDER:[Chunk;NLEN]=[0xFFF80000FFF001,0xBFDE0070FE7800,0x3000049C5EDF1C,0xC40007F910007A,0x14641004C,0x0,0x0];
#[cfg(feature = "BLS383")]
pub const CURVE_B:[Chunk;NLEN]=[0x9,0x0,0x0,0x0,0x0,0x0,0x0];
#[cfg(feature = "BLS383")]
pub const CURVE_COF:[Chunk;NLEN]=[0x2A00000052B,0x5560AAAAAB2CA0,0x6055,0x0,0x0,0x0,0x0];
#[cfg(feature = "BLS383")]
pub const CURVE_GX:[Chunk;NLEN]=[0xD59B348D10786B,0x3477C0E3F54AD0,0xBF25B734578B9B,0x4F6AC007BB6F65,0xEFD5830FF57E9C,0xADB9F88FB6EC02,0xB08CEE4BC98];
#[cfg(feature = "BLS383")]
pub const CURVE_GY:[Chunk;NLEN]=[0x5DA023D145DDB,0x13F518C5FEF7CC,0x56EC3462B2A66F,0x96F3019C7A925F,0x9061047981223E,0x4810AD8F5BE59,0x1F3909337671];

#[cfg(feature = "BLS383")]
pub const CURVE_BNX:[Chunk;NLEN]=[0x1000000040,0x110,0x0,0x0,0x0,0x0,0x0];
#[cfg(feature = "BLS383")]
pub const CURVE_CRU:[Chunk;NLEN]=[0xA3AAC4EDA155A9,0xDF2FE8761E5E3D,0xBCDFAADE632625,0x5123128D3035A6,0xDBF3A2BBEAD683,0x5C5FAB20424190,0x7AC52080A9F7];
#[cfg(feature = "BLS383")]
pub const CURVE_FRA:[Chunk;NLEN]=[0x2BA59A92B4508B,0x63DB7A06EEF343,0x40341CB1DFBC74,0x1639E9D32D55D3,0xB19B3F05CC36D4,0xF323EE4D86AB98,0x5A5FB198672];
#[cfg(feature = "BLS383")]
pub const CURVE_FRB:[Chunk;NLEN]=[0x81051A97F904E0,0xB7D49A6F086A37,0x8F3EEB8B7DB8AB,0xEEF7983C6C9543,0x7A65F6549CB35D,0x693D1777CBFBEC,0x751F25672384];
#[cfg(feature = "BLS383")]
pub const CURVE_PXA:[Chunk;NLEN]=[0x6059885BAC9472,0x7C4D31DE2DC36D,0xBDC90C308C88A7,0x29F01971C688FC,0x3693539C43F167,0xD81E5A561EB8BF,0x4D50722B56BF];
#[cfg(feature = "BLS383")]
pub const CURVE_PXB:[Chunk;NLEN]=[0x9B4BD7A272AB23,0x7AF19D4F44DCE8,0x3F6F7B93206A34,0x571DD3E2A819FB,0x3A2BA3B635D7EE,0xAC28C780C1A126,0xEE3617C3E5B];
#[cfg(feature = "BLS383")]
pub const CURVE_PYA:[Chunk;NLEN]=[0x81D230977BD4FD,0xB660720DFDFC6,0x41FC9590C89A0C,0x2E1FBCF878287A,0x11C23014EEE65,0x28878816BB325E,0x8F40859A05C];
#[cfg(feature = "BLS383")]
pub const CURVE_PYB:[Chunk;NLEN]=[0xA5E20A252C4CE6,0x5907A74AFF40C8,0x41760A42448EF3,0xFFEF82B0FDA199,0xA0F29A18D4EA49,0xAC7F7B86E4997B,0x1DCABBA88C12];

#[cfg(feature = "BLS383")]
pub const CURVE_W:[[Chunk;0];2]=[[],[]];
#[cfg(feature = "BLS383")]
pub const CURVE_SB:[[[Chunk;0];2];2]=[[[],[]],[[],[]]];
#[cfg(feature = "BLS383")]
pub const CURVE_WB:[[Chunk;0];4]=[[],[],[],[]];
#[cfg(feature = "BLS383")]
pub const CURVE_BB:[[[Chunk;0];4];4]=[[[],[],[],[]],[[],[],[],[]],[[],[],[],[]],[[],[],[],[]]];


#[cfg(feature = "BLS383")]
pub const USE_GLV:bool = true;
#[cfg(feature = "BLS383")]
pub const USE_GS_G2:bool = true;
#[cfg(feature = "BLS383")]
pub const USE_GS_GT:bool = true;
#[cfg(feature = "BLS383")]
pub const GT_STRONG:bool = false;


// BLS455 Curve
#[cfg(feature = "BLS455")]
pub const CURVETYPE:usize = WEIERSTRASS;
#[cfg(feature = "BLS455")]
pub const CURVE_PAIRING_TYPE:usize = BLS_CURVE;
#[cfg(feature = "BLS455")]
pub const CURVE_A:isize = 0;

#[cfg(feature = "BLS455")]
pub const CURVE_ORDER:[Chunk;NLEN]=[0x7FFFFC00001,0xA00000400001C,0x25E000750001D10,0xE0000F10004F000,0x80000380002,0x10,0x0,0x0];
#[cfg(feature = "BLS455")]
pub const CURVE_B:[Chunk;NLEN]=[0xA,0x0,0x0,0x0,0x0,0x0,0x0,0x0];
#[cfg(feature = "BLS455")]
pub const CURVE_COF:[Chunk;NLEN]=[0xA9557FFAABFFAAB,0xAAB15555B54AAB6,0x555556AA,0x0,0x0,0x0,0x0,0x0];
#[cfg(feature = "BLS455")]
pub const CURVE_GX:[Chunk;NLEN]=[0x6D4C5DDFDFCEDD1,0x35C6F43B3A034FB,0x7F05B56A579C725,0xB1F2B8ECE11B321,0x9F342AB0CFE8392,0xA5911EE32767994,0x3005E40CC56ABED,0x18855F3B];
#[cfg(feature = "BLS455")]
pub const CURVE_GY:[Chunk;NLEN]=[0x404FD79A6619B9B,0x69D80A5D6FA0286,0xEE722322D91A493,0xB1EE58431C1E968,0xCA9BC8953801F5F,0xDFAFD40FE9E388E,0x9F8985FC3DEB0D6,0x19A8DB77E];

#[cfg(feature = "BLS455")]
pub const CURVE_BNX:[Chunk;NLEN]=[0x20000080000800,0x10000,0x0,0x0,0x0,0x0,0x0,0x0];
#[cfg(feature = "BLS455")]
pub const CURVE_CRU:[Chunk;NLEN]=[0x9202FFC00000AA9,0xFA5190F4A3762A,0x8B2B9BDD548FEC9,0xD7B469DB33A586A,0xC91731354CAFD99,0xF5B48D02FFFE695,0x57955572A900E0E,0x555559555];
#[cfg(feature = "BLS455")]
pub const CURVE_FRA:[Chunk;NLEN]=[0x9CCFBDCA2EBF21,0x572F54A73379964,0x72819F887545498,0x22BBC1CAD1F8534,0xA82CD7D435944F0,0x4594F818D030F7B,0xEDCBE3ADC0016A7,0x397EA4973];
#[cfg(feature = "BLS455")]
pub const CURVE_FRB:[Chunk;NLEN]=[0xA033043B5D1438A,0x6E5A00C3F72FC06,0x4717AB46118C70E,0x16993AE842C0609,0x3B78DA012CA06E9,0x501F99EA300E6EA,0x69C971C4E9FF768,0x1BD6B4BE1];
#[cfg(feature = "BLS455")]
pub const CURVE_PXA:[Chunk;NLEN]=[0x475F20F0C1F542,0x65D6070F8567E10,0xD780698BB33D776,0x71F685ED1531721,0x303D3FEC5B6A49C,0x8DEF064FF553CEB,0xC0E9A31B4C463,0x2ECB12FA8];
#[cfg(feature = "BLS455")]
pub const CURVE_PXB:[Chunk;NLEN]=[0x99086EE6749F03D,0xE89A55A5AC5EF2E,0x7B41AECD88EA016,0x622450FE6163E06,0x755066E1C8E296F,0xA80F219487326E8,0x66DBFBB0BEAEE59,0xECFFCE0];
#[cfg(feature = "BLS455")]
pub const CURVE_PYA:[Chunk;NLEN]=[0x83235A4581A77F4,0x9F0F367B7A7E10A,0x8FA0C4A66D55B9D,0xEF03F65E0D6EC4C,0x9C7DC299C1A9EC2,0x32453CA21CFA5AC,0x6C3DCD5ABB9C544,0x22471D90A];
#[cfg(feature = "BLS455")]
pub const CURVE_PYB:[Chunk;NLEN]=[0xF413B6D9E1FDBA2,0xA7E630913DA0356,0xFBC913D9AC488E2,0x72E7CF61B401585,0x656D801B21C89ED,0xF9E921EEE0558F9,0x3D2B7B03CFC8698,0x33503CA8];

#[cfg(feature = "BLS455")]
pub const CURVE_W:[[Chunk;0];2]=[[],[]];
#[cfg(feature = "BLS455")]
pub const CURVE_SB:[[[Chunk;0];2];2]=[[[],[]],[[],[]]];
#[cfg(feature = "BLS455")]
pub const CURVE_WB:[[Chunk;0];4]=[[],[],[],[]];
#[cfg(feature = "BLS455")]
pub const CURVE_BB:[[[Chunk;0];4];4]=[[[],[],[],[]],[[],[],[],[]],[[],[],[],[]],[[],[],[],[]]];


#[cfg(feature = "BLS455")]
pub const USE_GLV:bool = true;
#[cfg(feature = "BLS455")]
pub const USE_GS_G2:bool = true;
#[cfg(feature = "BLS455")]
pub const USE_GS_GT:bool = true;
#[cfg(feature = "BLS455")]
pub const GT_STRONG:bool = false;