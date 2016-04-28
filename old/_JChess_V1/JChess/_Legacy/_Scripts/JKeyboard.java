import java.awt.event.*;

/*
	JKeyboard is used as a way to feed KeyEvents into
	the current GameState. Every JChess GameState 
	implements the methods contained in JKeyboard
*/


public class JKeyboard implements KeyListener{

	public JChess parent;
	
	public JKeyboard(JChess parent){
		this.parent = parent;
	}
	
	public void keyPressed(KeyEvent e){
		parent.gameState.keyPressed(e);
	}
	
	public void keyReleased(KeyEvent e){
		parent.gameState.keyReleased(e);
	}
	
	public void keyTyped(KeyEvent e){
		parent.gameState.keyTyped(e);
	}


}