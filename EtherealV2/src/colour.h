#ifndef COLOUR_H
#define COLOUR_H

#include "colour.h"

/* Color Definitions */
#define ColourWhite		(0)
#define ColourBlack		(1)
#define ColourNone		(2)

#define WhiteFlag		(0 << 0)
#define BlackFlag		(1 << 0)
#define NoneFlag		(1 << 1)

/* Colour Macro Definitions */
#define PIECE_COLOUR(piece)		((piece) & ColourBlack)
#define PIECE_IS_WHITE(piece)	(!PIECE_COLOUR((piece)))
#define PIECE_IS_BLACK(piece)	( PIECE_COLOUR((piece)))
#endif