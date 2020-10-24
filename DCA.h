#pragma once
#include "synthfunctions.h"

#define AMP_MOD_RANGE -96	// -96dB

class CDCA
{
public:
	CDCA(void);
	~CDCA(void);

	// --- ATTRIBUTES
	// --- PUBLIC: these variables may be get/set
protected:
	// --- PROTECTED: generally these are either basic calc variables
	//                and modulation stuff
	//
	// --- our internal gain variable
	double m_dGain;

	// --- velocity input from MIDI keyboard
	UINT m_uMIDIVelocity; // 0 -> 127

	// --- controls for user GUI (optional)
	double m_dAmplitude_dB;		// the user's control setting in dB
	double m_dAmplitudeControl;	// the user's control setting, converted from dB

	// --- pan control
	double m_dPanControl;	/* -1 to +1 == left to right */

	// --- modulate amplitude
	double m_dAmpMod_dB;	/* for tremolo, not true AM */

	// --- input to EGMod is EXPONENTIAL
	double m_dEGMod;		 /* modulation input for EG 0 to +1 */

	// --- input to modulate pan control is bipolar
	double m_dPanMod;		/* modulation input for -1 to +1 */

public:
	// --- FUNCTIONS: all public
	//
	// --- MIDI controller functions
	inline void setMIDIVelocity(UINT u){m_uMIDIVelocity = u;}

	// --- Pan control
	inline void setPanControl(double d){m_dPanControl = d;}

	// --- reset mods
	inline void reset()
	{
		m_dEGMod = 1.0;
		m_dAmpMod_dB = 0.0;
	}

	// --- NOTE: -96dB to +24dB
	inline void setAmplitude_dB(double d)
	{
		m_dAmplitude_dB = d;
		m_dAmplitudeControl = pow((double)10.0, m_dAmplitude_dB/(double)20.0);
	}

	// --- modulation functions - NOT needed/used if you implement the Modulation Matrix!
	//
	// --- expecting connection from bipolar source (LFO)
	//	   but this component will only be able to attenuate
	//	   so convert to unipolar
	inline void setAmpMod_dB(double d){m_dAmpMod_dB = bipolarToUnipolar(d);}

	// --- EG Mod Input Functions
	inline void setEGMod(double d){m_dEGMod = d;}

	// --- Pan modulation
	inline void setPanMod(double d){m_dPanMod = d;}


	// --- DCA operation functions
	// --- recalculate gain values
	inline void update()
	{
		// --- check polarity
		if(m_dEGMod >= 0)
			m_dGain = m_dEGMod;
		else
			m_dGain = m_dEGMod + 1.0;

		// --- amp mod is attenuation only, in dB
		m_dGain *= pow(10.0, m_dAmpMod_dB/(double)20.0);

		// --- use MMA MIDI->Atten (convex) transform
		m_dGain *= mmaMIDItoAtten(m_uMIDIVelocity);
	}

	// --- do the DCA: uses pass-by-reference for outputs
	//     For mono-in, just repeat the inputs
	inline void doDCA(double dLeftInput, double dRightInput, double& dLeftOutput, double& dRightOutput)
	{
		// total pan value
		double dPanTotal = m_dPanControl + m_dPanMod;

		// limit in case pan control is biased
		dPanTotal = fmin(dPanTotal, (double)1.0);
		dPanTotal = fmax(dPanTotal, (double)-1.0);

		double dPanLeft = 0.707;
		double dPanRight = 0.707;

		// equal power calculation in synthfunction.h
		calculatePanValues(dPanTotal, dPanLeft, dPanRight);

		// form left and right outputs
		dLeftOutput =  dPanLeft*m_dAmplitudeControl*dLeftInput*m_dGain;
		dRightOutput =  dPanRight*m_dAmplitudeControl*dRightInput*m_dGain;
	}
};
