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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_POINTS 1024*1024

#include <math.h>

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

struct Vec3 {
	Vec3(const Vec3& v) { x = v.x; y = v.y; z = v.z; };
	Vec3() : x(0), y(0), z(0) {};
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};
	Vec3(Vec2 & v) { x = v.x; y = v.y; z = 1.f; };
	bool operator!=(const Vec2 & v) const { return ((x != v.x) || (y != v.y)); }
	Vec3 operator+(const Vec3& v) const { return Vec3(x+v.x, y+v.y, z+v.z); };
	Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; };
	Vec3 operator-(const Vec3& v) const { return Vec3(x-v.x, y-v.y, z-v.z); };
	Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; };
	Vec3 operator*(float f) const { return Vec3(x*f, y*f, z*f); };
	Vec3& operator*=(float f) { x *= f; y *= f; z *= f; return *this; };
	Vec3 operator/(float f) const { return this->operator*(1.0f / f); }
	Vec3& operator/=(float f) { return this->operator*=(1.0f / f); }
	float operator*(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; };
	Vec3 operator%(const Vec3 & v) const { return Vec3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); };
	Vec3 operator-() const { return Vec3(-x, -y, -z); };
	Vec3& normalize() { this->operator/=(sqrtf(x*x + y*y + z*z)); return *this; };
	float magnitude() { return sqrtf(x*x + y*y + z*z); };
	float sqrMagnitude() { return x*x + y*y + z*z; };
	float get(int element) const { return (&x)[element]; }
	void set(int element, float NewValue) { (&x)[element] = NewValue; }
	float x, y, z;
};

struct Tile {
    int n, e, s, w;
    int numSubtiles, numSubdivs, numPoints, numSubPoints;
    int ** subdivs;
    Vec2 * points, * subPoints;
};

static float toneScale = 200000;
static float clipMinX, clipMaxX, clipMinY, clipMaxY;
static int numTiles, numSubtiles, numSubdivs;
static Tile * tiles;
static Vec2 * points = new Vec2[MAX_POINTS];
static int numPoints;
static int densTexSize;
static float * densTex = 0;
static Vec3 vpos;

// Sample the density texture. By default we use linear filtering here.
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
    int numTests = mini(t.numSubPoints, int(powf(vpos.z, -2.f)/powf(float(numSubtiles), 2.f*level)*toneScale-t.numPoints));
    float factor = 1.f/powf(vpos.z, -2.f)*powf(float(numSubtiles), 2.f*level)/toneScale;
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
    if (powf(vpos.z, -2.f)/powf(float(numSubtiles), 2.f*level)*toneScale-t.numPoints > t.numSubPoints) {
        for (int ty = 0; ty < numSubtiles; ty++) {
            for (int tx = 0; tx < numSubtiles; tx++) {
				int tileIndex = t.subdivs[0][ty*numSubtiles+tx];
                Tile& subtile = tiles[tileIndex];
                recurseTile(subtile, x+tx*tileSize/numSubtiles, y+ty*tileSize/numSubtiles, level+1);
            }
        }
    }
}

static void appendPoints()
{
    clipMinX = vpos.x;
    clipMaxX = vpos.x+vpos.z;
    clipMinY = vpos.y;
    clipMaxY = vpos.y+vpos.z;
    numPoints = 0;
    int numTests = mini((int)tiles[0].numPoints, int(powf(vpos.z, -2.f)*toneScale));
    float factor = 1.f/powf(vpos.z, -2)/toneScale;
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

    printf("%d points\n", numPoints);
}

static void savePoints(const char * fileName)
{
    FILE * fout = fopen(fileName, "wb");
    fwritei(fout, numPoints);
    for (int i = 0; i < numPoints; i++) {
        fwritef(fout, points[i].x);
        fwritef(fout, points[i].y);
    }
    fclose(fout);
    printf("Generated %s\n", fileName);
}

static void drawPoints(const char* srcfile, const char* dstfile)
{
    const int size = 512;
    unsigned char* buffer = (unsigned char*) malloc(sqri(size));
    for (int i = 0; i < sqri(size); i++) {
        buffer[i] = 255;
    }
    FILE* fin = fopen(srcfile, "rb");
    int npts = freadi(fin);
    while (npts--) {
        float x = (freadf(fin) - vpos.x) / vpos.z;
        float y = (freadf(fin) - vpos.y) / vpos.z;
        int i = clampi(x * size, 0, size - 1);
        int j = clampi(y * size, 0, size - 1);
        assert(i >= 0 && i < size);
        assert(j >= 0 && j < size);
        buffer[i + j * size] = 0;
    }
    fclose(fin);
    stbi_write_png(dstfile, size, size, 1, buffer, size);
    free(buffer);
    printf("Write %s\n", dstfile);
}

static void loadTileSet(const char * fileName)
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

int main()
{
    loadTileSet("bluenoise.bin");

    int dims[3];
    stbi_uc* data = stbi_load("trillium.jpg", &dims[0], &dims[1], &dims[2], 1);
    printf("%d x %d x %d\n", dims[0], dims[1], dims[2]);

    densTexSize = dims[0];
    if (densTexSize != dims[1]) {
        printf("ERROR: only square density maps supported\n");
    }

    densTex = new float[sqri(densTexSize)];
    for (int i = 0; i < sqri(densTexSize); i++)
        densTex[i] = 1 - data[i]/255.f;
    stbi_image_free(data);

    vpos = Vec3(0, 0, 1);
    appendPoints();
    savePoints("build/output_01.pts");
    drawPoints("build/output_01.pts", "build/output_01.png");

    vpos = Vec3(0.25f, 0.25f, 0.5f);
    appendPoints();
    savePoints("build/output_02.pts");
    drawPoints("build/output_02.pts", "build/output_02.png");

    vpos = Vec3(0.45f, 0.45f, 0.1f);
    appendPoints();
    savePoints("build/output_03.pts");
    drawPoints("build/output_03.pts", "build/output_03.png");
    return 0;
}
