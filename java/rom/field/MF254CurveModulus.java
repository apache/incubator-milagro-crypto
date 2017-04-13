package rom.field;

public class MF254CurveModulus extends FieldDetails {

	// MF254 Modulus
	public static final int MODTYPE = MONTGOMERY_FRIENDLY;
	public static final int[] Modulus = { 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x3F80FF };
	public static final int MConst = 0x3F8100;
	
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
