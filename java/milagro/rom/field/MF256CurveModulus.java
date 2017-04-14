package milagro.rom.field;

public class MF256CurveModulus extends FieldDetails {

	// MF256 Modulus
	public static final int MODTYPE = 2;
	public static final int[] Modulus = { 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0x1FFFFFFF, 0xFFA7FF };
	public static final int MConst = 0xFFA800;
	
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
