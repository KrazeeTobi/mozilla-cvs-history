/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.    Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#include "nsIPref.h"

#include "nsIFileSpec.h"
#include "nsIModule.h"
#include "nsIGenericFactory.h"

#include "pratom.h"
#include "prefapi.h"
#include "nsIFactory.h"
#include "nsIComponentManager.h"
#ifdef XP_MAC
#include "nsIPrompt.h"
#include "nsIStreamListener.h"
#endif /* XP_MAC */
#include "nsIServiceManager.h"
#include "nsIFileLocator.h"
#include "nsCOMPtr.h"
#include "nsFileLocations.h"
#include "nsFileStream.h"
#include "nsIProfile.h"
#include "nsQuickSort.h"

#include "nsTextFormater.h"

#include "plhash.h"
#include "prmem.h"
#include "plstr.h"
#include "prprf.h"

#include "nsIJSRuntimeService.h"
#include "jsapi.h"

#ifdef _WIN32
#include "windows.h"
#endif /* _WIN32 */

#define PREFS_HEADER_LINE_1 "# Mozilla User Preferences"
#define PREFS_HEADER_LINE_2	"// This is a generated file!"

#include "prefapi_private_data.h"

static NS_DEFINE_CID(kFileLocatorCID,       NS_FILELOCATOR_CID);

//========================================================================================
class nsPref: public nsIPref
//========================================================================================
{
    NS_DECL_ISUPPORTS

public:
    static nsPref *GetInstance();

#ifdef PREF_SUPPORT_OLD_PATH_STRINGS
    NS_IMETHOD ReadUserJSFile(const char *filename); // deprecated
    NS_IMETHOD ReadLIJSFile(const char *filename); // deprecated
    NS_IMETHOD EvaluateConfigScript(const char * js_buffer, PRUint32 length,
                    const char* filename, 
                    PRBool bGlobalContext, 
                    PRBool bCallbacks); // deprecated
    NS_IMETHOD SavePrefFileAs(const char *filename);
    NS_IMETHOD SaveLIPrefFile(const char *filename);

    // Path prefs
    NS_IMETHOD CopyPathPref(const char *pref, char ** return_buf);
    NS_IMETHOD SetPathPref(const char *pref, 
             const char *path, PRBool set_default);
#endif

    /* Use xpidl-generated macro to declare everything required by nsIPref */
    NS_DECL_NSIPREF

protected:

    nsPref();
    virtual ~nsPref();

    nsresult useDefaultPrefFile();
    nsresult useUserPrefFile();
    static nsPref *gInstance;
    
    nsIFileSpec*                    mFileSpec;
    nsIFileSpec*                    mLIFileSpec;
}; // class nsPref

nsPref* nsPref::gInstance = NULL;

static PRInt32 g_InstanceCount = 0;

static PrefResult pref_OpenFileSpec(
    nsIFileSpec* fileSpec,
    PRBool is_error_fatal,
    PRBool verifyHash,
    PRBool bGlobalContext,
    PRBool skipFirstLine);

//----------------------------------------------------------------------------------------
static nsresult _convertRes(int res)
//----------------------------------------------------------------------------------------
{
    switch (res)
    {
        case PREF_OUT_OF_MEMORY:
            return NS_ERROR_OUT_OF_MEMORY;
        case PREF_NOT_INITIALIZED:
            return NS_ERROR_NOT_INITIALIZED;
        case PREF_TYPE_CHANGE_ERR:
        case PREF_ERROR:
        case PREF_BAD_LOCKFILE:
	case PREF_DEFAULT_VALUE_NOT_INITIALIZED:
            return NS_ERROR_UNEXPECTED;
        case PREF_VALUECHANGED:
            return NS_PREF_VALUE_CHANGED;
    }

    NS_ASSERTION((res >= PREF_DEFAULT_VALUE_NOT_INITIALIZED) && (res <= PREF_PROFILE_UPGRADE), "you added a new error code to prefapi.h and didn't update _convertRes");

    return NS_OK;
}

//----------------------------------------------------------------------------------------
nsPref::nsPref()
//----------------------------------------------------------------------------------------
	:	mFileSpec(nsnull)
	,   mLIFileSpec(nsnull)
{
    PR_AtomicIncrement(&g_InstanceCount);
    NS_INIT_REFCNT();
}

//----------------------------------------------------------------------------------------
nsPref::~nsPref()
//----------------------------------------------------------------------------------------
{
    NS_IF_RELEASE(mFileSpec);
    NS_IF_RELEASE(mLIFileSpec);
    ShutDown();
    PR_AtomicDecrement(&g_InstanceCount);
    gInstance = NULL;
}

//----------------------------------------------------------------------------------------
nsresult nsPref::useDefaultPrefFile()
//----------------------------------------------------------------------------------------
{
    nsIFileSpec* prefsFile = NS_LocateFileOrDirectory(nsSpecialFileSpec::App_PreferencesFile50);
    nsresult rv = NS_OK;
    if (!prefsFile)
    {
	    // There is no locator component. Or perhaps there is a locator, but the
	    // locator couldn't find where to put it. So put it in the cwd (NB, viewer comes here.)
        // #include nsIComponentManager.h
        rv = nsComponentManager::CreateInstance(
        	(const char*)NS_FILESPEC_PROGID,
        	(nsISupports*)nsnull,
        	(const nsID&)nsIFileSpec::GetIID(),
        	(void**)&prefsFile);
        NS_ASSERTION(NS_SUCCEEDED(rv), "ERROR: Could not make a file spec.");
	    if (!prefsFile)
	    	return NS_ERROR_FAILURE;
	    prefsFile->SetUnixStyleFilePath("default_prefs.js"); // in default working directory.
    }
    if (Exists(prefsFile)) {
        rv = ReadUserPrefsFrom(prefsFile);
        NS_RELEASE(prefsFile);
        return rv;
    }

    // need to save the prefs now
    mFileSpec = prefsFile; // Already addreffed when retrieved.
    rv = SavePrefFile(); 

    return rv;
} // nsPref::useDefaultPrefFile

//----------------------------------------------------------------------------------------
nsresult nsPref::useUserPrefFile()
//----------------------------------------------------------------------------------------
{
    nsresult rv = NS_OK;
    nsIFileSpec* userPrefFile;
    static const char* userFiles[] = {"user.js"};
    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_SUCCEEDED(rv) && locator)
    {
        rv = locator->GetFileLocation(nsSpecialFileSpec::App_UserProfileDirectory50,
                        &userPrefFile);
        if (NS_SUCCEEDED(rv) && userPrefFile) 
        {
            if NS_SUCCEEDED(userPrefFile->AppendRelativeUnixPath((char*)userFiles[0]))
            {
	            if (Exists(userPrefFile)) {
                  rv = ReadUserPrefsFrom(userPrefFile);
                  NS_RELEASE(userPrefFile);
                  return rv;
              }
              NS_RELEASE(userPrefFile);
              return rv;
            }
            NS_RELEASE(userPrefFile);
            return rv;
        }
    }
    return rv;
} // nsPref::useUserPrefFile

//----------------------------------------------------------------------------------------
nsPref* nsPref::GetInstance()
//----------------------------------------------------------------------------------------
{
    if (!gInstance)
    {
        gInstance = new nsPref();
        gInstance->StartUp();
    }
    return gInstance;
} // nsPref::GetInstance

NS_IMPL_ISUPPORTS1(nsPref, nsIPref);

//========================================================================================
// nsIPref Implementation
//========================================================================================

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::StartUp()
// It must be safe to call this multiple times.
//----------------------------------------------------------------------------------------
{
    return PREF_Init(nsnull) ? NS_OK : NS_ERROR_FAILURE;
} // nsPref::StartUp

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ReadUserPrefs()
//----------------------------------------------------------------------------------------
{
    nsresult rv = StartUp(); // just to be sure
	  if (NS_SUCCEEDED(rv))
		  rv = useDefaultPrefFile();  // really should return a value...
    if (NS_SUCCEEDED(rv))
		  useUserPrefFile(); 
    return rv;
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ReadUserPrefsFrom(nsIFileSpec* inFile)
//----------------------------------------------------------------------------------------
{
    if (mFileSpec == inFile)
        return NS_OK;

    NS_IF_RELEASE(mFileSpec);
    mFileSpec = inFile;
    NS_ADDREF(mFileSpec);
    
    gErrorOpeningUserPrefs = PR_FALSE;

    if (NS_FAILED(StartUp()))
    	return NS_ERROR_FAILURE;

	nsresult rv = NS_OK;
	JS_BeginRequest(gMochaContext);
    PRBool exists;
    if (!(NS_SUCCEEDED(mFileSpec->Exists(&exists)) && exists)
      || pref_OpenFileSpec(mFileSpec, PR_TRUE, PR_FALSE, PR_FALSE, PR_TRUE)
        								!= PREF_NOERROR)
      rv = NS_ERROR_FAILURE;
    JS_EndRequest(gMochaContext);
    gErrorOpeningUserPrefs = NS_FAILED(rv);
    return rv;
} // nsPref::ReadUserPrefsFrom

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ResetPrefs()
//----------------------------------------------------------------------------------------
{
    nsresult rv;
    PREF_CleanupPrefs();
    rv = StartUp();
    return rv;
} // nsPref::ResetPrefs


//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ShutDown()
//----------------------------------------------------------------------------------------
{
    PREF_Cleanup();
    return NS_OK;
} // nsPref::ShutDown

/*
 * Config file input
 */

#ifdef PREF_SUPPORT_OLD_PATH_STRINGS
//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ReadUserJSFile(const char *filename)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_ReadUserJSFile(filename));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ReadLIJSFile(const char *filename)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_ReadLIJSFile(filename));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::EvaluateConfigScript(const char * js_buffer,
                         PRUint32 length,
                         const char* filename, 
                         PRBool bGlobalContext, 
                         PRBool bCallbacks)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_EvaluateConfigScript(js_buffer,
                                 length,
                                 filename,
                                 bGlobalContext,
                                 bCallbacks,
                                 PR_TRUE));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SavePrefFileAs(const char *filename)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_SavePrefFileAs(filename));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SaveLIPrefFile(const char *filename)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_SaveLIPrefFile(filename));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::CopyPathPref(const char *pref_name, char ** return_buffer)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_CopyPathPref(pref_name, return_buffer));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SetPathPref(const char *pref_name, const char *path, PRBool set_default)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_SetPathPref(pref_name, path, set_default));
}

#endif /* PREF_SUPPORT_OLD_PATH_STRINGS */

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ReadUserJSFile(nsIFileSpec* fileSpec)
//----------------------------------------------------------------------------------------
{
    return pref_OpenFileSpec(fileSpec, PR_FALSE, PR_FALSE, PR_TRUE, PR_FALSE);
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::ReadLIJSFile(nsIFileSpec* fileSpec)
//----------------------------------------------------------------------------------------
{
    NS_IF_RELEASE(mLIFileSpec);
    mLIFileSpec = fileSpec;
    NS_IF_ADDREF(mLIFileSpec);
    return pref_OpenFileSpec(fileSpec, PR_FALSE, PR_FALSE, PR_FALSE, PR_FALSE);
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::EvaluateConfigScript(const char * js_buffer,
                         PRUint32 length,
                         PRBool bGlobalContext, 
                         PRBool bCallbacks)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_EvaluateConfigScript(js_buffer,
                                 length,
                                 nsnull, // bad, but not used for parsing.
                                 bGlobalContext,
                                 bCallbacks,
                                 PR_TRUE));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::EvaluateConfigScriptFile(const char * js_buffer,
                         PRUint32 length,
                         nsIFileSpec* fileSpec, 
                         PRBool bGlobalContext, 
                         PRBool bCallbacks)
//----------------------------------------------------------------------------------------
{
    char* path; // GRR COM again.
    fileSpec->GetNativePath(&path);
    return _convertRes(PREF_EvaluateConfigScript(js_buffer,
                                 length,
                                 path, // bad, but not used for parsing.
                                 bGlobalContext,
                                 bCallbacks,
                                 PR_TRUE));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SavePrefFileAs(nsIFileSpec* fileSpec)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_SavePrefFileSpecWith(fileSpec, (PLHashEnumerator)pref_savePref));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SaveLIPrefFile(nsIFileSpec* fileSpec)
//----------------------------------------------------------------------------------------
{
    if (!gHashTable)
        return PREF_NOT_INITIALIZED;
    PREF_SetSpecialPrefsLocal();
    return _convertRes(PREF_SavePrefFileSpecWith(fileSpec, (PLHashEnumerator)pref_saveLIPref));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SavePrefFile()
//----------------------------------------------------------------------------------------
{
    if (!gHashTable || !mFileSpec)
        return PREF_NOT_INITIALIZED;
#ifdef PREF_SUPPORT_OLD_PATH_STRINGS
    if (gFileName)
        return _convertRes(PREF_SavePrefFile());
#endif
    return _convertRes(PREF_SavePrefFileSpecWith(mFileSpec, (PLHashEnumerator)pref_savePref));
}

/*
 * JS stuff
 */

NS_IMETHODIMP nsPref::GetConfigContext(JSContext **js_context)
{
    return _convertRes(PREF_GetConfigContext(js_context));
}

NS_IMETHODIMP nsPref::GetGlobalConfigObject(JSObject **js_object)
{
    return _convertRes(PREF_GetGlobalConfigObject(js_object));
}

NS_IMETHODIMP nsPref::GetPrefConfigObject(JSObject **js_object)
{
    return _convertRes(PREF_GetPrefConfigObject(js_object));
}

/*
 * Getters
 */
NS_IMETHODIMP nsPref::GetPrefType(const char *pref, PRInt32 * return_type)
{
  // XXX The current implementation returns the raw type - the
  // enums defining the types have been duplicated. If the old
  // types don't all make sense in the context of the new API,
  // we might need to do some translation.

  *return_type = PREF_GetPrefType(pref);
  return NS_OK;
}

NS_IMETHODIMP nsPref::GetIntPref(const char *pref, PRInt32 * return_int)
{
    return _convertRes(PREF_GetIntPref(pref, return_int));
}

NS_IMETHODIMP nsPref::GetBoolPref(const char *pref, PRBool * return_val)
{
    return _convertRes(PREF_GetBoolPref(pref, return_val));
}

NS_IMETHODIMP nsPref::GetBinaryPref(const char *pref, 
                        void * return_val, int * buf_length)
{
    return _convertRes(PREF_GetBinaryPref(pref, return_val, buf_length));
}

NS_IMETHODIMP nsPref::GetColorPref(const char *pref,
                     PRUint8 *red, PRUint8 *green, PRUint8 *blue)
{
    return _convertRes(PREF_GetColorPref(pref, red, green, blue));
}

NS_IMETHODIMP nsPref::GetColorPrefDWord(const char *pref, 
                    PRUint32 *colorref)
{
    return _convertRes(PREF_GetColorPrefDWord(pref, colorref));
}

NS_IMETHODIMP nsPref::GetRectPref(const char *pref, 
                    PRInt16 *left, PRInt16 *top, 
                    PRInt16 *right, PRInt16 *bottom)
{
    return _convertRes(PREF_GetRectPref(pref, left, top, right, bottom));
}

/*
 * Setters
 */

NS_IMETHODIMP nsPref::SetCharPref(const char *pref,const char* value)
{
    return _convertRes(PREF_SetCharPref(pref, value));
}

NS_IMETHODIMP nsPref::SetUnicharPref(const char *pref, const PRUnichar *value)
{
    nsresult rv;
    nsAutoString str(value);

    char *utf8String = str.ToNewUTF8String();

    if (!utf8String) return NS_ERROR_OUT_OF_MEMORY;

    rv = SetCharPref(pref, utf8String);
    nsCRT::free(utf8String);

    return rv;
}

NS_IMETHODIMP nsPref::SetIntPref(const char *pref,PRInt32 value)
{
    return _convertRes(PREF_SetIntPref(pref, value));
}

NS_IMETHODIMP nsPref::SetBoolPref(const char *pref,PRBool value)
{
    return _convertRes(PREF_SetBoolPref(pref, value));
}

NS_IMETHODIMP nsPref::SetBinaryPref(const char *pref,void * value, PRUint32 size)
{
    return _convertRes(PREF_SetBinaryPref(pref, value, size));
}

NS_IMETHODIMP nsPref::SetColorPref(const char *pref, 
                     PRUint8 red, PRUint8 green, PRUint8 blue)
{
    return _convertRes(PREF_SetColorPref(pref, red, green, blue));
}

NS_IMETHODIMP nsPref::SetColorPrefDWord(const char *pref, 
                    PRUint32 value)
{
    return _convertRes(PREF_SetColorPrefDWord(pref, value));
}

NS_IMETHODIMP nsPref::SetRectPref(const char *pref, 
                    PRInt16 left, PRInt16 top, 
                    PRInt16 right, PRInt16 bottom)
{
    return _convertRes(PREF_SetRectPref(pref, left, top, right, bottom));
}

/*
 * Get Defaults
 */

NS_IMETHODIMP nsPref::GetDefaultIntPref(const char *pref,
                    PRInt32 * return_int)
{
    return _convertRes(PREF_GetDefaultIntPref(pref, return_int));
}

NS_IMETHODIMP nsPref::GetDefaultBoolPref(const char *pref,
                     PRBool * return_val)
{
    return _convertRes(PREF_GetDefaultBoolPref(pref, return_val));
}

NS_IMETHODIMP nsPref::GetDefaultBinaryPref(const char *pref, 
                         void * return_val,
                         int * buf_length)
{
    return _convertRes(PREF_GetDefaultBinaryPref(pref, return_val, buf_length));
}

NS_IMETHODIMP nsPref::GetDefaultColorPref(const char *pref, 
                        PRUint8 *red, PRUint8 *green, 
                        PRUint8 *blue)
{
    return _convertRes(PREF_GetDefaultColorPref(pref, red, green, blue));
}

NS_IMETHODIMP nsPref::GetDefaultColorPrefDWord(const char *pref, 
                                 PRUint32 *colorref)
{
    return _convertRes(PREF_GetDefaultColorPrefDWord(pref, colorref));
}

NS_IMETHODIMP nsPref::GetDefaultRectPref(const char *pref, 
                     PRInt16 *left, PRInt16 *top, 
                     PRInt16 *right, PRInt16 *bottom)
{
    return _convertRes(PREF_GetDefaultRectPref(pref, 
                             left, top, right, bottom));
}

/*
 * Set defaults
 */

NS_IMETHODIMP nsPref::SetDefaultCharPref(const char *pref,const char* value)
{
    return _convertRes(PREF_SetDefaultCharPref(pref, value));
}

NS_IMETHODIMP nsPref::SetDefaultIntPref(const char *pref,PRInt32 value)
{
    return _convertRes(PREF_SetDefaultIntPref(pref, value));
}

NS_IMETHODIMP nsPref::SetDefaultBoolPref(const char *pref, PRBool value)
{
    return _convertRes(PREF_SetDefaultBoolPref(pref, value));
}

NS_IMETHODIMP nsPref::SetDefaultBinaryPref(const char *pref,
                         void * value, PRUint32 size)
{
    return _convertRes(PREF_SetDefaultBinaryPref(pref, value, size));
}

NS_IMETHODIMP nsPref::SetDefaultColorPref(const char *pref, 
                        PRUint8 red, PRUint8 green, PRUint8 blue)
{
    return _convertRes(PREF_SetDefaultColorPref(pref, red, green, blue));
}

NS_IMETHODIMP nsPref::SetDefaultRectPref(const char *pref, 
                     PRInt16 left, PRInt16 top,
                     PRInt16 right, PRInt16 bottom)
{
    return _convertRes(PREF_SetDefaultRectPref(pref, left, top, right, bottom));
}

NS_IMETHODIMP nsPref::ClearUserPref(const char *pref_name)
{
    return _convertRes(PREF_ClearUserPref(pref_name));
}

/*
 * Copy prefs
 */

NS_IMETHODIMP nsPref::CopyCharPref(const char *pref, char ** return_buf)
{
    return _convertRes(PREF_CopyCharPref(pref, return_buf));
}

// unicode "%s" format string
static const PRUnichar unicodeFormatter[] = {
    (PRUnichar)'%',
    (PRUnichar)'s',
    (PRUnichar)0,
};

NS_IMETHODIMP nsPref::CopyUnicharPref(const char *pref, PRUnichar ** return_buf)
{
    nsresult rv;
    
    // get the UTF8 string for conversion
    char *utf8String;
    rv = CopyCharPref(pref, &utf8String);
    if (NS_FAILED(rv)) return rv;
    
    // convert to PRUnichar using nsTextFormatter
    // this is so ugly, it allocates memory at least 4 times :(
    PRUnichar *unicodeString =
        nsTextFormater::smprintf(unicodeFormatter, utf8String);
    PL_strfree(utf8String);
    if (!unicodeString) return NS_ERROR_OUT_OF_MEMORY;

    // use the right allocator
    *return_buf = nsCRT::strdup(unicodeString);
    nsTextFormater::smprintf_free(unicodeString);
    if (!*return_buf) return NS_ERROR_OUT_OF_MEMORY;

    return NS_OK;
}

NS_IMETHODIMP nsPref::CopyBinaryPref(const char *pref,
                         int *size, void ** return_value)
{
    return _convertRes(PREF_CopyBinaryPref(pref, return_value, size));
}

NS_IMETHODIMP nsPref::CopyDefaultCharPref( const char *pref,
                         char ** return_buffer )
{
    return _convertRes(PREF_CopyDefaultCharPref(pref, return_buffer));
}

NS_IMETHODIMP nsPref::CopyDefaultBinaryPref(const char *pref, 
                            int * size, void ** return_val)
{
    return _convertRes(PREF_CopyDefaultBinaryPref(pref, return_val, size));
}

#if 0
/*
 * Path prefs
 */

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::CopyPathPref(const char *pref, char ** return_buf)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_CopyPathPref(pref, return_buf));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SetPathPref(const char *pref, 
                    const char *path, PRBool set_default)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_SetPathPref(pref, path, set_default));
}
#endif

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::GetFilePref(const char *pref_name, nsIFileSpec** value)
//----------------------------------------------------------------------------------------
{
    if (!value)
        return NS_ERROR_NULL_POINTER;        

        nsresult rv = nsComponentManager::CreateInstance(
        	(const char*)NS_FILESPEC_PROGID,
        	(nsISupports*)nsnull,
        	(const nsID&)nsIFileSpec::GetIID(),
        	(void**)value);
        NS_ASSERTION(NS_SUCCEEDED(rv), "ERROR: Could not make a file spec.");
    if (!*value)
      return NS_ERROR_FAILURE;

    char *encodedString = nsnull;
    PrefResult result = PREF_CopyCharPref(pref_name, &encodedString);
    if (result != PREF_NOERROR)
        return _convertRes(result);

	PRBool valid;
    (*value)->SetPersistentDescriptorString(encodedString);
    (*value)->IsValid(&valid);
    if (! valid)
    	/* if the ecodedString wasn't a valid persitent descriptor, it might be a valid native path*/
    	(*value)->SetNativePath(encodedString);
    
    PR_Free(encodedString); // Allocated by PREF_CopyCharPref
    return NS_OK;
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::SetFilePref(const char *pref_name, 
                    nsIFileSpec* value, PRBool set_default)
//----------------------------------------------------------------------------------------
{
    if (!value)
        return NS_ERROR_NULL_POINTER;        
    nsresult rv = NS_OK;
    if (!Exists(value))
    {
        // nsPersistentFileDescriptor requires an existing
        // object. Make it first. COM makes this difficult, of course...
	    nsIFileSpec* tmp = nsnull;
        rv = nsComponentManager::CreateInstance(
        	(const char*)NS_FILESPEC_PROGID,
        	(nsISupports*)nsnull,
        	(const nsID&)nsIFileSpec::GetIID(),
        	(void**)&tmp);
        NS_ASSERTION(NS_SUCCEEDED(rv), "ERROR: Could not make a file spec.");
	    if (!tmp)
	    	return NS_ERROR_FAILURE;
		tmp->FromFileSpec(value);
        tmp->CreateDir();
        NS_RELEASE(tmp);
    }
    char* encodedString = nsnull;
    value->GetPersistentDescriptorString(&encodedString);
    if (encodedString && *encodedString)
    {
        if (set_default)
            rv = PREF_SetDefaultCharPref(pref_name, encodedString);
        else
            rv = PREF_SetCharPref(pref_name, encodedString);
    }
    PR_FREEIF(encodedString); // Allocated by nsOutputStringStream
    return _convertRes(rv);
}

/*
 * Pref info
 */

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::PrefIsLocked(const char *pref, PRBool *res)
//----------------------------------------------------------------------------------------
{
    if (res == NULL)
        return NS_ERROR_INVALID_POINTER;

    *res = PREF_PrefIsLocked(pref);
    return NS_OK;
}

/*
 * Callbacks
 */

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::RegisterCallback( const char* domain,
                    PrefChangedFunc callback, 
                    void* instance_data )
//----------------------------------------------------------------------------------------
{
    PREF_RegisterCallback(domain, callback, instance_data);
    return NS_OK;
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::UnregisterCallback( const char* domain,
                        PrefChangedFunc callback, 
                        void* instance_data )
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_UnregisterCallback(domain, callback, instance_data));
}

/*
 * Tree editing
 */

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::CopyPrefsTree(const char *srcRoot, const char *destRoot)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_CopyPrefsTree(srcRoot, destRoot));
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::DeleteBranch(const char *branchName)
//----------------------------------------------------------------------------------------
{
    return _convertRes(PREF_DeleteBranch(branchName));
}

#include "prprf.h"
/*
 * Creates an iterator over the children of a node.
 */
typedef struct 
{
	char*		 childList;
	char*		 parent;
	unsigned int bufsize;
} PrefChildIter; 

/* if entry begins with the given string, i.e. if string is
  "a"
  and entry is
  "a.b.c" or "a.b"
  then add "a.b" to the list. */
static PR_CALLBACK PRIntn
pref_addChild(PLHashEntry *he, int i, void *arg)
{
	PrefChildIter* pcs = (PrefChildIter*) arg;
	if ( PL_strncmp((char*)he->key, pcs->parent, strlen(pcs->parent)) == 0 )
	{
		char buf[512];
		char* nextdelim;
		PRUint32 parentlen = strlen(pcs->parent);
		char* substring;

		PL_strncpy(buf, (char*)he->key, PR_MIN(512, strlen((char*)he->key) + 1));
		nextdelim = buf + parentlen;
		if (parentlen < PL_strlen(buf))
		{
			/* Find the next delimiter if any and truncate the string there */
			nextdelim = strstr(nextdelim, ".");
			if (nextdelim)
			{
				*nextdelim = ';';
				*(nextdelim + 1) = '\0';
			}
		}

		substring = strstr(pcs->childList, buf);
		if (!substring)
		{
			unsigned int newsize = strlen(pcs->childList) + strlen(buf) + 2;
			if (newsize > pcs->bufsize)
			{
				pcs->bufsize *= 3;
				pcs->childList = (char*) realloc(pcs->childList, sizeof(char) * pcs->bufsize);
				if (!pcs->childList)
					return HT_ENUMERATE_STOP;
			}
			PL_strcat(pcs->childList, buf);
		}
	}
	return 0;
}


NS_IMETHODIMP nsPref::CreateChildList(const char* parent_node, char **child_list)
{
	PrefChildIter pcs;

	pcs.bufsize = 2048;
	pcs.childList = (char*) malloc(sizeof(char) * pcs.bufsize);
	if (*parent_node > 0)
		pcs.parent = PR_smprintf("%s.", parent_node);
	else
		pcs.parent = PL_strdup("");
	if (!pcs.parent || !pcs.childList)
		return PREF_OUT_OF_MEMORY;
	pcs.childList[0] = '\0';

	PL_HashTableEnumerateEntries(gHashTable, pref_addChild, &pcs);

	*child_list = pcs.childList;
	PR_Free(pcs.parent);
	
	return (pcs.childList == NULL) ? PREF_OUT_OF_MEMORY : PREF_OK;
}

NS_IMETHODIMP nsPref::NextChild(const char *child_list, PRInt16 *indx, char **listchild)
{
	char* temp = (char*)&child_list[*indx];
	char* child = strtok(temp, ";");
	if (child)
	{
		*indx += strlen(child) + 1;
		*listchild = child;
		return NS_OK;
	}
	else
		return NS_ERROR_NULL_POINTER;
}

struct EnumerateData {
    const char *parent;
    PrefEnumerationFunc callback;
    void *arg;
};

PR_STATIC_CALLBACK(PRIntn)
pref_enumChild(PLHashEntry *he, int i, void *arg)
{
    EnumerateData *d = (EnumerateData *) arg;
    if (PL_strncmp((char*)he->key, d->parent, strlen(d->parent)) == 0) {
        (*d->callback)((char*)he->key, d->arg);
    }
    return HT_ENUMERATE_NEXT;
}

NS_IMETHODIMP
nsPref::EnumerateChildren(const char *parent, PrefEnumerationFunc callback, void *arg) 
{   
    EnumerateData ed;
    ed.parent = parent;
    ed.callback = callback;
    ed.arg = arg;
    PL_HashTableEnumerateEntries(gHashTable, pref_enumChild, &ed);
    return NS_OK;
}

//========================================================================================
// C++ implementations of old C routines
//========================================================================================

//----------------------------------------------------------------------------------------
PrefResult pref_OpenFileSpec(
    nsIFileSpec* fileSpec,
    PRBool is_error_fatal,
    PRBool verifyHash,
    PRBool bGlobalContext,
    PRBool skipFirstLine)
//----------------------------------------------------------------------------------------
{
    PrefResult result = PREF_NOERROR;

    if (!Exists(fileSpec))
        return PREF_ERROR;

    char* readBuf;
    if (NS_FAILED(fileSpec->GetFileContents(&readBuf)))
    	return PREF_ERROR;
    long fileLength = strlen(readBuf);
    if (verifyHash && !pref_VerifyLockFile(readBuf, fileLength))
        result = PREF_BAD_LOCKFILE;
    if (!PREF_EvaluateConfigScript(readBuf, fileLength,
            nsnull, bGlobalContext, PR_FALSE, skipFirstLine))
        result = PREF_ERROR;
    PR_Free(readBuf);

    // If the user prefs file exists but generates an error,
    //   don't clobber the file when we try to save it
    if ((!readBuf || result != PREF_NOERROR) && is_error_fatal)
        gErrorOpeningUserPrefs = PR_TRUE;
#ifdef XP_PC
    if (gErrorOpeningUserPrefs && is_error_fatal)
        MessageBox(nsnull,"Error in preference file (prefs.js).  Default preferences will be used.","Netscape - Warning", MB_OK);
#endif

    JS_GC(gMochaContext);
    return result;
} // pref_OpenFile

//----------------------------------------------------------------------------------------
PR_IMPLEMENT(PrefResult) PREF_SavePrefFileSpecWith(
    nsIFileSpec* fileSpec,
    PLHashEnumerator heSaveProc)
//----------------------------------------------------------------------------------------
{
    if (!gHashTable)
        return PREF_NOT_INITIALIZED;

    /* ?! Don't save (blank) user prefs if there was an error reading them */
    if (gErrorOpeningUserPrefs)
        return PREF_NOERROR;

    char** valueArray = (char**) PR_Calloc(sizeof(char*), gHashTable->nentries);
    if (!valueArray)
        return PREF_OUT_OF_MEMORY;

    nsOutputFileStream stream(fileSpec);
    if (!stream.is_open())
        return PREF_ERROR;

    stream << PREFS_HEADER_LINE_1 << nsEndl << PREFS_HEADER_LINE_2 << nsEndl << nsEndl;
    
    /* LI_STUFF here we pass in the heSaveProc proc used so that li can do its own thing */
    PR_HashTableEnumerateEntries(gHashTable, heSaveProc, valueArray);
    
    /* Sort the preferences to make a readable file on disk */
    NS_QuickSort(valueArray, gHashTable->nentries, sizeof(char*), pref_CompareStrings, NULL);
    char** walker = valueArray;
    for (PRUint32 valueIdx = 0; valueIdx < gHashTable->nentries; valueIdx++,walker++)
    {
        if (*walker)
        {
            stream << *walker << nsEndl;
            PR_Free(*walker);
        }
    }
    PR_Free(valueArray);
	fileSpec->CloseStream();
    return PREF_NOERROR;
}

//----------------------------------------------------------------------------------------
extern "C" JSBool pref_InitInitialObjects()
// Initialize default preference JavaScript buffers from
// appropriate TEXT resources
//----------------------------------------------------------------------------------------
{
    JSBool funcResult;
    nsresult rv;
    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv))
    	return JS_TRUE;
    nsCOMPtr <nsIFileSpec> defaultPrefDir;
    rv = locator->GetFileLocation(nsSpecialFileSpec::App_PrefDefaultsFolder50, getter_AddRefs(defaultPrefDir));
    if (NS_FAILED(rv))
    	return JS_TRUE;
	static const char* specialFiles[] = {
		"initpref.js"
#ifdef XP_MAC
	,	"macprefs.js"
#elif defined(XP_PC)
	,	"winpref.js"
#elif defined(XP_UNIX)
	,	"unix.js"
#endif
	};
    
	int k;
	JSBool worked = JS_FALSE;
	// Parse all the random files that happen to be in the components directory.
    nsIDirectoryIterator* i = nsnull;
    rv = nsComponentManager::CreateInstance(
        	(const char*)NS_DIRECTORYITERATOR_PROGID,
        	(nsISupports*)nsnull,
        	(const nsID&)nsIDirectoryIterator::GetIID(),
        	(void**)&i);
    NS_ASSERTION(NS_SUCCEEDED(rv), "ERROR: Could not make a directory iterator.");
    if (!i || NS_FAILED(i->Init(defaultPrefDir, PR_TRUE)))
    	return JS_FALSE;

	// Get any old child of the components directory. Warning: aliases get resolved, so
	// SetLeafName will not work here.
    nsIFileSpec* specialChild;
    rv = locator->GetFileLocation(nsSpecialFileSpec::App_PrefDefaultsFolder50, &specialChild);
    if (NS_FAILED(rv))
    	return JS_TRUE;
	if NS_FAILED(specialChild->AppendRelativeUnixPath((char*)specialFiles[0]))
	{
		funcResult = JS_FALSE;
		goto done;
	}
	
    worked = (JSBool)(pref_OpenFileSpec(
    	specialChild,
    	PR_FALSE,
    	PR_FALSE,
    	PR_FALSE,
    	PR_FALSE) == PREF_NOERROR);
	NS_ASSERTION(worked, "initpref.js not parsed successfully");
	// Keep this child

	for (; Exists(i); i->Next())
	{
		nsIFileSpec* child;
		PRBool shouldParse = PR_TRUE;
		if NS_FAILED(i->GetCurrentSpec(&child))
			continue;
		char* leafName;
		rv = child->GetLeafName(&leafName);
		if NS_FAILED(rv)
			goto next_child;
		// Skip non-js files
		if (strstr(leafName, ".js") + strlen(".js") != leafName + strlen(leafName))
			shouldParse = PR_FALSE;
		// Skip files in the special list.
		if (shouldParse)
		{
			for (int j = 0; j < (int) (sizeof(specialFiles) / sizeof(char*)); j++)
				if (strcmp(leafName, specialFiles[j]) == 0)
					shouldParse = PR_FALSE;
		}
		nsCRT::free((char*)leafName);
		if (shouldParse)
		{
		    worked = (JSBool)(pref_OpenFileSpec(
		    	child,
		    	PR_FALSE,
		    	PR_FALSE,
		    	PR_FALSE,
		    	PR_FALSE) == PREF_NOERROR);
			NS_ASSERTION(worked, "Config file not parsed successfully");
		}
	next_child:
		NS_IF_RELEASE(child);
	}
	// Finally, parse any other special files (platform-specific ones).
	for (k = 1; k < (int) (sizeof(specialFiles) / sizeof(char*)); k++)
	{
        nsIFileSpec* specialChild2;
        if (NS_FAILED(locator->GetFileLocation(nsSpecialFileSpec::App_PrefDefaultsFolder50, &specialChild2)))
	    	continue;
		if (NS_FAILED(specialChild2->AppendRelativeUnixPath((char*)specialFiles[k])))
	    	continue;
	    worked = (JSBool)(pref_OpenFileSpec(
    		specialChild2,
	    	PR_FALSE,
	    	PR_FALSE,
	    	PR_FALSE,
	    	PR_FALSE) == PREF_NOERROR);
		NS_ASSERTION(worked, "<platform>.js was not parsed successfully");
		NS_RELEASE(specialChild2);
	}
done:
    NS_RELEASE(i);
	NS_RELEASE(specialChild);
    return JS_TRUE;
}

//----------------------------------------------------------------------------------------
// Module implementation for the pref library
class nsPrefModule : public nsIModule
{
public:
    nsPrefModule();
    virtual ~nsPrefModule();

    NS_DECL_ISUPPORTS

    NS_DECL_NSIMODULE

protected:
    nsresult Initialize();

    void Shutdown();

    PRBool mInitialized;
    nsCOMPtr<nsIGenericFactory> mFactory;
};


//----------------------------------------------------------------------------------------
// Functions used to create new instances of a given object by the
// generic factory.

static NS_IMETHODIMP
CreateNewPref(nsISupports *aDelegate, REFNSIID aIID, void **aResult)
{
    if (aDelegate != NULL)
        return NS_ERROR_NO_AGGREGATION;

    nsPref *t = nsPref::GetInstance();
    
    if (t == NULL)
        return NS_ERROR_OUT_OF_MEMORY;
    
    nsresult res = t->QueryInterface(aIID, aResult);
    
    if (NS_FAILED(res))
        *aResult = NULL;

    return res;
}

// The list of components we register
static nsModuleComponentInfo components[] = {
    { NS_PREF_CLASSNAME, NS_PREF_CID, NS_PREF_PROGID, CreateNewPref, },
};


extern "C" JSRuntime* PREF_GetJSRuntime()
{
    nsresult rv;

    NS_WITH_SERVICE(nsIJSRuntimeService, rtsvc, "nsJSRuntimeService", &rv);
    if (NS_FAILED(rv)) return nsnull;

    JSRuntime* rt;
    rv = rtsvc->GetRuntime(&rt);
    if (NS_FAILED(rv)) return nsnull;

    return rt;
}

////////////////////////////////////////////////////////////////////////
// Implement the NSGetModule() exported function for your module
// and the entire implementation of the module object.
//
NS_IMPL_NSGETMODULE("nsPrefModule", components)
