#pragma once
#include "oscillator.h"

class CLFO : public COscillator
{
public:
	CLFO(void);
	~CLFO(void);
	
	// virtual overrides
	virtual void reset();
	virtual void startOscillator();
	virtual void stopOscillator();

	inline virtual double doOscillate(double* pQuadPhaseOutput = NULL)
	{
		if(!m_bNoteOn)
		{
			if (pQuadPhaseOutput)
			{
				*pQuadPhaseOutput = 0.0;
			}

			return 0.0;
		}

		double dOut = 0.0;
		double dQPOut = 0.0;
			
		bool bWrap =  checkWrapModulo();

		if(m_uLFOMode == shot && bWrap)
		{
			m_bNoteOn = false;
					
			if(pQuadPhaseOutput)
				*pQuadPhaseOutput = 0.0;

			return 0.0;
		}

		double dQuadModulo = m_dModulo + 0.25;

		if (dQuadModulo >= 1.0)
		{
			dQuadModulo -= 1.0;
		}

		switch(m_uWaveform)
		{
			case sine:
			{
				double dAngle = m_dModulo*2.0*pi - pi;
				dOut = parabolicSine(-dAngle);	
				dAngle = dQuadModulo*2.0*pi - pi;
				dQPOut = parabolicSine(-dAngle);
				break;
			}

			case usaw:
			case dsaw:
			{	
				if(m_uLFOMode != shot)
				{
					dOut = unipolarToBipolar(m_dModulo);
					dQPOut = unipolarToBipolar(dQuadModulo);
				}
				else
				{
					dOut = m_dModulo - 1.0;
					dQPOut = dQuadModulo - 1.0;
				}

				if(m_uWaveform == dsaw)
				{
					dOut *= -1.0;
					dQPOut *= -1.0;
				}

				break;
			}

			case square:
			{
				dOut = m_dModulo > m_dPulseWidth/100.0 ? -1.0 : +1.0;
				dQPOut = dQuadModulo > m_dPulseWidth/100.0 ? -1.0 : +1.0;

				break;
			}

			case tri:
			{
				dOut = unipolarToBipolar(m_dModulo);
				dOut = 2.0*fabs(dOut) - 1.0;

				if (m_uLFOMode == shot)
				{
					dOut = bipolarToUnipolar(dOut);
				}

				dQPOut = unipolarToBipolar(dQuadModulo);

				dQPOut = 2.0*fabs(dQPOut) - 1.0;

				if (m_uLFOMode == shot)
				{
					dQPOut = bipolarToUnipolar(dQPOut);
				}

				break;
			}
			case expo:
			{
				dOut = concaveInvertedTransform(m_dModulo);
				dQPOut = concaveInvertedTransform(dQuadModulo);

				break;
			}
			
			case rsh:
			case qrsh:
			{
				if(m_nRSHCounter < 0)
				{
					if (m_uWaveform == rsh)
					{
						m_dRSHValue = doWhiteNoise();
					}
					else
					{
						m_dRSHValue = doPNSequence(m_uPNRegister);
					}

					m_nRSHCounter = 1.0;
				}
				else if (m_nRSHCounter > m_dSampleRate/m_dFo)
				{
					m_nRSHCounter -= m_dSampleRate/m_dFo;
					
					if (m_uWaveform == rsh)
					{
						m_dRSHValue = doWhiteNoise();
					}
					else
					{
						m_dRSHValue = doPNSequence(m_uPNRegister);
					}
				}

				m_nRSHCounter += 1.0;
				dOut = m_dRSHValue;
				dQPOut = m_dRSHValue;
				break;
			}

			default:
				break;
		}

		incModulo();

		if (pQuadPhaseOutput)
		{
			*pQuadPhaseOutput = dQPOut * m_dAmplitude * m_dAmpMod;
		}

		return dOut*m_dAmplitude*m_dAmpMod;
	}
};
