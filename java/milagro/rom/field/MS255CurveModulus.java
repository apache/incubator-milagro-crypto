package milagro.rom.field;

public class MS255CurveModulus extends FieldDetails {

	// MS255 Modulus
	public static final int MODTYPE = 1;
	public static final int[] Modulus = { 0x1FFFFD03, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x7FFFFF };
	public static final int MConst = 0x2FD;
	
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
