#pragma once
#include "oscillator.h"

#define WT_LENGTH 512
#define NUM_TABLES 9

class CWTOscillator : public COscillator
{
public:
	CWTOscillator(void);
	~CWTOscillator(void);

protected:	
	// oscillator
	double m_dReadIndex;
	double m_dWT_inc;
		
	// our tables
	double m_dSineTable[WT_LENGTH];
	double* m_pSawTables[NUM_TABLES];
	double* m_pTriangleTables[NUM_TABLES];

	// for storing current table
	double* m_pCurrentTable;
	int m_nCurrentTableIndex; //0 - 9

	// correction factor table sum-of-sawtooth
	double m_dSquareCorrFactor[NUM_TABLES];

	// find the table with the proper number of harmonics for our pitch
	int getTableIndex();
	void selectTable();

	// create an destory tables
	void createWaveTables();
	void destroyWaveTables();

	// do the selected wavetable
	double doWaveTable(double& dReadIndex, double dWT_inc);

	// for square wave 
	double doSquareWave();

public:
	// --- while loop is for phase modulation
	inline void checkWrapIndex(double& dIndex)
	{
		while(dIndex < 0.0)
			dIndex += WT_LENGTH;

		while(dIndex >= WT_LENGTH)
			dIndex -= WT_LENGTH;
	}

	// typical overrides
	virtual void reset();
	virtual void startOscillator();
	virtual void stopOscillator();
	
	// render a sample
	// for LFO:		pAuxOutput = QuadPhaseOutput
	//     Pitched: pAuxOutput = Right channel (return value is left Channel
	virtual double doOscillate(double* pAuxOutput = NULL);

	// wave table specific
	virtual void setSampleRate(double dFs);
	virtual void update();
};
