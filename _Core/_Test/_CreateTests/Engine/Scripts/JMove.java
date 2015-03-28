public class JMove{

	public static int[] values = new int[]{100,300,300,500,900,10000};
	public int value = 0;
	
	public int startX, startY, endX, endY;
	public int rookStartX, rookStartY, rookEndX, rookEndY;
	
	public boolean savedMoved;
	public int capturedType;
	public boolean capturedColor;
	public boolean capturedMoved;
	
	public boolean isEnpass;
	public boolean isCastle;
	public boolean isPromotion;
	public boolean enablesEnpass;
	
	public void makeMove(Integer[][] t, Boolean[][] c, Boolean[][] m){}
	public void undoMove(Integer[][] t, Boolean[][] c, Boolean[][] m){}	
}

class MoveToEmpty extends JMove{
	public MoveToEmpty(int startX, int startY, int endX, int endY){
		this.startX = startX;
		this.startY = startY;
		this.endX = endX;
		this.endY = endY;
	}
	
	public MoveToEmpty(int startX, int startY, int endX, int endY, boolean enables){
		this.startX = startX;
		this.startY = startY;
		this.endX = endX;
		this.endY = endY;
		this.enablesEnpass = true;
	}
	
	public void makeMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		savedMoved = m[startX][startY];
		t[endX][endY] = t[startX][startY];
		t[startX][startY] = null;
		c[endX][endY] = c[startX][startY];
		c[startX][startY] = null;
		m[endX][endY] = true;
		m[startX][startY] = null;
	}
	
	public void undoMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[startX][startY] = t[endX][endY];
		t[endX][endY] = null;
		c[startX][startY] = c[endX][endY];
		c[endX][endY] = null;
		m[startX][startY] = savedMoved;
		m[endX][endY] = null;
	}
}

class MoveToOccupied extends JMove{	
	public MoveToOccupied(int startX, int startY, int endX, int endY){
		this.startX = startX;
		this.startY = startY;
		this.endX = endX;
		this.endY = endY;
	}
	
	public void makeMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		value = values[t[endX][endY]];
		savedMoved = m[startX][startY];
		capturedType = t[endX][endY];
		capturedColor = c[endX][endY];
		capturedMoved = m[endX][endY];
		t[endX][endY] = t[startX][startY];
		t[startX][startY] = null;
		c[endX][endY] = c[startX][startY];
		c[startX][startY] = null;
		m[endX][endY] = true;
		m[startX][startY] = null;
	}
	
	public void undoMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[startX][startY] = t[endX][endY];
		t[endX][endY] = capturedType;
		c[startX][startY] = c[endX][endY];
		c[endX][endY] = capturedColor;
		m[startX][startY] = savedMoved;
		m[endX][endY] = capturedMoved;
	}
}

class PromotionToEmpty extends JMove{	
	public PromotionToEmpty(int startX, int startY, int endX, int endY, int promotionType){
		this.isPromotion = true;
		this.startX = startX;
		this.startY = startY;
		this.endX = endX;
		this.endY = endY;
		this.rookStartX = promotionType;
	}
	
	public void makeMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		value = values[rookStartX];
		t[endX][endY] = rookStartX;
		t[startX][startY] = null;
		c[endX][endY] = c[startX][startY];
		c[startX][startY] = null;
		m[endX][endY] = true;
		m[startX][startY] = null;
	}
	
	public void undoMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[startX][startY] = 0;
		t[endX][endY] = null;
		c[startX][startY] = c[endX][endY];
		c[endX][endY] = null;
		m[startX][startY] = true;
		m[endX][endY] = null;
	}
}

class PromotionToOccupied extends JMove{
	public PromotionToOccupied(int startX, int startY, int endX, int endY, int promotionType){
		this.isPromotion = true;
		this.startX = startX;
		this.startY = startY;
		this.endX = endX;
		this.endY = endY;
		this.rookStartX = promotionType;
	}
	
	public void makeMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		value = values[rookStartX];
		value += values[t[endX][endY]];
		capturedType = t[endX][endY];
		capturedColor = c[endX][endY];
		capturedMoved = m[endX][endY];
		t[endX][endY] = rookStartX;
		t[startX][startY] = null;
		c[endX][endY] = c[startX][startY];
		c[startX][startY] = null;
		m[endX][endY] = true;
		m[startX][startY] = null;
	}
	
	public void undoMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[startX][startY] = 0;
		t[endX][endY] = capturedType;
		c[startX][startY] = c[endX][endY];
		c[endX][endY] = capturedColor;
		m[startX][startY] = true;
		m[endX][endY] = capturedMoved;
	}
}

class Castle extends JMove{
	public Castle(int startX, int startY, int endX, int endY, int rookStartX, int rookStartY, int rookEndX, int rookEndY){
		this.value = JChessAI.valueOfCastle;
		this.isCastle = true;
		this.startX = startX;
		this.startY = startY;
		this.endX = endX;
		this.endY = endY;
		this.rookStartX = rookStartX;
		this.rookStartY = rookStartY;
		this.rookEndX = rookEndX;
		this.rookEndY = rookEndY;
	}
	
	public void makeMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[endX][endY] = t[startX][startY];
		t[startX][startY] = null;
		c[endX][endY] = c[startX][startY];
		c[startX][startY] = null;
		m[endX][endY] = true;
		m[startX][startY] = null;
		
		t[rookEndX][rookEndY] = t[rookStartX][rookStartY];
		t[rookStartX][rookStartY] = null;
		c[rookEndX][rookEndY] = c[rookStartX][rookStartY];
		c[rookStartX][rookStartY] = null;
		m[rookEndX][rookEndY] = true;
		m[rookStartX][rookStartY] = null;
	}
	
	public void undoMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[startX][startY] = 5;
		t[endX][endY] = null;
		t[rookStartX][rookStartY] = 3;
		t[rookEndX][rookEndY] = null;
		
		c[startX][startY] = c[endX][endY];
		c[endX][endY] = null;
		c[rookStartX][rookStartY] = c[rookEndX][rookEndY];
		c[rookEndX][rookEndY] = null;
		
		m[startX][startY] = false;
		m[endX][endY] = null;
		m[rookStartX][rookStartY] = false;
		m[rookEndX][rookEndY] = null;
	}
}

class EnPassant extends JMove{
	public EnPassant(int startX, int startY, int endX, int endY, int passX, int passY){
		value = 100;
		this. isEnpass = true;
		this.startX = startX;
		this.startY = startY;
		this.endX = endX;
		this.endY = endY;
		this.rookStartX = passX;
		this.rookStartY = passY;
	}
	
	public void makeMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[endX][endY] = t[startX][startY];			
		t[startX][startY] = null;
		c[endX][endY] = c[startX][startY];			
		c[startX][startY] = null;
		m[endX][endY] = true;			
		m[startX][startY] = null;
		t[rookStartX][rookStartY] = null;
		c[rookStartX][rookStartY] = null;
		m[rookStartX][rookStartY] = null;
	}
	
	public void undoMove(Integer[][] t, Boolean[][] c, Boolean[][] m){
		t[startX][startY] = 0;
		c[startX][startY] = c[endX][endY];
		m[startX][startY] = true;
			
		t[endX][endY] = null;
		c[endX][endY] = null;
		m[endX][endY] = null;
			
		t[rookStartX][rookStartY]  = 0;
		c[rookStartX][rookStartY]  = !c[startX][startY];
		m[rookStartX][rookStartY]  = true;
	}
}