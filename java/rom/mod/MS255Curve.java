package rom.mod;

public class MS255Curve implements ModCurve{

	public static final int MODBITS=255; /* Number of bits in Modulus */
	public static final int MOD8=3;  /* Modulus mod 8 */
	
	@Override
	public int getModBits() {
		return MODBITS;
	}
	@Override
	public int getMod8() {
		return MOD8;
	}
}
