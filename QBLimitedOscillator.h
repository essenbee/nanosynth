#pragma once

#include "Oscillator.h"

class CQBLimitedOscillator : public COscillator
{
public:
	CQBLimitedOscillator(void);
	~CQBLimitedOscillator(void);

	double doSawtooth(double dModulo, double dInc);
	double doSquare(double dModulo, double dInc);
	double doTriangle(double dModulo, double dInc, double dFo, double dSquareModulator, double* pZ_register);

	virtual void reset();
	virtual void startOscillator();
	virtual void stopOscillator();
	virtual inline double doOscillate(double* pAuxOutput = NULL)
	{
		double dOut = 0.0;

		if (!m_bNoteOn)
		{
			return dOut;
		}

		bool bWrap = checkWrapModulo();

		// Phase Modulation - for the future
		double dCalcModulo = m_dModulo + m_dPhaseMod;
		checkWrapIndex(dCalcModulo);

		switch (m_uWaveform)
		{
			case SINE:
			{
				double dAngle = m_dModulo * 2.0 * pi - pi;
				dOut = parabolicSine(dAngle);

				break;
			}
			case SAW1:
			case SAW2:
			case SAW3:
			{
				dOut = doSawtooth(dCalcModulo, m_dInc);
				break;
			}
			case SQUARE:
			{
				dOut = doSquare(dCalcModulo, m_dInc);
				break;
			}
			case TRI:
			{
				if (bWrap)
				{
					m_dDPWSquareModulator += 1.0;
				}

				dOut = doTriangle(dCalcModulo, m_dInc, m_dFo, m_dDPWSquareModulator, &m_dDPW_z1);
				break;
			}
			case NOISE:
			{
				dOut = doWhiteNoise();
				break;
			}
			case PNOISE:
			{
				dOut = doPNSequence(m_uPNRegister);
				break;
			}
		}

		incModulo();

		//Because the algorithm for triange wave requires this...
		if (m_uWaveform == TRI)
		{
			incModulo();
		}

		if (pAuxOutput)
		{
			*pAuxOutput = dOut * m_dAmplitude * m_dAmpMod;
		}

		return dOut * m_dAmplitude * m_dAmpMod;
	}
};