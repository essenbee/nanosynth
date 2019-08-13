#include "VSTGUIController.h"

namespace VSTGUI
{

// --- CTor
CVSTGUIController::CVSTGUIController()
{
	m_pVSTParameterConnector = NULL;
	m_pRAFXGUISynchronizer = NULL;
	m_bIsRAFXPlugin = false;

#ifdef AAXPLUGIN
    m_pAAXParameters = NULL;
#endif
	// create a timer used for idle update: will call notify method
	timer = new CVSTGUITimer (dynamic_cast<CBaseObject*>(this));
}

// --- DTor
CVSTGUIController::~CVSTGUIController()
{
	// --- stop timer
	if(timer)
		timer->forget();
}

// --- timer notification
CMessageResult CVSTGUIController::notify(CBaseObject* /*sender*/, const char* message)
{
    if(message == CVSTGUITimer::kMsgTimer)
    {
        if(frame)
            idle();

        return kMessageNotified;
    }
    return kMessageUnknown;
}

// --- you can adjust the knob mode by changing the return type
int32_t CVSTGUIController::getKnobMode() const
{
	/* choices are: kLinearMode;
				    kRelativCircularMode;
					kCircularMode; */

	return kLinearMode;
}

// --- base class open() -- MUST be called first in derived class version
bool CVSTGUIController::open(CPlugIn* pPlugIn, VSTGUI_VIEW_INFO* info)
{
	if(!info->window) return false;

	// --- our plugin
	m_pPlugIn = pPlugIn;

	// --- there are different sync methods for different API's
	if (info->hRAFXInstance)
	{
		// --- save flag for RAFX only
		m_bIsRAFXPlugin = true;

		// --- For RackAFX:
		m_pRAFXGUISynchronizer = (GUI_PARAM_SYNCH_STRUCT*)info->pGUISynchronizer;
	}
	else // must be AAX, AU or VST
	{
		#if !defined AUPLUGIN && !defined AAXPLUGIN
		// --- For VST:
		m_pVSTParameterConnector = (CVSTParameterConnector*)info->pGUISynchronizer;
		#endif
	}

#ifdef AAXPLUGIN
	m_pAAXParameters = (AAX_IEffectParameters*)info->pAAXParameters;
#endif

#if MAC && AUPLUGIN
	m_AUInstance = (AudioUnit)info->hPlugInInstance; // this is the AU AudioComponentInstance
#else
	m_hPlugInInstance = info->hPlugInInstance; // this is needed to ensure the resources exist
#endif

    // --- set/start the timer
	if(timer)
	{
        timer->setFireTime(METER_UPDATE_INTERVAL_MSEC);
        timer->start();
    }

	return true;
}

// --- close()
void CVSTGUIController::close()
{
	if(!frame) return;

	// --- stop timer
	if(timer)
		timer->stop();

	// --- for AU; dispose of event listener
#ifdef AUPLUGIN
	if (m_AUEventListener) verify_noerr(AUListenerDispose(m_AUEventListener));
	m_AUEventListener = NULL;
	m_AUInstance = NULL;
#endif

	//-- on close we need to delete the frame object.
	//-- once again we make sure that the member frame variable is set to zero before we delete it
	//-- so that calls to setParameter won't crash.
	CFrame* oldFrame = frame;
	frame = 0;
	oldFrame->forget(); // this will remove/destroy controls
}

// --- idle loop processing
void CVSTGUIController::idle()
{
	// --- then, update frame - important; this updates edit boxes, sliders, etc...
	if(frame)
		frame->idle();
}

// --- helper to get a bitmap
CBitmap* CVSTGUIController::getBitmap(const CResourceDescription& desc, CCoord left, CCoord top, CCoord right, CCoord bottom)
{
	// --- if coords are all >= 0 then this is a nine-part tiled, else normal
	if (left >= 0 && top >= 0 && right >= 0 && bottom >= 0)
		return loadTiledBitmap(desc, left, top, right, bottom);
	else
		return loadBitmap(desc);

	return NULL; // should never happen
}

// --- helper to load a tiled bitmap
CNinePartTiledBitmap* CVSTGUIController::loadTiledBitmap(const CResourceDescription& desc, CCoord left, CCoord top, CCoord right, CCoord bottom)
{
#if defined _WINDOWS || defined _WINDLL
	// --- save
	// void* pInstance = hInstance;

	// HRSRC rsrc = FindResourceA((HMODULE)hInstance, desc.u.name, "PNG");
	// if (!rsrc)
		// hInstance = g_hModule;

	CNinePartTiledBitmap* pBM = new CNinePartTiledBitmap(desc, (CNinePartTiledDescription(left, top, right, bottom)));

	// --- restore
	// hInstance = pInstance;

	return pBM;

#else
	CNinePartTiledBitmap* pBM = new CNinePartTiledBitmap(desc, (CNinePartTiledDescription(left, top, right, bottom)));
	return pBM;
#endif
}

// --- helper to load a bitmap
CBitmap* CVSTGUIController::loadBitmap(const CResourceDescription& desc)
{
#if defined _WINDOWS || defined _WINDLL
	// --- save
	// void* pInstance = hInstance;

	// --- choose the proper resource stream
	// HRSRC rsrc = FindResourceA((HMODULE)hInstance, desc.u.name, "PNG");
	// if (!rsrc)
		// hInstance = g_hModule;

	CBitmap* pBM = new CBitmap(desc);

	// --- restore
	// hInstance = pInstance;

	return pBM;

#else
	CBitmap* pBM = new CBitmap(desc);
	return pBM;
#endif

}

// --- handle actual values (AU and checkSendUpdateGUI())
void CVSTGUIController::setGUIControlsWithActualParameter(int tag, double actualValue, VSTGUI::CControl* pControl)
{
	// --- convert to normalized value
	float fNormalizedValue = getNormalizedValue(tag, actualValue, true);
	setGUIControlsWithNormalizedParameter(tag, fNormalizedValue, pControl);
}

// --- for VST/AAX only
void CVSTGUIController::controlBeginEdit(CControl* pControl)
{
	if(m_pVSTParameterConnector)
	{
		m_pVSTParameterConnector->beginValueChange(pControl->getTag());
	}
#ifdef AAXPLUGIN
    if(m_pAAXParameters )
    {
        int nTag = pControl->getTag();
        std::stringstream str;
        str << nTag+1;
        m_pAAXParameters->TouchParameter(str.str().c_str());
    }
#endif
 }

// --- for VST/AAX only
void CVSTGUIController::controlEndEdit(CControl* pControl)
{
	if(m_pVSTParameterConnector)
	{
		m_pVSTParameterConnector->endValueChange(pControl->getTag());
	}
#ifdef RAFX_AAX_PLUGIN
    if(m_pAAXParameters )
    {
        int nTag = pControl->getTag();
        std::stringstream str;
        str << nTag+1;
        m_pAAXParameters->ReleaseParameter(str.str().c_str());
    }
#endif
}

// --- get the normalized value of a parameter from the actual value
float CVSTGUIController::getNormalizedValue(int tag, double actualValue, bool bWarpValue)
{
	// --- get the control for re-broadcast of some types
	CUICtrl* pCtrl = m_pPlugIn->getUICtrlByListIndex(tag);
	if(!pCtrl) return 0.0;

	float fNormalizedValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, actualValue);

	if(bWarpValue)
	{
		if(pCtrl->bLogSlider && pCtrl->uUserDataType != UINTData)
		{
			fNormalizedValue = calcLogParameter(fNormalizedValue);
		}
		else if(pCtrl->bExpSlider && pCtrl->uUserDataType != UINTData)
		{
			fNormalizedValue = calcVoltOctaveParameter(actualValue, pCtrl);
		}
	}

	return fNormalizedValue;
}

// --- get the actual value of a parameter from the normalized value
float CVSTGUIController::getActualValue(float fNormalizedValue, CUICtrl* pUICtrl, bool bWarpValue)
{
	if(bWarpValue)
	{
		if(pUICtrl->bLogSlider && pUICtrl->uUserDataType != UINTData)
		{
			fNormalizedValue = calcLogPluginValue(fNormalizedValue);
		}
		else if(pUICtrl->bExpSlider && pUICtrl->uUserDataType != UINTData)
		{
			if(pUICtrl->fUserDisplayDataLoLimit > 0)
			{
				fNormalizedValue = calcVoltOctavePluginValue(fNormalizedValue, pUICtrl);
			}
		}
	}
	float fActualValue = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit,
											 pUICtrl->fUserDisplayDataHiLimit,
											 fNormalizedValue);
	return fActualValue;

}

// --- for RAFX only: get the normalized parameter
float CVSTGUIController::getRAFXParameterNormalized(int nIndex)
{
	float fNormalizedValue = 0.0;
#if defined _WINDOWS || defined _WINDLL
	if(nIndex < 0) return fNormalizedValue;
	if(!m_pPlugIn || !m_pRAFXGUISynchronizer) return fNormalizedValue;

	CUICtrl* pUICtrl = m_pPlugIn->getUICtrlByListIndex(nIndex);
	if(!pUICtrl) return fNormalizedValue;

	EnterCriticalSection(&m_pRAFXGUISynchronizer->cs);
		fNormalizedValue = m_pRAFXGUISynchronizer->pGUIParameters[nIndex].fNormalizedValue;
	LeaveCriticalSection(&m_pRAFXGUISynchronizer->cs);

	if(pUICtrl->bLogSlider && pUICtrl->uUserDataType != UINTData)
	{
		fNormalizedValue = calcLogParameter(fNormalizedValue);
	}
	else if(pUICtrl->bExpSlider && pUICtrl->uUserDataType != UINTData)
	{
		float fActualValue = getActualValue(fNormalizedValue, pUICtrl);
		fNormalizedValue = calcVoltOctaveParameter(fActualValue, pUICtrl);
	}
#endif
	return fNormalizedValue;
}

// --- for AU only: get the actual parameter
#ifdef AUPLUGIN
Float32 CVSTGUIController::getAUParameterActual(int nIndex)
{
    Float32 paramValue = 0.0;
    AudioUnitGetParameter(m_AUInstance, nIndex, kAudioUnitScope_Global, 0, &paramValue);
    return paramValue;
}
#endif

// --- for AAX only: get the normalized value
#ifdef AAXPLUGIN
double CVSTGUIController::getAAXParameterNormalized(int nIndex)
{
	double dParam = 0.0;
    if(m_pAAXParameters)
    {
        std::stringstream str;
        str << nIndex + 1;
        m_pAAXParameters->GetParameterNormalizedValue(str.str().c_str(), &dParam);
    }
	return dParam;
	}
#endif

// --- helpers to simplify linking tags to controlId values
int CVSTGUIController::getTagForRAFXControlId(int nControlId)
{
	if(!m_pPlugIn) return -1;

	CUICtrl* pUICtrl = m_pPlugIn->getUICtrlByControlID(nControlId);
	if(!pUICtrl) return - 1; // failure
	return pUICtrl->nGUIRow; // index of control, sync'd to VST/AU/AAX/RAFX parameters
}
int CVSTGUIController::getRAFXControlIdForTag(int nTag)
{
	if(!m_pPlugIn) return -1;

	CUICtrl* pUICtrl = m_pPlugIn->getUICtrlByListIndex(nTag);
	if(!pUICtrl) return - 1; // failure
	return pUICtrl->uControlId; // index of control
}

// --- helper to make updating text controls (CTextEdit and CTextLabel) easy
void CVSTGUIController::updateTextControl(CTextLabel* pControl, int tag, double normalizedValue)
{
	CUICtrl* pUICtrl = m_pPlugIn->getUICtrlByListIndex(tag);
	if(!pUICtrl) return;

	// --- do warping
    if(pUICtrl->bLogSlider)
    {
        normalizedValue = calcLogPluginValue(normalizedValue);
    }
    else if(pUICtrl->bExpSlider)
    {
        normalizedValue = calcVoltOctavePluginValue(normalizedValue, pUICtrl);
    }

    float fDisplayVariable = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, normalizedValue);
	stringstream ss;
	unsigned int uPrecision = pControl->getPrecision();

    switch(pUICtrl->uUserDataType)
    {
        case floatData:
		case doubleData:
        {
			ss.precision(uPrecision);
			ss << fixed << fDisplayVariable;
            break;
        }
        case intData:
        {
            ss << (int)fDisplayVariable;
            break;
        }
        case UINTData:
        {
            getEnumString(pUICtrl->cEnumeratedList, floor(fDisplayVariable + 0.5), ss);
            break;
        }
        default:
            break;
	}
	pControl->setText(ss.str().c_str());
}

}




