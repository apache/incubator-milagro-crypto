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

var PAIR = function(ctx) {
    "use strict";

    var PAIR = {
        /* Line function */
        line: function(A, B, Qx, Qy) {
            var r = new ctx.FP12(1),
                c = new ctx.FP4(0),
                XX, YY, ZZ, YZ, sb,
                X1, Y1, T1, T2,
                a, b;

            if (A == B) { /* Doubling */
                XX = new ctx.FP2(A.getx());
                YY = new ctx.FP2(A.gety());
                ZZ = new ctx.FP2(A.getz());
                YZ = new ctx.FP2(YY);

                YZ.mul(ZZ); //YZ
                XX.sqr(); //X^2
                YY.sqr(); //Y^2
                ZZ.sqr(); //Z^2

                YZ.imul(4);
                YZ.neg();
                YZ.norm(); //-2YZ
                YZ.pmul(Qy); //-2YZ.Ys

                XX.imul(6); //3X^2
                XX.pmul(Qx); //3X^2.Xs

                sb = 3 * ctx.ROM_CURVE.CURVE_B_I;
                ZZ.imul(sb);
                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.D_TYPE) {
                    ZZ.div_ip2();
                }
                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.M_TYPE) {
                    ZZ.mul_ip();
                    ZZ.add(ZZ);
                    YZ.mul_ip();
                    YZ.norm();
                }
                ZZ.norm(); // 3b.Z^2

                YY.add(YY);
                ZZ.sub(YY);
                ZZ.norm(); // 3b.Z^2-Y^2

                a = new ctx.FP4(YZ, ZZ); // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs
                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.D_TYPE) {
                    b = new ctx.FP4(XX); // L(0,1) | L(0,0) | L(1,0)
                    c = new ctx.FP4(0);
                }
                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.M_TYPE) {
                    b = new ctx.FP4(0);
                    c = new ctx.FP4(XX); c.times_i();
                }

                A.dbl();
            } else { /* Addition */
                X1 = new ctx.FP2(A.getx()); // X1
                Y1 = new ctx.FP2(A.gety()); // Y1
                T1 = new ctx.FP2(A.getz()); // Z1
                T2 = new ctx.FP2(A.getz()); // Z1

                T1.mul(B.gety()); // T1=Z1.Y2
                T2.mul(B.getx()); // T2=Z1.X2

                X1.sub(T2);
                X1.norm(); // X1=X1-Z1.X2
                Y1.sub(T1);
                Y1.norm(); // Y1=Y1-Z1.Y2

                T1.copy(X1); // T1=X1-Z1.X2
                X1.pmul(Qy); // X1=(X1-Z1.X2).Ys

                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.M_TYPE) {
                    X1.mul_ip();
                    X1.norm();
                }

                T1.mul(B.gety()); // T1=(X1-Z1.X2).Y2

                T2.copy(Y1); // T2=Y1-Z1.Y2
                T2.mul(B.getx()); // T2=(Y1-Z1.Y2).X2
                T2.sub(T1);
                T2.norm(); // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
                Y1.pmul(Qx);
                Y1.neg();
                Y1.norm(); // Y1=-(Y1-Z1.Y2).Xs

                a = new ctx.FP4(X1, T2); // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.D_TYPE) {
                    b = new ctx.FP4(Y1);
                    c = new ctx.FP4(0);
                }
                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.M_TYPE) {
                    b = new ctx.FP4(0);
                    c = new ctx.FP4(Y1); c.times_i();
                }

                A.add(B);
            }

            r.set(a, b, c);
			r.settype(ctx.FP.SPARSER);

            return r;
        },

/* prepare for multi-pairing */
		initmp: function() {
			var r=[];
			for (var i=0;i<ctx.ECP.ATE_BITS;i++)
				r[i] = new ctx.FP12(1);
			return r;
		},

/* basic Miller loop */
		miller: function(r) {
			var res=new ctx.FP12(1);
			for (var i=ctx.ECP.ATE_BITS-1; i>=1; i--)
			{
				res.sqr();
				res.ssmul(r[i]); 
			}

			if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX)
				res.conj();
			res.ssmul(r[0]);

			return res;
		},

/* Accumulate another set of line functions for n-pairing */
		another: function(r,P1,Q1) {

			var f;
			var n=new ctx.BIG(0);
			var n3=new ctx.BIG(0);
			var K=new ctx.ECP2();
			var lv,lv2;
			var bt;

// P is needed in affine form for line function, Q for (Qx,Qy) extraction
			var P=new ctx.ECP2(); P.copy(P1); P.affine();
			var Q=new ctx.ECP(); Q.copy(Q1); Q.affine();

			P.affine();
			Q.affine();

			if (ctx.ECP.CURVE_PAIRING_TYPE==ctx.ECP.BN)
			{
                var fa = new ctx.BIG(0);
                fa.rcopy(ctx.ROM_FIELD.Fra);
                var fb = new ctx.BIG(0);
                fb.rcopy(ctx.ROM_FIELD.Frb);
                f = new ctx.FP2(fa, fb); 
				if (ctx.ECP.SEXTIC_TWIST==ctx.ECP.M_TYPE)
				{
					f.inverse();
					f.norm();
				}
			}

			var Qx=new ctx.FP(Q.getx());
			var Qy=new ctx.FP(Q.gety());

			var A=new ctx.ECP2();
			A.copy(P);

			var MP=new ctx.ECP2();
			MP.copy(P); MP.neg();

			var nb=PAIR.lbits(n3,n);

			for (var i=nb-2;i>=1;i--)
			{
				lv=PAIR.line(A,A,Qx,Qy);

				bt=n3.bit(i)-n.bit(i); 
				if (bt==1)
				{
					lv2=PAIR.line(A,P,Qx,Qy);
					lv.smul(lv2);
				}
				if (bt==-1)
				{
					lv2=PAIR.line(A,MP,Qx,Qy);
					lv.smul(lv2);
				}
				r[i].ssmul(lv);
			}

/* R-ate fixup required for BN curves */
			if (ctx.ECP.CURVE_PAIRING_TYPE==ctx.ECP.BN)
			{
				if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX)
				{
					A.neg();
				}
				K.copy(P);
				K.frob(f);
				lv=PAIR.line(A,K,Qx,Qy);
				K.frob(f);
				K.neg();
				lv2=PAIR.line(A,K,Qx,Qy);
				lv.smul(lv2);
				r[0].ssmul(lv);
			}	 
		},

        /* Optimal R-ate pairing */
        ate: function(P1, Q1) {
            var fa, fb, f, x, n, n3, K, lv, lv2,
                Qx, Qy, A, NP, r, nb, bt,
                i;

            n = new ctx.BIG(0);
			n3 = new ctx.BIG(0);
            K = new ctx.ECP2();

            if (ctx.ECP.CURVE_PAIRING_TYPE == ctx.ECP.BN) {

                fa = new ctx.BIG(0);
                fa.rcopy(ctx.ROM_FIELD.Fra);
                fb = new ctx.BIG(0);
                fb.rcopy(ctx.ROM_FIELD.Frb);
                f = new ctx.FP2(fa, fb); 

                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.M_TYPE) {
                    f.inverse();
                    f.norm();
                }
            }

			var P=new ctx.ECP2(); P.copy(P1); P.affine();
			var Q=new ctx.ECP(); Q.copy(Q1); Q.affine();

            Qx = new ctx.FP(Q.getx()); 
            Qy = new ctx.FP(Q.gety()); 

            A = new ctx.ECP2();
            r = new ctx.FP12(1);
            A.copy(P);

			NP = new ctx.ECP2();
			NP.copy(P);
			NP.neg();

            nb = PAIR.lbits(n3,n);

            for (i = nb - 2; i >= 1; i--) {
                r.sqr();
                lv = PAIR.line(A, A, Qx, Qy);
                bt=n3.bit(i)-n.bit(i);

                if (bt == 1) {
                    lv2 = PAIR.line(A, P, Qx, Qy);
                    lv.smul(lv2);
                }
                if (bt == -1) {
                    lv2 = PAIR.line(A, NP, Qx, Qy);
                    lv.smul(lv2);
                }
                r.ssmul(lv);
            }

            if (ctx.ECP.SIGN_OF_X == ctx.ECP.NEGATIVEX) {
                r.conj();
            }

            /* R-ate fixup */
            if (ctx.ECP.CURVE_PAIRING_TYPE == ctx.ECP.BN) {
                if (ctx.ECP.SIGN_OF_X == ctx.ECP.NEGATIVEX) {
                    A.neg();
                }

                K.copy(P);
                K.frob(f);

                lv = PAIR.line(A, K, Qx, Qy);
                K.frob(f);
                K.neg();
                lv2 = PAIR.line(A, K, Qx, Qy);
				lv.smul(lv2);
                r.ssmul(lv);
            }

            return r;
        },

        /* Optimal R-ate double pairing e(P,Q).e(R,S) */
	
        ate2: function(P1, Q1, R1, S1) {
            var fa, fb, f, x, n, n3, K, lv, lv2,
                Qx, Qy, Sx, Sy, A, B, NP,NR,r, nb, bt,
                i;

            n = new ctx.BIG(0);
			n3 = new ctx.BIG(0);
            K = new ctx.ECP2();

            if (ctx.ECP.CURVE_PAIRING_TYPE == ctx.ECP.BN) {
                fa = new ctx.BIG(0);
                fa.rcopy(ctx.ROM_FIELD.Fra);
                fb = new ctx.BIG(0);
                fb.rcopy(ctx.ROM_FIELD.Frb);
                f = new ctx.FP2(fa, fb);

                if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.M_TYPE) {
                    f.inverse();
                    f.norm();
                }
            } 

			var P=new ctx.ECP2(); P.copy(P1); P.affine();
			var Q=new ctx.ECP(); Q.copy(Q1); Q.affine();
			var R=new ctx.ECP2(); R.copy(R1); R.affine();
			var S=new ctx.ECP(); S.copy(S1); S.affine();


            Qx = new ctx.FP(Q.getx()); 
            Qy = new ctx.FP(Q.gety()); 

            Sx = new ctx.FP(S.getx()); 
            Sy = new ctx.FP(S.gety()); 

            A = new ctx.ECP2();
            B = new ctx.ECP2();
            r = new ctx.FP12(1);

            A.copy(P);
            B.copy(R);

			NP = new ctx.ECP2();
			NP.copy(P);
			NP.neg();
			NR = new ctx.ECP2();
			NR.copy(R);
			NR.neg();

            nb = PAIR.lbits(n3,n);

            for (i = nb - 2; i >= 1; i--) {
                r.sqr();
                lv = PAIR.line(A, A, Qx, Qy);
                lv2 = PAIR.line(B, B, Sx, Sy);
				lv.smul(lv2);
                r.ssmul(lv);

                bt=n3.bit(i)-n.bit(i);

                if (bt == 1) {
                    lv = PAIR.line(A, P, Qx, Qy);
                    lv2 = PAIR.line(B, R, Sx, Sy);
					lv.smul(lv2);
                    r.ssmul(lv);
                }
                if (bt == -1) {
                    lv = PAIR.line(A, NP, Qx, Qy);
                    lv2 = PAIR.line(B, NR, Sx, Sy);
					lv.smul(lv2);
                    r.ssmul(lv);
                }
            }

            if (ctx.ECP.SIGN_OF_X == ctx.ECP.NEGATIVEX) {
                r.conj();
            }


            // R-ate fixup required for BN curves 
            if (ctx.ECP.CURVE_PAIRING_TYPE == ctx.ECP.BN) {
                if (ctx.ECP.SIGN_OF_X == ctx.ECP.NEGATIVEX) {
                    A.neg();
                    B.neg();
                }
                K.copy(P);
                K.frob(f);

                lv = PAIR.line(A, K, Qx, Qy);
                K.frob(f);
                K.neg();
                lv2 = PAIR.line(A, K, Qx, Qy);
				lv.smul(lv2);
                r.ssmul(lv);

                K.copy(R);
                K.frob(f);

                lv = PAIR.line(B, K, Sx, Sy);
                K.frob(f);
                K.neg();
                lv2 = PAIR.line(B, K, Sx, Sy);
				lv.smul(lv2);
                r.ssmul(lv);
            }

            return r;
        },

        /* final exponentiation - keep separate for multi-pairings and to avoid thrashing stack */
        fexp: function(m) {
            var fa, fb, f, x, r, lv,
                x0, x1, x2, x3, x4, x5,
                y0, y1, y2, y3;

            fa = new ctx.BIG(0);
            fa.rcopy(ctx.ROM_FIELD.Fra);
            fb = new ctx.BIG(0);
            fb.rcopy(ctx.ROM_FIELD.Frb);
            f = new ctx.FP2(fa, fb);
            x = new ctx.BIG(0);
            x.rcopy(ctx.ROM_CURVE.CURVE_Bnx);

            r = new ctx.FP12(m);

            /* Easy part of final exp */
            lv = new ctx.FP12(r);
            lv.inverse();
            r.conj();
            r.mul(lv);
            lv.copy(r);
            r.frob(f);
            r.frob(f);
            r.mul(lv);
			if (r.isunity())
			{
				r.zero();
				return r;
			}
            /* Hard part of final exp */
            if (ctx.ECP.CURVE_PAIRING_TYPE == ctx.ECP.BN) {
                lv.copy(r);
                lv.frob(f);
                x0 = new ctx.FP12(lv); //x0.copy(lv);
                x0.frob(f);
                lv.mul(r);
                x0.mul(lv);
                x0.frob(f);
                x1 = new ctx.FP12(r); //x1.copy(r);
                x1.conj();

                x4 = r.pow(x);
                if (ctx.ECP.SIGN_OF_X == ctx.ECP.POSITIVEX) {
                    x4.conj();
                }

                x3 = new ctx.FP12(x4); //x3.copy(x4);
                x3.frob(f);
                x2 = x4.pow(x);
                if (ctx.ECP.SIGN_OF_X == ctx.ECP.POSITIVEX) {
                    x2.conj();
                }
                x5 = new ctx.FP12(x2); /*x5.copy(x2);*/
                x5.conj();
                lv = x2.pow(x);
                if (ctx.ECP.SIGN_OF_X == ctx.ECP.POSITIVEX) {
                    lv.conj();
                }
                x2.frob(f);
                r.copy(x2);
                r.conj();

                x4.mul(r);
                x2.frob(f);

                r.copy(lv);
                r.frob(f);
                lv.mul(r);

                lv.usqr();
                lv.mul(x4);
                lv.mul(x5);
                r.copy(x3);
                r.mul(x5);
                r.mul(lv);
                lv.mul(x2);
                r.usqr();
                r.mul(lv);
                r.usqr();
                lv.copy(r);
                lv.mul(x1);
                r.mul(x0);
                lv.usqr();
                r.mul(lv);
                r.reduce();
            } else {
                // Ghamman & Fouotsa Method
                y0 = new ctx.FP12(r);
                y0.usqr();
                y1 = y0.pow(x);
                if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX) {
                    y1.conj();
                }
                x.fshr(1);
                y2 = y1.pow(x);
                if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX) {
                    y2.conj();
                }
                x.fshl(1);
                y3 = new ctx.FP12(r);
                y3.conj();
                y1.mul(y3);

                y1.conj();
                y1.mul(y2);

                y2 = y1.pow(x);
                if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX) {
                    y2.conj();
                }

                y3 = y2.pow(x);
                if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX) {
                    y3.conj();
                }
                y1.conj();
                y3.mul(y1);

                y1.conj();
                y1.frob(f);
                y1.frob(f);
                y1.frob(f);
                y2.frob(f);
                y2.frob(f);
                y1.mul(y2);

                y2 = y3.pow(x);
                if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX) {
                    y2.conj();
                }
                y2.mul(y0);
                y2.mul(r);

                y1.mul(y2);
                y2.copy(y3);
                y2.frob(f);
                y1.mul(y2);
                r.copy(y1);
                r.reduce();
            }

            return r;
        }
    };

/* prepare ate parameter, n=6u+2 (BN) or n=u (BLS), n3=3*n */
	PAIR.lbits = function(n3,n)
	{
		n.rcopy(ctx.ROM_CURVE.CURVE_Bnx);
		if (ctx.ECP.CURVE_PAIRING_TYPE==ctx.ECP.BN)
		{
			n.pmul(6);
			if (ctx.ECP.SIGN_OF_X==ctx.ECP.POSITIVEX)
			{
				n.inc(2);
			} else {
				n.dec(2);
			}
		}
		
		n.norm();
		n3.copy(n);
		n3.pmul(3);
		n3.norm();
		return n3.nbits();
	},

    /* GLV method */
    PAIR.glv = function(e) {
        var u = [],
            t, q, v, d, x, x2, i, j;

        if (ctx.ECP.CURVE_PAIRING_TYPE == ctx.ECP.BN) {
            t = new ctx.BIG(0);
            q = new ctx.BIG(0);
            v = [];

            q.rcopy(ctx.ROM_CURVE.CURVE_Order);

            for (i = 0; i < 2; i++) {
                t.rcopy(ctx.ROM_CURVE.CURVE_W[i]);
                d = ctx.BIG.mul(t, e);
                v[i] = new ctx.BIG(d.div(q));
                u[i] = new ctx.BIG(0);
            }

            u[0].copy(e);

            for (i = 0; i < 2; i++) {
                for (j = 0; j < 2; j++) {
                    t.rcopy(ctx.ROM_CURVE.CURVE_SB[j][i]);
                    t.copy(ctx.BIG.modmul(v[j], t, q));
                    u[i].add(q);
                    u[i].sub(t);
                    u[i].mod(q);
                }
            }
        } else { // -(x^2).P = (Beta.x,y)
            q = new ctx.BIG(0);
            q.rcopy(ctx.ROM_CURVE.CURVE_Order);
            x = new ctx.BIG(0);
            x.rcopy(ctx.ROM_CURVE.CURVE_Bnx);
            x2 = ctx.BIG.smul(x, x);
            u[0] = new ctx.BIG(e);
            u[0].mod(x2);
            u[1] = new ctx.BIG(e);
            u[1].div(x2);
            u[1].rsub(q);
        }

        return u;
    };

    /* Galbraith & Scott Method */
    PAIR.gs = function(e) {
        var u = [],
            i, j, t, q, v, d, x, w;

        if (ctx.ECP.CURVE_PAIRING_TYPE == ctx.ECP.BN) {
            t = new ctx.BIG(0);
            q = new ctx.BIG(0);
            q.rcopy(ctx.ROM_CURVE.CURVE_Order);

            v = [];

            for (i = 0; i < 4; i++) {
                t.rcopy(ctx.ROM_CURVE.CURVE_WB[i]);
                d = ctx.BIG.mul(t, e);
                v[i] = new ctx.BIG(d.div(q));
                u[i] = new ctx.BIG(0);
            }

            u[0].copy(e);

            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    t.rcopy(ctx.ROM_CURVE.CURVE_BB[j][i]);
                    t.copy(ctx.BIG.modmul(v[j], t, q));
                    u[i].add(q);
                    u[i].sub(t);
                    u[i].mod(q);
                }
            }
        } else {
            x = new ctx.BIG(0);
            x.rcopy(ctx.ROM_CURVE.CURVE_Bnx);
            q = new ctx.BIG(0);
            q.rcopy(ctx.ROM_CURVE.CURVE_Order);
            w = new ctx.BIG(e);

            for (i = 0; i < 3; i++) {
                u[i] = new ctx.BIG(w);
                u[i].mod(x);
                w.div(x);
            }

            u[3] = new ctx.BIG(w);
            if (ctx.ECP.SIGN_OF_X==ctx.ECP.NEGATIVEX) {
                u[1].copy(ctx.BIG.modneg(u[1], q));
                u[3].copy(ctx.BIG.modneg(u[3], q));
            }
        }

        return u;
    };

    /* Multiply P by e in group G1 */
    PAIR.G1mul = function(P, e) {
        var R, Q, q, bcru, cru, t, u, np, nn;

        if (ctx.ROM_CURVE.USE_GLV) {
            R = new ctx.ECP();
            R.copy(P);
            Q = new ctx.ECP();
            Q.copy(P); Q.affine();
            q = new ctx.BIG(0);
            q.rcopy(ctx.ROM_CURVE.CURVE_Order);
            bcru = new ctx.BIG(0);
            bcru.rcopy(ctx.ROM_CURVE.CURVE_Cru);
            cru = new ctx.FP(bcru);
            t = new ctx.BIG(0);
            u = PAIR.glv(e);

            Q.getx().mul(cru);

            np = u[0].nbits();
            t.copy(ctx.BIG.modneg(u[0], q));
            nn = t.nbits();
            if (nn < np) {
                u[0].copy(t);
                R.neg();
            }

            np = u[1].nbits();
            t.copy(ctx.BIG.modneg(u[1], q));
            nn = t.nbits();
            if (nn < np) {
                u[1].copy(t);
                Q.neg();
            }
            u[0].norm();
            u[1].norm();
            R = R.mul2(u[0], Q, u[1]);
        } else {
            R = P.mul(e);
        }

        return R;
    };

    /* Multiply P by e in group G2 */
    PAIR.G2mul = function(P, e) {
        var R, Q, fa, fb, f, q, u, t, i, np, nn;

        if (ctx.ROM_CURVE.USE_GS_G2) {
            Q = [];
            fa = new ctx.BIG(0);
            fa.rcopy(ctx.ROM_FIELD.Fra);
            fb = new ctx.BIG(0);
            fb.rcopy(ctx.ROM_FIELD.Frb);
            f = new ctx.FP2(fa, fb);

            if (ctx.ECP.SEXTIC_TWIST == ctx.ECP.M_TYPE) {
                f.inverse();
                f.norm();
            }

            q = new ctx.BIG(0);
            q.rcopy(ctx.ROM_CURVE.CURVE_Order);

            u = PAIR.gs(e);
            t = new ctx.BIG(0);
            Q[0] = new ctx.ECP2();
            Q[0].copy(P);

            for (i = 1; i < 4; i++) {
                Q[i] = new ctx.ECP2();
                Q[i].copy(Q[i - 1]);
                Q[i].frob(f);
            }

            for (i = 0; i < 4; i++) {
                np = u[i].nbits();
                t.copy(ctx.BIG.modneg(u[i], q));
                nn = t.nbits();

                if (nn < np) {
                    u[i].copy(t);
                    Q[i].neg();
                }
                u[i].norm();
            }

            R = ctx.ECP2.mul4(Q, u);
        } else {
            R = P.mul(e);
        }
        return R;
    };

    /* Note that this method requires a lot of RAM! Better to use compressed XTR method, see ctx.FP4.js */
    PAIR.GTpow = function(d, e) {
        var r, g, fa, fb, f, q, t, u, i, np, nn;

        if (ctx.ROM_CURVE.USE_GS_GT) {
            g = [];
            fa = new ctx.BIG(0);
            fa.rcopy(ctx.ROM_FIELD.Fra);
            fb = new ctx.BIG(0);
            fb.rcopy(ctx.ROM_FIELD.Frb);
            f = new ctx.FP2(fa, fb);
            q = new ctx.BIG(0);
            q.rcopy(ctx.ROM_CURVE.CURVE_Order);
            t = new ctx.BIG(0);
            u = PAIR.gs(e);

            g[0] = new ctx.FP12(d);

            for (i = 1; i < 4; i++) {
                g[i] = new ctx.FP12(0);
                g[i].copy(g[i - 1]);
                g[i].frob(f);
            }

            for (i = 0; i < 4; i++) {
                np = u[i].nbits();
                t.copy(ctx.BIG.modneg(u[i], q));
                nn = t.nbits();

                if (nn < np) {
                    u[i].copy(t);
                    g[i].conj();
                }
                u[i].norm();
            }

            r = ctx.FP12.pow4(g, u);
        } else {
            r = d.pow(e);
        }

        return r;
    };

    return PAIR;
};
