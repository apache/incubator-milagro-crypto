package milagro;
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

/* MPIN API Functions */

import java.util.Date;

public class MPIN
{
	public static final int EFS=ROM.MODBYTES;
	public static final int EGS=ROM.MODBYTES;
	public static final int PAS=16;
	public static final int INVALID_POINT=-14;
	public static final int BAD_PARAMS=-11;
	public static final int WRONG_ORDER=-18;
	public static final int BAD_PIN=-19;

/* Configure your PIN here */

	public static final int MAXPIN=10000;  /* PIN less than this */
	public static final int PBLEN=14;      /* Number of bits in PIN */
	public static final int TS=10;         /* 10 for 4 digit PIN, 14 for 6-digit PIN - 2^TS/TS approx = sqrt(MAXPIN) */
	public static final int TRAP=200;      /* 200 for 4 digit PIN, 2000 for 6-digit PIN  - approx 2*sqrt(MAXPIN) */

/* Hash number (optional) and string to point on curve */

	public static byte[] hashit(int n,byte[] ID)
	{
		HASH H=new HASH();
		if (n!=0) H.process_num(n);
		H.process_array(ID);
		byte[] h=H.hash();
		return h;
	}

	public static ECP mapit(byte[] h)
	{
		BIG q=new BIG(ROM.ROM.fieldDetails.getModulus());
		BIG x=BIG.fromBytes(h);
		x.mod(q);
		ECP P;
		while (true)
		{
			P=new ECP(x,0);
			if (!P.is_infinity()) break;
			x.inc(1); x.norm();
		}
		return P;
	}

/* needed for SOK */
	public static ECP2 mapit2(byte[] h)
	{
		BIG q=new BIG(ROM.ROM.fieldDetails.getModulus());
		BIG x=BIG.fromBytes(h);
		BIG one=new BIG(1);
		FP2 X;
		ECP2 Q,T,K;
		x.mod(q);
		while (true)
		{
			X=new FP2(one,x);
			Q=new ECP2(X);
			if (!Q.is_infinity()) break;
			x.inc(1); x.norm();
		}
/* Fast Hashing to G2 - Fuentes-Castaneda, Knapp and Rodriguez-Henriquez */
		BIG Fra=new BIG(ROM.ROM.curveDetails.getCurveFra());
		BIG Frb=new BIG(ROM.ROM.curveDetails.getCurveFrb());
		X=new FP2(Fra,Frb);
		x=new BIG(ROM.ROM.curveDetails.getCurveBnx());

		T=new ECP2(); T.copy(Q);
		T.mul(x); T.neg();
		K=new ECP2(); K.copy(T);
		K.dbl(); K.add(T); K.affine();

		K.frob(X);
		Q.frob(X); Q.frob(X); Q.frob(X);
		Q.add(T); Q.add(K);
		T.frob(X); T.frob(X);
		Q.add(T);
		Q.affine();
		return Q;
	}

/* return time in slots since epoch */
	public static int today() {
		Date date=new Date();
		return (int) (date.getTime()/(1000*60*1440));
	}

/* these next two functions help to implement elligator squared - http://eprint.iacr.org/2014/043 */
/* maps a random u to a point on the curve */
	public static ECP map(BIG u,int cb)
	{
		ECP P;
		BIG x=new BIG(u);
		BIG p=new BIG(ROM.ROM.fieldDetails.getModulus());
		x.mod(p);
		while (true)
		{
			P=new ECP(x,cb);
			if (!P.is_infinity()) break;
			x.inc(1);  x.norm();
		}
		return P;
	}

/* returns u derived from P. Random value in range 1 to return value should then be added to u */
	public static int unmap(BIG u,ECP P)
	{
		int s=P.getS();
		ECP R;
		int r=0;
		BIG x=P.getX();
		u.copy(x);
		while (true)
		{
			u.dec(1); u.norm();
			r++;
			R=new ECP(u,s);
			if (!R.is_infinity()) break;
		}
		return r;
	}

	public static byte[] HASH_ID(byte[] ID)
	{
		return hashit(0,ID);
	}


/* these next two functions implement elligator squared - http://eprint.iacr.org/2014/043 */
/* Elliptic curve point E in format (0x04,x,y} is converted to form {0x0-,u,v} */
/* Note that u and v are indistinguisible from random strings */
	public static int ENCODING(RAND rng,byte[] E)
	{
		int rn,m,su,sv;
		byte[] T=new byte[EFS];

		for (int i=0;i<EFS;i++) T[i]=E[i+1];
		BIG u=BIG.fromBytes(T);
		for (int i=0;i<EFS;i++) T[i]=E[i+EFS+1];
		BIG v=BIG.fromBytes(T);

		ECP P=new ECP(u,v);
		if (P.is_infinity()) return INVALID_POINT;

		BIG p=new BIG(ROM.ROM.fieldDetails.getModulus());
		u=BIG.randomnum(p,rng);

		su=rng.getByte(); /*if (su<0) su=-su;*/ su%=2;

		ECP W=map(u,su);
		P.sub(W);
		sv=P.getS();
		rn=unmap(v,P);
		m=rng.getByte(); /*if (m<0) m=-m;*/ m%=rn;
		v.inc(m+1);
		E[0]=(byte)(su+2*sv);
		u.toBytes(T);
		for (int i=0;i<EFS;i++) E[i+1]=T[i];
		v.toBytes(T);
		for (int i=0;i<EFS;i++) E[i+EFS+1]=T[i];

		return 0;
	}

	public static int DECODING(byte[] D)
	{
		int su,sv;
		byte[] T=new byte[EFS];

		if ((D[0]&0x04)!=0) return INVALID_POINT;

		for (int i=0;i<EFS;i++) T[i]=D[i+1];
		BIG u=BIG.fromBytes(T);
		for (int i=0;i<EFS;i++) T[i]=D[i+EFS+1];
		BIG v=BIG.fromBytes(T);

		su=D[0]&1;
		sv=(D[0]>>1)&1;
		ECP W=map(u,su);
		ECP P=map(v,sv);
		P.add(W);
		u=P.getX();
		v=P.getY();
		D[0]=0x04;
		u.toBytes(T);
		for (int i=0;i<EFS;i++) D[i+1]=T[i];
		v.toBytes(T);
		for (int i=0;i<EFS;i++) D[i+EFS+1]=T[i];

		return 0;
	}

/* R=R1+R2 in group G1 */
	public static int RECOMBINE_G1(byte[] R1,byte[] R2,byte[] R)
	{
		ECP P=ECP.fromBytes(R1);
		ECP Q=ECP.fromBytes(R2);

		if (P.is_infinity() || Q.is_infinity()) return INVALID_POINT;

		P.add(Q);

		P.toBytes(R);
		return 0;
	}

/* W=W1+W2 in group G2 */
	public static int RECOMBINE_G2(byte[] W1,byte[] W2,byte[] W)
	{
		ECP2 P=ECP2.fromBytes(W1);
		ECP2 Q=ECP2.fromBytes(W2);

		if (P.is_infinity() || Q.is_infinity()) return INVALID_POINT;

		P.add(Q);

		P.toBytes(W);
		return 0;
	}

/* create random secret S */
	public static int RANDOM_GENERATE(RAND rng,byte[] S)
	{
		BIG s;
		BIG r=new BIG(ROM.ROM.curveDetails.getCurveOrder());
		s=BIG.randomnum(r,rng);

		s.toBytes(S);
		return 0;
	}

/* Extract PIN from TOKEN for identity CID */
	public static int EXTRACT_PIN(byte[] CID,int pin,byte[] TOKEN)
	{
		ECP P=ECP.fromBytes(TOKEN);
		if (P.is_infinity()) return INVALID_POINT;
		byte[] h=hashit(0,CID);
		ECP R=mapit(h);


		pin%=MAXPIN;

		R=R.pinmul(pin,PBLEN);
		P.sub(R);

		P.toBytes(TOKEN);

		return 0;
	}

/* Implement step 2 on client side of MPin protocol */
	public static int CLIENT_2(byte[] X,byte[] Y,byte[] SEC)
	{
		BIG r=new BIG(ROM.ROM.curveDetails.getCurveOrder());
		ECP P=ECP.fromBytes(SEC);
		if (P.is_infinity()) return INVALID_POINT;

		BIG px=BIG.fromBytes(X);
		BIG py=BIG.fromBytes(Y);
		px.add(py);
		px.mod(r);
		px.rsub(r);

		PAIR.G1mul(P,px).toBytes(SEC);
		return 0;
	}

/* Implement step 1 on client side of MPin protocol */
	public static int CLIENT_1(int date,byte[] CLIENT_ID,RAND rng,byte[] X,int pin,byte[] TOKEN,byte[] SEC,byte[] xID,byte[] xCID,byte[] PERMIT)
	{
		BIG r=new BIG(ROM.ROM.curveDetails.getCurveOrder());
//		BIG q=new BIG(ROM.Modulus);
		BIG x;
//		BIG m=new BIG(0);
		if (rng!=null)
		{
			x=BIG.randomnum(r,rng);
			x.toBytes(X);
		}
		else
		{
			x=BIG.fromBytes(X);
		}
		ECP P,T,W;
//		BIG px;
//		byte[] t=new byte[EFS];

		byte[] h=hashit(0,CLIENT_ID);
		P=mapit(h);

		T=ECP.fromBytes(TOKEN);
		if (T.is_infinity()) return INVALID_POINT;

		pin%=MAXPIN;
		W=P.pinmul(pin,PBLEN);
		T.add(W);
		if (date!=0)
		{
			W=ECP.fromBytes(PERMIT);
			if (W.is_infinity()) return INVALID_POINT;
			T.add(W);
			h=hashit(date,h);
			W=mapit(h);
			if (xID!=null)
			{
				P=PAIR.G1mul(P,x);
				P.toBytes(xID);
				W=PAIR.G1mul(W,x);
				P.add(W);
			}
			else
			{
				P.add(W);
				P=PAIR.G1mul(P,x);
			}
			if (xCID!=null) P.toBytes(xCID);
		}
		else
		{
			if (xID!=null)
			{
				P=PAIR.G1mul(P,x);
				P.toBytes(xID);
			}
		}


		T.toBytes(SEC);
		return 0;
	}

/* Extract Server Secret SST=S*Q where Q is fixed generator in G2 and S is master secret */
	public static int GET_SERVER_SECRET(byte[] S,byte[] SST)
	{
		ECP2 Q=new ECP2(new FP2(new BIG(ROM.ROM.curveDetails.getCurvePxa()),new BIG(ROM.ROM.curveDetails.getCurvePxb())),new FP2(new BIG(ROM.ROM.curveDetails.getCurvePya()),new BIG(ROM.ROM.curveDetails.getCurvePyb())));

		BIG s=BIG.fromBytes(S);
		Q=PAIR.G2mul(Q,s);
		Q.toBytes(SST);
		return 0;
	}

/*
 W=x*H(G);
 if RNG == NULL then X is passed in
 if RNG != NULL the X is passed out
 if type=0 W=x*G where G is point on the curve, else W=x*M(G), where M(G) is mapping of octet G to point on the curve
*/
	public static int GET_G1_MULTIPLE(RAND rng, int type,byte[] X,byte[] G,byte[] W)
	{
		BIG x;
		BIG r=new BIG(ROM.ROM.curveDetails.getCurveOrder());
		if (rng!=null)
		{
			x=BIG.randomnum(r,rng);
			x.toBytes(X);
		}
		else
		{
			x=BIG.fromBytes(X);
		}
		ECP P;
		if (type==0)
		{
			P=ECP.fromBytes(G);
			if (P.is_infinity()) return INVALID_POINT;
		}
		else
			P=mapit(G);

		PAIR.G1mul(P,x).toBytes(W);
		return 0;
	}

/* Client secret CST=S*H(CID) where CID is client ID and S is master secret */
/* CID is hashed externally */
	public static int GET_CLIENT_SECRET(byte[] S,byte[] CID,byte[] CST)
	{
		return GET_G1_MULTIPLE(null,1,S,CID,CST);
	}

/* Time Permit CTT=S*(date|H(CID)) where S is master secret */
	public static int GET_CLIENT_PERMIT(int date,byte[] S,byte[] CID,byte[] CTT)
	{
		byte[] h=hashit(date,CID);
		ECP P=mapit(h);

		BIG s=BIG.fromBytes(S);
		PAIR.G1mul(P,s).toBytes(CTT);
		return 0;
	}

/* Outputs H(CID) and H(T|H(CID)) for time permits. If no time permits set HID=HTID */
	public static void SERVER_1(int date,byte[] CID,byte[] HID,byte[] HTID)
	{
		byte[] h=hashit(0,CID);
		ECP R,P=mapit(h);

		if (date!=0)
		{
			if (HID!=null) P.toBytes(HID);
			h=hashit(date,h);
			R=mapit(h);
			P.add(R);
			P.toBytes(HTID);
		}
		else P.toBytes(HID);
	}

/* Implement step 2 of MPin protocol on server side */
	public static int SERVER_2(int date,byte[] HID,byte[] HTID,byte[] Y,byte[] SST,byte[] xID,byte[] xCID,byte[] mSEC,byte[] E,byte[] F)
	{
		new BIG(ROM.ROM.fieldDetails.getModulus());
		ECP2 Q=new ECP2(new FP2(new BIG(ROM.ROM.curveDetails.getCurvePxa()),new BIG(ROM.ROM.curveDetails.getCurvePxb())),new FP2(new BIG(ROM.ROM.curveDetails.getCurvePya()),new BIG(ROM.ROM.curveDetails.getCurvePyb())));
		ECP2 sQ=ECP2.fromBytes(SST);
		if (sQ.is_infinity()) return INVALID_POINT;

		ECP R;
		if (date!=0)
			R=ECP.fromBytes(xCID);
		else
		{
			if (xID==null) return BAD_PARAMS;
			R=ECP.fromBytes(xID);
		}
		if (R.is_infinity()) return INVALID_POINT;

		BIG y=BIG.fromBytes(Y);
		ECP P;
		if (date!=0) P=ECP.fromBytes(HTID);
		else
		{
			if (HID==null) return BAD_PARAMS;
			P=ECP.fromBytes(HID);
		}

		if (P.is_infinity()) return INVALID_POINT;

		P=PAIR.G1mul(P,y);
		P.add(R);
		R=ECP.fromBytes(mSEC);
		if (R.is_infinity()) return INVALID_POINT;

		FP12 g;
//		FP12 g1=new FP12(0);

		g=PAIR.ate2(Q,R,sQ,P);
		g=PAIR.fexp(g);

		if (!g.isunity())
		{
			if (HID!=null && xID!=null && E!=null && F!=null)
			{
				g.toBytes(E);
				if (date!=0)
				{
					P=ECP.fromBytes(HID);
					if (P.is_infinity()) return INVALID_POINT;
					R=ECP.fromBytes(xID);
					if (R.is_infinity()) return INVALID_POINT;

					P=PAIR.G1mul(P,y);
					P.add(R);
				}
				g=PAIR.ate(Q,P);
				g=PAIR.fexp(g);
				g.toBytes(F);
			}
			return BAD_PIN;
		}

		return 0;
	}

/* Pollards kangaroos used to return PIN error */
	public static int KANGAROO(byte[] E,byte[] F)
	{
		FP12 ge=FP12.fromBytes(E);
		FP12 gf=FP12.fromBytes(F);
		int[] distance = new int[TS];
		FP12 t=new FP12(gf);
		FP12[] table=new FP12[TS];
		int i,j,m,s,dn,dm,res,steps;

		s=1;
		for (m=0;m<TS;m++)
		{
			distance[m]=s;
			table[m]=new FP12(t);
			s*=2;
			t.usqr();
		}
		t.one();
		dn=0;
		for (j=0;j<TRAP;j++)
		{
			i=t.geta().geta().getA().lastbits(8)%TS;
			t.mul(table[i]);
			dn+=distance[i];
		}
		gf.copy(t); gf.conj();
		steps=0; dm=0;
		res=0;
		while (dm-dn<MAXPIN)
		{
			steps++;
			if (steps>4*TRAP) break;
			i=ge.geta().geta().getA().lastbits(8)%TS;
			ge.mul(table[i]);
			dm+=distance[i];
			if (ge.equals(t))
			{
				res=dm-dn;
				break;
			}
			if (ge.equals(gf))
			{
				res=dn-dm;
				break;
			}

		}
		if (steps>4*TRAP || dm-dn>=MAXPIN) {res=0; }    // Trap Failed  - probable invalid token
		return res;
	}

/* Functions to support M-Pin Full */

	public static int PRECOMPUTE(byte[] TOKEN,byte[] CID,byte[] G1,byte[] G2)
	{
		ECP P,T;
		FP12 g;

		T=ECP.fromBytes(TOKEN);
		if (T.is_infinity()) return INVALID_POINT;

		P=mapit(CID);

		ECP2 Q=new ECP2(new FP2(new BIG(ROM.ROM.curveDetails.getCurvePxa()),new BIG(ROM.ROM.curveDetails.getCurvePxb())),new FP2(new BIG(ROM.ROM.curveDetails.getCurvePya()),new BIG(ROM.ROM.curveDetails.getCurvePyb())));

		g=PAIR.ate(Q,T);
		g=PAIR.fexp(g);
		g.toBytes(G1);

		g=PAIR.ate(Q,P);
		g=PAIR.fexp(g);
		g.toBytes(G2);

		return 0;
	}

/* calculate common key on client side */
/* wCID = w.(A+AT) */
	public static int CLIENT_KEY(byte[] G1,byte[] G2,int pin,byte[] R,byte[] X,byte[] wCID,byte[] CK)
	{
		HASH H=new HASH();
		byte[] t=new byte[EFS];

		FP12 g1=FP12.fromBytes(G1);
		FP12 g2=FP12.fromBytes(G2);
		BIG z=BIG.fromBytes(R);
		BIG x=BIG.fromBytes(X);

		ECP W=ECP.fromBytes(wCID);
		if (W.is_infinity()) return INVALID_POINT;

		W=PAIR.G1mul(W,x);

		FP2 f=new FP2(new BIG(ROM.ROM.curveDetails.getCurveFra()),new BIG(ROM.ROM.curveDetails.getCurveFrb()));
		BIG r=new BIG(ROM.ROM.curveDetails.getCurveOrder());
		BIG q=new BIG(ROM.ROM.fieldDetails.getModulus());

		BIG m=new BIG(q);
		m.mod(r);

		BIG a=new BIG(z);
		a.mod(m);

		BIG b=new BIG(z);
		b.div(m);

		g2.pinpow(pin,PBLEN);
		g1.mul(g2);

		FP4 c=g1.trace();
		g2.copy(g1);
		g2.frob(f);
		FP4 cp=g2.trace();
		g1.conj();
		g2.mul(g1);
		FP4 cpm1=g2.trace();
		g2.mul(g1);
		FP4 cpm2=g2.trace();

		c=c.xtr_pow2(cp,cpm1,cpm2,a,b);

		c.geta().getA().toBytes(t);
		H.process_array(t);
		c.geta().getB().toBytes(t);
		H.process_array(t);
		c.getb().getA().toBytes(t);
		H.process_array(t);
		c.getb().getB().toBytes(t);
		H.process_array(t);

		W.getX().toBytes(t);
		H.process_array(t);
		W.getY().toBytes(t);
		H.process_array(t);

		t=H.hash();
		for (int i=0;i<PAS;i++) CK[i]=t[i];

		return 0;
	}

/* calculate common key on server side */
/* Z=r.A - no time permits involved */

	public static int SERVER_KEY(byte[] Z,byte[] SST,byte[] W,byte[] xID,byte[] xCID,byte[] SK)
	{
		HASH H=new HASH();
		byte[] t=new byte[EFS];

		ECP2 sQ=ECP2.fromBytes(SST);
		if (sQ.is_infinity()) return INVALID_POINT;
		ECP R=ECP.fromBytes(Z);
		if (R.is_infinity()) return INVALID_POINT;

		ECP U;
		if (xCID!=null)
			U=ECP.fromBytes(xCID);
		else
			U=ECP.fromBytes(xID);
		if (U.is_infinity()) return INVALID_POINT;

		BIG w=BIG.fromBytes(W);
		U=PAIR.G1mul(U,w);
		FP12 g=PAIR.ate(sQ,R);
		g=PAIR.fexp(g);

		FP4 c=g.trace();
		c.geta().getA().toBytes(t);
		H.process_array(t);
		c.geta().getB().toBytes(t);
		H.process_array(t);
		c.getb().getA().toBytes(t);
		H.process_array(t);
		c.getb().getB().toBytes(t);
		H.process_array(t);

		U.getX().toBytes(t);
		H.process_array(t);
		U.getY().toBytes(t);
		H.process_array(t);

		t=H.hash();
		for (int i=0;i<PAS;i++) SK[i]=t[i];

		return 0;
	}

/* return time since epoch */
	public static int GET_TIME() {
		Date date=new Date();
		return (int) (date.getTime()/1000);
	}

/* Generate Y = H(epoch, xCID/xID) */
        public static void GET_Y(int TimeValue,byte[] xCID,byte[] Y)
        {
          byte[] h = hashit(TimeValue,xCID);
          BIG y = BIG.fromBytes(h);
          BIG q=new BIG(ROM.ROM.curveDetails.getCurveOrder());
          y.mod(q);
          y.toBytes(Y);
        }

/* One pass MPIN Client */
        public static int CLIENT(int date,byte[] CLIENT_ID,RAND RNG,byte[] X,int pin,byte[] TOKEN,byte[] SEC,byte[] xID,byte[] xCID,byte[] PERMIT, int TimeValue, byte[] Y)
        {
          int rtn=0;

          byte[] pID;
          if (date == 0)
            pID = xID;
          else
            pID = xCID;

          rtn = CLIENT_1(date,CLIENT_ID,RNG,X,pin,TOKEN,SEC,xID,xCID,PERMIT);
          if (rtn != 0)
            return rtn;

          GET_Y(TimeValue,pID,Y);

          rtn = CLIENT_2(X,Y,SEC);
          if (rtn != 0)
            return rtn;

          return 0;
        }

/* One pass MPIN Server */
        public static int SERVER(int date,byte[] HID,byte[] HTID,byte[] Y,byte[] SST,byte[] xID,byte[] xCID,byte[] SEC,byte[] E,byte[] F,byte[] CID, int TimeValue)
        {
          int rtn=0;

          byte[] pID;
          if (date == 0)
            pID = xID;
          else
            pID = xCID;

          SERVER_1(date,CID,HID,HTID);

          GET_Y(TimeValue,pID,Y);

          rtn = SERVER_2(date,HID,HTID,Y,SST,xID,xCID,SEC,E,F);
          if (rtn != 0)
            return rtn;

          return 0;
        }

}
