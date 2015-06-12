public class Board{
	public Integer[][] types = new Integer[8][8];
	public Boolean[][] colors = new Boolean[8][8];
	public Boolean[][] moved = new Boolean[8][8];
	
	public Board(){
		/*
		int[][] data = { 
			{31,11,21,41,51,21,11,31},
			{01,01,01,01,01,01,01,01},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{00,00,00,00,00,00,00,00},
			{30,10,20,40,50,20,10,30},
		};*/
		
		int[][] data = {
			{99,99,99,41,51,99,99,31},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,50,99,99,99},
		};
		
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if (data[x][y] == 99){
					types[x][y] = null;
					colors[x][y] = null;
					moved[x][y] = null;
				}else{
					types[x][y] = data[x][y] / 10;
					if (data[x][y] % 10 == 0)
						colors[x][y] = true;
					else
						colors[x][y] = false;
					moved[x][y] = false;
				}
			}
		}
	}
}