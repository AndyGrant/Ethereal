#ifndef UCI_H
#define UCI_H

#include <iostream
#include <string>
#include <vector>

extern "C"{
	#include "move.h"
};

std::string convert_move_to_string(board_t * board, move_t move)
void make_move_from_string(board_t * board, std::string move)
bool contains(std::string a, std::string b);
std::vector<std::string> parse_moves(std::string line);
void foo();

#endif