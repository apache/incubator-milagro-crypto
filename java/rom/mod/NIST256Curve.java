package rom.mod;

public class NIST256Curve implements ModCurve{

	public static final int MODBITS=256; /* Number of bits in Modulus */
	public static final int MOD8=7;  /* Modulus mod 8 */
	
	@Override
	public int getModBits() {
		return MODBITS;
	}
	@Override
	public int getMod8() {
		return MOD8;
	}
}
