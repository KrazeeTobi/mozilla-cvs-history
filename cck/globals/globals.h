#include "stdafx.h"
#include "WizardTypes.h"

extern __declspec(dllimport) WIDGET 	GlobalWidgetArray[];
extern __declspec(dllimport) int 		GlobalArrayIndex;

extern "C" __declspec(dllimport) char  * GetGlobal(CString theName);
extern "C" __declspec(dllimport) WIDGET* SetGlobal(CString theName, CString theValue);
extern "C" __declspec(dllimport) WIDGET* findWidget(CString theName);
