Fathom
======

Fathom is a stand-alone Syzygy tablebase probing tool.  The aims of Fathom
are:

* To make it easy to integrate the Syzygy tablebases into existing chess
  engines;
* To make it easy to create stand-alone applications that use the Syzygy
  tablebases;

Tool
----

Fathom includes a stand-alone command-line syzygy probing tool `fathom`.  To
probe a position, simply run the command:

    fathom --path=<path-to-TB-files> "FEN-string"

The tool will print out a PGN representation of the probe result, including:

* Result: "1-0" (white wins), "1/2-1/2" (draw), or "0-1" (black wins)
* The Win-Draw-Loss (WDL) value for the next move: "Win", "Draw", "Loss",
  "CursedWin" (win but 50-move draw) or "BlessedLoss" (loss but 50-move draw)
* The Distance-To-Zero (DTZ) value (in plys) for the next move
* WinningMoves: The list of all winning moves
* DrawingMoves: The list of all drawing moves
* LosingMoves: The list of all losing moves
* A pseudo "principle variation" of Syzygy vs. Syzygy for the input position.

For more information, run the following command:

    fathom --help

Pre-compiled versions of `fathom` (for all platforms) are available from here:

* https://github.com/basil00/Fathom/releases

Programming API
---------------

Fathom provides a simple API.  There are three main function calls:

* `tb_init` initializes the tablebase
* `tb_probe_wdl` probes the Win-Draw-Loss (WDL) table for a given position
* `tb_probe_root` probes the Distance-To-Zero (DTZ) table for the given
  position.

All of the API functions use basic integer types, i.e. there is no need to
create and initialize data-structures.  Fathom does not require the callee
to provide any additional functionality (e.g. move generation) unlike the
traditional `tbprobe` code.  However, chess engines can opt to replace some
of the functionality of Fathom for better performance (see below).

Chess Engines
-------------

Chess engines can be `tb_probe_wdl` to get the WDL value during search.  The
`tb_probe_root` functional can be used to help pick the best move at the root.
Note that `tb_probe_root` is slower and therefore should only be used at the
root.

Chess engines can opt for a tighter integration of Fathom by configuring
`tbconfig.h`.  Specifically, the chess engines can define `TB_*_ATTACKS`
macros that replace the default definitions with the engine's own definitions,
avoiding duplication of functionality.

Credits
-------

The Syzygy tablebases were created by Ronald de Man.  Much of the probing code
`tbprobe.c` is a modified version of Ronald's `tbprobe.cpp` for Stockfish (all
Stockfish-specific code has been removed).  The `tbcore.c` file is virtually
unchanged from Ronald's original version.

License
-------

(C) 2013-2015 Ronald de Man (original code)
(C) 2015 basil (new modifications)

Ronald de Man's original code can be "redistributed and/or modified without
restrictions".

The new modifications are released under the permissive MIT License:

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

