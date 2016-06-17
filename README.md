# Ethereal

Ethereal is a UCI compliant chess engine which uses a BitBoard board representation and an alpha beta pruning framework. Additionally, the engine uses the follow improvements to pruning, move ordering, and other time saving methods; Null move pruning, Razoring at pre-frontier and frontier nodes, Delta pruning in the quiescence search, Late move reductions in the main alpha beta algorithm, Null windows for principle variation searches, a transposition table, and finally, a killer move heuristic. This engine was originally developed to give myself a worthy opponent, but has since become a worthy opponent for other chess engines. This engine can compete with various engines in the lower rankings of the CCLR tier lists. Estimations of elo can be found at the bottom

# History

In the beginnings of my computer science education I attempted to create a graphical interface in python which would allow two players using the same computer to play chess. These very early versions were limited in their functionality and often did not completely follow the rules of chess. The code behind these versions are shameful at best, one such example being an if statement with a whopping 18 conditionals to determine if enpassant is a valid move. Eventually I began to write my chess game in Java, a language I was learning for school at the time (I despise Java!).  Initially the program was able to have two players play chess on the same computer, and eventually with web sockets two players could play together over our school network. Finally, before finishing with the Java development I managed to write a very nasty engine which could beat me and my classmates. After my time in the computer science program at my school I returned to do Research and Development, or independent work. In order to make my engine faster I taught myself C and converted the engine portion of the project (JChess at the time) to C. So the GUI was in Java and would run an executable to get the next move for the engine. Later on I scrapped the interface completely and began to work under the UCI protocol. Since then I have gone through three major versions of the engine in C. The first used a 64 size array to store the board, the second used the 256 size array, (With an implementation similar to Fruit), and finally, my current version, uses Bit Board representations. 

#To Play Against Ethereal:

  Download http://www.playwitharena.com/
    
  On the top bar select "Engines" and "Install New Engine..."
    
  Select the radio button "UCI"
    
  On the top bar select "Engines" and "Load Engine"
    
  Select Ethereal and then the "Okay"
    
  Select "File" and "New Game" to Play
  
  
# ELO Estimation

Please refer to EloTables.txt
    
    
# Special Thanks

  I would like to thank my previous instructor, Zachary Littrell, for all of his help in my endeavors. When I entered the Computer Science program he was my teacher. When I left he became one of the most important people in my life. His help, insight, and motivation made me the programmer and person I am today. He provided the guidance I needed at such a crucial time in my life, and for that I am forever thankful, whether he knows it or not.
