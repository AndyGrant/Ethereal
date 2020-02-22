# Ethereal

Ethereal is a UCI-compliant chess engine which uses the alpha-beta framework. Ethereal is inspired by a number of open source projects and aims to serve as both a high-end engine and reference for other authors. The project strives to keep the source and its ideas, however complex, clean and digestible. To read more about some of the techniques used in Ethereal, see [Ethereal's Chess Programming Wiki Page](https://www.chessprogramming.org/Ethereal)

# Development

The primary testing platform for Ethereal is [OpenBench](https://github.com/AndyGrant/OpenBench) which is a Fishtest inspired framework which has been simplified and generalized so that other engines may make use of the framework with limited effort. [The primary instance of OpenBench can be found here.](http://chess.grantnet.us/)

All versions of Ethereal in this repository are considered official releases, and are given a unique version number which can be found in ``uci.c``, or by using the ``uci`` command inside the engine.

[Elo Progression on CCRL over time 40/4](http://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?family=Ethereal&print=Rating+list&print=Results+table&print=LOS+table&print=Ponder+hit+table&print=Eval+difference+table&print=Comopp+gamenum+table&print=Overlap+table&print=Score+with+common+opponents)

[Elo Progression on CCRL over time 40/40](http://www.computerchess.org.uk/ccrl/4040/cgi/compare_engines.cgi?family=Ethereal&print=Rating+list&print=Results+table&print=LOS+table&print=Ponder+hit+table&print=Eval+difference+table&print=Comopp+gamenum+table&print=Overlap+table&print=Score+with+common+opponents)

# Configuration

Ethereal supports a number of relatively standard options. Definitions and recommendations are below.
Most GUIs should support a method to set each option. If they do not, then refer to the UCI specification.

### Hash

The size of the hash table in megabytes. For analysis the more hash given the better. For testing against other engines, just be sure to give each engine the same amount of Hash. 64MB/thread/minute is generally a good value. For testing against non-classical engines, reach out to me and I will make a recommendation.

### Threads

Number of threads given to Ethereal while moving. Typically the more threads the better. There is some debate as to whether using hyper-threads provides an elo gain. I firmly believe that for Ethereal the answer is yes, and recommend all users make use of the maximum number of threads.

### MultiPV

The number of lines to output for each search iteration. For best performance, MultiPV should be left at the default value of 1 in all cases. This option should only be used for analysis.

### ContemptDrawPenalty

The number of centipawns added to the evaluation of the side to move. A positive value incentivizes preferring slightly negative evaluations to forced draws and leads to more decisive games. A small positive value is recommended in most situations.

### ContemptComplexity

The number of centipawns added to the evaluation of the side to move when all minor and major pieces are on the board, progressively reduced to zero with less pieces. A positive value incentivizes Ethereal to favor lines where the opponent cannot force simplifications as easily and leads to more decisive games. It is expected to hurt performance against significantly stronger engines, but to help against weaker engines. The optimal value for best results thus depends on conditions.

### MoveOverhead

The time buffer when playing games under time constraints. If you notice any time losses you should increase the move overhead. Additionally when playing with Syzygy Table bases a larger than default overhead is recommended.

### SyzygyPath

Path to Syzygy table bases. Separate multiple files paths with a semicolon on Windows, and by a colon on Unix-based systems.

### SyzygyProbeDepth

Minimum depth to start probing table bases (although this depth is ignored when a position with a cardinality less than the size of the given table bases is reached). Without a strong SSD, this option may need to be increased from the default of 0. I have a SyzygyProbeDepth of 6 or 8 to be acceptable.

# Special Thanks

I would like to thank my previous instructor, Zachary Littrell, for all of his help in my endeavors. He was my Computer Science instructor for two semesters during my senior year of high school. His encouragement, mentoring, and assistance played a vital role in the development of my Computer Science skills. In addition to being a wonderful instructor, he is also an excellent friend. He provided the guidance I needed at such a crucial time in my life, allowing me to pursue Computer Science in a way I never imagined I could.
