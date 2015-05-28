import java.util.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*; 
import java.io.*;
 
public class Chess implements GameState{
	
	public static String[] Colors = {"White","Black"};
	public static String[] Names = {"Pawn","Knight","Bishop","Rook","Queen","King"};
	public static String[] gameModeLevels = new String[]{"EASY","MEDIUM","HARD","MASTER"};
	public String selectedDifficulty;

	public HashMap<String,BufferedImage> assets = new HashMap<String, BufferedImage>();
	public JChess parent;
	public Button[] buttons;
	
	public boolean gameTurn = true;
	public Integer[][] types = new Integer[8][8];
	public Boolean[][] colors = new Boolean[8][8];
	public Boolean[][] moved = new Boolean[8][8];
	
	public Integer selectedX;
	public Integer selectedY;
	public Integer oldCX;
	public Integer oldCY;
	public boolean activeGame = false;
	
	public static int EASY = 100000;
	public static int MEDIUM = 500000;
	public static int HARD = 1250000;
	public static int MASTER = 10000000;
	
	public boolean waitingOnComputer = false;
	
	public JMove lastMove;
	public ArrayList<JMove> previousMoves = new ArrayList<JMove>();
	
	public boolean online = false;
	public boolean computer = false;
	
	public ArrayList<JMove> validMoves = new ArrayList<JMove>();
	
	public Chess(JChess parent,int gameType){
		if(gameType == 2)
			computer = true;		
		
		this.parent = parent;
		for(int i = 0; i < 2; i++)
			for(int j = 0; j < 6; j++)
				assets.put((i == 0)+Names[j],ImageLoader.Load("../_Assets/ChessGameState/Pieces/"+ Colors[i]+Names[j] + ".png"));
		assets.put("option",ImageLoader.Load("../_Assets/ChessGameState/Option.png"));
		assets.put("leftPane",ImageLoader.Load("../_Assets/ChessGameState/clearSlate.png"));
		assets.put("board",ImageLoader.Load("../_Assets/ChessGameState/Board.png"));
		
		if (computer){
			for(String s :	gameModeLevels)
				assets.put(s,ImageLoader.Load("../_Assets/ChessGameState/Boards/" + s + ".png"));
			assets.put("SelectDiff",ImageLoader.Load("../_Assets/ChessGameState/SelectDiff.png"));
		}
		
		if (computer)
			buttons = new Button[8];
		else
			buttons = new Button[4];
			
		buttons[0] = new Button(parent,160,185,50,50,assets.get("true"+Names[4]),false){
			@Override
			public void onClick(){
				for (JMove m : parent.jChessEngine.validMoves)
					if (m.isPromotion && m.startX == selectedX && m.startY == selectedY && m.rookStartX == 4 && oldCX == m.endX && oldCY == m.endY){
						parent.jChessEngine.makeMove(m);
						break;
					}
				for(int i = 0; i < 4; i++)
					buttons[i].active = false;
			}
		};
		
		buttons[1] = new Button(parent,210,185,50,50,assets.get("true"+Names[3]),false){
			@Override
			public void onClick(){
				for (JMove m : parent.jChessEngine.validMoves)
					if (m.isPromotion && m.startX == selectedX && m.startY == selectedY && m.rookStartX == 3 && oldCX == m.endX && oldCY == m.endY){
						parent.jChessEngine.makeMove(m);
						break;
					}
				for(int i = 0; i < 4; i++)
					buttons[i].active = false;
					
			}
		};
		
		buttons[2] = new Button(parent,160,235,50,50,assets.get("true"+Names[2]),false){
			@Override
			public void onClick(){
				for (JMove m : parent.jChessEngine.validMoves)
					if (m.isPromotion && m.startX == selectedX && m.startY == selectedY && m.rookStartX == 2 && oldCX == m.endX && oldCY == m.endY){
						parent.jChessEngine.makeMove(m);
						break;
					}
				for(int i = 0; i < 4; i++)
					buttons[i].active = false;	
			}
		};
		
		buttons[3] = new Button(parent,210,235,50,50,assets.get("true"+Names[1]),false){
			@Override
			public void onClick(){
				for (JMove m : parent.jChessEngine.validMoves){
					if (m.isPromotion && m.startX == selectedX && m.startY == selectedY && m.rookStartX == 1 && oldCX == m.endX && oldCY == m.endY){
						parent.jChessEngine.makeMove(m);
						break;
					}
				}
				for(int i = 0; i < 4; i++)
					buttons[i].active = false;	
			}
		};
		
		if (computer){
			buttons[4] = new Button(parent,42,82+25,338,38,null,true){
				@Override
				public void onClick(){
					parent.jChessEngine.selectedDifficulty = "EASY";
					parent.jChessEngine.run();
				}
			};
			
			buttons[5] = new Button(parent,42,124+25,338,38,null,true){
				@Override
				public void onClick(){
					parent.jChessEngine.selectedDifficulty = "MEDIUM";
					parent.jChessEngine.run();
				}
			};
			
			buttons[6] = new Button(parent,42,166+25,338,38,null,true){
				@Override
				public void onClick(){
					parent.jChessEngine.selectedDifficulty = "HARD";
					parent.jChessEngine.run();
				}
			};
			
			buttons[7] = new Button(parent,42,208+25,338,38,null,true){
				@Override
				public void onClick(){
					parent.jChessEngine.selectedDifficulty = "MASTER";
					parent.jChessEngine.run();
				}
			};
		}
	}
	
	public void run(){
		
		if (computer && selectedDifficulty == null)
			return;
			
		activeGame = true;

		int[][] data = { 
			{31,11,21,41,51,21,11,31},
			{01,01,01,01,01,01,01,01},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{99,99,99,99,99,99,99,99},
			{00,00,00,00,00,00,00,00},
			{30,10,20,40,50,20,10,30},
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
		
		validMoves = JChessEngine.getAllValid(types,colors,moved,lastMove,gameTurn);
		//makeMove(null);
	}
	
	public void paint(Graphics g){
		
		if (!activeGame){
			g.drawImage(assets.get("SelectDiff"),0,25,null);
			return;
		}
		
		if (computer)
			g.drawImage(assets.get(selectedDifficulty),420,25,null);
		else
			g.drawImage(assets.get("board"),420,25,null);
			
		g.drawImage(assets.get("leftPane"),0,25,null);
		
		for(Button b : buttons)
			b.draw(g);		
		
		if (selectedX != null)
			for(JMove m: validMoves)
				if (m.startX == selectedX && m.startY == selectedY)
					g.drawImage(assets.get("option"),430+(m.endY *50),35+(m.endX*50),null);
				
		for(int x = 0; x < 8; x++)	
			for(int y = 0; y < 8; y++)
				if (types[x][y] != null)
					g.drawImage(assets.get(colors[x][y]+Names[types[x][y]]),430+(y*50),35+(x*50),null);
		
	}
	
	public void makeMove(final JMove m){
		if (waitingOnComputer)
			return;
		
		if (m != null)
			m.makeMove(types,colors,moved);
		lastMove = m;
		
		if(computer){	
			waitingOnComputer = true;
			parent.repaint();
			EventQueue.invokeLater(
				new Runnable() {
					public void run() {
						while(true){	
							try{
								Process engine = Runtime.getRuntime().exec(buildCommandLineExecuteString(lastMove));
								
								
								InputStream stdout = engine.getInputStream();
								InputStreamReader isr = new InputStreamReader(stdout);
								BufferedReader br = new BufferedReader(isr);
								
								String line = null;
								while ( (line = br.readLine()) != null)
									System.out.println(line);
								
								int AImoveIndex = engine.waitFor();
								
								
								
								if (AImoveIndex == -1){
									activeGame = false;
									System.out.println("Fatal Error");
									while (true){
										
									}
								}
								
								
								else{
									JMove AIMove = JChessEngine.getAllValid(types,colors,moved,lastMove,!gameTurn).get(AImoveIndex);
									AIMove.makeMove(types,colors,moved);
									lastMove = AIMove;
									validMoves = JChessEngine.getAllValid(types,colors,moved,lastMove,gameTurn);
								}
								
								waitingOnComputer = false;
								parent.repaint();
							} catch(Exception e){
								e.printStackTrace();
							}
							
							break;
						}
					}
				}
			);
		}
		
		else {
			gameTurn = !gameTurn;
			validMoves = JChessEngine.getAllValid(types,colors,moved,lastMove,gameTurn);
		}
		
		selectedX = null;
		selectedY = null;
		parent.repaint();
	}
	
	public String[] buildCommandLineExecuteString(JMove last_move){
		String core = "../_Core/engineFromJava.exe";
		String command = "";
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++){
				if (types[i][j] == null)
					command += "999";
				else{
					int moved_int = 1;
					if (moved[i][j])
						moved_int = 0;
					
					int color_int = 1;
					if (colors[i][j])
						color_int = 0;
					
					command += types[i][j] + "" + color_int + "" + moved_int;
				}
			}	
		}
		
		if (gameTurn)
			command += "1";
		else
			command += "0";
		
		if (last_move != null && last_move.enablesEnpass)
			command += "4" + last_move.endX + "" + last_move.endY;
		else
			command += "000";
		
		System.out.println(command);
		return new String[]{core,command};	
	}
	
	public void mousePressed(MouseEvent e){
		int x = e.getX();
		int y = e.getY();
		int cy = (x - 430) / 50;
		int cx = (y-35) / 50;
		
		if (!activeGame){
			for(int i = 0; i < 8; i++){
				if (buttons[i].clicked(x,y)){
					buttons[i].onClick();
					for(int z = 4; z < 8; z++)
						buttons[z].active = false;
					parent.repaint();
					return;
				}
			}
		}
		
		for(Button b : buttons)
			if (b.clicked(x,y))
				b.onClick();
		
		if (!activeGame)
			return;
		
		if (cx < 0 || cy < 0 || cx >= 8 || cy >= 8){
			selectedX = null;
			selectedY = null;
			parent.repaint();
			return;
		}
		
		if (selectedX == null){
			if (colors[cx][cy] != null && colors[cx][cy] == gameTurn){
				selectedX = cx;
				selectedY = cy;
			}
			parent.repaint();
			return;
		}
		
		for(final JMove m: validMoves){
			if (m.startX == selectedX && m.startY == selectedY && m.endX == cx && m.endY == cy){
				if (m.isPromotion){
					for(int i = 0; i < 4; i++)
						buttons[i].active = true;
					oldCY = cy;
					oldCX = cx;
					parent.repaint();
					return;
				}
				
				makeMove(m);
				return;
			}
		}
		
		if (colors[cx][cy] != null && colors[cx][cy] == gameTurn){
			selectedX = cx;
			selectedY = cy;
			parent.repaint();
			return;
		}
		
		parent.repaint();
	}
	
	public void keyPressed(KeyEvent e){}
	public void keyReleased(KeyEvent e){}
	public void keyTyped(KeyEvent e){}
	public void mouseDragged(MouseEvent e){}
	
}
