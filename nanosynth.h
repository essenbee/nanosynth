/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Definition
	Copyright(c) Tritone Systems Inc. 2006-2012

	Your plug-in must implement the constructor,
	destructor and virtual Plug-In API Functions below.
*/

#pragma once

// base class
#include "plugin.h"
#include "synthfunctions.h"

// --- synth objects
#include "QBLimitedOscillator.h"
#include "LFO.h"
#include "WTOscillator.h"
#include "EnvelopeGenerator.h"
#include "DCA.h"
#include "MoogLadderFilter.h"

// abstract base class for RackAFX filters
class CNanoSynth : public CPlugIn
{
public:
	// RackAFX Plug-In API Member Methods:
	// The followung 5 methods must be impelemented for a meaningful Plug-In
	//
	// 1. One Time Initialization
	CNanoSynth();

	// 2. One Time Destruction
	virtual ~CNanoSynth(void);

	// 3. The Prepare For Play Function is called just before audio streams
	virtual bool __stdcall prepareForPlay();

	// 4. processAudioFrame() processes an audio input to create an audio output
	virtual bool __stdcall processAudioFrame(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels);

	// 5. userInterfaceChange() occurs when the user moves a control.
	virtual bool __stdcall userInterfaceChange(int nControlIndex);


	// OPTIONAL ADVANCED METHODS ------------------------------------------------------------------------------------------------
	// These are more advanced; see the website for more details
	//
	// 6. initialize() is called once just after creation; if you need to use Plug-In -> Host methods
	//				   such as sendUpdateGUI(), you must do them here and NOT in the constructor
	virtual bool __stdcall initialize();

	// 7. joystickControlChange() occurs when the user moves a control.
	virtual bool __stdcall joystickControlChange(float fControlA, float fControlB, float fControlC, float fControlD, float fACMix, float fBDMix);

	// 8. process buffers instead of Frames:
	// NOTE: set m_bWantBuffers = true to use this function
	virtual bool __stdcall processRackAFXAudioBuffer(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels, UINT uBufferSize);

	// 9. rocess buffers instead of Frames:
	// NOTE: set m_bWantVSTBuffers = true to use this function
	virtual bool __stdcall processVSTAudioBuffer(float** inBuffer, float** outBuffer, UINT uNumChannels, int inFramesToProcess);

	// 10. MIDI Note On Event
	virtual bool __stdcall midiNoteOn(UINT uChannel, UINT uMIDINote, UINT uVelocity);

	// 11. MIDI Note Off Event
	virtual bool __stdcall midiNoteOff(UINT uChannel, UINT uMIDINote, UINT uVelocity, bool bAllNotesOff);


	// 12. MIDI Modulation Wheel uModValue = 0 -> 127
	virtual bool __stdcall midiModWheel(UINT uChannel, UINT uModValue);

	// 13. MIDI Pitch Bend
	//					nActualPitchBendValue = -8192 -> 8191, 0 is center, corresponding to the 14-bit MIDI value
	//					fNormalizedPitchBendValue = -1.0 -> +1.0, 0 is at center by using only -8191 -> +8191
	virtual bool __stdcall midiPitchBend(UINT uChannel, int nActualPitchBendValue, float fNormalizedPitchBendValue);

	// 14. MIDI Timing Clock (Sunk to BPM) function called once per clock
	virtual bool __stdcall midiClock();


	// 15. all MIDI messages -
	// NOTE: set m_bWantAllMIDIMessages true to get everything else (other than note on/off)
	virtual bool __stdcall midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char cData1, unsigned char cData2);

	// 16. initUI() is called only once from the constructor; you do not need to write or call it. Do NOT modify this function
	virtual bool __stdcall initUI();



	// Add your code here: ----------------------------------------------------------- //

	// Select Oscillator Type Here
	// ===========================

	// Quasi-bandlimted oscillators
	CQBLimitedOscillator m_Osc1;
	CQBLimitedOscillator m_Osc2;

	// Wavetable oscillators
	// CWTOscillator m_Osc1;
	// CWTOscillator m_Osc2;

	// Low Frequency Oscillator
	CLFO m_LFO1;

	// Envelope Generator 1
	CEnvelopeGenerator m_EG1;

	// Digitally Controlled Amplifier
	CDCA m_DCA;

	// Select Filter Here
	// ==================

	CMoogLadderFilter m_Filter1;

	void update();
	UINT m_uMidiRxChannel;
	// END OF USER CODE -------------------------------------------------------------- //


	// ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ----------------------------------- //
	//  **--0x07FD--**

	UINT m_uOscWaveform;
	enum{SINE,SAW1,SAW2,SAW3,TRI,SQUARE,NOISE,PNOISE};
	UINT m_uLFO1Waveform;
	enum{sine,usaw,dsaw,square,expo,rsh,qrsh};
	double m_dAttackTime_mSec;
	double m_dPanControl;
	double m_dVolume_dB;
	UINT m_uLegatoMode;
	enum{mono,legato};
	double m_dFcControl;
	UINT m_uLFO1Mode;
	enum{sync,shot,free};
	double m_dDecayTime_mSec;
	double m_dEG1DCAIntensity;
	UINT m_uResetToZero;
	enum{OFF,ON};
	double m_dQControl;
	double m_dLFO1Rate;
	double m_dSustainLevel;
	double m_dFilterKeyTrackIntensity;
	double m_dLFO1Amplitude;
	double m_dReleaseTime_mSec;
	double m_dEG1OscIntensity;
	UINT m_uFilterKeyTrack;

	// **--0x1A7F--**
	// ------------------------------------------------------------------------------- //

};
