package rom.mod;

public class Curve25519 implements ModCurve{

	public static final int MODBITS=255; /* Number of bits in Modulus */
	public static final int MOD8=5;  /* Modulus mod 8 */
	
	@Override
	public int getModBits() {
		return MODBITS;
	}
	@Override
	public int getMod8() {
		return MOD8;
	}
}
