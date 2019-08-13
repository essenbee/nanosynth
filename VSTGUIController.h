#ifndef __CVSTGUIController__
#define __CVSTGUIController__

extern void* hInstance;   // Sock2VST3 LIB module handle
extern void* g_hModule;   // DLL module handle

#include "../vstgui4/vstgui/vstgui.h"
#include "plugin.h"
#include <cstdio>
#include <string>
#include <vector>
#include <map>

// --- AU supoort
#ifdef AUPLUGIN
    #include <AudioToolbox/AudioToolbox.h>
#endif

// --- AAX support
#ifdef AAXPLUGIN
#include "AAX_IEffectParameters.h"
#endif

using namespace std;

namespace VSTGUI {

// --- tiny helper object for VST2/3 parameter sync
class CVSTParameterConnector
{
public:
	CVSTParameterConnector(){};
	~CVSTParameterConnector(){};
	virtual void beginValueChange(int nTag){};
	virtual void endValueChange(int nTag){};
	virtual void guiControlValueChanged(int nTag, float fNormalizedValue){};
};

// --- the main VSTGUI object, platform independent
class CVSTGUIController : public VSTGUIEditorInterface, public IControlListener, public CBaseObject
{
public:
	CVSTGUIController();
	virtual ~CVSTGUIController();

	// --- MAIN OVERRIDES: you need to implement these on your derived class
	//
	// --- the return variables are the window width/height in pixels, passed back via the info struct
	virtual bool open(CPlugIn* pPlugIn, VSTGUI_VIEW_INFO* info);

	// --- close function; destroy frame and forget timer; optional oeverride
	virtual void close();

	// --- do idle processing; optional override
	virtual void idle();

	// --- VSTGUIEditorInterface override; you must override this if you want to alter the knob mode
	virtual int32_t getKnobMode() const;

	// --- IControlListener override (pure abstract, so you **must** override this
	virtual void valueChanged(VSTGUI::CControl* pControl) = 0;

	// --- for VST only, these are handled for you on this class, nothing for you to do
	virtual void controlBeginEdit(CControl* pControl);
	virtual void controlEndEdit(CControl* pControl);

	// --- you must override this functions to set your control locations based on the normalized values
	virtual void setGUIControlsWithNormalizedParameter(int tag, double value, VSTGUI::CControl* pControl = NULL) = 0;

	// --- this implementation calculates the actual value and then calls the function above; override if you wish
	virtual void setGUIControlsWithActualParameter(int tag, double value, VSTGUI::CControl* pControl = NULL);
		
	// --- helpers for above functions
	void updateTextControl(CTextLabel* pControl, int tag, double normalizedValue);

	// --- timer norification callback
   	CMessageResult notify(CBaseObject* sender, const char* message);

	// --- get a bitmap (PNG FILES ONLY!)
	CBitmap* getBitmap(const CResourceDescription& desc, CCoord left = -1, CCoord top = -1, CCoord right = -1, CCoord bottom = -1);
	
	// --- bitmap helpers
	CNinePartTiledBitmap* loadTiledBitmap(const CResourceDescription& desc, CCoord left, CCoord top, CCoord right, CCoord bottom);
	CBitmap* loadBitmap(const CResourceDescription& desc);

	// --- plugin helpers
	//
	// --- RAFX uses non-zero-indexed control ID values
	//     Both the RAFX-as-VST-DLL and [Make VST] use zero-idexed values (tags)
	//     
	//     These are conversion functions to go between the two ID values as needed: 
	//     In general, anything involving a GUI control is going to use a tag, while
	//     things involving RAFX parameter objects will use the ControlId value
	int getTagForRAFXControlId(int nControlId);
	int getRAFXControlIdForTag(int nTag);
    
    // --- helpers to convert back and forth from normalized to actual values;
    //     bWarpValue = apply log or volt/octave tapering
    float getActualValue(float fNormalizedValue, CUICtrl* pUICtrl, bool bWarpValue = false);
    float getNormalizedValue(int tag, double actualValue, bool bWarpValue = false);

	// --- for RAFX plugins
	bool m_bIsRAFXPlugin;
	float getRAFXParameterNormalized(int nIndex);

	// --- for AU plugins
#ifdef AUPLUGIN
    Float32 getAUParameterActual(int nIndex);
#endif

#ifdef AAXPLUGIN
	double getAAXParameterNormalized(int nIndex);
#endif

protected:
	void* m_hPlugInInstance;	// HINSTANCE of this DLL (WinOS only) may NOT be NULL

	// --- our plugin
	CPlugIn* m_pPlugIn;
	
	// --- for VST only 
	CVSTParameterConnector* m_pVSTParameterConnector;

	// --- RAFX GUI Syncbronizer
	GUI_PARAM_SYNCH_STRUCT* m_pRAFXGUISynchronizer; 

	// --- AU only
#ifdef AUPLUGIN
	// --- the AU Instance
    AudioUnit m_AUInstance;

	// --- the event listener
    AUEventListenerRef m_AUEventListener;
#endif
    
	// --- AAX Only
#ifdef AAXPLUGIN
    AAX_IEffectParameters* m_pAAXParameters;
#endif

	// --- timer for GUI updates
    CVSTGUITimer* timer;

// --- miscellaneous functions
public:
	// --- helpers for log/volt-octave controls
	static inline float fastpow2 (float p)
	{
	  float offset = (p < 0) ? 1.0f : 0.0f;
	  float clipp = (p < -126) ? -126.0f : p;
	  int w = clipp;
	  float z = clipp - w + offset;
	  union { unsigned int i; float f; } v = { static_cast<unsigned int> ( (1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) ) };

	  return v.f;
	}

	static inline float fastlog2 (float x)
	{
	  union { float f; unsigned int i; } vx = { x };
	  union { unsigned int i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
	  float y = vx.i;
	  y *= 1.1920928955078125e-7f;

	  return y - 124.22551499f
			   - 1.498030302f * mx.f
			   - 1.72587999f / (0.3520887068f + mx.f);
	}
	inline float calcLogParameter(double fNormalizedParam)
	{
		return (pow(10.0, fNormalizedParam) - 1.0)/9.0;
	}

	inline float calcLogPluginValue(double fPluginValue)
	{
		return log10(9.0*fPluginValue + 1.0);
	}

	inline float calcVoltOctaveParameter(double fCookedParam, CUICtrl* pCtrl)
	{
		double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);				
		return fastlog2(fCookedParam/pCtrl->fUserDisplayDataLoLimit)/dOctaves;
	}
	
	inline float calcVoltOctavePluginValue(double fPluginValue, CUICtrl* pCtrl)
	{
		if(pCtrl->uUserDataType == UINTData)
			return *(pCtrl->m_pUserCookedUINTData);

		double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);				
		float fDisplay = pCtrl->fUserDisplayDataLoLimit*fastpow2(fPluginValue*dOctaves); //(m_fDisplayMax - m_fDisplayMin)*value + m_fDisplayMin; //m_fDisplayMin*fastpow2(value*dOctaves);
		float fDiff = pCtrl->fUserDisplayDataHiLimit - pCtrl->fUserDisplayDataLoLimit;
		return (fDisplay - pCtrl->fUserDisplayDataLoLimit)/fDiff;
	}

	inline int getEnumStringIndex(char* enumString, const char* testString)
	{
		string sEnumStr(enumString);
		string sTestStr(testString);
		int index = 0;
		bool bWorking = true;
		while(bWorking)
		{
			int nComma = sEnumStr.find_first_of(',');
			if(nComma <= 0)
			{
				if(sEnumStr == sTestStr)
					return index;

				bWorking = false;
			}
			else
			{
				string sL = sEnumStr.substr(0, nComma);
				sEnumStr = sEnumStr.substr(nComma+1);

				if(sL == sTestStr)
					return index;

				index++;
			}
		}

		return -1;
	}

	// --- old VST2 function for safe strncpy()
	inline char* vst_strncpy(char* dst, const char* src, size_t maxLen)
	{
		char* result = strncpy(dst, src, maxLen);
		dst[maxLen] = 0;
		return result;
	}

	// --- NOTE: string is returned via stringstream
	inline bool getEnumString(char* string, int index, stringstream& enumString)
	{
		int nLen = strlen(string);
		char* copyString = new char[nLen+1];

		vst_strncpy(copyString, string, strlen(string));
		
		for(int i=0; i<index+1; i++)
		{
			char* comma = ",";
			int j = strcspn(copyString,comma);
			if(i == index)
			{
				char* pType = new char[j+1];
				strncpy(pType, copyString, j);
				pType[j] = '\0';

				if(strcmp(pType, "SWITCH_OFF") == 0)
				{
					delete [] pType;
					enumString << "OFF";
				}
				else if(strcmp(pType, "SWITCH_ON") == 0)
				{
					delete [] pType;
					enumString << "ON";
				}
				else 
				{
					enumString << pType;
					delete [] pType;
				}

				delete [] copyString;
				return true;
			}
			else // remove it
			{
				char* pch = strchr(copyString,',');

				if(!pch)
				{
					delete [] copyString;
					return false; // out of bounds index
				}

				int nLen = strlen(copyString);
				memcpy (copyString,copyString+j+1,nLen-j);
			}
		}

		delete [] copyString;
		return false;
	}

	// --- called when value changes in text edit; it needs to store the new stringindex value and update itself
	//     this is also called to update the control after an edit of another control with same tag
	//     NOTE: this works for BOTH CTextLabel and CTextEdit
	//     NOTE: this function does NOT write the new value into the CUICtrl object, only updates with what the value should be (thread safety)
	inline float parseAndUpdateTextControl(CTextLabel* pControl, CUICtrl* pCtrl)
	{
		if(!pControl) return -1.0;
		if(!pCtrl) return -1.0;

		const char* p = pControl->getText();
		unsigned int uPrecision = pControl->getPrecision();

		float fValue = 0.0;
		switch(pCtrl->uUserDataType)
		{
			case floatData:
			{
				float f = atof(p);
				if(f > pCtrl->fUserDisplayDataHiLimit) f = pCtrl->fUserDisplayDataHiLimit;
				if(f < pCtrl->fUserDisplayDataLoLimit) f = pCtrl->fUserDisplayDataLoLimit;
				p = floatToString(f,uPrecision);

				float fDiff = pCtrl->fUserDisplayDataHiLimit - pCtrl->fUserDisplayDataLoLimit;
				float fCookedData = (f - pCtrl->fUserDisplayDataLoLimit)/fDiff;

				// --- v6.6
				if(pCtrl->bLogSlider)
					fValue = calcLogParameter(fCookedData); //(pow((float)10.0, fCookedData) - 1.0)/9.0;
				else if(pCtrl->bExpSlider)
				{
					fValue = calcVoltOctaveParameter(f, pCtrl);
				}
				else
					fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, f);

				pControl->setValue(fValue);
				pControl->setText(p);
				break;
			}
			case doubleData:
			{
				float f = atof(p);
				if(f > pCtrl->fUserDisplayDataHiLimit) f = pCtrl->fUserDisplayDataHiLimit;
				if(f < pCtrl->fUserDisplayDataLoLimit) f = pCtrl->fUserDisplayDataLoLimit;
				p = floatToString(f,uPrecision);

				float fDiff = pCtrl->fUserDisplayDataHiLimit - pCtrl->fUserDisplayDataLoLimit;
				float fCookedData = (f - pCtrl->fUserDisplayDataLoLimit)/fDiff;

				if(pCtrl->bLogSlider)
					fValue = calcLogParameter(fCookedData);
				else if(pCtrl->bExpSlider)
				{
					fValue = calcVoltOctaveParameter(f, pCtrl);
				}
				else
					fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, f);

				pControl->setValue(fValue);
				pControl->setText(p);
				break;
			}
			case intData:
			{
				int f = atoi(p);
				if(f > pCtrl->fUserDisplayDataHiLimit) f = pCtrl->fUserDisplayDataHiLimit;
				if(f < pCtrl->fUserDisplayDataLoLimit) f = pCtrl->fUserDisplayDataLoLimit;
				p = intToString(f);

				float fDiff = pCtrl->fUserDisplayDataHiLimit - pCtrl->fUserDisplayDataLoLimit;
				float fCookedData = (f - pCtrl->fUserDisplayDataLoLimit)/fDiff;

				if(pCtrl->bLogSlider)
					fValue = calcLogParameter(fCookedData);
				else if(pCtrl->bExpSlider)
				{
					fValue = calcVoltOctaveParameter(f, pCtrl);
				}
				else
					fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, f);

				pControl->setValue(fValue);
				pControl->setText(p);
				break;
			}
			case UINTData:
			{
				string str(p);
				string list(pCtrl->cEnumeratedList);
				if(list == "SWITCH_OFF,SWITCH_ON")
					list.assign("OFF,ON");
				if(list.find(str) == -1) // this should never happen
				{
					fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, 0);
					pControl->setValue(fValue);
					stringstream enumString;
					getEnumString(pCtrl->cEnumeratedList, 0, enumString);
					pControl->setText(enumString.str().c_str());
				}
				else
				{
					int t = getEnumStringIndex((char*)list.c_str(), p);
					if(t < 0)
					{
						// this should never happen...
						stringstream enumString;
						getEnumString(pCtrl->cEnumeratedList, 0, enumString);
						pControl->setText(enumString.str().c_str());
						fValue = 0.0;
						pControl->setValue(fValue);
					}
					else
					{
						fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, (float)t);
						pControl->setValue(fValue);
						pControl->setText(str.c_str());
					}
				}

				break;
			}
			default:
				break;
		}
		return fValue;
	}
};

}

#endif // __CRafxVSTEditor__



