package rom.field;

public class BNTCurveModulus extends FieldDetails {

	// BNT Curve Modulus
	public static final int MODTYPE = NOT_SPECIAL;
	public static final int[] Modulus = { 0xEB4A713, 0x14EDDFF7, 0x1D192EAF, 0x14AAAC29, 0xD5F06E8, 0x159B4B7C, 0x53BE82E, 0x1B6CA2E0, 0x240120 };
	public static final int MConst = 0x1914C4E5;
	
	@Override
	public int getModType() {
		return MODTYPE;
	}
	@Override
	public int[] getModulus() {
		return Modulus;
	}
	@Override
	public int getMConst() {
		return MConst;
	}
	
	
}
