pub mod aes;
pub mod gcm;
pub mod hash256;
pub mod hash384;
pub mod hash512;
pub mod rand;
pub mod sha3;
pub mod nhs;
pub mod types;
#[cfg(target_pointer_width = "32")]
#[path = "arch/arch32.rs"]
pub mod arch;
#[cfg(target_pointer_width = "64")]
#[path = "arch/arch64.rs"]
pub mod arch;

#[cfg(feature = "bls48")]
#[path = "./"]
pub mod bls48 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_bls48_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_bls48_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecp8;
    pub mod fp2;
    pub mod fp4;
    pub mod fp8;
    pub mod fp16;
    pub mod fp48;
    pub mod pair256;
    pub mod mpin256;
    pub mod bls256;
}

#[cfg(feature = "bls461")]
#[path = "./"]
pub mod bls461 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_bls461_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_bls461_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecp2;
    pub mod fp2;
    pub mod fp4;
    pub mod fp12;
    pub mod pair;
    pub mod mpin;
    pub mod bls;
}

#[cfg(feature = "bls383")]
#[path = "./"]
pub mod bls383 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_bls383_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_bls383_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecp2;
    pub mod fp2;
    pub mod fp4;
    pub mod fp12;
    pub mod pair;
    pub mod mpin;
    pub mod bls;
}

#[cfg(feature = "bls381")]
#[path = "./"]
pub mod bls381 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_bls381_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_bls381_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecp2;
    pub mod fp2;
    pub mod fp4;
    pub mod fp12;
    pub mod pair;
    pub mod mpin;
    pub mod bls;
}

#[cfg(feature = "fp512bn")]
#[path = "./"]
pub mod fp512bn {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_fp512bn_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_fp512bn_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecp2;
    pub mod fp2;
    pub mod fp4;
    pub mod fp12;
    pub mod pair;
    pub mod mpin;
    pub mod bls;
}

#[cfg(feature = "fp256bn")]
#[path = "./"]
pub mod fp256bn {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_fp256bn_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_fp256bn_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecp2;
    pub mod fp2;
    pub mod fp4;
    pub mod fp12;
    pub mod pair;
    pub mod mpin;
    pub mod bls;
}

#[cfg(feature = "bls24")]
#[path = "./"]
pub mod bls24 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_bls24_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_bls24_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod fp2;
    pub mod fp4;
    pub mod fp8;
    pub mod fp24;
    pub mod ecp;
    pub mod ecp4;
    pub mod pair192;
    pub mod mpin192;
    pub mod bls192;
}

#[cfg(feature = "anssi")]
#[path = "./"]
pub mod anssi {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_anssi_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_anssi_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "brainpool")]
#[path = "./"]
pub mod brainpool {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_brainpool_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_brainpool_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "goldilocks")]
#[path = "./"]
pub mod goldilocks {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_goldilocks_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_goldilocks_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "hifive")]
#[path = "./"]
pub mod hifive {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_hifive_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_hifive_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nist256")]
#[path = "./"]
pub mod nist256 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nist256_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nist256_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nist384")]
#[path = "./"]
pub mod nist384 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nist384_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nist384_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nist521")]
#[path = "./"]
pub mod nist521 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nist521_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nist521_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nums256e")]
#[path = "./"]
pub mod nums256e {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nums256e_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nums256e_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nums256w")]
#[path = "./"]
pub mod nums256w {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nums256w_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nums256w_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nums384e")]
#[path = "./"]
pub mod nums384e {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nums384e_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nums384e_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nums384w")]
#[path = "./"]
pub mod nums384w {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nums384w_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nums384w_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nums512w")]
#[path = "./"]
pub mod nums512w {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nums512w_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nums512w_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "nums512e")]
#[path = "./"]
pub mod nums512e {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_nums512e_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_nums512e_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "secp256k1")]
#[path = "./"]
pub mod secp256k1 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_secp256k1_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_secp256k1_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "c25519")]
#[path = "./"]
pub mod c25519 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_c25519_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_c25519_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "c41417")]
#[path = "./"]
pub mod c41417 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_c41417_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_c41417_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "ed25519")]
#[path = "./"]
pub mod ed25519 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_ed25519_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_ed25519_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
}

#[cfg(feature = "bn254CX")]
#[path = "./"]
pub mod bn254CX {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_bn254CX_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_bn254CX_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
    pub mod ecp2;
    pub mod fp2;
    pub mod fp4;
    pub mod fp12;
    pub mod pair;
    pub mod mpin;
    pub mod bls;
}

#[cfg(feature = "bn254")]
#[path = "./"]
pub mod bn254 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_bn254_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_bn254_64.rs"]
    pub mod rom;

    pub mod big;
    pub mod dbig;
    pub mod fp;
    pub mod ecp;
    pub mod ecdh;
    pub mod ecp2;
    pub mod fp2;
    pub mod fp4;
    pub mod fp12;
    pub mod pair;
    pub mod mpin;
    pub mod bls;
}

#[cfg(feature = "rsa2048")]
#[path = "./"]
pub mod rsa2048 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_rsa2048_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_rsa2048_64.rs"]
    pub mod rom;
    pub mod big;
    pub mod dbig;
    pub mod ff;
    pub mod rsa;
}

#[cfg(feature = "rsa3072")]
#[path = "./"]
pub mod rsa3072 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_rsa3072_32.rs"]
    pub mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_rsa3072_64.rs"]
    pub mod rom;
    pub mod big;
    pub mod dbig;
    pub mod ff;
    pub mod rsa;
}

#[cfg(feature = "rsa4096")]
#[path = "./"]
pub mod rsa4096 {
    #[cfg(target_pointer_width = "32")]
    #[path = "roms/rom_rsa4096_32.rs"]
    mod rom;
    #[cfg(target_pointer_width = "64")]
    #[path = "roms/rom_rsa4096_64.rs"]
    mod rom;
    pub mod big;
    pub mod dbig;
    pub mod ff;
    pub mod rsa;
}