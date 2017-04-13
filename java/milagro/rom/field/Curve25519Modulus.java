package milagro.rom.field;

public class Curve25519Modulus extends FieldDetails {
	// Curve25519 Modulus
 	public static final int MODTYPE=PSEUDO_MERSENNE;
	public static final int[] Modulus={0x1FFFFFED,0x1FFFFFFF,0x1FFFFFFF,0x1FFFFFFF,0x1FFFFFFF,0x1FFFFFFF,0x1FFFFFFF,0x1FFFFFFF,0x7FFFFF};
	public static final int MConst=19;
	
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
