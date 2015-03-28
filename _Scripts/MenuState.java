import java.util.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;

public class MenuState implements GameState{

	public HashMap<String,BufferedImage> assets = new HashMap<String, BufferedImage>();
	public JChess parent;
	public Button[] buttons;
	
	public MenuState(JChess parent){
		this.parent = parent;
		assets.put("Menu",ImageLoader.Load("../_Assets/MainMenu.png"));
		buttons = new Button[4];
		
		buttons[0] = new Button(parent,50,271,157,38,null,true){
			@Override
			public void onClick(){
				parent.jChessEngine = new Chess(parent,0);
				parent.gameState = parent.jChessEngine;
				parent.jChessEngine.run();
				parent.repaint();
			}
		};
		
		buttons[1] = new Button(parent,50,315,157,38,null,true){
			@Override
			public void onClick(){
				parent.jChessEngine = new Chess(parent,1);
				parent.gameState = parent.jChessEngine;
				parent.jChessEngine.run();
				parent.repaint();
			}
		};
		
		buttons[2] = new Button(parent,212,271,157,38,null,true){
			@Override
			public void onClick(){
			}
		};
		
		buttons[3] = new Button(parent,212,315,157,38,null,true){
			@Override
			public void onClick(){
				parent.jChessEngine = new Chess(parent,2);
				parent.gameState = parent.jChessEngine;
				parent.jChessEngine.run();
				parent.repaint();
			}
		};
	}
	
	public void paint(Graphics g){
		g.drawImage(assets.get("Menu"),0,25,null);
	}
	
	public void keyPressed(KeyEvent e){}
	public void keyReleased(KeyEvent e){}
	public void keyTyped(KeyEvent e){}
	
	public void mousePressed(MouseEvent e){
		int x = e.getX();
		int y = e.getY();
		for(Button b: buttons)
			if (b.clicked(x,y))
				b.onClick();
				
	}
	public void mouseDragged(MouseEvent e){}
}