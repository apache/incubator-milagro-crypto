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

use nums256w::big::NLEN;
use super::super::arch::Chunk;
use types::{ModType, CurveType, CurvePairingType, SexticTwist, SignOfX};

// Base Bits= 28
// nums256 modulus
pub const MODULUS: [Chunk; NLEN] = [
    0xFFFFF43, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF,
    0xFFFFFFF, 0xF,
];
pub const R2MODP: [Chunk; NLEN] = [0x0, 0x8900000, 0x8B, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0];
pub const MCONST: Chunk = 0xBD;

// nums256w curve
pub const CURVE_COF_I: isize = 1;
pub const CURVE_A: isize = -3;
pub const CURVE_B_I: isize = 152961;
pub const CURVE_COF: [Chunk; NLEN] = [0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0];
pub const CURVE_B: [Chunk; NLEN] = [0x25581, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0];
pub const CURVE_ORDER: [Chunk; NLEN] = [
    0x751A825, 0xAB20294, 0x65C6020, 0x8275EA2, 0xFFFE43C, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF,
    0xFFFFFFF, 0xF,
];
pub const CURVE_GX: [Chunk; NLEN] = [
    0x21AACB1, 0x52EE1EB, 0x4C73ABC, 0x9B0903D, 0xB098357, 0xA04F42C, 0x1297A95, 0x5AAADB6,
    0xC9ED6B6, 0xB,
];
pub const CURVE_GY: [Chunk; NLEN] = [
    0x184DE9F, 0xB5B9CB2, 0x10FBB80, 0xC3D1153, 0x35C955, 0xF77E04E, 0x673448B, 0x3399B6A,
    0x8FC0F1, 0xD,
];

pub const MODBYTES: usize = 32;
pub const BASEBITS: usize = 28;

pub const MODBITS: usize = 256;
pub const MOD8: usize = 3;
pub const MODTYPE: ModType = ModType::PSEUDO_MERSENNE;
pub const SH: usize = 14;

pub const CURVETYPE: CurveType = CurveType::WEIERSTRASS;
pub const CURVE_PAIRING_TYPE: CurvePairingType = CurvePairingType::NOT;
pub const SEXTIC_TWIST: SexticTwist = SexticTwist::NOT;
pub const SIGN_OF_X: SignOfX = SignOfX::NOT;
pub const HASH_TYPE: usize = 32;
pub const AESKEY: usize = 16;
