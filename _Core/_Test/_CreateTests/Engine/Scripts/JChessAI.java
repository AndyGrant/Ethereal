import java.util.*;
import java.util.concurrent.*;


public class JChessAI{
	
	public static int[] values = new int[]{100,300,300,500,900,10000};
	public static int searchDepth = 1;
	public static int valueOfCastle = 10;
	public static int valueOfPawnMovement = 1;
	public static int maxDepth = 10000000;
	
	public static JMove findMove(Integer[][] t, Boolean[][] c, Boolean[][] m, JMove last, boolean turn){		
	
		if (JChessEngine.getAllValid(t,c,m,last,turn).size() == 0)
			return null;
			
		int value = valueBoard(t,c,turn);
		
		double start = System.nanoTime();
		
		System.out.println("Searching Depth Level : " + searchDepth);
		Branch root = new Branch(searchDepth,t,c,m,turn,last,true,value);
		
		
		Branch best;
		
		int increase = 0;
		while (true){
			
			best = root.getBest();
			
			if (best.value > 800000)
				break;
	
			if (root.getDepth() < maxDepth){
				increase ++;
				searchDepth ++;
				System.out.println("Searching Depth Level : " + searchDepth);
				root = new Branch(searchDepth,t,c,m,turn,last,true,value);
			}
			
			else
				break;
		}
		
		searchDepth -= increase;
		System.out.println("Combinations Found :  " + root.getDepth());
		System.out.println("Seconds Elapsed : " + (System.nanoTime() - start) / 1e9);
		System.out.println("");
		return best.move;
	}
	
	public static int valueBoard(Integer[][] t, Boolean[][] c, boolean turn){
		int value = 0;
		for(int x= 0 ; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if (t[x][y] != null)
					if (c[x][y] == turn)
						value += values[t[x][y]];
					else
						value -= values[t[x][y]];
			}
		}
		return value;
	}
}

class Branch{
	public static ExecutorService pool = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
	public static int[] values = new int[]{100,300,300,500,900,10000};
	public JMove move;
	public int value;
	public int treeSize = 0;
	public Branch[] kids;
	
	public int getDepth(){
		return treeSize;
	}
	
	public Branch getBest(){
		Branch bestKid = kids[0];
		for (Branch b : kids)
			if (b.value > bestKid.value)
				bestKid = b;
		return bestKid;
	}
	
	public Integer[][] copyint(Integer[][] t){
		Integer[][] r = new Integer[8][8];
		for(int x = 0; x < 8; x++)
			for(int y = 0; y < 8; y++)
				r[x][y] = t[x][y];
		return r;
	}
	
	public Boolean[][] copybool(Boolean[][] b){
		Boolean[][] r = new Boolean[8][8];
		for(int x = 0; x < 8; x++)
			for(int y = 0; y < 8; y++)
				r[x][y] = b[x][y];
		return r;
	}
	
	public Branch(final int depth, final Integer[][] t, final Boolean[][] c, final Boolean[][] m, final boolean turn, final JMove last, final boolean isRoot,final int value){
	
		final ArrayList<JMove> moves = JChessEngine.getAllValid(t,c,m,last,turn);
		final int size = moves.size();
		final int numOfProcessors = Runtime.getRuntime().availableProcessors();
		kids = new Branch[moves.size()];
		
		List<Future> futures = new ArrayList<>();
		for (int i = 0; i < numOfProcessors; i++){
			final int start = i;
			futures.add(pool.submit(new Runnable(){
				public void run(){
					for (int j = start; j < size; j+=numOfProcessors)
						kids[j] = new Branch(depth - 1, copyint(t), copybool(c), copybool(m), turn, moves.get(j),value);
				}
			}));
		}

		for (Future future : futures)
			try{
				future.get();
			}catch (InterruptedException | ExecutionException exception){
				System.err.println("Frick.");
				exception.printStackTrace();
				System.exit(-1);
			}
		
		for(Branch kid : kids)
			treeSize += kid.treeSize;
		treeSize += kids.length;
	}
	
	public Branch(int depth, Integer[][] t, Boolean[][] c, Boolean[][] m, boolean turn, JMove move,int oldValue){
		value = oldValue + move.value;
		
		if (depth == JChessAI.searchDepth - 1)
			this.move = move;
		move.makeMove(t,c,m);
		
		if (depth >= JChessAI.searchDepth - 2)
			value += JChessEngine.getAllValid(t,c,m,move,turn).size();
		
		if (depth > 0){
			ArrayList<JMove> moves = JChessEngine.getAllValid(t,c,m,move,!turn);
			if (moves.size() == 0){
				
				if (JChessEngine.currentlyChecked(t, c, m, !turn))
					value += 999999;
				else
					value = 0;
				
			}
			else{
				kids = new Branch[moves.size()];
				treeSize = kids.length;
				for(int i = 0; i < moves.size(); i++){					
					kids[i] = new Branch(depth - 1, t, c, m, !turn, moves.get(i),-1*value);
					treeSize += kids[i].treeSize;
				}
				
				Branch bestKid = kids[0];
				for (Branch b : kids)
					if (b.value > bestKid.value)
						bestKid = b;
				value -= bestKid.value;
				kids = null;
				
			}
		}
		
		move.undoMove(t,c,m);
	}	
}