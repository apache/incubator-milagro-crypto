#[derive(PartialEq)]
pub enum ModType {
    NOT_SPECIAL,
    PSEUDO_MERSENNE,
    MONTGOMERY_FRIENDLY,
    GENERALISED_MERSENNE,
}

#[derive(PartialEq)]
pub enum CurveType {
    EDWARDS,
    WEIERSTRASS,
    MONTGOMERY,
}

#[derive(PartialEq)]
pub enum CurvePairingType {
    NOT,
    BN,
    BLS,
}

#[derive(PartialEq)]
pub enum SexticTwist {
    NOT,
    D_TYPE,
    M_TYPE,
}
impl Into<usize> for SexticTwist {
    fn into(self) -> usize {
        match self {
            SexticTwist::NOT => 0,
            SexticTwist::D_TYPE => 0,
            SexticTwist::M_TYPE => 1,
        }
    }
}

#[derive(PartialEq)]
pub enum SignOfX {
    NOT,
    POSITIVEX,
    NEGATIVEX,
}