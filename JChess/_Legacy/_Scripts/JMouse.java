import java.awt.event.*;

/*
	JMouse is used as a way to feed MouseEvents into
	the current GameState. Every JChess GameState 
	implements the methods contained in JMouse
*/

public class JMouse extends MouseAdapter{
	public JChess parent;
	
	public JMouse(JChess parent){
		super();
		this.parent = parent;
	}
	
	public void mousePressed(MouseEvent e){
		parent.gameState.mousePressed(e);
	}	

	public void mouseDragged(MouseEvent e){
		parent.gameState.mouseDragged(e);
	}
}