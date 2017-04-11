package rom;

public class ANSSICurve implements Curve {

	public static final int MODBITS=256; /* Number of bits in Modulus */
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
