import java.awt.image.*;
import java.awt.*;

public class Button{
	
	public JChess parent;
	public int x,y,dx,dy;
	public BufferedImage image;
	public boolean active;
	
	public Button(JChess parent, int x, int y, int dx, int dy,BufferedImage image, boolean active){
		this.parent = parent;
		this.x = x;
		this.y = y;
		this.dx = dx + x;
		this.dy = dy + y;
		this.image = image;
		this.active = active;
	}
	
	public boolean clicked(int x, int y){
		if (!active)
			return false;
		return x > this.x && x < this.dx && y > this.y && y < this.dy;
	}
	
	public void onClick(){
		
	}
	
	public void draw(Graphics g){
		if (image == null || !active)
			return;
		g.drawImage(image,x,y,null);
	}
}