package rom.field;

public class BNT2CurveModulus extends FieldDetails {

	// BNT2 Curve Modulus
	public static final int MODTYPE = NOT_SPECIAL;
	public static final int[] Modulus = { 0x1460A48B, 0x596E15D, 0x1C35947A, 0x1F27C851, 0x1D00081C, 0x10079DC4, 0xAB6DD38, 0x104821EB, 0x240004 };
	public static final int MConst = 0x6505CDD;
	
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
