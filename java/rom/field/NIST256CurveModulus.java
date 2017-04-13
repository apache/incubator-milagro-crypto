package rom.field;

public class NIST256CurveModulus extends FieldDetails {

	// NIST-256 Modulus
	public static final int MODTYPE = NOT_SPECIAL;
	public static final int[] Modulus = { 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FF, 0x0, 0x0, 0x40000, 0x1FE00000, 0xFFFFFF };
	public static final int MConst = 1;
	
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
