package rom;

public class MF254Curve implements Curve {

	public static final int MODBITS=254; /* Number of bits in Modulus */
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
