/*
	Copyright 2006 Johannes Kopf (kopf@inf.uni-konstanz.de)

	Implementation of the algorithms described in the paper

	Recursive Wang Tiles for Real-Time Blue Noise
	Johannes Kopf, Daniel Cohen-Or, Oliver Deussen, Dani Lischinski
	In ACM Transactions on Graphics 25, 3 (Proc. SIGGRAPH 2006)

	If you use this software for research purposes, please cite
	the aforementioned paper in any resulting publication.

	You can find updated versions and other supplementary materials
	on our homepage:
	http://graphics.uni-konstanz.de/publications/2006/blue_noise
*/

#include "util.h"
#include "geometry.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Tile
{
	int n, e, s, w;
	int numSubtiles, numSubdivs, numPoints, numSubPoints;
	int ** subdivs;
	Vec2 * points, * subPoints;
};

float toneScale = 200000;

float clipMinX, clipMaxX, clipMinY, clipMaxY;
int numTiles, numSubtiles, numSubdivs;
Tile * tiles;
#define MAX_POINTS 1024*1024
Vec2 * points = new Vec2[MAX_POINTS];
int numPoints;
int densTexSize;
float * densTex = 0;
Vec3 vpos;

// Sample the density texture. By default we use linear filtering here.
float sampleDensMap(float x, float y)
{
#define SAMPLE_LINEAR
#ifdef SAMPLE_LINEAR
	float tx = x*densTexSize;
	float ty = y*densTexSize;
	int ix = clampi(floori(tx), 0, densTexSize-2);
	int iy = clampi(floori(ty), 0, densTexSize-2);
	tx -= ix;
	ty -= iy;
	float sample = (densTex[iy*densTexSize+ix]*(1-tx)*(1-ty) + 
					densTex[iy*densTexSize+ix+1]*tx*(1-ty) + 
					densTex[(iy+1)*densTexSize+ix]*(1-tx)*ty + 
					densTex[(iy+1)*densTexSize+ix+1]*tx*ty);
	return sample;
#else
	int ix = clampi(x*densTexSize, 0, densTexSize-1);
	int iy = clampi(y*densTexSize, 0, densTexSize-1);
	return densTex[iy*densTexSize+ix];
#endif
}

void recurseTile(Tile & t, float x, float y, int level)
{
	float tileSize = 1.f/powf(float(numSubtiles), float(level));
	if ((x+tileSize < clipMinX) || (x > clipMaxX) || (y+tileSize < clipMinY) || (y > clipMaxY))
		return;

	int numTests = mini(t.numSubPoints, int(powf(vpos.z, -2.f)/powf(float(numSubtiles), 2.f*level)*toneScale-t.numPoints));
	float factor = 1.f/powf(vpos.z, -2.f)*powf(float(numSubtiles), 2.f*level)/toneScale;

	for (int i = 0; i < numTests; i++)
	{
		float px = x+t.subPoints[i].x*tileSize, py = y+t.subPoints[i].y*tileSize;

		// skip point if it lies outside of the clipping window
		if ((px < clipMinX) || (px > clipMaxX) || (py < clipMinY) || (py > clipMaxY))
			continue;

		// reject point based on its rank
		if (sampleDensMap(px, py) < (i+t.numPoints)*factor)
			continue;

		// "draw" point
		points[numPoints].x = px;
		points[numPoints].y = py;
		numPoints++;
	}

	// recursion
	if (powf(vpos.z, -2.f)/powf(float(numSubtiles), 2.f*level)*toneScale-t.numPoints > t.numSubPoints)
	{
		for (int ty = 0; ty < numSubtiles; ty++)
			for (int tx = 0; tx < numSubtiles; tx++)
				recurseTile(tiles[t.subdivs[0][ty*numSubtiles+tx]], x+tx*tileSize/numSubtiles, y+ty*tileSize/numSubtiles, level+1);
	}
}

void paintPoints()
{
	clipMinX = vpos.x;
	clipMaxX = vpos.x+vpos.z;
	clipMinY = vpos.y;
	clipMaxY = vpos.y+vpos.z;
	numPoints = 0;

	int numTests = mini((int)tiles[0].numPoints, int(powf(vpos.z, -2.f)*toneScale));
	float factor = 1.f/powf(vpos.z, -2)/toneScale;
	for (int i = 0; i < numTests; i++)
	{
		float px = tiles[0].points[i].x, py = tiles[0].points[i].y;

		// skip point if it lies outside of the clipping window
		if ((px < clipMinX) || (px > clipMaxX) || (py < clipMinY) || (py > clipMaxY))
			continue;

		// reject point based on its rank
		if (sampleDensMap(px, py) < i*factor)
			continue;

		// "draw" point
		points[numPoints].x = px;
		points[numPoints].y = py;
		numPoints++;
	}

	// recursion
	recurseTile(tiles[0], 0, 0, 0);

	printf("%d points\n", numPoints);
}

void savePoints(const char * fileName)
{
	FILE * fout = fopen(fileName, "wb");
	fwritei(fout, numPoints);
	for (int i = 0; i < numPoints; i++)
	{
		fwritef(fout, points[i].x);
		fwritef(fout, points[i].y);
	}
	fclose(fout);
}

void loadTileSet(const char * fileName)
{
	FILE * fin = fopen(fileName, "rb");
	numTiles = freadi(fin);
	numSubtiles = freadi(fin);
	numSubdivs = freadi(fin);

	tiles = new Tile[numTiles];

	for (int i = 0; i < numTiles; i++)
	{
		tiles[i].n = freadi(fin);
		tiles[i].e = freadi(fin);
		tiles[i].s = freadi(fin);
		tiles[i].w = freadi(fin);

		tiles[i].subdivs = new int * [numSubdivs];
		for (int j = 0; j < numSubdivs; j++)
		{
			int * subdiv = new int[sqri(numSubtiles)];
			for (int k = 0; k < sqri(numSubtiles); k++)
				subdiv[k] = freadi(fin);
			tiles[i].subdivs[j] = subdiv;
		}

		tiles[i].numPoints = freadi(fin);
		tiles[i].points = new Vec2[tiles[i].numPoints];
		for (int j = 0; j < tiles[i].numPoints; j++)
		{
			tiles[i].points[j].x = freadf(fin);
			tiles[i].points[j].y = freadf(fin);
			freadi(fin);freadi(fin);freadi(fin);freadi(fin);
		}

		tiles[i].numSubPoints = freadi(fin);
		tiles[i].subPoints = new Vec2[tiles[i].numSubPoints];
		for (int j = 0; j < tiles[i].numSubPoints; j++)
		{
			tiles[i].subPoints[j].x = freadf(fin);
			tiles[i].subPoints[j].y = freadf(fin);
			freadi(fin);freadi(fin);freadi(fin);freadi(fin);
		}
	}
	fclose(fin);
}

int main()
{
	loadTileSet("data/tileset.dat");

	int dims[3];
	stbi_uc* data = stbi_load("trillium.jpg", &dims[0], &dims[1], &dims[2], 1);
	printf("%d x %d x %d\n", dims[0], dims[1], dims[2]);

	densTexSize = dims[0];
	if (densTexSize != dims[1])
		error("ERROR: only square density maps supported");
	if (dims[2] != 1)
		error("ERROR: only grayscale density maps supported");

	densTex = new float[sqri(densTexSize)];
	for (int i = 0; i < sqri(densTexSize); i++)
		densTex[i] = 1 - data[i]/255.f;
	stbi_image_free(data);

	vpos = Vec3(0, 0, 1);
	paintPoints();
	savePoints("data/output_01.points");

	vpos = Vec3(0.25f, 0.25f, 0.5f);
	paintPoints();
	savePoints("data/output_02.points");

	vpos = Vec3(0.45f, 0.45f, 0.1f);
	paintPoints();
	savePoints("data/output_03.points");
	return 0;
}
