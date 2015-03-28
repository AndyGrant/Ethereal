import java.util.ArrayList;
import java.util.ArrayList;
import java.io.*;

public class CreateEngineTests{
	public static void main(String[] args){
		
		ArrayList<int[][]> boards  = new ArrayList<int[][]>();
		
		boards.add(new int[][]{
			{31,11,21,41,51,21,11,31},
			{01,01,01,01,01,01,01,99},
			{99,99,99,99,00,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,01},
			{99,99,01,99,01,01,99,99},									
			{00,99,00,00,99,00,00,00},
			{30,10,20,40,50,20,10,30}
		});
		
		boards.add(new int[][]{
			{31,11,21,41,51,21,11,31},
			{01,01,01,01,01,01,01,01},
			{99,99,99,99,99,99,99,99},
			{99,00,99,99,99,99,99,99},
			{99,99,99,99,00,99,99,99},
			{99,99,99,99,99,99,99,99},									
			{00,99,00,00,99,00,00,00},
			{30,10,20,40,50,20,10,30}
		});
		
		boards.add(new int[][]{
			{31,11,21,41,51,21,11,31},
			{01,01,01,01,01,01,01,01},
			{99,99,99,99,99,99,99,99},
			{99,00,99,99,99,99,99,99},
			{99,99,99,99,00,99,99,99},
			{99,99,99,99,99,99,99,99},									
			{00,99,00,00,99,00,00,00},
			{30,10,20,40,50,20,10,30}
		});
		
		boards.add(new int[][]{
			{31,11,21,41,51,21,11,31},
			{01,01,01,01,01,01,01,01},
			{99,99,99,99,99,99,99,99},
			{99,99,01,99,99,99,99,99},
			{99,99,99,99,00,99,99,99},
			{99,99,99,99,99,99,99,99},									
			{00,99,00,00,99,00,00,00},
			{30,10,20,40,50,20,10,30}
		});
		
		
	
		
		
		
		
		
		
		
		ArrayList<String> data = new ArrayList<String>();
		for(int[][] board : boards){
			Integer[][] types = new Integer[8][8];
			Boolean[][] colors = new Boolean[8][8];
			Boolean[][] moved = new Boolean[8][8];
			createBoard(board,types,colors,moved);
		
			boolean turn = false;
			recursiveDepth(types,colors,moved,turn,null,data,3);
		}
		
		createOutPut(data);
	}
	
	public static void recursiveDepth(Integer[][] t, Boolean[][] c, Boolean[][] m, boolean turn, JMove lastMove, ArrayList<String> data, int depth){
	
		if (depth == 0)
			return;
			
		ArrayList<JMove> temp = JChessEngine.getAllValid(t,c,m,lastMove,turn);
		data.add(createData(t,c,turn,lastMove,temp));
		
		for(JMove move : temp){
			move.makeMove(t,c,m);
			recursiveDepth(t,c,m,!turn,move,data,depth-1);
			move.undoMove(t,c,m);
		}

		return;
	}
	
	public static void createOutPut(ArrayList<String> data){
		try{
			PrintWriter writer = new PrintWriter("../../../_TestData/EngineTests.txt", "UTF-8");
			for(String info : data)
				writer.write(info);
			writer.close();
		}catch(Exception e){
			e.printStackTrace();
		}
	}
	
	public static String createData(Integer[][] t, Boolean[][] c, boolean turn, JMove lastMove,ArrayList<JMove> moves){
		String data = "";
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if (t[x][y] == null)
					data += "99";
				else
					data += t[x][y] + "" + ((c[x][y]) ? 0 : 1);
			}
		}
		
		data += (turn ? 0 : 1) + " ";
		
		data += createMoveData(lastMove);
		
		for(String moveString : createMoveStrings(moves))
			data += moveString;
		return data + "\r\n";
	}
	
	public static String[] createMoveStrings(ArrayList<JMove> moves){
		String[] data = new String[moves.size()];
		for(int i = 0; i < moves.size(); i++)
			data[i] = createMoveData(moves.get(i));
		return data;		
	}
	
	public static String createMoveData(JMove move){
			if (move == null)
				return "00 00 00 ";
			int[] data = new int[3];
			
			if (move.isCastle)
				data[0] = 1	;
			else if(move.isPromotion)
				data[0] = 2;
			else if(move.isEnpass)
				data[0] = 3;
			else if(move.enablesEnpass)
				data[0] = 4;
			else 
				data[0] = 0;
				
			data[1] = move.startX * 8 + move.startY;
			data[2] = move.endX * 8 + move.endY;
			
			String s = "";
			for(int i : data){
				if (i < 10)
					s += "0" + i + " ";
				else
					s += i + " ";	
			}
			
			return s;		
	}
	
	public static void createBoard(int[][] board, Integer[][] t, Boolean[][] c, Boolean[][] m){		
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if (board[x][y] != 99){
					t[x][y] = board[x][y] / 10;
					if (board[x][y] % 10 == 0)
						c[x][y] = true;
					else
						c[x][y] = false;
					m[x][y] = false;
				}else{
					t[x][y] = null;
					c[x][y] = null;
					m[x][y] = null;
				}
			}
		}
		
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if (t[x][y] != null && t[x][y] == 0){
					if (c[x][y] == true && x != 6)
						m[x][y] = true;
					if (c[x][y] == false && x != 1)
						m[x][y] = true;
				}
				if (t[x][y] != null && t[x][y] == 5){
					if (c[x][y] == true && x * 8 + y != 60)
						m[x][y] = true;
					if (c[x][y] == false && x * 8 + y != 4)
						m[x][y] = true;
				}
			}
		}		
	}
}

class Foo{
	public String data = "";
	public Foo(){
	}
}