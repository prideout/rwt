/*
    Copyright 2006 Johannes Kopf
	Rewritten 2015 by Philip Rideout

    Implementation of the algorithms described in:

    Recursive Wang Tiles for Real-Time Blue Noise
    Johannes Kopf, Daniel Cohen-Or, Oliver Deussen, Dani Lischinski
    In ACM Transactions on Graphics 25, 3 (Proc. SIGGRAPH 2006)

    If you use this software for research purposes, please cite
    the aforementioned paper in any resulting publication.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

extern "C" {
void par_bluenoise_create(const char * fileName);
float* par_bluenoise_generate(float x, float y, float z, int* npts);
void par_bluenoise_set_density(const unsigned char* pixels, int size);
}

#define MAX_POINTS 1024*1024

static int sqri(int a) { return a*a; };
static int maxi(int a, int b) { if (a < b) return b; else return a; };
static int mini(int a, int b) { if (a < b) return a; else return b; };
static int mini(int a, int b, int c) { if (a < b) { if(a < c) return a; else return c; } else { if (b < c) return b; else return c; } };
static int floori(float a) { return int(floor(a)); };

static int clampi(int x, int min, int max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

static int freadi(FILE * fIn)
{
	int iTemp;
	fread(&iTemp, sizeof(int), 1, fIn);
	return iTemp;
}

static float freadf(FILE * fIn)
{
	float fTemp;
	fread(&fTemp, sizeof(float), 1, fIn);
	return fTemp;
}

static void fwritef(FILE * fOut, float f)
{
	fwrite(&f, sizeof(float), 1, fOut);
}

static void fwritei(FILE * fOut, int i)
{
	fwrite(&i, sizeof(int), 1, fOut);
}

struct Vec2 {
	Vec2(const Vec2 & v) { x = v.x; y = v.y; };
	Vec2() : x(0), y(0) {};
	Vec2(float _x, float _y) : x(_x), y(_y) {};
	Vec2 operator+(const Vec2& v) const { return Vec2(x+v.x, y+v.y); };
	Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; };
	Vec2 operator-(const Vec2& v) const { return Vec2(x-v.x, y-v.y); };
	Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; };
	Vec2 operator*(float f) const { return Vec2(x*f, y*f); };
	Vec2& operator*=(float f) { x *= f; y *= f; return *this; };
	Vec2 operator/(float f) const { return this->operator*(1.0f / f); }
	Vec2& operator/=(float f) { return this->operator*=(1.0f / f); }
	float operator*(const Vec2& v) const { return x*v.x + y*v.y; };
	float operator%(const Vec2& v) const { return x*v.y - y*v.x; };
	Vec2 operator-() const { return Vec2(-x, -y); };
	Vec2& normalize() { this->operator/=(sqrtf(x*x + y*y)); return *this; };
	float magnitude() { return sqrtf(x*x + y*y); };
	float sqrMagnitude() { return x*x + y*y; };
	Vec2 perp() { return Vec2(-y, x); };
	float get(int element) const { return (&x)[element]; }
	void set(int element, float NewValue) { (&x)[element] = NewValue; }
	float x, y;
};

struct Tile {
    int n, e, s, w;
    int numSubtiles, numSubdivs, numPoints, numSubPoints;
    int ** subdivs;
    Vec2 * points, * subPoints;
};

static Vec2* points = new Vec2[MAX_POINTS];
static Tile* tiles;
static float toneScale = 200000;
static float clipMinX, clipMaxX, clipMinY, clipMaxY;
static int numTiles, numSubtiles, numSubdivs;
static float vpos[3];
static int numPoints;
static int densTexSize;
static float* densTex = 0;

static float sampleDensMap(float x, float y)
{
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
}

static void recurseTile(Tile& t, float x, float y, int level)
{
    float tileSize = 1.f/powf(float(numSubtiles), float(level));
    if ((x+tileSize < clipMinX) || (x > clipMaxX) || (y+tileSize < clipMinY) || (y > clipMaxY)) {
        return;
	}
    int numTests = mini(t.numSubPoints, int(powf(vpos[2], -2.f)/powf(float(numSubtiles), 2.f*level)*toneScale-t.numPoints));
    float factor = 1.f/powf(vpos[2], -2.f)*powf(float(numSubtiles), 2.f*level)/toneScale;
    for (int i = 0; i < numTests; i++) {
        float px = x+t.subPoints[i].x*tileSize, py = y+t.subPoints[i].y*tileSize;

        // skip point if it lies outside of the clipping window
        if ((px < clipMinX) || (px > clipMaxX) || (py < clipMinY) || (py > clipMaxY)) {
            continue;
        }

        // reject point based on its rank
        if (sampleDensMap(px, py) < (i+t.numPoints)*factor) {
            continue;
        }

        points[numPoints].x = px;
        points[numPoints].y = py;
        numPoints++;
    }

    // recursion
    if (powf(vpos[2], -2.f)/powf(float(numSubtiles), 2.f*level)*toneScale-t.numPoints > t.numSubPoints) {
        for (int ty = 0; ty < numSubtiles; ty++) {
            for (int tx = 0; tx < numSubtiles; tx++) {
				int tileIndex = t.subdivs[0][ty*numSubtiles+tx];
                Tile& subtile = tiles[tileIndex];
                recurseTile(subtile, x+tx*tileSize/numSubtiles, y+ty*tileSize/numSubtiles, level+1);
            }
        }
    }
}

float* par_bluenoise_generate(float x, float y, float z, int* npts)
{
    vpos[0] = x;
    vpos[1] = y;
    vpos[2] = z;
    clipMinX = vpos[0];
    clipMaxX = vpos[0] + vpos[2];
    clipMinY = vpos[1];
    clipMaxY = vpos[1] + vpos[2];
    numPoints = 0;
    int numTests = mini((int)tiles[0].numPoints, int(powf(vpos[2], -2.f)*toneScale));
    float factor = 1.f/powf(vpos[2], -2)/toneScale;
    for (int i = 0; i < numTests; i++) {
        float px = tiles[0].points[i].x, py = tiles[0].points[i].y;

        // skip point if it lies outside of the clipping window
        if ((px < clipMinX) || (px > clipMaxX) || (py < clipMinY) || (py > clipMaxY)) {
            continue;
        }

        // reject point based on its rank
        if (sampleDensMap(px, py) < i*factor) {
            continue;
        }

        // "draw" point
        points[numPoints].x = px;
        points[numPoints].y = py;
        numPoints++;
    }

    // recursion
    recurseTile(tiles[0], 0, 0, 0);

	*npts = numPoints;
	return &points->x;
}

void par_bluenoise_create(const char * fileName)
{
    FILE * fin = fopen(fileName, "rb");
    numTiles = freadi(fin);
    numSubtiles = freadi(fin);
    numSubdivs = freadi(fin);
    tiles = new Tile[numTiles];
    for (int i = 0; i < numTiles; i++) {
        tiles[i].n = freadi(fin);
        tiles[i].e = freadi(fin);
        tiles[i].s = freadi(fin);
        tiles[i].w = freadi(fin);
        tiles[i].subdivs = new int * [numSubdivs];
        for (int j = 0; j < numSubdivs; j++) {
            int * subdiv = new int[sqri(numSubtiles)];
            for (int k = 0; k < sqri(numSubtiles); k++) {
                subdiv[k] = freadi(fin);
			}
            tiles[i].subdivs[j] = subdiv;
        }
        tiles[i].numPoints = freadi(fin);
        tiles[i].points = new Vec2[tiles[i].numPoints];
        for (int j = 0; j < tiles[i].numPoints; j++) {
            tiles[i].points[j].x = freadf(fin);
            tiles[i].points[j].y = freadf(fin);
        }
        tiles[i].numSubPoints = freadi(fin);
        tiles[i].subPoints = new Vec2[tiles[i].numSubPoints];
        for (int j = 0; j < tiles[i].numSubPoints; j++) {
            tiles[i].subPoints[j].x = freadf(fin);
            tiles[i].subPoints[j].y = freadf(fin);
        }
    }
    fclose(fin);
}

void par_bluenoise_set_density(const unsigned char* pixels, int size)
{
    densTexSize = size;
    densTex = (float*) malloc(sqri(size) * sizeof(float));
	float scale = 1.0f / 255.0f;
    for (int i = 0; i < sqri(size); i++) {
        densTex[i] = 1 - pixels[i] * scale;
	}
}
