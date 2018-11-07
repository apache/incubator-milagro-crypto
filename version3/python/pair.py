#
# Optimal Ate Pairing
# M.Scott August 2018
#

from constants import *

from XXX import curve
from XXX import big
from XXX.fp2 import *
from XXX.fp4 import *
from XXX.fp12 import *

from XXX import ecp
from XXX import ecp2

# line function


def g(A, B, Qx, Qy):
    if A == B:
        XX, YY, ZZ = A.getxyz()
        YZ = YY * ZZ
        XX.sqr()
        YY.sqr()
        ZZ.sqr()

        ZZ = ZZ.muli(3 * curve.B)
        if curve.SexticTwist == D_TYPE:
            ZZ = ZZ.divQNR2()
        else:
            ZZ = ZZ.mulQNR()
            ZZ += ZZ
            YZ = YZ.mulQNR()

        a = Fp4(-YZ.muls(Qy).muli(4), ZZ - (YY + YY))
        if curve.SexticTwist == D_TYPE:
            b = Fp4(XX.muli(6).muls(Qx))
            c = Fp4()
        else:
            b = Fp4()
            c = Fp4(XX.muli(6).muls(Qx))
            c.times_i()
        A.dbl()
        return Fp12(a, b, c)
    else:
        X1, Y1, Z1 = A.getxyz()
        X2, Y2 = B.getxy()
        T1 = (X1 - Z1 * X2)
        T2 = (Y1 - Z1 * Y2)

        if curve.SexticTwist == D_TYPE:
            a = Fp4(T1.muls(Qy), T2 * X2 - T1 * Y2)
            b = Fp4(-T2.muls(Qx))
            c = Fp4()
        else:
            a = Fp4(T1.muls(Qy).mulQNR(), T2 * X2 - T1 * Y2)
            b = Fp4()
            c = Fp4(-T2.muls(Qx))
            c.times_i()
        A.add(B)
        return Fp12(a, b, c)

# full pairing - miller loop followed by final exponentiation


def e(P, Q):
    r = miller(P, Q)
    return fexp(r)


def miller(P1, Q1):
    x = curve.x
    if curve.PairingFriendly == BN:
        n = 6 * x
        if curve.SignOfX == POSITIVEX:
            n += 2
        else:
            n -= 2
    else:
        n = x
    n3 = 3 * n

    P = P1.copy()
    Q = Q1.copy()

    P.affine()
    Q.affine()
    A = P.copy()
    Qx, Qy = Q.getxy()
    nb = n3.bit_length()
    r = Fp12.one()
# miller loop
    for i in range(nb - 2, 0, -1):
        r.sqr()
        r *= g(A, A, Qx, Qy)

        if big.bit(n3, i) == 1 and big.bit(n, i) == 0:
            r *= g(A, P, Qx, Qy)
        if big.bit(n3, i) == 0 and big.bit(n, i) == 1:
            r *= g(A, -P, Qx, Qy)

# adjustment
    if curve.SignOfX == NEGATIVEX:
        r.conj()

    if curve.PairingFriendly == BN:
        KA = P.copy()
        KA.frobenius()
        if curve.SignOfX == NEGATIVEX:
            A = -A
        r *= g(A, KA, Qx, Qy)
        KA.frobenius()
        KA = -KA
        r *= g(A, KA, Qx, Qy)

    return r


def double_miller(P1, Q1, U1, V1):
    x = curve.x

    if curve.PairingFriendly == BN:
        n = 6 * x
        if curve.SignOfX == POSITIVEX:
            n += 2
        else:
            n -= 2
    else:
        n = x

    n3 = 3 * n

    P = P1.copy()
    Q = Q1.copy()
    U = U1.copy()
    V = V1.copy()

    P.affine()
    Q.affine()
    U.affine()
    V.affine()
    A = P.copy()
    Qx, Qy = Q.getxy()
    B = U.copy()
    Wx, Wy = V.getxy()
    nb = n3.bit_length()
    r = Fp12.one()
# miller loop
    for i in range(nb - 2, 0, -1):
        r.sqr()
        r *= g(A, A, Qx, Qy)
        r *= g(B, B, Wx, Wy)
        if big.bit(n3, i) == 1 and big.bit(n, i) == 0:
            r *= g(A, P, Qx, Qy)
            r *= g(B, U, Wx, Wy)
        if big.bit(n3, i) == 0 and big.bit(n, i) == 1:
            r *= g(A, -P, Qx, Qy)
            r *= g(B, -U, Wx, Wy)
# adjustment
    if curve.SignOfX == NEGATIVEX:
        r.conj()

    if curve.PairingFriendly == BN:
        KA = P.copy()
        KA.frobenius()
        if curve.SignOfX == NEGATIVEX:
            A = -A
            B = -B
        r *= g(A, KA, Qx, Qy)
        KA.frobenius()
        KA = -KA
        r *= g(A, KA, Qx, Qy)

        KB = U.copy()
        KB.frobenius()

        r *= g(B, KB, Wx, Wy)
        KB.frobenius()
        KB = -KB
        r *= g(B, KB, Wx, Wy)

    return r


def fexp(r):
    # final exp - easy part
    t0 = r.copy()
    r.conj()
    r *= t0.inverse()

    t0 = r.copy()
    r.powq()
    r.powq()
    r *= t0
# final exp - hard part
    x = curve.x

    if curve.PairingFriendly == BN:

        res = r.copy()

        t0 = res.copy()
        t0.powq()
        x0 = t0.copy()
        x0.powq()

        x0 *= (res * t0)
        x0.powq()

        x1 = res.copy()
        x1.conj()
        x4 = res.pow(x)
        if curve.SignOfX == POSITIVEX:
            x4.conj()
        x3 = x4.copy()
        x3.powq()

        x2 = x4.pow(x)
        if curve.SignOfX == POSITIVEX:
            x2.conj()

        x5 = x2.copy()
        x5.conj()
        t0 = x2.pow(x)
        if curve.SignOfX == POSITIVEX:
            t0.conj()

        x2.powq()
        res = x2.copy()
        res.conj()
        x4 = x4 * res
        x2.powq()

        res = t0.copy()
        res.powq()
        t0 *= res

        t0.usqr()
        t0 *= x4
        t0 *= x5
        res = x3 * x5
        res *= t0
        t0 *= x2
        res.usqr()
        res *= t0
        res.usqr()
        t0 = res * x1
        res *= x0
        t0.usqr()
        res *= t0

    else:  # its a BLS curve

        # Ghamman & Fouotsa Method
        y0 = r.copy()
        y0.usqr()
        y1 = y0.pow(x)
        if curve.SignOfX == NEGATIVEX:
            y1.conj()
        x //= 2
        y2 = y1.pow(x)
        if curve.SignOfX == NEGATIVEX:
            y2.conj()
        x *= 2

        y3 = r.copy()
        y3.conj()
        y1 *= y3
        y1.conj()
        y1 *= y2

        y2 = y1.pow(x)
        if curve.SignOfX == NEGATIVEX:
            y2.conj()
        y3 = y2.pow(x)
        if curve.SignOfX == NEGATIVEX:
            y3.conj()
        y1.conj()
        y3 *= y1

        y1.conj()
        y1.powq()
        y1.powq()
        y1.powq()

        y2.powq()
        y2.powq()

        y1 *= y2

        y2 = y3.pow(x)
        if curve.SignOfX == NEGATIVEX:
            y2.conj()
        y2 *= y0
        y2 *= r
        y1 *= y2
        y3.powq()
        y1 *= y3
        res = y1

    return res

# G=ecp.generator()
# P=ecp2.generator()
# pr=e(P,G)
# print(pr)

# print(pr.pow(curve.r))

# P7=7*P
# pr=e(P7,G)
# print(pr)

# G7=7*G
# pr=e(P,G7)
# print(pr)
