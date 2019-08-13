#include "plugin.h"

// for MacOS VST only, ignore otherwise
extern char gPath;

//#define LEAN_RAFX_PLUGIN

CPlugIn::CPlugIn()
{
	// Normal Construction Stuff
	m_pOutGUIParameters = NULL;
	m_PlugInName = "Name Not Set.";
	m_uVersion = CURRENT_PLUGIN_API;

	m_bUseCustomVSTGUI = false;
	m_nGUIThemeID = 0xFFFFFFFF;
	m_nGUIType = -1;
	memset(&m_uControlTheme[0], 0, PLUGIN_CONTROL_THEME_SIZE*sizeof(UINT));
	m_uControlTheme[CONTROL_THEME] = 0xFFFFFFFF;

	// gen purpose arrays for future use extensions
	memset(&m_uPlugInEx[0], 0, PLUGIN_CONTROL_THEME_SIZE*sizeof(UINT));
	memset(&m_fPlugInEx[0], 0, PLUGIN_CONTROL_THEME_SIZE*sizeof(float));
	memset(&m_TextLabels[0], 0, PLUGIN_CONTROL_THEME_SIZE*sizeof(char*));
	memset(&m_uLabelCX[0], 0, PLUGIN_CONTROL_THEME_SIZE*sizeof(UINT));
	memset(&m_uLabelCY[0], 0, PLUGIN_CONTROL_THEME_SIZE*sizeof(UINT));

	m_bOutputOnlyPlugIn = false;
	m_uMaxInputChannels = 2;
	m_uMaxOutputChannels = 2;

	m_uX_TrackPadIndex = -1;
	m_uY_TrackPadIndex = -1;

	m_AssignButton1Name = "B1";
	m_AssignButton2Name = "B2";
	m_AssignButton3Name = "B3";

	m_bLatchingAssignButton1 = false;
	m_bLatchingAssignButton2 = false;
	m_bLatchingAssignButton3 = false;

	m_nNumWAVEChannels = 2;
	m_nSampleRate = 44100;
	m_nBitDepth = 16;

	// set this true if you want buffer data
	m_bWantBuffers = false;

	// set this true if you want VST buffer data
	m_bWantVSTBuffers = false;

	// set to true if you want IRs
	m_bWantIRs = false;

	// zero out the length
	m_nIRLength = 0;

	// use of MIDI controllers to adjust sliders/knobs
	m_bEnableMIDIControl = true;		// by default this is enabled
	m_bLinkGUIRowsAndButtons = false;	// obsolete
	m_bUserCustomGUI = false;
	m_bUseCustomVSTGUI = false;
	m_bOutputOnlyPlugIn = false;
	m_bWantAllMIDIMessages = false;
	m_ppControlTable = NULL;
	m_uControlListCount = 0;

	// zero out impulse responses
	memset(&m_h_Left, 0, 1024*sizeof(float));
	memset(&m_h_Right, 0, 1024*sizeof(float));

	// create frequency table for MIDI support
	// make frequency (Hz) table
	double k = 1.059463094359;	// 12th root of 2
	double a = 6.875;	// a
	a *= k;	// b
	a *= k;	// bb
	a *= k;	// c, frequency of midi note 0
	for (int i = 0; i < 127; i++)	// 128 midi notes
	{
		// Hz Table
		m_MIDIFreqTable[i] = (float)a;

		// update for loop . . .
		a *= k;
	}

	m_pVectorJSProgram = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	memset(m_pVectorJSProgram, 0, MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS*sizeof(float));

	m_JS_XCtrl.cControlName = "MIDI JS X";
	m_JS_XCtrl.uControlId = 0;
	m_JS_XCtrl.bMIDIControl = false;
	m_JS_XCtrl.uMIDIControlCommand = 176;
	m_JS_XCtrl.uMIDIControlName = 16;
	m_JS_XCtrl.uMIDIControlChannel = 0;

	m_JS_YCtrl.cControlName = "MIDI JS Y";
	m_JS_YCtrl.uControlId = 0;
	m_JS_YCtrl.bMIDIControl = false;
	m_JS_YCtrl.uMIDIControlCommand = 176;
	m_JS_YCtrl.uMIDIControlName = 17;
	m_JS_YCtrl.uMIDIControlChannel = 0;

	for(int i=0; i<PRESET_COUNT; i++)
	{
		m_PresetJSPrograms[i] = NULL;
		m_PresetNames[i] = NULL;
		m_AddlPresetValues[i] = NULL;
	}
}

CPlugIn::~CPlugIn(void)
{
	if(m_pOutGUIParameters)
		delete [] m_pOutGUIParameters;

	int nCount = m_UIControlList.count();
	for(int j=nCount-1; j>=0; j--)
	{
		const CUICtrl* p = m_UIControlList.getAt(j);
		m_UIControlList.del(*p);
	}

	if(m_ppControlTable)
		delete [] m_ppControlTable;

	delete [] m_pVectorJSProgram;

	for(int i=0; i<PRESET_COUNT; i++)
	{
		if(m_PresetJSPrograms[i])
		{
			delete [] m_PresetJSPrograms[i];
			 m_PresetJSPrograms[i] = NULL;
		}

		if(m_AddlPresetValues[i])
		{
			delete [] m_AddlPresetValues[i];
			 m_AddlPresetValues[i] = NULL;
		}
	}
}

bool __stdcall CPlugIn::initialize()
{
	return true;
}

bool __stdcall CPlugIn::initUI()
{
	return true;
}


bool __stdcall CPlugIn::prepareForPlay()
{
	// --- v6.7.1.2 adds built-in param smoothing
	if (!m_ppControlTable) return true;
	int nParams = m_UIControlList.count();
	for(int i=0; i<nParams; i++)
	{
		CUICtrl* pUICtrl = m_ppControlTable[i];
		if(pUICtrl) // should never fail...
		{
			if(pUICtrl->bEnableParamSmoothing)
			{
				if(pUICtrl->uUserDataType == floatData && pUICtrl->m_pUserCookedFloatData)
				{
					pUICtrl->m_FloatParamSmoother.initParamSmoother(pUICtrl->fSmoothingTimeInMs, (float)this->m_nSampleRate, *pUICtrl->m_pUserCookedFloatData);
					pUICtrl->fSmoothingFloatValue = *pUICtrl->m_pUserCookedFloatData;
				}
				if(pUICtrl->uUserDataType == doubleData && pUICtrl->m_pUserCookedDoubleData)
				{
					pUICtrl->m_FloatParamSmoother.initParamSmoother(pUICtrl->fSmoothingTimeInMs, (float)this->m_nSampleRate, (float)*pUICtrl->m_pUserCookedDoubleData);
					pUICtrl->fSmoothingFloatValue = *pUICtrl->m_pUserCookedDoubleData;
				}
			}
		}
	}

	if(m_JS_XCtrl.bEnableParamSmoothing)
	{
		m_JS_XCtrl.m_FloatParamSmoother.initParamSmoother(m_JS_XCtrl.fSmoothingTimeInMs, (float)this->m_nSampleRate, m_JS_XCtrl.fJoystickValue);
		m_JS_XCtrl.fSmoothingFloatValue = m_JS_XCtrl.fJoystickValue;
	}

	if(m_JS_YCtrl.bEnableParamSmoothing)
	{
		m_JS_YCtrl.m_FloatParamSmoother.initParamSmoother(m_JS_YCtrl.fSmoothingTimeInMs, (float)this->m_nSampleRate, m_JS_YCtrl.fJoystickValue);
		m_JS_YCtrl.fSmoothingFloatValue = m_JS_YCtrl.fJoystickValue;
	}

	return true;
}


bool __stdcall CPlugIn::processAudioFrame(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels)
{
	return true;
}


bool __stdcall CPlugIn::processRackAFXAudioBuffer(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels, UINT uBufferSize)
{
	return true;
}


bool __stdcall CPlugIn::processVSTAudioBuffer(float** inBuffer, float** outBuffer, UINT uNumChannels, int inFramesToProcess)
{
	return true;
}


bool __stdcall CPlugIn::userInterfaceChange(int nControlIndex)
{
	return true;
}

bool __stdcall CPlugIn::joystickControlChange(float fControlA, float fControlB, float fControlC, float fControlD, float fACMix, float fBDMix)
{
	return true;
}

bool __stdcall CPlugIn::midiNoteOn(UINT uChannel, UINT uMIDINote, UINT uVelocity)
{
	return true;
}

bool __stdcall CPlugIn::midiNoteOff(UINT uChannel, UINT uMIDINote, UINT uVelocity, bool bAllNotesOff)
{
	return true;
}

bool __stdcall CPlugIn::midiModWheel(UINT uChannel, UINT uModValue)
{
	return true;
}

bool __stdcall CPlugIn::midiPitchBend(UINT uChannel, int nActualPitchBendValue, float fNormalizedPitchBendValue)
{
	return true;
}

bool __stdcall CPlugIn::midiClock()
{
	return true;
}

bool __stdcall CPlugIn::midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char
						   cData1, unsigned char cData2)
{
	return true;
}

// --- for RackAFX only
bool __stdcall CPlugIn::activate(bool bActivate)
{
	return true;
}

CUICtrl* __stdcall CPlugIn::getUICtrlByControlID(UINT uID)
{
	if (!m_ppControlTable) return NULL;
	for(int i=0; i<m_uControlListCount; i++)
	{
		CUICtrl* p = m_ppControlTable[i];
		if(p->uControlId == uID)
			return p;
	}

	return NULL;
}

void __stdcall CPlugIn::copyControlList(CUIControlList* pList)
{
	if (!m_ppControlTable) return;

	// --- make a copy of the control list for external read-only operations
	for(int i=0; i<m_uControlListCount; i++)
	{
		const CUICtrl* p = m_ppControlTable[i];
		if(p)
		{
			pList->append(*p);
		}
	}
}

//-----------------------------------------------------------------------------------------
// --- Normalized parameter support
void __stdcall CPlugIn::setNormalizedParameter(UINT index, float value, bool bIgnoreSmoothing)
{
	if(!m_ppControlTable) return;
	CUICtrl* pUICtrl = m_ppControlTable[index];
	setNormalizedParameter(pUICtrl, value, bIgnoreSmoothing);
}
void __stdcall CPlugIn::setNormalizedParameter(CUICtrl* pUICtrl, float value, bool bIgnoreSmoothing)
{
	if(!pUICtrl)
		return;

	// --- auto cook the data first
	switch(pUICtrl->uUserDataType)
	{
		case intData:
		{
			float fValue = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, value);
			*(pUICtrl->m_pUserCookedIntData) = floor(fValue + 0.5);

			// call the interface function
			userInterfaceChange(pUICtrl->uControlId);
			break;
		}
		case floatData:
		{
			if(pUICtrl->bEnableParamSmoothing && !bIgnoreSmoothing)
				pUICtrl->fSmoothingFloatValue = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, value);
			else
				*(pUICtrl->m_pUserCookedFloatData) = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, value);

			// call the interface function
			userInterfaceChange(pUICtrl->uControlId);
			break;
		}
		case doubleData:
		{
			if(pUICtrl->bEnableParamSmoothing && !bIgnoreSmoothing)
				pUICtrl->fSmoothingFloatValue = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, value);
			else
				*(pUICtrl->m_pUserCookedDoubleData) = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, value);

			// call the interface function
			userInterfaceChange(pUICtrl->uControlId);
			break;
		}
		case UINTData:
		{
			float fValue = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, value);
			UINT u = floor(fValue + 0.5);

			if (*(pUICtrl->m_pUserCookedUINTData) != u)
			{
				*(pUICtrl->m_pUserCookedUINTData) = u;

				// call the interface function
				userInterfaceChange(pUICtrl->uControlId);
			}
			break;
		}

		default: // do nothing for nonData types, no call to userInterfaceChange
			break;
	}
}

// return the 0->1 version of the variable
float __stdcall CPlugIn::getNormalizedParameter(UINT index)
{
	if(!m_ppControlTable) return 0.0;
	CUICtrl* pUICtrl = m_ppControlTable[index];
	if(!pUICtrl)
		return 0.0;

	float fRawValue = 0;
	switch(pUICtrl->uUserDataType)
	{
		case intData:
		{
			fRawValue = calcSliderVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, *(pUICtrl->m_pUserCookedIntData));
			break;
		}

		case floatData:
		{
			fRawValue = calcSliderVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, *(pUICtrl->m_pUserCookedFloatData));
			break;
		}

		case doubleData:
		{
			fRawValue = calcSliderVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, *(pUICtrl->m_pUserCookedDoubleData));
			break;
		}

		case UINTData:
		{
			fRawValue = calcSliderVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, *(pUICtrl->m_pUserCookedUINTData));
			break;
		}

		default:
			break;
	}

	return fRawValue;
}

// --- Actual Value parameter support
void __stdcall CPlugIn::setParameterValue(UINT index, float value)
{
	if(!m_ppControlTable) return;
	CUICtrl* pUICtrl =m_ppControlTable[index];
	setParameterValue(pUICtrl, value);
}

void __stdcall CPlugIn::setParameterValue(CUICtrl* pUICtrl, float value)
{
	if(!pUICtrl)
		return;

	// --- auto cook the data first
	switch(pUICtrl->uUserDataType)
	{
		case intData:
			*(pUICtrl->m_pUserCookedIntData) = (int)value;

			// call the interface function
			userInterfaceChange(pUICtrl->uControlId);
			break;

		case floatData:
			if(pUICtrl->bEnableParamSmoothing)
				pUICtrl->fSmoothingFloatValue = value;
			else
				*(pUICtrl->m_pUserCookedFloatData) = value;

			// call the interface function
			userInterfaceChange(pUICtrl->uControlId);
			break;

		case doubleData:
			if(pUICtrl->bEnableParamSmoothing)
				pUICtrl->fSmoothingFloatValue = (double)value;
			else
				*(pUICtrl->m_pUserCookedDoubleData) = (double)value;

			// call the interface function
			userInterfaceChange(pUICtrl->uControlId);
			break;

		case UINTData:
		{
			UINT u = floor(value + 0.5);
			if (*(pUICtrl->m_pUserCookedUINTData) != u)
			{
				*(pUICtrl->m_pUserCookedUINTData) = u;

				// call the interface function
				userInterfaceChange(pUICtrl->uControlId);
			}
			break;
		}

		default: // do nothing for nonData types, no call to userInterfaceChange
			break;
	}
}
float __stdcall CPlugIn::getParameterValue(UINT index)
{
	if(!m_ppControlTable) return 0.0;
	CUICtrl* pUICtrl = m_ppControlTable[index];
	if(!pUICtrl)
		return 0.0;

	float fActualValue = 0;
	switch(pUICtrl->uUserDataType)
	{
		case intData:
		{
			fActualValue = *(pUICtrl->m_pUserCookedIntData);
			break;
		}

		case floatData:
		{
			fActualValue = *(pUICtrl->m_pUserCookedFloatData);
			break;
		}

		case doubleData:
		{
			fActualValue = *(pUICtrl->m_pUserCookedDoubleData);
			break;
		}

		case UINTData:
		{
			fActualValue = *(pUICtrl->m_pUserCookedUINTData);
			break;
		}

		default:
			break;
	}

	return fActualValue;
}

// --- VJS Values are [-1.0, +1.0]
void __stdcall CPlugIn::setVectorJSXValue(float fJSX, bool bKorgVJSOrientation)
{
	if(m_JS_XCtrl.bEnableParamSmoothing)
		m_JS_XCtrl.fSmoothingFloatValue = fJSX;
	else
		m_JS_XCtrl.fJoystickValue = fJSX;

	m_JS_XCtrl.bKorgVectorJoystickOrientation = bKorgVJSOrientation;

	// --- recalculate if needed (both JS will have same smoothing, so only need to check one of them)
	if(!m_JS_XCtrl.bEnableParamSmoothing)
	{
		double dA = 0.0; double dB = 0.0; double dC = 0.0; double dD = 0.0; double dAC = 0.0; double dBD = 0.0;
		calculateRAFXVectorMixValues(m_JS_XCtrl.fJoystickValue, m_JS_YCtrl.fJoystickValue, dA, dB, dC, dD, dAC, dBD, m_JS_XCtrl.bKorgVectorJoystickOrientation);
		joystickControlChange(dA, dB, dC, dD, dAC, dBD);
	}
}

// --- VJS Values are [-1.0, +1.0]
void __stdcall CPlugIn::setVectorJSYValue(float fJSY, bool bKorgVJSOrientation)
{
	if(m_JS_YCtrl.bEnableParamSmoothing)
		m_JS_YCtrl.fSmoothingFloatValue = fJSY;
	else
		m_JS_YCtrl.fJoystickValue = fJSY;

	m_JS_YCtrl.bKorgVectorJoystickOrientation = bKorgVJSOrientation;

	// --- recalculate if needed (both JS will have same smoothing, so only need to check one of them)
	if(!m_JS_XCtrl.bEnableParamSmoothing)
	{
		double dA = 0.0; double dB = 0.0; double dC = 0.0; double dD = 0.0; double dAC = 0.0; double dBD = 0.0;
		calculateRAFXVectorMixValues(m_JS_XCtrl.fJoystickValue, m_JS_YCtrl.fJoystickValue, dA, dB, dC, dD, dAC, dBD, m_JS_XCtrl.bKorgVectorJoystickOrientation);
		joystickControlChange(dA, dB, dC, dD, dAC, dBD);
	}
}

void __stdcall CPlugIn::getVectorJSValues(float& fJSX, float&fJSY)
{
	fJSX = m_JS_XCtrl.fJoystickValue;
	fJSY = m_JS_YCtrl.fJoystickValue;
}

//-----------------------------------------------------------------------------------------

void __stdcall CPlugIn::smoothParameterValue(CUICtrl* pUICtrl)
{
	if(!pUICtrl) return;
	bool bSmoothed = false;
	if(pUICtrl->bEnableParamSmoothing)
	{
		if(pUICtrl->uUserDataType == floatData && pUICtrl->m_pUserCookedFloatData)
			bSmoothed = pUICtrl->m_FloatParamSmoother.smoothParameter(pUICtrl->fSmoothingFloatValue, *pUICtrl->m_pUserCookedFloatData);
		else if(pUICtrl->uUserDataType == doubleData && pUICtrl->m_pUserCookedDoubleData)
			bSmoothed = pUICtrl->m_FloatParamSmoother.smoothDoubleParameter(pUICtrl->fSmoothingFloatValue, *pUICtrl->m_pUserCookedDoubleData);

		if(bSmoothed)
			userInterfaceChange(pUICtrl->uControlId);
	}
}

void __stdcall CPlugIn::smoothParameterValues()
{
	if(!m_ppControlTable) return;
	for(int i=0; i<m_uControlListCount; i++)
		smoothParameterValue(m_ppControlTable[i]);

	// --- joystick
	if(m_JS_XCtrl.bEnableParamSmoothing)
	{
		double dA = 0.0; double dB = 0.0; double dC = 0.0; double dD = 0.0; double dAC = 0.0; double dBD = 0.0;

		 m_JS_XCtrl.m_FloatParamSmoother.smoothParameter(m_JS_XCtrl.fSmoothingFloatValue, m_JS_XCtrl.fJoystickValue);
		 m_JS_YCtrl.m_FloatParamSmoother.smoothParameter(m_JS_YCtrl.fSmoothingFloatValue, m_JS_YCtrl.fJoystickValue);

		calculateRAFXVectorMixValues(m_JS_XCtrl.fJoystickValue, m_JS_YCtrl.fJoystickValue, dA, dB, dC, dD, dAC, dBD, m_JS_XCtrl.bKorgVectorJoystickOrientation);
		joystickControlChange(dA, dB, dC, dD, dAC, dBD);
	}
}

// --- process messaging system
void __stdcall CPlugIn::processRackAFXMessage(UINT uMessage, PROCESS_INFO& processInfo)
{
	switch(uMessage)
	{
		case updateHostInfo:
		{
			if(!processInfo.pHostInfo) return;
			memcpy(&m_HostProcessInfo, processInfo.pHostInfo, sizeof(m_HostProcessInfo));
			break;
		}

		case preProcessData:
		{
			// --- process GUI updates; thread safe!
			if(!processInfo.pInGUIParameters || !processInfo.pHostInfo || !m_ppControlTable) return;

			// --- check to make sure param count matches (should never fail)
			if(processInfo.nNumParams != m_uControlListCount + numAddtlParams) return;

			// --- info from host
			memcpy(&m_HostProcessInfo, processInfo.pHostInfo, sizeof(m_HostProcessInfo));

			// --- loop and update any changed GUI params
			//     NOTE: pPreProcessData->pGUIParameters is a COPY of the GUI Params, so you can't hurt anything
			//     First, do the normal params
			for(int i=0; i<m_uControlListCount; i++)
			{
				// --- process (NOTE: pInGUIParameters is const so you can not mess with it)
				float fNormalizedParam = processInfo.pInGUIParameters[i].fNormalizedValue;

				if(m_ppControlTable[i])
				{
					if(processInfo.pInGUIParameters[i].bDirty) // NOTE: you do NOT have to clear this flag!
						setNormalizedParameter(m_ppControlTable[i], fNormalizedParam, processInfo.bIgnoreSmoothing);
				}
			}

			// --- now do the (optional) Vector Joystick X,Y NOTE: Vector Joystick uses actual params, this
			//     has to do with the XYPad being flipped in VSTGUI4 on the y-axis; easier for GUI to set
			//     actual values here, and 4 fewer math operations when updating
			float fActualVJS_XParam = processInfo.pInGUIParameters[m_uControlListCount + vectorJoystickX_Offset].fActualValue;
			float fActualVJS_YParam = processInfo.pInGUIParameters[m_uControlListCount + vectorJoystickY_Offset].fActualValue;

			// --- only update if one changed
			if(processInfo.pInGUIParameters[m_uControlListCount + vectorJoystickX_Offset].bDirty ||
				processInfo.pInGUIParameters[m_uControlListCount + vectorJoystickY_Offset].bDirty) // NOTE: you do not need to clear these flags!
			{
				m_JS_XCtrl.bKorgVectorJoystickOrientation = processInfo.pInGUIParameters[m_uControlListCount + vectorJoystickX_Offset].bKorgVectorJoystickOrientation;
				m_JS_YCtrl.bKorgVectorJoystickOrientation = m_JS_XCtrl.bKorgVectorJoystickOrientation;

				// --- add smoothing here...
				if(m_JS_XCtrl.bEnableParamSmoothing && !processInfo.bIgnoreSmoothing)
					m_JS_XCtrl.fSmoothingFloatValue = fActualVJS_XParam;
				else
					m_JS_XCtrl.fJoystickValue = fActualVJS_XParam;

				if(m_JS_YCtrl.bEnableParamSmoothing && !processInfo.bIgnoreSmoothing)
					m_JS_YCtrl.fSmoothingFloatValue = fActualVJS_YParam;
				else
					m_JS_YCtrl.fJoystickValue = fActualVJS_YParam;

				if(!m_JS_XCtrl.bEnableParamSmoothing || processInfo.bIgnoreSmoothing)
				{
					double dA = 0.0; double dB = 0.0; double dC = 0.0; double dD = 0.0; double dAC = 0.0; double dBD = 0.0;
					calculateRAFXVectorMixValues(m_JS_XCtrl.fJoystickValue, m_JS_YCtrl.fJoystickValue, dA, dB, dC, dD, dAC, dBD, m_JS_XCtrl.bKorgVectorJoystickOrientation);
					joystickControlChange(dA, dB, dC, dD, dAC, dBD);
				}
			}

			break;
		}

		// --- for outbound parameter changes:
		//     - LED and Analog Meter values
		case postProcessData:
		{
			// --- process GUI updates; thread safe!
			if(!m_pOutGUIParameters || !m_ppControlTable) return;

			// --- check to make sure param count matches (should never fail)
			if(processInfo.nNumParams != m_uControlListCount + numAddtlParams) return;

			// -- write out Meter Values
			//    NOTE: pPreProcessData->pGUIParameters is a COPY of the GUI Params, so you can't hurt anything
			for(int i=0; i<m_uControlListCount; i++)
			{
				if(m_ppControlTable[i]) // should never fail
				{
					// --- clear out dirty flags
					m_pOutGUIParameters[m_ppControlTable[i]->nGUIRow].bDirty = false;

					if(m_ppControlTable[i]->uControlType == FILTER_CONTROL_LED_METER && m_ppControlTable[i]->m_pCurrentMeterValue && m_ppControlTable[i]->nGUIRow >= 0)
					{
						m_pOutGUIParameters[m_ppControlTable[i]->nGUIRow].fNormalizedValue = *m_ppControlTable[i]->m_pCurrentMeterValue;
						m_pOutGUIParameters[m_ppControlTable[i]->nGUIRow].bDirty = true; // update me, I'm dirty!
					}
				}
			}

			// --- send back (NOTE: m_pOutGUIParameters is const so host can not mess with it)
			processInfo.pOutGUIParameters = m_pOutGUIParameters;
			break;
		}

		default:
			break;
	}
}

// --- message for updating GUI from plugin
bool __stdcall CPlugIn::checkUpdateGUI(int nControlIndex, float fValue, CLinkedList<GUI_PARAMETER>& guiParameters, bool bLoadingPreset)
{
	return false;
}

void __stdcall CPlugIn::clearUpdateGUIParameters(CLinkedList<GUI_PARAMETER>& guiParameters)
{
	guiParameters.deleteAll();
}

#if defined _WINDOWS || defined _WINDLL
void __stdcall CPlugIn::sendStatusWndText(char* pText)
{
	char   cText[1024];
    strncpy(cText, pText, 1023);
    cText[1023] = '\0';

	if(m_hParentWnd)
		SendMessage(m_hParentWnd, SEND_STATUS_WND_MESSAGE, 0, (LPARAM)&cText[0]);
}
#endif

// main message handler
void* __stdcall CPlugIn::showGUI(void* pInfo)
{
	VSTGUI_VIEW_INFO* info = (VSTGUI_VIEW_INFO*)pInfo;
	if(!info) return NULL;

#ifndef LEAN_RAFX_PLUGIN
	switch(info->message)
	{
		case GUI_RAFX_OPEN:
		{
			return CRafxViewFactory::createGUI(info, this);
		}
		case GUI_RAFX_CLOSE:
		{
			return CRafxViewFactory::destroyGUI();
		}
		case GUI_TIMER_PING:
		{
			return CRafxViewFactory::timerPing();
		}
		case GUI_RAFX_INIT:
		{
			return CRafxViewFactory::initControls();
		}
		case GUI_RAFX_SYNC:
		{
			return CRafxViewFactory::syncGUI();
		}
	}
#endif
	return NULL;
}

// --- process aux inputs (currently sidechain only)
bool __stdcall CPlugIn::processAuxInputBus(audioProcessData* pAudioProcessData)
{
	return true;
}

/* doVSTSampleAccurateParamUpdates
Short handler for VST3 sample accurate automation added in v6.8.0.5
There is nothing for you to modify here.
*/
void CPlugIn::doVSTSampleAccurateParamUpdates()
{
	// --- for sample accurate parameter automation in VST3 plugins; ignore otherwise
	if (!m_ppControlTable) return; /// should NEVER happen
	for (int i = 0; i < m_uControlListCount; i++)
	{
		if (m_ppControlTable[i] && m_ppControlTable[i]->pvAddlData)
		{
			double dValue = 0;
			if (((IParamUpdateQueue *)m_ppControlTable[i]->pvAddlData)->getNextValue(dValue))
			{
				setNormalizedParameter(m_ppControlTable[i], dValue, true);
			}
		}
	}
}

/* caller must delete the returned char* when done
	call like this:

	char* pDLLpath = getMyDLLDirectory();
	.. do something with the string

	delete [] pDLLPath; // its an array so you use []

*/
#if defined AUPLUGIN || defined AAXPLUGIN && !defined _WINDOWS && !defined _WINDLL
// --- for MacOS AU Plugins
char* CPlugIn::getMyDLLDirectory(CFStringRef bundleID)
{
    if (bundleID != NULL)
    {
        CFBundleRef helixBundle = CFBundleGetBundleWithIdentifier( bundleID );
        if(helixBundle != NULL)
        {
            CFURLRef bundleURL = CFBundleCopyBundleURL ( helixBundle );
            if(bundleURL != NULL)
            {
                CFURLRef componentFolderPathURL = CFURLCreateCopyDeletingLastPathComponent(NULL, bundleURL);
                CFStringRef myComponentPath = CFURLCopyFileSystemPath(componentFolderPathURL, kCFURLPOSIXPathStyle);
                CFRelease(componentFolderPathURL);
                if(myComponentPath != NULL)
                {
                    int nSize = CFStringGetLength(myComponentPath);
                    char* path = new char[nSize+1];
                    memset(path, 0, (nSize+1)*sizeof(char));
                    bool success = CFStringGetCString(myComponentPath, path, nSize+1, kCFStringEncodingASCII);
                    CFRelease(myComponentPath);
                    if(success) return path;
                    else return NULL;
                }
                CFRelease(bundleURL);
            }
        }
        CFRelease(bundleID);
    }
    return NULL;
}
#else
char* __stdcall CPlugIn::getMyDLLDirectory()
{
#if defined _WINDOWS || defined _WINDLL
	bool bIsVST3 = false;

	HMODULE hmodule = GetModuleHandle(m_PlugInName);
	if (!hmodule)
	{
		char* vst3Plugin = addStrings(m_PlugInName, ".vst3");
		hmodule = GetModuleHandle(vst3Plugin);
		delete[] vst3Plugin;
		if (hmodule)
			bIsVST3 = true;
		else
			return false;
	}

	char dir[MAX_PATH];
	memset(&dir[0], 0, MAX_PATH*sizeof(char));
	dir[MAX_PATH-1] = '\0';

	if(hmodule)
		GetModuleFileName(hmodule, &dir[0], MAX_PATH);
	else
		return NULL;

	int nLenDir = strlen(dir);
	if(nLenDir <= 0)
		return NULL;

	char* pDLLRoot = new char[MAX_PATH];

	// --- fixed bug in beta 6.8.0.8
	int nLenDLL = !bIsVST3 ? strlen(m_PlugInName) + 5 : strlen(m_PlugInName) + 6; // .dll = 4, .vst3 = 5
	memcpy(pDLLRoot, &dir[0], nLenDir-nLenDLL);
	pDLLRoot[nLenDir-nLenDLL] = '\0';

	return pDLLRoot;
#else
	// --- for MacOS VST Plugins
	char* pDLLRoot = new char[2048];
    int nLenDir = strlen(&gPath);
	if(nLenDir <= 0)
		return NULL;

  	memcpy(pDLLRoot, &gPath, nLenDir);
    char *pos = strrchr(pDLLRoot, '//');
    if (pos != NULL) {
        *pos = '\0'; //this will put the null terminator here.
    }
	return pDLLRoot;
#endif
}
#endif










