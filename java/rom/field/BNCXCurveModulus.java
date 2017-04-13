package rom.field;

public class BNCXCurveModulus extends FieldDetails {
	//BNCX Curve Modulus
	public static final int MODTYPE=NOT_SPECIAL;
	public static final int[] Modulus= {0x1C1B55B3,0x13311F7A,0x24FB86F,0x1FADDC30,0x166D3243,0xFB23D31,0x836C2F7,0x10E05,0x240000};
	public static final int MConst=0x19789E85;
	
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
