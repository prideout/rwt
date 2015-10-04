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

#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <stdio.h>
#include <math.h>

#define PI 3.141592654f
#define FLOAT_MAX  99999999999999999.f
#define FLOAT_MIN -99999999999999999.f

typedef unsigned char ubyte;

// math quickies
inline float cotf(float a) { return cosf(a)/sinf(a); };
inline float fractf(float a) { return a - floorf(a); };
inline float sqrf(float a) { return a*a; };
inline int sqri(int a) { return a*a; };
inline int powi(int base, int exp) { return int(powf((float)base, (float)exp)); };
inline float logf(float base, float x) { return logf(x)/logf(base); };
inline int logi(int base, int x) { return int(logf((float)x)/logf((float)base)); };
inline float signf(float x) { if (x > 0) return 1; else if (x < 0) return -1; else return 0; };
inline float maxf(float a, float b) { if (a < b) return b; else return a; };
inline float minf(float a, float b) { if (a < b) return a; else return b; };
inline int maxi(int a, int b) { if (a < b) return b; else return a; };
inline int mini(int a, int b) { if (a < b) return a; else return b; };
inline int mini(int a, int b, int c) { if (a < b) { if(a < c) return a; else return c; } else { if (b < c) return b; else return c; } };
inline int absi(int a) { if (a < 0) return -a; else return a; };
inline short abss(short a) { if (a < 0) return -a; else return a; };
inline void swapi(int & a, int & b) { int t = a; a = b; b = t; };
inline int floori(float a) { return int(floor(a)); };
inline int ceili(float a) { return int(ceil(a)); };
inline float clampf(float x, float min, float max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}
inline int clampi(int x, int min, int max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

// errors
void error(const char * szErrorString, ...);
void error(char * szErrorString);
void error(const unsigned char * szErrorString);
void error(std::string sErrorString);

// file handling
unsigned char freaduc(FILE * fIn);
short freads(FILE * fIn);
unsigned short freadus(FILE * fIn);
int freadi(FILE * fIn);
unsigned int freadui(FILE * fIn);
float freadf(FILE * fIn);
double freadd(FILE * fIn);

void fwrites(FILE * fOut, short s);
void fwrited(FILE * fOut, double d);
void fwritef(FILE * fOut, float f);
void fwritei(FILE * fOut, int i);
void fwriteui(FILE * fOut, unsigned int ui);
void fwriteuc(FILE * fOut, unsigned char ui);

// time
void initTime();
float getTime();

#endif