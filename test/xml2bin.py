#!/usr/bin/env python

import struct
import xml.etree.ElementTree as ET

INPATH = 'wangTileSet_1024spt_3cols.xml'
OUTPATH = 'wangtiles.bin'


def process(el):
    'Convert the ElementTree root into a packed binary string.'
    ncolors = int(el.find('numColors').text)
    tiles = el.find('tiles').findall('WangTile')
    ntiles = len(tiles)
    print '{} colors\n{} tiles'.format(ncolors, ntiles)
    result = struct.pack('ii', ncolors, ntiles)
    for tile in tiles:
        cols = [int(c.text) for c in tile.find('edgeColors').iter('int')]
        print cols
        result += struct.pack('bbbb', *cols)
        samples = [s for s in tile.find('distribution').iter('PoissonSample')]
        result += struct.pack('i', len(samples))
        for s in samples:
            x = float(s.find('x').text)
            y = float(s.find('y').text)
            # We'll ignore "radius" and "ranking" for now.
            result += struct.pack('ff', x, y)
    return result

if __name__ == "__main__":
    print 'Parsing...'
    root = ET.parse(INPATH)
    packed = process(root)
    print 'Done!'
    open(OUTPATH, 'wb').write(packed)
