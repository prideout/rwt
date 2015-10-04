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

#include <windows.h>
#include <iostream>
#include "util.h"

void error(const char * szErrorString, ...)
{
	char cpBuffer[1024];
	va_list tArgPtr;
	va_start(tArgPtr, szErrorString);
	vsprintf(cpBuffer, szErrorString, tArgPtr);

	std::cout << cpBuffer << std::endl;
	MessageBoxA(NULL, (LPCSTR)cpBuffer, (LPCSTR)"ERROR", MB_OK|MB_ICONEXCLAMATION);
	exit(0);
}

void error(char * szErrorString)
{
	error((const char *)szErrorString);
}

void error(const unsigned char * szErrorString)
{
	error((const char *)szErrorString);
}

void error(std::string sErrorString)
{
	error(sErrorString.c_str());
}

unsigned char freaduc(FILE * fIn)
{
	unsigned char ucTemp;
	fread(&ucTemp, sizeof(unsigned char), 1, fIn);
	return ucTemp;
}

short freads(FILE * fIn)
{
	short sTemp;
	fread(&sTemp, sizeof(short), 1, fIn);
	return sTemp;
}

unsigned short freadus(FILE * fIn)
{
	unsigned short usTemp;
	fread(&usTemp, sizeof(unsigned short), 1, fIn);
	return usTemp;
}

int freadi(FILE * fIn)
{
	int iTemp;
	fread(&iTemp, sizeof(int), 1, fIn);
	return iTemp;
}

unsigned int freadui(FILE * fIn)
{
	unsigned int uiTemp;
	fread(&uiTemp, sizeof(unsigned int), 1, fIn);
	return uiTemp;
}

float freadf(FILE * fIn)
{
	float fTemp;
	fread(&fTemp, sizeof(float), 1, fIn);
	return fTemp;
}

double freadd(FILE * fIn)
{
	double dTemp;
	fread(&dTemp, sizeof(double), 1, fIn);
	return dTemp;
}

void fwrites(FILE * fOut, short s)
{
	fwrite(&s, sizeof(short), 1, fOut);
}

void fwrited(FILE * fOut, double d)
{
	fwrite(&d, sizeof(double), 1, fOut);
}

void fwritef(FILE * fOut, float f)
{
	fwrite(&f, sizeof(float), 1, fOut);
}

void fwritei(FILE * fOut, int i)
{
	fwrite(&i, sizeof(int), 1, fOut);
}

void fwriteui(FILE * fOut, unsigned int ui)
{
	fwrite(&ui, sizeof(unsigned int), 1, fOut);
}

void fwriteuc(FILE * fOut, unsigned char uc)
{
	fwrite(&uc, sizeof(unsigned char), 1, fOut);
}

// time
float fTimerRateInv;			// 1 / (ticks per second)
__int64 i64StartTick;

void initTime()
{
	__int64 i64TimerRate;
	if(!QueryPerformanceFrequency((LARGE_INTEGER *) &i64TimerRate))
		error("ERROR: can't initialize timer");

	if(!i64TimerRate)
		error("ERROR: can't initialize timer");

	fTimerRateInv= 1.0f / i64TimerRate;
	if(!QueryPerformanceCounter((LARGE_INTEGER *) &i64StartTick))
		error("ERROR: can't initialize timer");
}

float getTime()
{
	__int64 i64CurTick;
	QueryPerformanceCounter((LARGE_INTEGER *) &i64CurTick);
	return (i64CurTick-i64StartTick)*fTimerRateInv;
}
