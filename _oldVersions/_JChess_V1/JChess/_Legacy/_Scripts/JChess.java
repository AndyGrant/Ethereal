import java.util.*;
import java.io.*;
import javax.swing.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import java.awt.Color.*;
import javax.imageio.*;

public class JChess extends JFrame{
	
	public static boolean debug = true;
	
	public int width = 840;
	public int height = 445;
	
	public MenuState menuState;
	public Chess jChessEngine;
	public GameState gameState;
	
	public BufferedImage backBuffer = new BufferedImage(width,height,BufferedImage.TYPE_INT_RGB);
	
	public static void main(String[] args){
		EventQueue.invokeLater(new Runnable() {
				@Override
				public void run() {
					JChess ex = new JChess();
				}
			}
		);
	}
	
	public JChess(){
		setupJFrame();
		setupGameStates();
	}
	
	public void setupJFrame(){		
		setTitle("JChess Written By : Andrew Grant");
		setSize(width,height);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
		setLocationRelativeTo(null);
		setResizable(false);
		addMouseListener(new JMouse(this));
		addKeyListener(new JKeyboard(this));
		setVisible(true);
	}
	
	public void setupGameStates(){
		menuState = new MenuState(this);
		gameState = menuState;		
	}
	
	public void paint(Graphics g){
		Graphics buffer = backBuffer.getGraphics();
		super.paint(buffer);	
		gameState.paint(buffer);
		buffer.dispose();
		g.drawImage(backBuffer,0,0, null);
	}	
}
