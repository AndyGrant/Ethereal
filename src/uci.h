#ifndef UCI_H
#define UCI_H

#include <iostream>
#include <string>
#include <vector>

extern "C"{
	#include "move.h"
};

char * decode_fen(char enc[73], std::string fen);
std::string convert_move_to_string( move_t move);
void print_move_standard_notation(move_t move);
void make_move_from_string(std::string move);
bool contains(std::string a, std::string b);
std::vector<std::string> parse_moves(std::string line);

void foo();

#endif