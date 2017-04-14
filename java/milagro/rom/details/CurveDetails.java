package milagro.rom.details;

public interface CurveDetails {

	int getCurveType();
	int getCurveA();

	int[] getCurveB();
	int[] getCurveOrder();
	int[] getCurveBnx();
	int[] getCurveCru();
	int[] getCurveFra();
	int[] getCurveFrb();
	int[] getCurvePxa();
	int[] getCurvePxb();
	int[] getCurvePya();
	int[] getCurvePyb();
	int[] getCurveGx();
	int[] getCurveGy();
	
	int[][] getCurveW();
	int[][][] getCurveSB();
	int[][] getCurveWB();
	int[][][] getCurveBB();
	
}
