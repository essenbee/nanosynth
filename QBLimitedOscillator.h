#pragma once

#include "Oscillator.h"

class CQBLimitedOscillator : public COscillator
{
public:
	CQBLimitedOscillator(void);
	~CQBLimitedOscillator(void);

	virtual void reset();
	virtual void startOscillator();
	virtual void stopOscillator();
	virtual double doOscillate(double* pQuadPhaseOutput);

	double doSawtooth(double dModulo, double dInc);
	double doSquare(double dModulo, double dInc);
	double doTriangle(double dModulo, double dInc, double dFo, double dSquareModulator, double* pZ_register);
};