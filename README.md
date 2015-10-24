
_Recursive Wang Tiles_ (RWT) can be used to create an infinite, well-distributed, and progressive sequence of point samples.  They were introduced by Kopf's 2006 paper, [Recursive Wang Tiles for Real-Time Blue Noise](http://github.prideout.net/rwt/Kopf2006.pdf).

To build the demo:

```
brew install cmake
cmake -H. -Bbuild
cmake --build build
```

This repo makes Kopf's original dataset available via GitHub Pages at the following URL:

http://github.prideout.net/rwt/bluenoise.bin

The above file is 6 MB and contains 8 tiles with ~2048 points in each tile.
