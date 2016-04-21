#!/bin/sh

scite "-position.left=0px" 		"-position.top=0px" "-position.width=960px" "-position.height=1080px" main.c board.c util.c &
scite "-position.left=960px"	"-position.top=0px" "-position.width=960px" "-position.height=1080px" move.c & 
scite "-position.left=1920px"	"-position.top=0px" "-position.width=640px" "-position.height=1024px" types.h util.h search.h move.h &
scite "-position.left=2560px"	"-position.top=0px" "-position.width=640px" "-position.height=1024px" board.h castle.h colour.h piece.h &

