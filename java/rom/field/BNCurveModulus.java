package rom.field;

public class BNCurveModulus extends FieldDetails {

	// BN Curve Modulus
	public static final int MODTYPE = NOT_SPECIAL;
	public static final int[] Modulus = { 0x13, 0x18000000, 0x4E9, 0x2000000, 0x8612, 0x6C00000, 0x6E8D1, 0x10480000, 0x252364 };
	public static final int MConst = 0x179435E5;
	
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
