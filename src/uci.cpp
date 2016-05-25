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

std::string convertMoveToString(uint16_t move){
    std::string str;
    int from = MoveFrom(move);
    int to = MoveTo(move);
    char arr[4] = {'k','b','r','q'};
    
    str += 'a' + (from%8);
    str += '1' + (from/8);
    str += 'a' + (to%8);
    str += '1' + (to/8);
    
    if (MoveType(move) == PromotionMove)
        str += arr[MovePromoType(move) >> 14];
    
    return str;
}

void applyMoveFromString(Board * board, std::string move){
    int size = 0;
    uint16_t moves[256];
    Undo undo[1];
    
    genAllMoves(board,&(moves[0]),&size);
    
    for (size -= 1; size >= 0; size--){
        if (move == convertMoveToString(moves[size])){
            applyMove(board,moves[size],undo);
            return;
        }
    }
}

bool contains(std::string a, std::string b){
    return (a.find(b) != std::string::npos);
}

std::vector<std::string> parseMoves(std::string line){
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
                initalizeBoard(&board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            
            if (contains(line,"fen")){
                
                std::string fen = line.substr(line.find("fen")+4,std::string::npos);
                char cfen[fen.length()];
                
                for (unsigned int i = 0; i < fen.length(); i++)
                    cfen[i] = fen.at(i);
                
                initalizeBoard(&board,cfen);
            }

            std::vector<std::string> moves = parseMoves(line);
            for(unsigned int i = 0; i < moves.size(); i++)
                applyMoveFromString(&board,moves[i]);           
        }

        if (contains(line,"go")){
            std::cout << "bestmove " << convertMoveToString(getBestMove(&board,8,1)) << "\n";
        }

        if (line == "quit")
            break;
    }   
}