import java.util.*;

public class JChessEngine{
	public static HashMap<Integer,ArrayList<int[][]>> moveMappings = createMoveHashMap(); 
	public static ArrayList<JMove> getAllValid(Integer[][] t, Boolean[][] c, Boolean[][] m, JMove last, boolean turn){
		ArrayList<JMove> raw = new ArrayList<JMove>();
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				getOneRaw(x,y,turn,false,last,raw,t,c,m);
			}
		}
		
		
		ArrayList<JMove> valid = new ArrayList<JMove>();
		
		int[] kingCords = findKing(t,c,turn);
		for(JMove move : raw)
			if (checkMove(t,c,m,move,turn,kingCords))
				valid.add(move);
		
		validateCastles(t,valid);
		return valid;
	}
	
	public static boolean currentlyChecked(Integer[][] t, Boolean[][] c, Boolean[][] m, boolean turn){
		return !checkMove(t,c,m,null,turn,findKing(t,c,turn));
	}
	
	public static boolean checkMove(Integer[][] t, Boolean[][] c, Boolean[][] m, JMove move, boolean turn, int[] kingCords){
		ArrayList<JMove> temp = new ArrayList<JMove>();
	
		if (move != null){
			move.makeMove(t,c,m);
			if (t[move.endX][move.endY] != null && t[move.endX][move.endY] == 5)
				kingCords = findKing(t,c,turn);
			
		}
		
		int kingX = kingCords[0];
		int kingY = kingCords[1];
		
			
		for(int i = 5; i >= 0; i--){
			t[kingX][kingY] = i;
			getOneRaw(kingX,kingY,turn,true,null,temp,t,c,m);
			for(JMove newmove : temp){
				if (t[newmove.endX][newmove.endY] != null && t[newmove.endX][newmove.endY] == i && c[newmove.endX][newmove.endY] != turn){
					t[kingX][kingY] = 5;
					if (move != null)
						move.undoMove(t,c,m);
					return false;
				}
			}
			temp.clear();
		}
		
		t[kingX][kingY] = 5;
		if (move != null)
			move.undoMove(t,c,m);
		return true;
	}
		
	public static int[] findKing(Integer[][] t, Boolean[][] c,boolean turn){
		for(int x = 0; x < 8; x++)
			for(int y = 0; y < 8; y++)
				if (t[x][y] != null && t[x][y] == 5 && c[x][y] == turn)
					return new int[]{x,y};
		return null;
	}
	
	public static void validateCastles(Integer[][] t,ArrayList<JMove> raw){
		JMove leftCastle = null;
		JMove rightCastle = null;
		
		for(JMove x: raw){
			if (x.isCastle) {
				if (x.endY == 6)
					rightCastle = x;
				else
					leftCastle = x;
			}
		}
		
		if (leftCastle == null && rightCastle == null)
			return;
		
		boolean leftWorks = false;
		boolean rightWorks = false;
		
		for(JMove x: raw){
			if (t[x.startX][x.startY] == 5){
				if (x.endY == 5 && x.startX == x.endX)
					rightWorks = true;
				else if (x.endY == 3 && x.startX == x.endX)
					leftWorks = true;
			}
		}
		
		if (!leftWorks)
			raw.remove(leftCastle);
		if (!rightWorks)
			raw.remove(rightCastle);
	}
	
	public static void getOneRaw(int x, int y, boolean turn, boolean findingCheck, JMove lastmove, ArrayList<JMove> raw, Integer[][] t, Boolean[][] c, Boolean[][] m){
		if (t[x][y] == null)
			return;
		if (c[x][y] != turn)
			return;
			
		int type = t[x][y];
		boolean color = c[x][y];
		boolean hasmoved = m[x][y];
		
		int newX;
		int newY;
		
		/*
		int maxIters = 8;
		if (type == 1 || type == 5)
			maxIters = 1;
		*/
		boolean capped;
		if (type == 1 || type == 5)
			capped = true;
		else
			capped = false;
		
		int dir = 1;
		if (turn == true)
			dir = -1;
		newX = x+dir;
		
		if (!findingCheck){
			if (type == 0){
				if ( newX >= 0 && newX < 8){
					if (c[newX][y] == null){
					
						// One Forward Promotion
						if (newX == 0 || newX == 7)
							for(int newtype = 4; newtype > 0; newtype--)
								raw.add(new PromotionToEmpty(x,y,newX,y,newtype));
								
						//One Forward Non-Promotion
						else
							raw.add(new MoveToEmpty(x,y,newX,y));
						
						// Two Forward
						if (hasmoved == false && newX + dir  >= 0 && newX + dir < 8 && c[newX+dir][y] == null)
							raw.add(new MoveToEmpty(x,y,newX+dir,y,true));
					}
				
					
					
					// Enpassant
					if (lastmove != null && lastmove.enablesEnpass)
						if (lastmove.endY + 1 == y || lastmove.endY - 1 == y)
							if (lastmove.endX == x && c[x][lastmove.endY] != color)
								raw.add(new EnPassant(x,y,newX,lastmove.endY,lastmove.endX,lastmove.endY));
							
					
				}
			}
			
			if (type == 5){
				if (!hasmoved){
					if ( newX >= 0 && newX < 8){
						if (!currentlyChecked(t,c,m,turn)){
							// Castle Queen
							if (m[x][0] != null && m[x][0] == false)
								if (t[x][1] == null && t[x][2] == null && t[x][3] == null)
									raw.add(new Castle(x,y,x,y-2,x,0,x,3));
									
							// Castle King
							if (m[x][7] != null && m[x][7] == false)
								if (t[x][5] == null && t[x][6] == null)
									raw.add(new Castle(x,y,x,y+2,x,7,x,5));
						}
					}
				}		
			}
		}
		
		// Pawn captures need to always be found
		if (type == 0){
			if ( newX >= 0 && newX < 8){
				// Capture King Side
				if (y+1 < 8 && c[newX][y+1] != null && c[newX][y+1] != color){
					if (newX == 0 || newX == 7)
						for(int newtype = 4; newtype > 0; newtype--)
							raw.add(new PromotionToOccupied(x,y,newX,y+1,newtype));
					else
						raw.add(new MoveToOccupied(x,y,newX,y+1));
				}
				
				// Capture Queen Side
				if (y-1 >= 0 && c[newX][y-1] != null && c[newX][y-1] != color){
					if (newX == 0 || newX == 7)
						for(int newtype = 4; newtype > 0; newtype--)
							raw.add(new PromotionToOccupied(x,y,newX,y-1,newtype));
					else
						raw.add(new MoveToOccupied(x,y,newX,y-1));
				}
			}
		}
		
		ArrayList<int[][]> pathSets = moveMappings.get(type);
		if (capped){
			for(int[][] paths : pathSets){
				for(int[] path : paths){
					
					newX = x + (path[0]);
					newY = y + (path[1]);
					
					if (newX >= 0 && newX < 8 && newY >= 0 && newY < 8){						
						if (t[newX][newY] == null)
							raw.add(new MoveToEmpty(x,y,newX,newY));
						
						else if (c[newX][newY] != c[x][y])
							raw.add(new MoveToOccupied(x,y,newX,newY));
						
					}
				}
			}
		
		}
		
		else{
			for(int[][] paths : pathSets){
				for(int[] path : paths){
					newX = x;
					newY = y;
					while (true){
						newX +=  path[0];
						newY +=  path[1];
						if (newX < 0 || newX >= 8 || newY < 0 || newY >= 8)
							break;
							
						if (t[newX][newY] == null)
							raw.add(new MoveToEmpty(x,y,newX,newY));
						
						else if (c[newX][newY] == c[x][y])
							break;
							
						else{
							raw.add(new MoveToOccupied(x,y,newX,newY));
							break;
						}
					}
				}
			}
		}
	}
	
	public static HashMap<Integer,ArrayList<int[][]>> createMoveHashMap(){
	
		int[][] Diagnols = {{1,1},{-1,1},{-1,-1},{1,-1}};
		int[][] Straights = {{1,0},{-1,0},{0,1},{0,-1}};
		int[][] Knights = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};		
		
		HashMap<Integer,ArrayList<int[][]>> moves = new HashMap<Integer,ArrayList<int[][]>>();
		
		moves.put(0,new ArrayList<int[][]>());
		
		ArrayList<int[][]> knightmoves = new ArrayList<int[][]>();
		knightmoves.add(Knights);
		moves.put(1,knightmoves);
		
		ArrayList<int[][]> bishopmoves = new ArrayList<int[][]>();
		bishopmoves.add(Diagnols);
		moves.put(2,bishopmoves);

		ArrayList<int[][]> rookmoves = new ArrayList<int[][]>();
		rookmoves.add(Straights);
		moves.put(3,rookmoves);
		
		ArrayList<int[][]> queenmoves = new ArrayList<int[][]>();
		queenmoves.add(Diagnols);
		queenmoves.add(Straights);
		moves.put(4,queenmoves);
	
		moves.put(5,queenmoves);
		return moves;		
	}
}
