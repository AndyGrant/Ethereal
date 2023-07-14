With the introduction of NNUE, the cost of training networks and competing with top engines, while still making an original effort, has gone up significantly. As a result, Ethereal moved to being commercial with the release of NNUE, a move which was met by support from the community, in order to ensure I could continue to work on the project. To purchase a copy of Ethereal, you can head to [the official webpage](http://chess.grantnet.us/Ethereal), to read more.

# Ethereal

Ethereal is a UCI-compliant chess engine operating under the alpha-beta framework, paired with a Neural Network for positional evaluations. Ethereal is inspired by a number of open source projects and aims to serve as both a high-end engine and reference for other authors. The project strives to keep the source and its ideas, however complex, clean and digestible. To read more about some of the techniques used in Ethereal, see [Ethereal's Chess Programming Wiki Page](https://www.chessprogramming.org/Ethereal)

# Development

The primary testing platform for Ethereal is [OpenBench](https://github.com/AndyGrant/OpenBench), a Fishtest inspired framework. OpenBench is a simplified and generalized framework, allowing many other engines to share the same framework for testing. [The primary instance of OpenBench can be found here.](http://chess.grantnet.us/) This instance houses development for a dozen or more engines, while private instances of OpenBench exist for many other engines in the open source community.

All versions of Ethereal in this repository are considered official releases, and are given a unique version number which can be found in ``uci.c``, or by using the ``uci`` command inside the engine.

The strength of Ethereal can be tracked by following various rating lists, including [CCRL's Blitz List](https://ccrl.chessdom.com/ccrl/404/), [CCRL's 40/15 List](https://ccrl.chessdom.com/ccrl/4040/), [CCRL's FRC List](https://ccrl.chessdom.com/ccrl/404FRC/), many of [FastGM's Lists](http://www.fastgm.de/#), as well as at [CEGT](http://cegt.net/) and [SPCC](https://www.sp-cc.de/).

# A Note about the GPLv3

Ethereal, as well as the projects that support Ethereal like [OpenBench](https://github.com/AndyGrant/OpenBench) and [NNTrainer](https://github.com/AndyGrant/NNTrainer) are licensed under the GPLv3. The GPLv3 gives you, the user, the right to have access to the source code of the engine, the right to redistribute the GPLv3'ed portions of the project, as well as the right to reuse the Ethereal source in any capacity so long as you continue to comply with the GPLv3's license.

Open Source chess engines have accelerated the development of computer chess in immeasurable ways. If not for the early adopters of the Open Source methods, computer chess would not be what it is today. Powerful programs like Stockfish simply would not exist in their current forms. All of this is possible because the authors have empowered users by granting them rights to the code, only asking that you carry on propagating the licenses attached to their code. This is a small ask, for such a great gift, and yet we live in a time where that gift is not appreciated by some, and worse taken advantage of.

Ethereal shares in the collective knowledge generated and maintained by the Computer Chess Community. However, there are three elements of Ethereal for which explicit attribution is necessary for a good faith effort at carrying out the GPLv3. This attribution has been given when the code was committed, but is restated here for clarity: Ethereal makes use of the universally adopted [Syzygy Tablebases](https://github.com/syzygy1/tb), a project under the GPLv2 and other compatible licenses. Ethereal makes use of a forked version of [Fathom](https://github.com/jdart1/Fathom), a project under the MIT license, used to implement Syzygy. Lastly, Ethereal shares a chunk of code for dealing with the Windows Operating System, which was originally written by Texel author Peter Österlund, and has since been refined and improved in various Stockfish forks, once again under GPLv3 compatible licenses.

# Configuration

Ethereal supports a number of relatively standard options. Definitions and recommendations are below.
Most GUIs should support a method to set each option. If they do not, then refer to the UCI specification.

### Hash

The size of the hash table in megabytes. For analysis the more hash given the better. For testing against other engines, just be sure to give each engine the same amount of Hash. 64MB/thread/minute is generally a good value. For testing against non-classical engines, reach out to me and I will make a recommendation.

### Threads

Number of threads given to Ethereal while moving. Typically the more threads the better. There is some debate as to whether using hyper-threads provides an elo gain. I firmly believe that for Ethereal the answer is yes, and recommend all users make use of the maximum number of threads.

### MultiPV

The number of lines to output for each search iteration. For best performance, MultiPV should be left at the default value of 1 in all cases. This option should only be used for analysis.

### MoveOverhead

The time buffer when playing games under time constraints. If you notice any time losses you should increase the move overhead. Additionally when playing with Syzygy Table bases a larger than default overhead is recommended.

### SyzygyPath

Path to Syzygy table bases. Separate multiple files paths with a semicolon on Windows, and by a colon on Unix-based systems.

### SyzygyProbeDepth

Minimum depth to start probing table bases (although this depth is ignored when a position with a cardinality less than the size of the given table bases is reached). Without a strong SSD, this option may need to be increased from the default of 0. I have a SyzygyProbeDepth of 6 or 8 to be acceptable.

# Special Thanks

I would like to thank my previous instructor, Zachary Littrell, for all of his help in my endeavors. He was my Computer Science instructor for two semesters during my senior year of high school. His encouragement, mentoring, and assistance played a vital role in the development of my Computer Science skills. In addition to being a wonderful instructor, he is also an excellent friend. He provided the guidance I needed at such a crucial time in my life, allowing me to pursue Computer Science in a way I never imagined I could.


