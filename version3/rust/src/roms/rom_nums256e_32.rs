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

use nums256e::big::NLEN;
use super::super::arch::Chunk;
use types::{ModType, CurveType, CurvePairingType, SexticTwist, SignOfX};

// Base Bits= 29

// Base Bits= 29
// nums256 Modulus
pub const MODULUS: [Chunk; NLEN] = [
    0x1FFFFF43, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF,
    0xFFFFFF,
];
pub const R2MODP: [Chunk; NLEN] = [0x22E2400, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0];
pub const MCONST: Chunk = 0xBD;

// nums256e Curve
pub const CURVE_COF_I: isize = 4;
pub const CURVE_A: isize = 1;
pub const CURVE_B_I: isize = -15342;
pub const CURVE_COF: [Chunk; NLEN] = [0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0];
pub const CURVE_B: [Chunk; NLEN] = [
    0x1FFFC355, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF,
    0xFFFFFF,
];
pub const CURVE_ORDER: [Chunk; NLEN] = [
    0xEDD4AF5, 0x123D8C87, 0x1650E6C6, 0xAB54A5E, 0x419, 0x0, 0x0, 0x0, 0x400000,
];
pub const CURVE_GX: [Chunk; NLEN] = [
    0xEED13DA, 0x6F60481, 0x20D61A8, 0x13141DC6, 0x9BD60C3, 0x1EAFB490, 0xDF73478, 0x1F6D5D44,
    0x8A7514,
];
pub const CURVE_GY: [Chunk; NLEN] = [
    0x198A89E6, 0x1D30B73B, 0x15BB4CB, 0x1EC3B021, 0x18010715, 0x12ECD325, 0x171F3A59, 0x13FB3B24,
    0x44D53E,
];

pub const MODBYTES: usize = 32;
pub const BASEBITS: usize = 29;

pub const MODBITS: usize = 256;
pub const MOD8: usize = 3;
pub const MODTYPE: ModType = ModType::PSEUDO_MERSENNE;
pub const SH: usize = 5;

pub const CURVETYPE: CurveType = CurveType::EDWARDS;
pub const CURVE_PAIRING_TYPE: CurvePairingType = CurvePairingType::NOT;
pub const SEXTIC_TWIST: SexticTwist = SexticTwist::NOT;
pub const SIGN_OF_X: SignOfX = SignOfX::NOT;
pub const HASH_TYPE: usize = 32;
pub const AESKEY: usize = 16;

