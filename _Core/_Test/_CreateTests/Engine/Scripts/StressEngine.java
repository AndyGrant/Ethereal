import java.util.*;

public class StressEngine{
	public static void main(String[] args){
		int[][] board = {
			{31,11,21,41,51,21,11,31},
			{01,01,01,01,01,01,01,01},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},									
			{00,00,00,00,00,00,00,00},
			{30,10,20,40,50,20,10,30}
		};
		
		Integer[][] t = new Integer[8][8];
		Boolean[][] c = new Boolean[8][8];
		Boolean[][] m = new Boolean[8][8];
		
		CreateEngineTests.createBoard(board,t,c,m);
		
		long start = System.currentTimeMillis();
		for(int i = 0; i < 2000000; i++){
			ArrayList<JMove> temp = JChessEngine.getAllValid(t,c,m,null,true);
		}
		
			
		long end = System.currentTimeMillis();
		
		System.out.print(((end-start) / 1000));
		
	}
}