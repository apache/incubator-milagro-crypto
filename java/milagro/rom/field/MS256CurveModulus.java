package milagro.rom.field;

public class MS256CurveModulus extends FieldDetails {

	// MS256 Modulus
	public static final int MODTYPE = 1;
	public static final int[] Modulus = { 0x1FFFFF43, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0xFFFFFF };
	public static final int MConst = 0xBD;
	
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
