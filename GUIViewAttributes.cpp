#include "GUIViewAttributes.h"
extern void* hInstance;   // Sock2VST3 LIB module handle
extern void* g_hModule;   // DLL module handle

CVSTGUIHelper::CVSTGUIHelper(void)
{
}

CVSTGUIHelper::~CVSTGUIHelper(void)
{

}

CBitmap* CVSTGUIHelper::loadBitmap(VSTGUI_VIEW_INFO* info)
{
	CBitmap* pBitmap = NULL;
	if(strlen(info->customViewBitmapName) == 0)
		return NULL;

	if(info->isNinePartTiledBitmap)
		pBitmap = loadTiledBitmap(info->customViewBitmapName, info->nptoLeft, info->nptoTop, info->nptoRight, info->nptoBottom);
	else
		pBitmap = loadBitmap(info->customViewBitmapName);

	return pBitmap;
}

CBitmap* CVSTGUIHelper::loadHandleBitmap(VSTGUI_VIEW_INFO* info)
{
	CBitmap* pBitmap = NULL;
	if(strlen(info->customViewBitmapName) == 0)
		return NULL;

	if(info->isNinePartTiledBitmap)
		pBitmap = loadTiledBitmap(info->customViewHandleBitmapName, info->nptoLeft, info->nptoTop, info->nptoRight, info->nptoBottom);
	else
		pBitmap = loadBitmap(info->customViewHandleBitmapName);

	return pBitmap;
}

CBitmap* CVSTGUIHelper::loadOffBitmap(VSTGUI_VIEW_INFO* info)
{
	CBitmap* pBitmap = NULL;
	if(strlen(info->customViewBitmapName) == 0)
		return NULL;

	if(info->isNinePartTiledBitmap)
		pBitmap = loadTiledBitmap(info->customViewOffBitmapName, info->nptoLeft, info->nptoTop, info->nptoRight, info->nptoBottom);
	else
		pBitmap = loadBitmap(info->customViewOffBitmapName);

	return pBitmap;
}


CNinePartTiledBitmap* CVSTGUIHelper::loadTiledBitmap(const CResourceDescription& desc, CCoord left, CCoord top, CCoord right, CCoord bottom)
{

#if defined _WINDOWS || defined _WINDLL
	// --- save
	void* pInstance = hInstance;

	HRSRC rsrc = FindResourceA((HMODULE)hInstance, desc.u.name, "PNG");
	if(!rsrc)
		hInstance = g_hModule;

	#ifdef VSTGUI_43
		CNinePartTiledBitmap* pBM = new CNinePartTiledBitmap(desc, (CNinePartTiledDescription(left, top, right, bottom)));
	#else
		CNinePartTiledBitmap* pBM = new CNinePartTiledBitmap(desc, (CNinePartTiledBitmap::PartOffsets(left, top, right, bottom)));
	#endif

	// --- restore
	hInstance = pInstance;

	return pBM;

#else

	#ifdef VSTGUI_43
		CNinePartTiledBitmap* pBM = new CNinePartTiledBitmap(desc, (CNinePartTiledDescription(left, top, right, bottom)));
	#else
		CNinePartTiledBitmap* pBM = new CNinePartTiledBitmap(desc, (CNinePartTiledBitmap::PartOffsets(left, top, right, bottom)));
	#endif
	return pBM;

#endif

}

CBitmap* CVSTGUIHelper::loadBitmap(const CResourceDescription& desc)
{
#if defined _WINDOWS || defined _WINDLL
	// --- save
	void* pInstance = hInstance;

	// --- choose the proper resource stream
	HRSRC rsrc = FindResourceA((HMODULE)hInstance, desc.u.name, "PNG");
	if(!rsrc)
		hInstance = g_hModule;

	CBitmap* pBM = new CBitmap(desc);

	// --- restore
	hInstance = pInstance;

	return pBM;

#else

	CBitmap* pBM = new CBitmap(desc);
	return pBM;

#endif

}


