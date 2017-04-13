package milagro;
import milagro.rom.details.BNCXCurve;
import milagro.rom.details.CurveDetails;
import milagro.rom.field.BNCXCurveModulus;
import milagro.rom.field.FieldDetails;
import milagro.rom.mod.BNCurve;
import milagro.rom.mod.ModCurve;

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

/* Fixed Data in ROM - Field and Curve parameters */

public class ROM
{

	public final ModCurve modCurve;
	public final FieldDetails fieldDetails;
	public final CurveDetails curveDetails;
	
	
	public ROM(ModCurve modCurve, FieldDetails fieldCurve, CurveDetails curveDetails) {
		this.modCurve = modCurve;
		this.fieldDetails = fieldCurve;
		this.curveDetails = curveDetails;
	}

	public static final ROM ROM = new ROM(new BNCurve(), new BNCXCurveModulus(), new BNCXCurve());
	
/* Enter Some Field details here  */
	

/* Don't Modify from here... */
	public static final int NLEN=9;
	public static final int CHUNK=32;
	public static final int DNLEN=2*NLEN;
	public static final int BASEBITS=29;
	public static final int MASK=(((int)1<<BASEBITS)-1);
	public static final int MODBYTES=32;
	public static final int NEXCESS =((int)1<<(CHUNK-BASEBITS-1));
	public static final int FEXCESS =((int)1<<(BASEBITS*NLEN-ROM.modCurve.getModBits()));
	public static final int OMASK=(int)(-1)<<(ROM.modCurve.getModBits()%BASEBITS);
	public static final int TBITS=ROM.modCurve.getModBits()%BASEBITS; // Number of active bits in top word
	public static final int TMASK=((int)1<<TBITS)-1;
/* ...to here */


/* Finite field support - for RSA, DH etc. */
	public static final int FF_BITS=2048; /* Finite Field Size in bits - must be 256.2^n */
	public static final int FFLEN=(FF_BITS/256);
	public static final int HFLEN=(FFLEN/2);  /* Useful for half-size RSA private key operations */

	
// START SPECIFY FIELD DETAILS HERE
//*********************************************************************************
//	public static final FieldDetails FIELD_DETAILS = new BNCXCurveModulus();

// START SPECIFY CURVE DETAILS HERE
//*********************************************************************************

//	public static final CurveDetails CURVE_DETAILS = new BNCXCurve();
	
	public static final boolean USE_GLV =true;
	public static final boolean USE_GS_G2 =true;
	public static final boolean USE_GS_GT =true;
	public static final boolean GT_STRONG=true;

}
