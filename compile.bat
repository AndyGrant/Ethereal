cd _Scripts
javac -d ../_Classes JChess.java
cd ../

cd _Core
gcc -o3 _Scripts/Engine.c _Scripts/Moves.c _Scripts/ChessAI.c _Scripts/TranspositionTable.c engineFromJava.c -o engineFromJava.exe
cd ../


