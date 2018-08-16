# Ethereal

Ethereal is a UCI-compliant chess engine. It uses the traditional alpha-beta framework in addition to a variety of pruning, reduction, extension, and other improvements.

To read more about the techniques used in Ethereal, see [Ethereal's Chess Programming Wiki Page](https://www.chessprogramming.org/Ethereal)

# Configuration

Ethereal supports a number of relatively standard options. Definitions and recommendations are below.
Additionally, most GUI's should support a method to set each option. If they do not, then refer to the UCI specification.

### Hash

The size of the hash table in megabytes. For analysis the more hash given the better. For testing against other classical engines, just be sure to give each engine the same amount of Hash. For testing against non-classical engines, reach out to me and I will make a recommendation.
  
### Threads

Number of threads given to Ethereal while moving. Typically the more threads the better. There is some debate about the value of using hyper-threading, but either way should be fine. 

### MoveOverhead

Buffer when playing games under time constraints. If you notice any time losses you should increase the move overhead. Additionally, if playing with Syzygy Table bases, a larger than default overhead is recommended.

### SyzygyPath

Path to Syzygy table bases. Separate multiple files paths with a semicolon on Windows, and by a colon on Unix-based systems.

### SyzygyProbeDepth

Minimum depth to start probing table bases (although this depth is ignored when a position with a cardinality less than the size of the given table bases is reached). Without a strong SSD, this option may need to be increased from the default of 0. I have done some of my testing on an standard hard drive, and found a Probe Depth of 8 to be acceptable.

# Development

All versions of Ethereal in this repository are considered official releases

[Progression on CCRL 40/4](http://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?family=Ethereal&print=Rating+list&print=Results+table&print=LOS+table&print=Ponder+hit+table&print=Eval+difference+table&print=Comopp+gamenum+table&print=Overlap+table&print=Score+with+common+opponents)

[Progression on CCRL 40/40](http://www.computerchess.org.uk/ccrl/4040/cgi/compare_engines.cgi?family=Ethereal&print=Rating+list&print=Results+table&print=LOS+table&print=Ponder+hit+table&print=Eval+difference+table&print=Comopp+gamenum+table&print=Overlap+table&print=Score+with+common+opponents)

# Special Thanks

I would like to thank my previous instructor, Zachary Littrell, for all of his help in my endeavors. He was my Computer Science instructor for two semesters during my senior year of high school. His encouragement, mentoring, and assistance played a vital role in the development of my Computer Science skills. In addition to being a wonderful instructor, he is also a excellent friend. He provided the guidance I needed at such a crucial time in my life, allowing me to pursue Computer Science in a way I never imagined I could.
