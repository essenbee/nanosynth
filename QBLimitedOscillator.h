#pragma once
#include "oscillator.h"

class CQBLimitedOscillator : public COscillator
{
public:
	CQBLimitedOscillator(void);
	~CQBLimitedOscillator(void);

	inline double doSawtooth(double dModulo, double dInc)
	{
		double dTrivialSaw = 0.0;
		double dOut = 0.0;

		if(m_uWaveform == SAW1)			// SAW1 = normal sawtooth (ramp)
			dTrivialSaw = unipolarToBipolar(dModulo);
		else if(m_uWaveform == SAW2)	// SAW2 = one sided wave shaper
			dTrivialSaw = 2.0*(tanh(1.5*dModulo)/tanh(1.5)) - 1.0; 
		else if(m_uWaveform == SAW3)	// SAW3 = double sided wave shaper
		{
			dTrivialSaw = unipolarToBipolar(dModulo);
			dTrivialSaw = tanh(1.5*dTrivialSaw)/tanh(1.5);
		}

		if(m_dFo <= m_dSampleRate/8.0)
		{
			dOut = dTrivialSaw + doBLEP_N(&dBLEPTable_8_BLKHAR[0], 4096, dModulo, fabs(dInc), 1.0, false, 4, false);
		}
		else
		{
			dOut = dTrivialSaw + doBLEP_N(&dBLEPTable[0], 4096, dModulo, fabs(dInc), 1.0, false, 1, true);
		}
	
		// PolyBLEP
		//dOut = dTrivialSaw + doPolyBLEP_2(dModulo, abs(dInc), 1.0, false);
										
		return dOut;
	}

	inline double doSquare(double dModulo, double dInc)
	{
		// Using sum-of-saws method
		m_uWaveform = SAW1;
		double dSaw1 = doSawtooth(dModulo, dInc);

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

		// Negative frequencies
		if (dInc < 0 && dModulo <= 0.0)
		{
			dModulo += 1.0;
		}

		double dSaw2 = doSawtooth(dModulo, dInc);

		// Mix, 180 out of phase
		double dOut = 0.5*dSaw1 - 0.5*dSaw2;
		
		// DC correction
		double dCorr = 1.0/(m_dPulseWidth/100.0);

		if ((m_dPulseWidth / 100.0) < 0.5)
		{
			dCorr = 1.0 / (1.0 - (m_dPulseWidth / 100.0));
		}

		dOut *= dCorr;
		m_uWaveform = SQUARE;

		return dOut;
	}

	inline double doTriangle(double dModulo, double dInc, double dFo, double dSquareModulator, double* pZ_register)
	{
		double dOut = 0.0;
		bool bDone = false;

		double dBipolar = unipolarToBipolar(dModulo);
		double dSq = dBipolar*dBipolar;

		double dInv = 1.0 - dSq;

		double dSqMod = dInv*dSquareModulator;

		double dDifferentiatedSqMod = dSqMod - *pZ_register;
		*pZ_register = dSqMod;

		// c = fs/[4fo(1-2fo/fs)]
		double c = m_dSampleRate/(4.0*2.0*dFo*(1-dInc));

		return dDifferentiatedSqMod*c;
	}

	
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

		// For future use with Phase Modulation
		double dCalcModulo = m_dModulo + m_dPhaseMod;
		checkWrapIndex(dCalcModulo);

		switch(m_uWaveform)
		{
			case SINE:
			{
				double dAngle = dCalcModulo*2.0*(double)pi - (double)pi;
				dOut = parabolicSine(-1.0*dAngle);
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
					m_dDPWSquareModulator *= -1.0;
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
			default:
				break;
		}


		incModulo();

		if (m_uWaveform == TRI)
		{
			incModulo();
		}

		if (pAuxOutput)
		{
			*pAuxOutput = dOut * m_dAmplitude * m_dAmpMod;
		}

		return dOut*m_dAmplitude*m_dAmpMod;
	}
};
