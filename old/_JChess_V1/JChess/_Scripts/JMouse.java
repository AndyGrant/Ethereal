import java.awt.event.*;

public class JMouse extends MouseAdapter{
	public JChess parent;
	
	public JMouse(JChess parent){
		super();
		this.parent = parent;
	}
	
	public void mousePressed(MouseEvent e){
		parent.mousePressed(e);
	}	

	public void mouseDragged(MouseEvent e){
		//parent.mouseDragged(e);
	}
}