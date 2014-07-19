#ifndef RECORDER_H
#define RECORDER_H

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define NUM_BUFFER		10
#define MAX_BUFF_SIZE	4096

void Initialize(int num);
void Clear();
DWORD CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, 
	DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

// header of .wav file
struct RIFF_HEADER
{
	char	szRiffID[4];	//'R','I','F','F'
	DWORD	dwRiffSize;		//总文件长度-8
	char	szRiffFormat[4];//'W','A','V','E'
};

struct WAVE_FORMAT//16字节
{
	WORD    wFormatTag;        // format type 
	WORD    nChannels;         // number of channels (i.e. mono, stereo...) 
	DWORD   nSamplesPerSec;    // sample rate 
	DWORD   nAvgBytesPerSec;   // for buffer estimation 
	WORD    nBlockAlign;       // block size of data 
	WORD    wBitsPerSample;    // Number of bits per sample of mono data 
};

struct FMT_BLOCK
{
	char		szFmtID[4]; // 'f','m','t',' '
	DWORD		dwFmtSize;	//WAVE格式所占字节16
	WAVE_FORMAT wavFormat;
};

struct DATA_BLOCK
{
	char	szDataID[4]; // 'd','a','t','a'
	DWORD	dwDataSize;
};

#endif //RECORER_H 