#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <stdio.h>
#include <stdint.h>

extern "C" {	
	#include "move.h"
	#include "board.h"	
	#include "types.h"
	#include "search.h"
	#include "movegen.h"	
};

std::string convert_move_to_string(uint16_t move){
	std::string str;
	int from = MOVE_FROM(move);
	int to = MOVE_TO(move);
	
	str += 'a' + (from%8);
	str += '1' + (from/8);
	str += 'a' + (to%8);
	str += '1' + (to/8);
    
    if (MOVE_TYPE(move) == PromotionMove){
        char arr[4] = {'k','b','r','q'};
        printf("MOVE_T %d \n",MOVE_PROMO_TYPE(move) >> 14);
        str += arr[MOVE_PROMO_TYPE(move) >> 14];
    }
	
	return str;
}

void make_move_from_string(Board * board, std::string move){
	int size = 0;
	uint16_t moves[256];
	
	gen_all_moves(board,&(moves[0]),&size);
	
	
	for (size -= 1; size >= 0; size--){
		if (move == convert_move_to_string(moves[size])){
			Undo undo[1];
			apply_move(board,moves[size],undo);
			return;
		}
	}
}

bool contains(std::string a, std::string b){
	return (a.find(b) != std::string::npos);
}

std::vector<std::string> parse_moves(std::string line){
	unsigned int start = line.find("moves")+6;
	std::vector<std::string> moves;
	std::string buff = "";
	char c;
	
	if (start -1 >= line.length())
		return moves;
	
	while(true){
		c = line[start++];
		
		if (c == ' '){
			moves.push_back(buff);
			buff = "";
		}
		else {
			buff += c;
		}
		
		if (start >= line.length()){
			moves.push_back(buff);
			break;
		}
	}
	
	return moves;
}

int main(){	
	std::string line;
	Board board;
	while(getline(std::cin,line)){		
		if (line == "uci"){
			std::cout << "id name EtherealBitBoard\n";
			std::cout << "id author Andrew Grant\n";
			std::cout << "uciok\n";
		}

		if (contains(line,"debug")){

		}

		if (line == "isready"){
			std::cout << "readyok\n";
		}

		if (contains(line,"setoption")){

		}

		if (line == "register"){
			std::cout << "register later";
		}

		if (line == "ucinewgame"){

		}

		if (contains(line,"position")){		
		
			if (contains(line,"startpos"))
				init_board(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			
			if (contains(line,"fen")){
                
                std::string fen = line.substr(line.find("fen")+4,std::string::npos);
                char cfen[fen.length()];
                
                for (unsigned int i = 0; i < fen.length(); i++)
                    cfen[i] = fen.at(i);
                
				init_board(&board,cfen);
            }

			std::vector<std::string> moves = parse_moves(line);
			for(unsigned int i = 0; i < moves.size(); i++)
				make_move_from_string(&board,moves[i]);			
		}

		if (contains(line,"go")){
			std::cout << "bestmove " << convert_move_to_string(get_best_move(&board,20,1)) << "\n";
		}

		if (line == "quit")
			break;
	}	
}