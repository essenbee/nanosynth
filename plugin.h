/*
	RackAFX(TM)
	Applications Programming Interface
	Base Class Object Definition
	Copyright(C) Tritone Systems Inc. 2002-2016

	In order to write a RackAFX plug-in, you need to create a C++ object that is
	derived from the CPlugIn base class. Your plug-in must implement the constructor,
	destructor and virtual Plug-In API Functions below.
*/

#pragma once
#ifndef __CPlugIn__
#define __CPlugIn__

#if defined AAXPLUGIN && !defined _WINDOWS && !defined _WINDLL
#include <CoreFoundation/CoreFoundation.h>
#endif


// RackAFX Includes
#include "pluginconstants.h"

// RackAFX abstract base class for RackAFX Plug-Ins
class CPlugIn
{
public:
	// Plug-In API Member Methods:
	// The followung 5 methods must be impelemented for a meaningful Plug-In
	//
	// 1. One Time Initialization
	CPlugIn();

	// 2. One Time Destruction
	virtual ~CPlugIn(void);

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
	// NOTE: set m_bWantAllMIDIMessages true to get everything else (other than note on/off, ModWheel, PitchBend, Timing Clock)
	virtual bool __stdcall midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char cData1, unsigned char cData2);

	// 16. initUI() is called only once from the constructor; you do not need to write or call it. Do NOT modify this function
	virtual bool __stdcall initUI();

	// 17. processRackAFXMessage() for thread safe handling of GUI parameters and
	//	   (future) sample accurate MIDI messaging
	virtual void __stdcall processRackAFXMessage(UINT uMessage, PROCESS_INFO& processInfo);

	// 18. message for updating GUI from plugin
	virtual bool __stdcall checkUpdateGUI(int nControlIndex, float fValue, CLinkedList<GUI_PARAMETER>& guiParameters, bool bLoadingPreset);
	virtual void __stdcall clearUpdateGUIParameters(CLinkedList<GUI_PARAMETER>& guiParameters);

	// --- for RackAFX ONLY
#if defined _WINDOWS || defined _WINDLL
	// 18. Output text to the status window
	void __stdcall sendStatusWndText(char* pText);
	// ptr to parent wnd for StatusWindow
	HWND m_hParentWnd;
#endif

#if defined AUPLUGIN
    // 21. get the directory where this DLL (component in MacOS) is located
    char* getMyDLLDirectory(CFStringRef bundleID);
#endif

#if defined AAXPLUGIN && !defined _WINDOWS && !defined _WINDLL
    // 21. get the directory where this DLL (component in MacOS) is located
    char* getMyDLLDirectory(CFStringRef bundleID);
#else
    // 21. get the directory where this DLL is located
    char* __stdcall getMyDLLDirectory();
#endif

	// --- custom GUI support; see www.willpirkle.com
	virtual void* __stdcall showGUI(void* pInfo);

	// --- process aux inputs
	virtual bool __stdcall processAuxInputBus(audioProcessData* pAudioProcessData);

	// --- for RackAFX only
	virtual bool __stdcall activate(bool bActivate);

	// --- function to handle VST sample accurate parameter updates
	void doVSTSampleAccurateParamUpdates();

	// Plug-In API Member Variables:
	//
	// Variables you CAN change in your code ----------------------------
	//
	// NOTE: DEPRACATED in v6.6 -- no longer used, see www.willpirkle.com for instructions on pure-custom GUIs
	bool m_bUserCustomGUI;

	// FLAGS: you can set these to indicate your plug-in wants something or is something
	// flag for Oscillator (output only) Plug-Ins / MIDI synth Pllug-ins
	bool m_bOutputOnlyPlugIn;

	// flag to rx all MIDI (note on/off, mod wheel, clock, pitchbend WILL STILL BE CALLED)
	bool m_bWantAllMIDIMessages;

	// flag to set to request impulse responses from the UI
	bool m_bWantIRs;

	// the length of the IR from 0 to 1024
	int m_nIRLength;

	// flag to set to request buffer data instead of frame data
	bool m_bWantBuffers;

	// flag for VST capable plugins to use the VST buffer system
	bool m_bWantVSTBuffers;

	// flag to enable/disable MIDI controllers
	bool m_bEnableMIDIControl;

	// flag to force link the rows and button banks for custom GUIs
	// DEPRACATED, no longer used
	bool m_bLinkGUIRowsAndButtons;

	// Variables you CAN NOT change in your code (RackAFX changes them for you)-----------
	//
	// information about the current playing-wave file
	int m_nNumWAVEChannels;
	int m_nSampleRate;
	int m_nBitDepth;

	// RackAFX stuff --- This can also be called by advanced users hooking up a commercial plugin
	// to a RackAFX Kernel
	public:
	// for VST Preset Support
	float*  m_PresetJSPrograms[PRESET_COUNT];
	char*  m_PresetNames[PRESET_COUNT];
	float*  m_AddlPresetValues[PRESET_COUNT];

	// Plug-In Members:
	char* m_PlugInName; // name for Socket

	UINT m_uVersion;	// versioning, RackAFX only

	// custom GUI stuff
	bool m_bUseCustomVSTGUI;
	int m_nGUIThemeID;
	int m_nGUIType;
	UINT m_uControlTheme[PLUGIN_CONTROL_THEME_SIZE];
	UINT m_uPlugInEx[PLUGIN_CONTROL_THEME_SIZE];
	float m_fPlugInEx[PLUGIN_CONTROL_THEME_SIZE];
	char*  m_TextLabels[PLUGIN_CONTROL_THEME_SIZE];
	UINT m_uLabelCX[PLUGIN_CONTROL_THEME_SIZE];
	UINT m_uLabelCY[PLUGIN_CONTROL_THEME_SIZE];

	// I/O capabilities
	UINT m_uMaxInputChannels;
	UINT m_uMaxOutputChannels;

	// for track pad
	int m_uX_TrackPadIndex;
	int m_uY_TrackPadIndex;

	// assignable buttons
	char* m_AssignButton1Name;
	char* m_AssignButton2Name;
	char* m_AssignButton3Name;
	bool m_bLatchingAssignButton1;
	bool m_bLatchingAssignButton2;
	bool m_bLatchingAssignButton3;

	// prebuilt table of MIDI Note Numbers --> Pitch Frequencies
	float m_MIDIFreqTable[128];

	// vector joystick program table
	float* m_pVectorJSProgram;
	CUICtrl m_JS_XCtrl; // joystick X control
	CUICtrl m_JS_YCtrl; // joystick Y control

	// impulse response buffers
	float m_h_Left[1024];
	float m_h_Right[1024];

	// --- functions added/changed in v6.8
	//
	// --- normalized params for GUI support
	void __stdcall setNormalizedParameter(UINT index, float value, bool bIgnoreSmoothing = false);
	void __stdcall setNormalizedParameter(CUICtrl* pUICtrl, float value, bool bIgnoreSmoothing = false);
	float __stdcall getNormalizedParameter(UINT index);

	// --- for actual values
	void __stdcall setParameterValue(UINT index, float value);
	void __stdcall setParameterValue(CUICtrl* pUICtrl, float value);
	float __stdcall getParameterValue(UINT index);

	// --- vector joystick only, for AAX and other external GUI needs
	void __stdcall setVectorJSXValue(float fJSX, bool bKorgVJSOrientation = true);
	void __stdcall setVectorJSYValue(float fJSY, bool bKorgVJSOrientation = true);
	void __stdcall getVectorJSValues(float& fJSX, float&fJSY);

	// --- smoothing always operates on actual values
	void __stdcall smoothParameterValue(CUICtrl* pUICtrl);
	void __stdcall smoothParameterValues();

	// --- info from host about current processing
	HOST_INFO m_HostProcessInfo;

	// --- UI List helper functions
	CUICtrl* __stdcall getUICtrlByControlID(UINT uID);
	CUICtrl* __stdcall getUICtrlByListIndex(UINT uIndex){return m_UIControlList.getAt(uIndex);}
	void __stdcall appendUICtrl(CUICtrl ctrl){return m_UIControlList.append(ctrl);}
	void __stdcall updateUICtrl(CUICtrl ctrl){return m_UIControlList.update(ctrl);}
	void __stdcall deleteUICtrl(CUICtrl ctrl){return m_UIControlList.del(ctrl);}
	int __stdcall getControlCount(){return m_UIControlList.count();}
	int __stdcall getControlCountMinimum(){return m_UIControlList.countLegalVSTIF();}
	void __stdcall copyControlList(CUIControlList* pList);
	CUIControlList* __stdcall getUIControlListPtr(){return &m_UIControlList;}

protected:
	CUICtrl** m_ppControlTable; // for instant table lookup of control objects
	unsigned int m_uControlListCount;
	GUI_PARAMETER* m_pOutGUIParameters;
	CUIControlList m_UIControlList;
	// --------------------------------------------------------------------------------------------
};

// --- view factory interface (implemented in Sock2VST3.lib)
class CRafxViewFactory
{
public:
	static void* createGUI(VSTGUI_VIEW_INFO* info, CPlugIn* pPlugIn);
	static void* destroyGUI();
	static void* timerPing();
	static void* initControls();
	static void* syncGUI();
};




#endif