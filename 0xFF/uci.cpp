#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#ifdef _cplusplus
extern "C" {
#endif
	
	#include "types.h"
	#include "board.h"
	#include "colour.h"
	#include "move.h"
	#include "piece.h"
	#include "search.h"
	#include "util.h"
	
#ifdef _cpluspls
};
#endif

char starting_position[73] = "rnbqkbnrppppppppeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeePPPPPPPPRNBQKBNR11110000";

std::string convert_move_to_string(board_t * board, move_t move){
	int from = MOVE_GET_FROM(move);
	int to = MOVE_GET_TO(move);
	std::string str;
	
	if (MOVE_IS_NORMAL(move)){
		str += ('a' - 4 + (from%16));
		str += ('8' + 4 - (from/16));
		str += ('a' - 4 + (to%16));
		str += ('8' + 4 - (to/16));
		return str;
			
	} else if (MOVE_IS_CASTLE(move)){
		if (from == 72 	&& to == 70 ) return "e8c8";
		if (from == 72 	&& to == 74 ) return "e8g8";
		if (from == 184	&& to == 182) return "e1c1";
		if (from == 184	&& to == 186) return "e1g1";
		
	} else if (MOVE_IS_ENPASS(move)){
		
	} else if (MOVE_IS_PROMOTION(move)){
		str += ('a' - 4 + (from%16));
		str += ('8' + 4 - (from/16));
		str += ('a' - 4 + (to%16));
		str += ('8' + 4 - (to/16));
		str += piece_to_char(MOVE_GET_PROMOTE_TYPE(move,1));
		return str;
	} else {
		assert(0);
	}
	
	assert(0);
}

void print_move_standard_notation(move_t move){
	std::string str = convert_move_to_string(NULL,move);
	std::cout << str;
}

void make_move_from_string(board_t * board, std::string move){
	int size = 0;
	move_t moves[MaxMoves];
	gen_all_legal_moves(board,&(moves[0]),&size);
	
	for(size -= 1; size >= 0; size--){
		if (move == convert_move_to_string(board,moves[size])){
			apply_move(board,moves[size]);
			return;
		}
	}
}

bool contains(std::string a, std::string b){
	return (a.find(b) != std::string::npos);
}

std::vector<std::string> parse_moves(std::string line){
	std::cout << "Given : " << line << std::endl;
	int start = line.find("moves")+6;
	std::vector<std::string> moves;
	std::string buff = "";
	char c;
	
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
	board_t board;
	std::string line;

	while(getline(std::cin,line)){
		
		if (line == "uci"){
			std::cout << "id name Ethereal\n";
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
			//if (contains(line,"startpos")){
				init_board_t(&board,starting_position);
				std::vector<std::string> moves = parse_moves(line);
				for(int i = 0; i < moves.size(); i++)
					make_move_from_string(&board,moves[i]);
			//} else {
			//	std::cout << "ERROR : GIVEN FEN POSITION\n";
			//}
		}

		if (contains(line,"go")){
			std::cout << "bestmove e7e5\n";
			//std::cout << "bestmove " << convert_move_to_string(&board,get_best_move(&board,1000)) << "\n";
		}

		if (line == "quit")
			break;
	}	
}