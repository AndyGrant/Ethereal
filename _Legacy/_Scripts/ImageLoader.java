import javax.imageio.*;
import java.awt.image.*;
import java.io.*;

public class ImageLoader{
	
	public static BufferedImage Load(String s){
		try{
			return ImageIO.read(new File(s));	
		}
		catch(Exception e){
			if (JChess.debug)
				System.out.println("Unable to load file " + s);
		}
		
		return null;
	}
}