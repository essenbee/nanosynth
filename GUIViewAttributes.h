#ifndef GUIVIEWATTRIBUTES
#define GUIVIEWATTRIBUTES

#include "PlugIn.h"

#include <cstdio>
#include <string>
#include <vector>
#include <map>

#include "../vstgui4/vstgui/vstgui.h"

// --- comment out for VSTUI4.2 and lower
#define VSTGUI_43

using namespace std;
using namespace VSTGUI;

// --- object to search attributes
class CVSTGUIHelper
{
public:
	CVSTGUIHelper(void);
	~CVSTGUIHelper(void);

	CNinePartTiledBitmap* loadTiledBitmap(const CResourceDescription& desc, CCoord left, CCoord top, CCoord right, CCoord bottom);
	CBitmap* loadBitmap(const CResourceDescription& desc);
	CBitmap* loadBitmap(VSTGUI_VIEW_INFO* info);
	CBitmap* loadHandleBitmap(VSTGUI_VIEW_INFO* info);
	CBitmap* loadOffBitmap(VSTGUI_VIEW_INFO* info);

	// --- helpers for building control attributes from string attributes
	inline const CRect getRectWithVSTGUIRECT(VSTGUIRECT vstguiRect)
	{
		CRect rect(vstguiRect.left, vstguiRect.top, vstguiRect.right, vstguiRect.bottom);
		return rect;
	}

	inline const CPoint getPointWithVSTGUIPOINT(VSTGUIPOINT vstguiPoint)
	{
		CPoint point(vstguiPoint.x, vstguiPoint.y);
		return point;
	}
};

#endif
