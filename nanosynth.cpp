/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Implementation
*/


#include "nanosynth.h"
#include "trace.h"

#define LOG_MIDI 1


/* constructor()
	You can initialize variables here.
	You can also allocate memory here as long is it does not
	require the plugin to be fully instantiated. If so, allocate in init()

*/
CNanosynth::CNanosynth()
{
	// Added by RackAFX - DO NOT REMOVE
	//
	// initUI() for GUI controls: this must be called before initializing/using any GUI variables
	initUI();
	// END initUI()

	// built in initialization
	m_PlugInName = "nanosynth";

	// Default to Stereo Operation:
	// Change this if you want to support more/less channels
	m_uMaxInputChannels = 2;
	m_uMaxOutputChannels = 2;

	// use of MIDI controllers to adjust sliders/knobs
	m_bEnableMIDIControl = true;		// by default this is enabled

	// DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bUseCustomVSTGUI = false;

	// output only - SYNTH - plugin DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bOutputOnlyPlugIn = true;

	// change to true if you want all MIDI messages
	m_bWantAllMIDIMessages = false;

	// un-comment this for VST/AU Buffer-style processing
	// m_bWantVSTBuffers = true;

	// Finish initializations here
	m_pMidiEventList = NULL; // --- for sample accurate MIDI in VST, AU (AAX has its own method)
	m_uMidiRxChannel = MIDI_CH_ALL;
	m_bWantAllMIDIMessages = true;
}


/* destructor()
	Destroy dynamically allocated variables
*/
CNanosynth::~CNanosynth(void)
{


}

/*
initialize()
	Called by the client after creation; the parent window handle is now valid
	so you can use the Plug-In -> Host functions here
	See the website www.willpirkle.com for more details
*/
bool __stdcall CNanosynth::initialize()
{
	// Add your code here

	return true;
}

/*
processRackAFXMessage()
	Called for a variety of reasons, but we override here to pick up a MIDIEventList interface
	for sample accurate MIDI in VST/AU NOTE: this is only for use in processVSTBuffer()
	If you use processAudioFrame( ) you do not need to bother with this as you already
	have sample accurate MIDI in v6.8.0.5 and above
	See the website www.willpirkle.com for more details
*/
void __stdcall CNanosynth::processRackAFXMessage(UINT uMessage, PROCESS_INFO& processInfo)
{
	// --- always call base class first
	CPlugIn::processRackAFXMessage(uMessage, processInfo);

	// --- for MIDI Event list handling (AU, VST, AAX with processVSTBuffer())
	if (uMessage == midiEventList)
	{
		m_pMidiEventList = processInfo.pIMidiEventList;
	}
}

/* prepareForPlay()
	Called by the client after Play() is initiated but before audio streams

	You can perform buffer flushes and per-run intializations.
	You can check the following variables and use them if needed:

	m_nNumWAVEChannels;
	m_nSampleRate;
	m_nBitDepth;

	NOTE: the above values are only valid during prepareForPlay() and
		  processAudioFrame() because the user might change to another wave file,
		  or use the sound card, oscillators, or impulse response mechanisms

	NOTE: if you alloctae memory in this function, destroy it in the destructor above
*/
bool __stdcall CNanosynth::prepareForPlay()
{
	// Add your code here:



	// --- let base class do its thing
	return CPlugIn::prepareForPlay();
}

/* processAudioFrame

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

LEFT INPUT = pInputBuffer[0];
RIGHT INPUT = pInputBuffer[1]

LEFT OUTPUT = pInputBuffer[0]
RIGHT OUTPUT = pOutputBuffer[1]

HOST INFORMATION is available in m_HostProcessInfo:

// --- for RackAFX and all derivative projects:
m_HostProcessInfo.uAbsoluteSampleBufferIndex = sample index of top of current audio buffer
m_HostProcessInfo.dAbsoluteSampleBufferTime = time (sec) of sample in top of current audio buffer
m_HostProcessInfo.dBPM = Host Tempo setting in BPM
m_HostProcessInfo.fTimeSigNumerator = Host Time Signature Numerator (if supported by host)
m_HostProcessInfo.uTimeSigDenomintor = Host Time Signature Denominator (if supported by host)

// --- see the definition of HOST_INFO in the pluginconstants.h file for variables that are
//     unique to AU, AAX and VST for use in your ported projects!
*/
bool __stdcall CNanosynth::processAudioFrame(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels)
{
	// --- for VST3 plugins only
	doVSTSampleAccurateParamUpdates();

	// --- smooth parameters (if enabled) DO NOT REMOVE
	smoothParameterValues();

	// output = input -- change this for meaningful processing
	//
	// Do LEFT (MONO) Channel; there is always at least one input/one output
	// (INSERT Effect)
	pOutputBuffer[0] = pInputBuffer[0];

	// Mono-In, Stereo-Out (AUX Effect)
	if (uNumInputChannels == 1 && uNumOutputChannels == 2)
		pOutputBuffer[1] = pInputBuffer[0];

	// Stereo-In, Stereo-Out (INSERT Effect)
	if (uNumInputChannels == 2 && uNumOutputChannels == 2)
		pOutputBuffer[1] = pInputBuffer[1];


	return true;
}


/* ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ----------------------------------- //
	**--0x2983--**

		Assignable Buttons               Index
	-----------------------------------------------
		B1		                          50
		B2		                          51
		B3		                          52

	-----------------------------------------------


	**--0xFFDD--**
// ------------------------------------------------------------------------------- */
// Add your UI Handler code here ------------------------------------------------- //
//
bool __stdcall CNanosynth::userInterfaceChange(int nControlIndex)
{
	// decode the control index, or delete the switch and use brute force calls
	switch (nControlIndex)
	{
	case 0:
	{
		break;
	}

	default:
		break;
	}

	return true;
}

// --- message for updating GUI from plugin; see the comment block above userInterfaceChange( ) for
//     the index values to use when sending outbound parameter changes
//     see www.willpirkle.com for information on using this function to update the GUI from your plugin
//     The bLoadingPreset flag will be set if this is being called as a result of a preset load; you may
//     want to ignore GUI updates for presets depending on how your update works!
//     NOTE: this function will be called even if no audio is flowing (unlike userInterfaceChange( ) which
//           will only get called while in the audio processing loop (see Thread Safety document on website)
bool __stdcall CNanosynth::checkUpdateGUI(int nControlIndex, float fValue, CLinkedList<GUI_PARAMETER>& guiParameters, bool bLoadingPreset)
{
	// decode the control index
	switch (nControlIndex)
	{
	case 0:
	{
		// return true; // if update needed
		// break;		// if no update needed
	}

	default:
		break;
	}
	return false;
}

// --- process aux inputs
//     This function will be called once for each Aux Input bus, currently:
//
//     Aux Input 1: Sidechain
//     May add more input busses in the future
//
//     see www.willpirkle.com for info on using the Aux input bus
bool __stdcall CNanosynth::processAuxInputBus(audioProcessData* pAudioProcessData)
{
	/* --- pick up pointers to the Aux Input busses for sidechain or other Aux processing

			Ordinarily, you just copy the buffer pointers to member variables and then
			use the pointers in your process( ) function.

			However, you also have the option of pre-processing the Aux inputs in-place
			in these buffers, though no idea when you might need that...

	Example:
	if(pAudioProcessData->uInputBus == 1)
	{
		// --- save varius pointers, in practice you only save the
		//     pointer you need for your particular process( ) function
		//     Note these are member variables you need to declare on your own in the .h file!
		//
		m_pSidechainFrameBuffer = pAudioProcessData->pFrameInputBuffer; // <-- for processAudioFrame( )
		m_pSidechainRAFXBuffer = pAudioProcessData->pRAFXInputBuffer;	// <-- for processRackAFXAudioBuffer
		m_ppSidechainVSTBuffer = pAudioProcessData->ppVSTInputBuffer;	// <-- for processVSTAudioBuffer

		// --- sidechain input activation/channels
		m_bSidechainEnabled = pAudioProcessData->bInputEnabled;
		m_uSidechainChannelCount = pAudioProcessData->uNumInputChannels;
	}

	*/

	return true;
}

/* joystickControlChange

	Indicates the user moved the joystick point; the variables are the relative mixes
	of each axis; the values will add up to 1.0

			B
			|
		A -	x -	C
			|
			D

	The point in the very center (x) would be:
	fControlA = 0.25
	fControlB = 0.25
	fControlC = 0.25
	fControlD = 0.25

	AC Mix = projection on X Axis (0 -> 1)
	BD Mix = projection on Y Axis (0 -> 1)
*/
bool __stdcall CNanosynth::joystickControlChange(float fControlA, float fControlB, float fControlC, float fControlD, float fACMix, float fBDMix)
{
	// add your code here

	return true;
}



/* processAudioBuffer

	// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	The I/O buffers are interleaved depending on the number of channels. If uNumChannels = 2, then the
	buffer is L/R/L/R/L/R etc...

	if uNumChannels = 6 then the buffer is L/R/C/Sub/BL/BR etc...

	It is up to you to decode and de-interleave the data.

	To use this function set m_bWantBuffers = true in your constructor.

	******************************
	********* IMPORTANT! *********
	******************************
	If you are going to ultimately use <Make VST> or <Make AU> to port your project and you want to process
	buffers instead of frames, you need to override processVSTAudioBuffer() below instead;

	processRackAFXAudioBuffer() is NOT supported in <Make VST>, <Make AU>, and <Make AAX>

	HOST INFORMATION is available in m_HostProcessInfo:

	m_HostProcessInfo.uAbsoluteSampleBufferIndex = sample index of top of current audio buffer
	m_HostProcessInfo.dAbsoluteSampleBufferTime = time (sec) of sample in top of current audio buffer
	m_HostProcessInfo.dBPM = Host Tempo setting in BPM
	m_HostProcessInfo.fTimeSigNumerator = Host Time Signature Numerator (if supported by host)
	m_HostProcessInfo.uTimeSigDenomintor = Host Time Signature Denominator (if supported by host)
*/
bool __stdcall CNanosynth::processRackAFXAudioBuffer(float* pInputBuffer, float* pOutputBuffer,
	UINT uNumInputChannels, UINT uNumOutputChannels,
	UINT uBufferSize)
{

	for (UINT i = 0; i < uBufferSize; i++)
	{
		// smooth parameters (if enabled) DO NOT REMOVE
		smoothParameterValues(); // done on a per-sample-interval basis

		// pass through code
		pOutputBuffer[i] = pInputBuffer[i];
	}


	return true;
}



/* processVSTAudioBuffer

	// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	NOTE: You do not have to implement this function if you don't want to; the processAudioFrame()
	will still work; however this using function will be more CPU efficient for your plug-in, and will
	override processAudioFrame().

	To use this function set m_bWantVSTBuffers = true in your constructor.

	The VST input and output buffers are pointers-to-pointers. The pp buffers are the same depth as uNumChannels, so
	if uNumChannels = 2, then ppInputs would contain two pointers,

		inBuffer[0] = a pointer to the LEFT buffer of data
		inBuffer[1] = a pointer to the RIGHT buffer of data

	Similarly, outBuffer would have 2 pointers, one for left and one for right.

	For 5.1 audio you would get 6 pointers in each buffer.

*/
bool __stdcall CNanosynth::processVSTAudioBuffer(float** inBuffer, float** outBuffer, UINT uNumChannels, int inFramesToProcess)
{
	// PASS Through example
	// MONO First
	float* pInputL = inBuffer[0];
	float* pOutputL = outBuffer[0];
	float* pInputR = NULL;
	float* pOutputR = NULL;

	// if STEREO,
	if (inBuffer[1])
		pInputR = inBuffer[1];

	if (outBuffer[1])
		pOutputR = outBuffer[1];

	// Process audio by de-referencing ptrs
	// this is siple pass through code
	unsigned int uSample = 0;
	while (--inFramesToProcess >= 0)
	{
		// --- fire midi events (AU, VST2, AAX buffer processing only; not needed if you use processAudioFrame())
		if (m_pMidiEventList)
			m_pMidiEventList->fireMidiEvent(uSample++);

		// --- sample accurate automation for VST3 only
		doVSTSampleAccurateParamUpdates();

		// --- smooth parameters (if enabled) DO NOT REMOVE
		smoothParameterValues(); // done on a per-sample-interval basis

		// --- Left channel processing
		*pOutputL = *pInputL;

		// --- If there is a right channel
		if (pInputR && pOutputR)
			* pOutputR = *pInputR;
		else if (pOutputR) // 1->2 mapping
			* pOutputR = *pOutputL;

		// --- advance pointers
		pInputL++;
		pOutputL++;
		if (pInputR) pInputR++;
		if (pOutputR) pOutputR++;
	}
	// --- all OK
	return true;
}

bool __stdcall CNanosynth::midiNoteOn(UINT uChannel, UINT uMIDINote, UINT uVelocity)
{
	if (m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
	{
		return false;
	}

#ifdef LOG_MIDI
	TRACE("-- Note On Ch: %d Note: %d Vel: %d \n", uChannel, uMIDINote, uVelocity);
#endif

	return true;
}

bool __stdcall CNanosynth::midiNoteOff(UINT uChannel, UINT uMIDINote, UINT uVelocity, bool bAllNotesOff)
{
	if (m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
	{
		return false;
	}

#ifdef LOG_MIDI
	if (bAllNotesOff)
	{
		TRACE("-- All Notes Off \n");
	}
	else
	{
		TRACE("-- Note Off Ch: %d Note: %d Vel: %d \n", uChannel, uMIDINote, uVelocity);
	}
#endif
	return true;
}

// uModValue = 0->127
bool __stdcall CNanosynth::midiModWheel(UINT uChannel, UINT uModValue)
{
	if (m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
	{
		return false;
	}

#ifdef LOG_MIDI
	TRACE("-- Mod Wheel Ch: %d Value: %d \n", uChannel, uModValue);
#endif

	return true;
}

// nActualPitchBendValue 		= -8192 -> +8191, 0 at center
// fNormalizedPitchBendValue 	= -1.0  -> +1.0,  0 at center
bool __stdcall CNanosynth::midiPitchBend(UINT uChannel, int nActualPitchBendValue, float fNormalizedPitchBendValue)
{
	if (m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
	{
		return false;
	}

#ifdef LOG_MIDI
	TRACE("-- Pitch Bend Ch: %d Actual Value: %d Norm. Value: %f \n", uChannel, nActualPitchBendValue, fNormalizedPitchBendValue);
#endif
	return true;
}

// MIDI Clock
// http://home.roadrunner.com/~jgglatt/tech/midispec/clock.htm
/* There are 24 MIDI Clocks in every quarter note. (12 MIDI Clocks in an eighth note, 6 MIDI Clocks in a 16th, etc).
   Therefore, when a slave device counts down the receipt of 24 MIDI Clock messages, it knows that one quarter note
   has passed. When the slave counts off another 24 MIDI Clock messages, it knows that another quarter note has passed.
   Etc. Of course, the rate that the master sends these messages is based upon the master's tempo.

   For example, for a tempo of 120 BPM (ie, there are 120 quarter notes in every minute), the master sends a MIDI clock
   every 20833 microseconds. (ie, There are 1,000,000 microseconds in a second. Therefore, there are 60,000,000
   microseconds in a minute. At a tempo of 120 BPM, there are 120 quarter notes per minute. There are 24 MIDI clocks
   in each quarter note. Therefore, there should be 24 * 120 MIDI Clocks per minute.
   So, each MIDI Clock is sent at a rate of 60,000,000/(24 * 120) microseconds).
*/
bool __stdcall CNanosynth::midiClock()
{

	return true;
}

// any midi message other than note on, note off, pitchbend, mod wheel or clock
bool __stdcall CNanosynth::midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char
	cData1, unsigned char cData2)
{
	auto uChannel = static_cast<UINT>(cChannel);
	auto uData1 = static_cast<UINT>(cData1);
	auto uData2 = static_cast<UINT>(cData2);

	if (m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
	{
		return false;
	}

	switch (cStatus)
	{
	case POLY_PRESSURE:
	{
#ifdef LOG_MIDI
		TRACE("-- Poly Pressure Ch: %d Note: %d Value: %d \n", uChannel, uData1, uData2);
#endif
		break;
	}
	case CONTROL_CHANGE:
	{
		switch (cData1)
		{
		case VOLUME_CC07:
		{
#ifdef LOG_MIDI
			TRACE("-- Volume Ch: %d Value: %d \n", uChannel, uData2);
#endif
			break;
		}
		case PAN_CC10:
		{
#ifdef LOG_MIDI
			TRACE("-- Pan Ch: %d Value: %d \n", uChannel, uData2);
#endif
			break;
		}
		case EXPRESSION_CC11:
		{
#ifdef LOG_MIDI
			TRACE("-- Expression Pedal Ch: %d Value: %d \n", uChannel, uData2);
#endif
			break;
		}
		case SUSTAIN_PEDAL:
		{
			auto sustainOn = uData2 > 63;
#ifdef LOG_MIDI
			if (sustainOn)
			{
				TRACE("-- Sustain Pedal ON Ch: %d \n", uChannel);
			}
			else
			{
				TRACE("-- Sustain Pedal OFF Ch: %d \n", uChannel);
			}
#endif
			break;
		}
		case ALL_NOTES_OFF:
		{
			break;
		}
		case MOD_WHEEL:
		{
			break;
		}
		default:
		{
#ifdef LOG_MIDI
			if (uData1 != RESET_ALL_CONTROLLERS)
			{
				TRACE("-- Control Ch: %d Num: %dValue: %d \n", uChannel, uData1, uData2);
			}
#endif
			break;
		}
		}
	case PROGRAM_CHANGE:
	{
#ifdef LOG_MIDI
		TRACE("-- Program Change Ch: %d Num: %d Value: %d \n", uChannel, uData1, uData2);
#endif
		break;
	}
	case CHANNEL_PRESSURE:
	{
#ifdef LOG_MIDI
		TRACE("-- Program Change Ch: %d Num: %d \n", uChannel, uData1);
#endif
		break;
	}
	default:
	{
		break;
	}

	}
	}

	return true;
}

/* doVSTSampleAccurateParamUpdates
	Short handler for VST3 sample accurate automation added in v6.8.0.5
	There is nothing for you to modify here.
*/
void CNanosynth::doVSTSampleAccurateParamUpdates()
{
	// --- for sample accurate parameter automation in VST3 plugins; ignore otherwise
	if (!m_ppControlTable) return; /// should NEVER happen
	for (int i = 0; i < m_uControlListCount; i++)
	{
		if (m_ppControlTable[i] && m_ppControlTable[i]->pvAddlData)
		{
			double dValue = 0;
			if (((IParamUpdateQueue*)m_ppControlTable[i]->pvAddlData)->getNextValue(dValue))
			{
				setNormalizedParameter(m_ppControlTable[i], dValue, true);
			}
		}
	}
}

// --- showGUI()
//     This is the main interface function for all GUIs, including custom GUIs
//     It is also where you deal with Custom Controls (see Advanced GUI API)
void* __stdcall CNanosynth::showGUI(void* pInfo)
{
	// --- ALWAYS try base class first in case of future updates
	void* result = CPlugIn::showGUI(pInfo);
	if (result)
		return result;

	/* Uncomment if using advanced GUI API: see www.willpirkle.com for details and sample code
	// --- uncloak the info struct
	VSTGUI_VIEW_INFO* info = (VSTGUI_VIEW_INFO*)pInfo;
	if(!info) return NULL;

	switch(info->message)
	{
		case GUI_DID_OPEN:
		{
			return NULL;
		}
		case GUI_WILL_CLOSE:
		{
			return NULL;
		}
		case GUI_CUSTOMVIEW:
		{
			// --- create custom view, return a CView* cloaked as void* or NULL if not supported
			return NULL;
		}

		case GUI_HAS_USER_CUSTOM:
		{
			// --- set this variable to true if you have a custom GUI
			info->bHasUserCustomView = false;
			return NULL;
		}

		// --- create your custom VSTGUI4 object using the CVSTGUIController (supplied),
		//     a subclass of the CVSTGUIController that you supply, a VSTGUI4 object
		//     that is derived at least from: VSTGUIEditorInterface, CControlListener, CBaseObject
		//     see VSTGUIController.h for an example
		//
		//     open() sets the new size of the window in info->size
		//     return a pointer to the newly created object
		case GUI_USER_CUSTOM_OPEN:
		{
			return NULL;
		}
		// --- call the close() function and delete the controller object
		case GUI_USER_CUSTOM_CLOSE:
		{
			return NULL;
		}
		// --- handle paint-specific timer stuff
		case GUI_TIMER_PING:
		{

			return NULL;
		}
	} */

	return NULL;
}

// --- DO NOT EDIT OR DELETE THIS FUNCTION ----------------------------------------------- //
bool __stdcall CNanosynth::initUI()
{
	// ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! -------------------------------------- //
	if (m_UIControlList.count() > 0)
		return true;

	// **--0xDEA7--**


	// **--0xEDA5--**
	// -------------------------------------------------------------------------------------- //

	return true;

}

