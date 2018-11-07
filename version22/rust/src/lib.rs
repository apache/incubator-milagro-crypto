#![allow(dead_code)]
#![allow(unused_variables)]

pub mod aes;
pub mod big;
pub mod dbig;
pub mod ecdh;
pub mod ecp;
pub mod ecp2;
pub mod ff;
pub mod fp;
pub mod fp2;
pub mod fp4;
pub mod fp12;
pub mod pair;
pub mod mpin;
pub mod rand;
pub mod hash256;
pub mod hash384;
pub mod hash512;
pub mod rsa;

#[cfg(target_pointer_width = "32")]
#[path = "rom32.rs"]
pub mod rom;

#[cfg(target_pointer_width = "64")]
#[path = "rom64.rs"]
pub mod rom;

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
    }
}
