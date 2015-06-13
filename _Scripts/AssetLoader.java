import java.util.*;
import javax.imageio.*;
import java.awt.image.*;
import java.io.*;

public class AssetLoader{
	public static HashMap<String,BufferedImage> loadImages(){
		
		HashMap<String,BufferedImage> map = new HashMap<String,BufferedImage>();
		
		String[][] toLoad = {
			{"board","Board.png"},
			{"option","Option.png"},
			{"bottombar","bottombar.png"}
		};	
		
		try{
			for(String[] n : toLoad)
				map.put(n[0],ImageIO.read(new File("../_Assets/" + n[1])));
			for(String color : JChess.PIECE_COLORS)
				for(String type : JChess.PIECE_TYPES)
					map.put(color + type, ImageIO.read(new File("../_Assets/Pieces/" + color + type + ".png")));
		}catch(Exception e){
			e.printStackTrace();
		}
		
		return map;			
	}
}