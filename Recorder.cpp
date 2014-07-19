#include "Recorder.h"

HWAVEIN			m_hWaveIn;
WAVEFORMATEX	m_waveFormat;
WAVEHDR			m_pWaveHdr[NUM_BUFFER];
CHAR			m_cBuffer[NUM_BUFFER][MAX_BUFF_SIZE];
FILE			*fp;
float			duration;
bool			IsRecord = false;
int				fileHeaderLength;

int main(int argc, char* argv[])
/// argv[1]	Duration of every voice file.
/// argv[2] Amount of voice files.
{	
	system("mkdir voiceData");
	duration = atof(argv[1]);
	fileHeaderLength = sizeof(RIFF_HEADER)+sizeof(FMT_BLOCK)+sizeof(DATA_BLOCK);

	//m_waveFormat
	memset(&m_waveFormat, 0, sizeof(m_waveFormat));
	m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_waveFormat.nChannels = 1;
	m_waveFormat.nSamplesPerSec = 16000;
	m_waveFormat.nAvgBytesPerSec = 32000;
	m_waveFormat.nBlockAlign = 2;
	m_waveFormat.wBitsPerSample = 16;
	m_waveFormat.cbSize = 0;
	
	for (int i = 0; i < atoi(argv[2]); ++i)
	{
		Initialize(i);
		Sleep(int(duration * 1000));
		Clear();
	}
	
	system("pause");
	return 0;
}

void Initialize(int num)
{
	char fileName[256];
	sprintf(fileName, "voiceData\\voice_data_%05d.wav", num);
	fp = fopen(fileName, "wb");
	if (fp == NULL) { printf("\tERROR: Can't open file %s.\n", fileName); exit(0); }
	printf("%s\n", fileName);
	// write placeholders into file to reserve .wav header
	for (int i = 0; i < fileHeaderLength; ++i)
		fputc('*', fp);

	int numDevs = waveInGetNumDevs();
	printf("\tNumber of Devices: %d\n", numDevs);
	if (numDevs < 1) { printf("\tERROR: No wave device!\n"); exit(0); }

	MMRESULT m_res = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_waveFormat,
		(DWORD)(waveInProc), 0, CALLBACK_FUNCTION);
	if (m_res != 0) { printf("\twaveInOpen() return %d\n", m_res); exit(0); }
	
	unsigned int id;
	waveInGetID(m_hWaveIn, &id);
	printf("\tWave ID: %d\n", id);

	for (int i = 0; i < NUM_BUFFER; ++i)
	{
		m_pWaveHdr[i].lpData = m_cBuffer[i];
		m_pWaveHdr[i].dwBufferLength = MAX_BUFF_SIZE;
		m_pWaveHdr[i].dwBytesRecorded = 0;
		m_pWaveHdr[i].dwUser = i;
		m_pWaveHdr[i].dwFlags = 0;
		waveInPrepareHeader(m_hWaveIn, &m_pWaveHdr[i], sizeof(WAVEHDR));
		waveInAddBuffer(m_hWaveIn, &m_pWaveHdr[i], sizeof(WAVEHDR));
	}

	IsRecord = true;
	waveInStart(m_hWaveIn);
}

DWORD CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg,
	DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	WAVEHDR *p = NULL;
	int index = -1;
	switch (uMsg)
	{
	case WIM_DATA:
		p = (WAVEHDR*)dwParam1;
		index = p->dwUser;
		if (!IsRecord) 
		{
			waveInUnprepareHeader(m_hWaveIn, p, sizeof(WAVEHDR));
			return 0;
		}
		else
		{
			fwrite(&m_cBuffer[index], 1, p->dwBytesRecorded, fp);
			waveInUnprepareHeader(m_hWaveIn, p, sizeof(WAVEHDR));
		}
		p->lpData = m_cBuffer[index];
		p->dwBufferLength = MAX_BUFF_SIZE;
		p->dwBytesRecorded = 0;
		p->dwUser = index;
		p->dwFlags = 0;
		waveInPrepareHeader(m_hWaveIn, p, sizeof(WAVEHDR));
		waveInAddBuffer(m_hWaveIn, p, sizeof(WAVEHDR));
		break;
	case WIM_OPEN:
		printf("\tWIM_OPEN\n");
		break;
	case MIM_CLOSE:
		printf("\tWIM_CLOSE\n");
		break;
	default:
		break;
	}//switch (uMsg)

	return 0;
}

void Clear()
{
	IsRecord = false;
	Sleep(3000);
	printf("\twaveInStop()\t");
	waveInStop(m_hWaveIn);
	printf("\twaveInReset()\t");
	waveInReset(m_hWaveIn);
	printf("\twaveInClose()\n");
	waveInClose(m_hWaveIn);

	long fileSize = 0;
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp) - fileHeaderLength;
	fseek(fp, 0, SEEK_SET);
	RIFF_HEADER m_riff = { 0 };
	FMT_BLOCK m_fmt = { 0 };
	DATA_BLOCK m_data = { 0 };
	//RIFF
	strncpy(m_riff.szRiffID, "RIFF", 4);
	m_riff.dwRiffSize = 4 + sizeof(FMT_BLOCK)+sizeof(DATA_BLOCK)+fileSize;
	strncpy(m_riff.szRiffFormat, "WAVE", 4);
	//fmt
	strncpy(m_fmt.szFmtID, "fmt ", 4);
	m_fmt.dwFmtSize = sizeof(WAVE_FORMAT);
	m_fmt.wavFormat = *(WAVE_FORMAT*)&m_waveFormat;
	//Data
	strncpy(m_data.szDataID, "data", 4);
	m_data.dwDataSize = fileSize;

	fwrite(&m_riff, 1, sizeof(RIFF_HEADER), fp);
	fwrite(&m_fmt, 1, sizeof(FMT_BLOCK), fp);
	fwrite(&m_data, 1, sizeof(DATA_BLOCK), fp);

	fclose(fp);
}