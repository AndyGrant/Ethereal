import java.util.*;
import java.io.*;
import javax.swing.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import java.awt.Color.*;
import javax.imageio.*;
import java.util.concurrent.*;


public class JChess extends JFrame{
	
	public int width = 420;
	public int height = 445;
	
	public HashMap<String,BufferedImage> assets = AssetLoader.loadImages();
	public Board tempBoard = new Board();
	public Integer[][] types = tempBoard.types;
	public Boolean[][] colors = tempBoard.colors;
	public Boolean[][] moved = tempBoard.moved;
	public boolean humanTurn = true;
	public ArrayList<JMove> moves = JChessEngine.getAllValid(types,colors,moved,null,humanTurn);
	
	public boolean piece_is_selected = false;
	public int selected_x;
	public int selected_y;
	
	public BufferedImage backBuffer = new BufferedImage(width,height,BufferedImage.TYPE_INT_RGB);
	
	public static String[] PIECE_COLORS = {"White","Black"};
	public static String[] PIECE_TYPES = {"Pawn","Knight","Bishop","Rook","Queen","King"};
	
	public boolean promptingPromotion = false;
	public boolean waitingForComputer = false;
	
	public static ExecutorService pool = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
	
	public static void main(String[] args){
		EventQueue.invokeLater(new Runnable() {
				@Override
				public void run() {
					JChess ex = new JChess();
					ex.run();
				}
		});
	}
	
	public JChess(){
		setupJFrame();
	}
	
	public void run(){
		
	}
	
	public void setupJFrame(){		
		setTitle("JChess Written By : Andrew Grant");
		setSize(width,height);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
		setLocationRelativeTo(null);
		setResizable(false);
		addMouseListener(new JMouse(this));
		setVisible(true);
	}
	
	public void paint(Graphics g){
		Graphics buffer = backBuffer.getGraphics();
		super.paint(buffer);
		buffer.drawImage(assets.get("board"),0,25,null);
		
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if (types[x][y] != null){
					String color = colors[x][y] == true ? "White" : "Black";
					String type = PIECE_TYPES[types[x][y]];
					buffer.drawImage(assets.get(color+type),10+y*50,35+x*50,null);
				}
			}
		}
		
		if (promptingPromotion){
			
		}
		else if(piece_is_selected)
			for(JMove m : moves)
				if (m.startX == selected_x && m.startY == selected_y)
					buffer.drawImage(assets.get("option"),10+m.endY*50,35+m.endX*50,null);
				
		
		
		buffer.dispose();
		g.drawImage(backBuffer,0,0, null);
	}	
	
	public void mousePressed(MouseEvent e){
		int x = e.getX();
		int y = e.getY();
		
		// Buttons
		
		if (waitingForComputer)
			return;
			
		if (promptingPromotion){
			return;
		}
		
		// Click is on the chess board
		if (x <= 410 && x >= 10 && y <= 425 && y >= 25){
			x = (x - 10) / 50;
			y = (y - 35) / 50;
			int t = x;
			x = y;
			y = t;
			System.out.println("Clicked " + x + " " + y);
			if (!piece_is_selected && types[x][y] != null && colors[x][y] == humanTurn){
				selected_x = x;
				selected_y = y;
				piece_is_selected = true;
				System.out.println("Selected");
				repaint();
				return;
			}
			
			if(piece_is_selected){
				for(JMove move : moves){
					if (move.startX == selected_x && move.startY == selected_y && move.endX == x && move.endY == y){
						if (!move.isPromotion){
							move.makeMove(types,colors,moved);
							waitingForComputer = true;
							piece_is_selected = false;
							repaint();
							makeAIMove(move);
							return;
						}
						else{
							System.out.println("dsda");
							promptingPromotion = true;
							repaint();
							return;
						}
					}
				}
				
				if (types[x][y] != null && colors[x][y] == humanTurn){
					selected_x = x;
					selected_y = y;
					repaint();
					return;
				}
			}
			
			piece_is_selected = false;
			
		}
	}
	
	public void makeAIMove(JMove lastMove){
		final JChess ref = this;
		pool.submit(new Runnable(){
			public void run(){
				try{
					String[] command = buildCommandLineExecuteString(lastMove);
					Process engine = Runtime.getRuntime().exec(command);
					
					InputStream stdout = engine.getInputStream();
					InputStreamReader isr = new InputStreamReader(stdout);
					BufferedReader br = new BufferedReader(isr);
					
					
					String line = null;
					while ( (line = br.readLine()) != null)
						System.out.println(line);
					 
				
					int AImoveIndex = engine.waitFor();
				
					JMove AIMove = JChessEngine.getAllValid(types,colors,moved,lastMove,!humanTurn).get(AImoveIndex);
					AIMove.makeMove(types,colors,moved);
					moves = JChessEngine.getAllValid(types,colors,moved,AIMove,humanTurn);
					
					ref.waitingForComputer = false;
					repaint();
				
				} catch(Exception e){
					e.printStackTrace();
				}
			}
		});
	}
	
	public String[] buildCommandLineExecuteString(JMove lastMove){
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
		
		if (humanTurn)
			command += "1";
		else
			command += "0";
		
		if (lastMove != null && lastMove.enablesEnpass)
			command += "4" + lastMove.endX + "" + lastMove.endY;
		else
			command += "000";
		
		System.out.println(command);
		return new String[]{core,command};	
	}
}