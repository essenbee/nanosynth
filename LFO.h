#pragma once

#include "Oscillator.h"

class CLFO : public COscillator
{
public:
	CLFO(void);
	~CLFO(void);

	virtual void reset();
	virtual void startOscillator();
	virtual void stopOscillator();
	virtual double doOscillate(double* pQuadPhaseOutput);
};