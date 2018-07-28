# JChess


To Run:
	cd into _Compilers
	
	for Linux or Bash Enviorment run the following:
		Java_compile
		C_compile
		
	for Windows run the following:
		Windows_Java_compile.bat
		Windows_C_compile
		
	if you do not have GCC:
		open the C_compile you plan on running and use your own compiler. Run with
		all optimizations and with a 32 bit flag
		
Known Issues:

	Currently no way of determing three-fold-repition, two-fold-repition, or 50-move rule. This Can cause circular gameplay. when the AI clearly can afford to not make the 'best' move, as the 'best'
	move results in the two-fold-repition.
	
	Promotion is not included for the human player. This has not yet been a problem on chess.com 