import java.util.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;

public interface GameState {
	public void paint(Graphics g);
	public void keyPressed(KeyEvent e);
	public void keyReleased(KeyEvent e);
	public void keyTyped(KeyEvent e);
	public void mousePressed(MouseEvent e);
	public void mouseDragged(MouseEvent e);
}