#pragma once

#include "QBLimitedOscillator.h"

CQBLimitedOscillator::CQBLimitedOscillator(void)
{
}

CQBLimitedOscillator::~CQBLimitedOscillator(void)
{
}

void CQBLimitedOscillator::reset()
{
	COscillator::reset();

	if (m_uWaveform == SAW1 || m_uWaveform == SAW2 || m_uWaveform == SAW3 || m_uWaveform == TRI)
	{
		m_dModulo = 0.5;
	}
}

void CQBLimitedOscillator::startOscillator()
{
	reset();

	m_bNoteOn = true;
}

void CQBLimitedOscillator::stopOscillator()
{
	m_bNoteOn = false;
}

inline double CQBLimitedOscillator::doSawtooth(double dModulo, double dInc)
{
	double dTrivialSawtooth = 0.0;
	double dOut = 0.0;

	//Normal ramp/sawtooth wave
	if (m_uWaveform == SAW1)
	{
		dTrivialSawtooth = unipolarToBipolar(dModulo);
	}
	else if (m_uWaveform == SAW2) // One-sided waveshaping with tanh
	{
		dTrivialSawtooth = 2.0 * (tanh(1.5 * dModulo)/tanh(1.5)) - 1.0;
	}
	else if (m_uWaveform == SAW3) // Dual-sided waveshaping with tanh
	{
		dTrivialSawtooth = unipolarToBipolar(dModulo);
		dTrivialSawtooth = (tanh(1.5 * dTrivialSawtooth) / tanh(1.5));
	}

	// BLEP correction using pre-calculated lookup tables
	if (m_dFo <= m_dSampleRate / 8.0)
	{
		// 4 sample points each side of discontinuity
		// Only use at frequencies < a quarter of Nyquist frequency
		dOut = dTrivialSawtooth + doBLEP_N(&dBLEPTable_8_BLKHAR[0], 4096, dModulo,
			fabs(dInc), 1.0, false, 4, false);
	}
	else
	{
		// 1 sample point each side of discontinuity
		dOut = dTrivialSawtooth + doBLEP_N(&dBLEPTable[0], 4096, dModulo,
			fabs(dInc), 1.0, false, 1, true);
	}

	// Using PolyBLEP method instead
	//dOut = dTrivialSawtooth + doPolyBLEP_2(dModulo, abs(dInc), 1.0, false);

	return dOut;
}

inline double CQBLimitedOscillator::doSquare(double dModulo, double dInc)
{
	// Generate square wave using sum-of-sawthooth waves method
	m_uWaveform == SAW1;
	double dSaw1 = doSawtooth(dModulo, dInc);

	// +ve frequencies
	if (dInc > 0)
	{
		dModulo += m_dPulseWidth / 100.0;
	}
	else
	{
		dModulo -= m_dPulseWidth / 100.0;
	}

	if (dInc > 0 && dModulo >= 1.0)
	{
		dModulo -= 1.0;
	}

	if (dInc < 0 && dModulo <= 0.0)
	{
		dModulo += 1.0;
	}

	double dSaw2 = doSawtooth(dModulo, dInc);
	double dOut = 0.5 * dSaw1 - 0.5 * dSaw2; // Mix 180 degrees out of phase

	double dCorr = 1.0 / (m_dPulseWidth / 100.0);

	if (m_dPulseWidth / 100.0 < 0.5)
	{
		dCorr = 1.0 / (1.0 - m_dPulseWidth / 100.0);
	}

	dOut *= dCorr;
	m_uWaveform = SQUARE;

	return dOut;
}

inline double CQBLimitedOscillator::doTriangle(double dModulo, double dInc, double dFo, double dSquareModulator, double* pZ_register)
{
	double dOut = 0.0;
	bool bDone = false;

	double dBipolar = unipolarToBipolar(dModulo);
	double dSq = dBipolar * dBipolar;
	double dInv = 1.0 - dSq;
	double dSqMod = dInv * dSquareModulator;

	double dDifferentiatedSqMod = dSqMod - *pZ_register;
	*pZ_register = dSqMod;

	double c = m_dSampleRate / (4.0 * 2.0 * dFo * (1 - dInc));
	dOut = dDifferentiatedSqMod * c;
	
	return dOut;
}

inline double CQBLimitedOscillator::doOscillate(double* pQuadPhaseOutput = NULL)
{
	double dOut = 0.0;

	return dOut;
}