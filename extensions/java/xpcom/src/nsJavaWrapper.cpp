/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Java XPCOM Bindings.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Javier Pedemonte (jhpedemonte@gmail.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsJavaWrapper.h"
#include "nsJavaXPTCStub.h"
#include "nsJavaXPCOMBindingUtils.h"
#include "jni.h"
#include "xptcall.h"
#include "nsIInterfaceInfoManager.h"
#include "nsString.h"
#include "nsCRT.h"
#include "prmem.h"

#define JAVAPROXY_NATIVE(func) Java_org_mozilla_xpcom_XPCOMJavaProxy_##func

static nsID nullID = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};


nsresult
CreateJavaArray(JNIEnv* env, PRUint8 aType, PRUint32 aSize, const nsID& aIID,
                jobject* aResult)
{
  jobject array = nsnull;
  switch (aType)
  {
    case nsXPTType::T_I8:
    case nsXPTType::T_U8:
      array = env->NewByteArray(aSize);
      break;

    case nsXPTType::T_I16:
    case nsXPTType::T_U16:
      array = env->NewShortArray(aSize);
      break;

    case nsXPTType::T_I32:
    case nsXPTType::T_U32:
    case nsXPTType::T_VOID:
      array = env->NewIntArray(aSize);
      break;

    case nsXPTType::T_I64:
    case nsXPTType::T_U64:
      array = env->NewLongArray(aSize);
      break;

    case nsXPTType::T_FLOAT:
      array = env->NewFloatArray(aSize);
      break;

    case nsXPTType::T_DOUBLE:
      array = env->NewDoubleArray(aSize);
      break;

    case nsXPTType::T_BOOL:
      array = env->NewBooleanArray(aSize);
      break;

    case nsXPTType::T_CHAR:
    case nsXPTType::T_WCHAR:
      array = env->NewCharArray(aSize);
      break;

    case nsXPTType::T_CHAR_STR:
    case nsXPTType::T_WCHAR_STR:
    case nsXPTType::T_IID:
    case nsXPTType::T_ASTRING:
    case nsXPTType::T_DOMSTRING:
    case nsXPTType::T_UTF8STRING:
    case nsXPTType::T_CSTRING:
      array = env->NewObjectArray(aSize, stringClass, nsnull);
      break;

    case nsXPTType::T_INTERFACE:
    case nsXPTType::T_INTERFACE_IS:
    {
      nsCOMPtr<nsIInterfaceInfoManager> iim = XPTI_GetInterfaceInfoManager();
      NS_ASSERTION(iim != nsnull, "Failed to get InterfaceInfoManager");
      if (!iim)
        return NS_ERROR_FAILURE;

      // Get interface info for given IID
      nsCOMPtr<nsIInterfaceInfo> info;
      nsresult rv = iim->GetInfoForIID(&aIID, getter_AddRefs(info));
      if (NS_FAILED(rv))
        return rv;

      // Get interface name
      const char* iface_name;
      rv = info->GetNameShared(&iface_name);
      if (NS_FAILED(rv))
        return rv;

      // Create proper Java interface name
      nsCAutoString class_name("org/mozilla/xpcom/");
      class_name.AppendASCII(iface_name);
      jclass ifaceClass = env->FindClass(class_name.get());
      if (!ifaceClass)
        return NS_ERROR_FAILURE;

      array = env->NewObjectArray(aSize, ifaceClass, nsnull);
      break;
    }

    default:
      NS_WARNING("unknown type");
      return NS_ERROR_FAILURE;
  }

  if (!array)
    return NS_ERROR_OUT_OF_MEMORY;

  *aResult = array;
  return NS_OK;
}

nsresult
GetNativeArrayElement(PRUint8 aType, void* aArray, PRUint32 aIndex,
                      nsXPTCVariant* aResult)
{
  switch (aType)
  {
    case nsXPTType::T_I8:
    case nsXPTType::T_U8:
      aResult->val.u8 = NS_STATIC_CAST(PRUint8*, aArray)[aIndex];
      break;

    case nsXPTType::T_I16:
    case nsXPTType::T_U16:
      aResult->val.u16 = NS_STATIC_CAST(PRUint16*, aArray)[aIndex];
      break;

    case nsXPTType::T_I32:
    case nsXPTType::T_U32:
      aResult->val.u32 = NS_STATIC_CAST(PRUint32*, aArray)[aIndex];
      break;

    case nsXPTType::T_I64:
    case nsXPTType::T_U64:
      aResult->val.u64 = NS_STATIC_CAST(PRUint64*, aArray)[aIndex];
      break;

    case nsXPTType::T_FLOAT:
      aResult->val.f = NS_STATIC_CAST(float*, aArray)[aIndex];
      break;

    case nsXPTType::T_DOUBLE:
      aResult->val.d = NS_STATIC_CAST(double*, aArray)[aIndex];
      break;

    case nsXPTType::T_BOOL:
      aResult->val.b = NS_STATIC_CAST(PRBool*, aArray)[aIndex];
      break;

    case nsXPTType::T_CHAR:
      aResult->val.c = NS_STATIC_CAST(char*, aArray)[aIndex];
      break;

    case nsXPTType::T_WCHAR:
      aResult->val.wc = NS_STATIC_CAST(PRUnichar*, aArray)[aIndex];
      break;

    case nsXPTType::T_CHAR_STR:
      aResult->val.p = NS_STATIC_CAST(char**, aArray)[aIndex];
      break;

    case nsXPTType::T_WCHAR_STR:
      aResult->val.p = NS_STATIC_CAST(PRUnichar**, aArray)[aIndex];
      break;

    case nsXPTType::T_IID:
      aResult->val.p = NS_STATIC_CAST(nsID**, aArray)[aIndex];
      break;

    case nsXPTType::T_INTERFACE:
    case nsXPTType::T_INTERFACE_IS:
      aResult->val.p = NS_STATIC_CAST(nsISupports**, aArray)[aIndex];
      break;

    case nsXPTType::T_ASTRING:
    case nsXPTType::T_DOMSTRING:
      aResult->val.p = NS_STATIC_CAST(nsString**, aArray)[aIndex];
      break;

    case nsXPTType::T_UTF8STRING:
    case nsXPTType::T_CSTRING:
      aResult->val.p = NS_STATIC_CAST(nsCString**, aArray)[aIndex];
      break;

    case nsXPTType::T_VOID:
      aResult->val.u32 = NS_STATIC_CAST(PRUint32*, aArray)[aIndex];
      break;

    default:
      NS_WARNING("unknown type");
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}

nsresult
CreateNativeArray(PRUint8 aType, PRUint32 aSize, void** aResult)
{
  void* array = nsnull;
  switch (aType)
  {
    case nsXPTType::T_I8:
    case nsXPTType::T_U8:
      array = PR_Malloc(aSize * sizeof(PRUint8));
      break;

    case nsXPTType::T_I16:
    case nsXPTType::T_U16:
      array = PR_Malloc(aSize * sizeof(PRUint16));
      break;

    case nsXPTType::T_I32:
    case nsXPTType::T_U32:
    case nsXPTType::T_VOID:
      array = PR_Malloc(aSize * sizeof(PRUint32));
      break;

    case nsXPTType::T_I64:
    case nsXPTType::T_U64:
      array = PR_Malloc(aSize * sizeof(PRUint64));
      break;

    case nsXPTType::T_FLOAT:
      array = PR_Malloc(aSize * sizeof(float));
      break;

    case nsXPTType::T_DOUBLE:
      array = PR_Malloc(aSize * sizeof(double));
      break;

    case nsXPTType::T_BOOL:
      array = PR_Malloc(aSize * sizeof(PRBool));
      break;

    case nsXPTType::T_CHAR:
      array = PR_Malloc(aSize * sizeof(char));
      break;

    case nsXPTType::T_WCHAR:
      array = PR_Malloc(aSize * sizeof(PRUnichar));
      break;

    case nsXPTType::T_CHAR_STR:
    case nsXPTType::T_WCHAR_STR:
    case nsXPTType::T_IID:
    case nsXPTType::T_ASTRING:
    case nsXPTType::T_DOMSTRING:
    case nsXPTType::T_UTF8STRING:
    case nsXPTType::T_CSTRING:
    case nsXPTType::T_INTERFACE:
    case nsXPTType::T_INTERFACE_IS:
      array = PR_Malloc(aSize * sizeof(void*));
      break;

    default:
      NS_WARNING("unknown type");
      return NS_ERROR_FAILURE;
  }

  if (!array)
    return NS_ERROR_OUT_OF_MEMORY;

  *aResult = array;
  return NS_OK;
}

/**
 * Handle 'in' and 'inout' params.
 */
nsresult
SetupParams(JNIEnv *env, const jobject aParam, PRUint8 aType, PRBool aIsOut,
            const nsID& aIID, PRUint8 aArrayType, PRUint32 aArraySize,
            PRBool aIsArrayElement, PRUint32 aIndex, nsXPTCVariant &aVariant)
{
  nsresult rv = NS_OK;

  // defaults
  aVariant.type = aType;

  switch (aType)
  {
    case nsXPTType::T_I8:
    case nsXPTType::T_U8:
    {
      LOG(("byte\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.u8 = env->CallByteMethod(aParam, byteValueMID);
      } else { // 'inout' & 'array'
        jbyte value;
        if (aParam) {
          env->GetByteArrayRegion((jbyteArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.u8 = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(PRUint8*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_I16:
    case nsXPTType::T_U16:
    {
      LOG(("short\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.u16 = env->CallShortMethod(aParam, shortValueMID);
      } else { // 'inout' & 'array'
        jshort value;
        if (aParam) {
          env->GetShortArrayRegion((jshortArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.u16 = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(PRUint16*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_I32:
    case nsXPTType::T_U32:
    {
      LOG(("int\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.u32 = env->CallIntMethod(aParam, intValueMID);
      } else { // 'inout' & 'array'
        jint value;
        if (aParam) {
          env->GetIntArrayRegion((jintArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.u32 = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(PRUint32*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_I64:
    case nsXPTType::T_U64:
    {
      LOG(("long\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.u64 = env->CallLongMethod(aParam, longValueMID);
      } else { // 'inout' & 'array'
        jlong value;
        if (aParam) {
          env->GetLongArrayRegion((jlongArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.u64 = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(PRUint64*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_FLOAT:
    {
      LOG(("float\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.f = env->CallFloatMethod(aParam, floatValueMID);
      } else { // 'inout' & 'array'
        jfloat value;
        if (aParam) {
          env->GetFloatArrayRegion((jfloatArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.f = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(float*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_DOUBLE:
    {
      LOG(("double\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.d = env->CallDoubleMethod(aParam, doubleValueMID);
      } else { // 'inout' & 'array'
        jdouble value;
        if (aParam) {
          env->GetDoubleArrayRegion((jdoubleArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.d = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(double*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_BOOL:
    {
      LOG(("boolean\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.b = env->CallBooleanMethod(aParam, booleanValueMID);
      } else { // 'inout' & 'array'
        jboolean value;
        if (aParam) {
          env->GetBooleanArrayRegion((jbooleanArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.b = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(PRBool*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_CHAR:
    {
      LOG(("char\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.c = env->CallCharMethod(aParam, charValueMID);
      } else { // 'inout' & 'array'
        jchar value;
        if (aParam) {
          env->GetCharArrayRegion((jcharArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.c = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(char*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_WCHAR:
    {
      LOG(("char\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.wc = env->CallCharMethod(aParam, charValueMID);
      } else { // 'inout' & 'array'
        jchar value;
        if (aParam) {
          env->GetCharArrayRegion((jcharArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.wc = value;
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(PRUnichar*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_CHAR_STR:
    case nsXPTType::T_WCHAR_STR:
    {
      LOG(("String\n"));
      jstring data = nsnull;
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        data = (jstring) aParam;
      } else if (aParam) {  // 'inout' & 'array'
        data = (jstring) env->GetObjectArrayElement((jobjectArray) aParam,
                                                    aIndex);
      }

      void* buf = nsnull;
      if (data) {
        jsize uniLength = env->GetStringLength(data);
        if (uniLength > 0) {
          if (aType == nsXPTType::T_CHAR_STR) {
            jsize utf8Length = env->GetStringUTFLength(data);
            buf = nsMemory::Alloc(utf8Length + 1);
            if (!buf) {
              rv = NS_ERROR_OUT_OF_MEMORY;
              break;
            }

            env->GetStringUTFRegion(data, 0, uniLength, (char*) buf);
            ((char*)buf)[utf8Length] = '\0';

          } else {  // if T_WCHAR_STR
            buf = nsMemory::Alloc((uniLength + 1) * sizeof(jchar));
            if (!buf) {
              rv = NS_ERROR_OUT_OF_MEMORY;
              break;
            }

            env->GetStringRegion(data, 0, uniLength, (jchar*) buf);
            ((jchar*)buf)[uniLength] = '\0';
          }
        } else {
          // create empty string
          buf = nsMemory::Alloc(2);
          if (!buf) {
            rv = NS_ERROR_OUT_OF_MEMORY;
            break;
          }
          ((jchar*)buf)[0] = '\0';
        }
      }

      if (!aIsArrayElement) { // 'in' & 'inout'
        aVariant.val.p = buf;
        if (aIsOut) { // 'inout'
          aVariant.ptr = &aVariant.val;
          aVariant.SetPtrIsData();
        }
      } else {  // 'array'
        if (aType == nsXPTType::T_CHAR_STR) {
          char* str = NS_STATIC_CAST(char*, buf);
          NS_STATIC_CAST(char**, aVariant.val.p)[aIndex] = str;
        } else {
          PRUnichar* str = NS_STATIC_CAST(PRUnichar*, buf);
          NS_STATIC_CAST(PRUnichar**, aVariant.val.p)[aIndex] = str;
        }
      }
    }
    break;

    case nsXPTType::T_IID:
    {
      LOG(("String(IID)\n"));
      jstring data = nsnull;
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        data = (jstring) aParam;
      } else if (aParam) {  // 'inout' & 'array'
        data = (jstring) env->GetObjectArrayElement((jobjectArray) aParam,
                                                    aIndex);
      }

      nsID* iid = new nsID;
      if (!iid) {
        rv = NS_ERROR_OUT_OF_MEMORY;
        break;
      }
      if (data) {
        // extract IID string from Java string
        const char* str = env->GetStringUTFChars(data, nsnull);
        if (!str) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }

        // parse string into IID object
        iid->Parse(str);
        env->ReleaseStringUTFChars(data, str);
      } else {
        *iid = nullID;
      }

      if (!aIsArrayElement) { // 'in' & 'inout'
        aVariant.val.p = iid;
        if (aIsOut) { // 'inout'
          aVariant.ptr = &aVariant.val;
          aVariant.SetPtrIsData();
        }
      } else {  // 'array'
        NS_STATIC_CAST(nsID**, aVariant.val.p)[aIndex] = iid;
      }
    }
    break;

    case nsXPTType::T_INTERFACE:
    case nsXPTType::T_INTERFACE_IS:
    {
      LOG(("nsISupports\n"));
      jobject java_obj = nsnull;
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        java_obj = (jobject) aParam;
      } else if (aParam) {  // 'inout' & 'array'
        java_obj = (jobject) env->GetObjectArrayElement((jobjectArray) aParam,
                                                        aIndex);
      }

      nsISupports* xpcom_obj;
      if (java_obj) {
        // If the requested interface is nsIWeakReference, then we look for or
        // create a stub for the nsISupports interface.  Then we create a weak
        // reference from that stub.
        PRBool isWeakRef;
        nsID iid;
        if (aIID.Equals(NS_GET_IID(nsIWeakReference))) {
          isWeakRef = PR_TRUE;
          iid = NS_GET_IID(nsISupports);
        } else {
          isWeakRef = PR_FALSE;
          iid = aIID;
        }

        PRBool isXPTCStub;
        rv = GetNewOrUsedXPCOMObject(env, java_obj, iid, &xpcom_obj,
                                     &isXPTCStub);
        if (NS_FAILED(rv))
          break;

        // If the function expects a weak reference, then we need to
        // create it here.
        if (isWeakRef) {
          if (isXPTCStub) {
            nsJavaXPTCStub* stub = NS_STATIC_CAST(nsJavaXPTCStub*,
                                                 NS_STATIC_CAST(nsXPTCStubBase*,
                                                                xpcom_obj));
            nsJavaXPTCStubWeakRef* weakref;
            weakref = new nsJavaXPTCStubWeakRef(env, java_obj, stub);
            if (!weakref) {
              rv = NS_ERROR_OUT_OF_MEMORY;
              break;
            }
            NS_RELEASE(xpcom_obj);
            xpcom_obj = weakref;
            NS_ADDREF(xpcom_obj);
          } else { // if is native XPCOM object
            nsCOMPtr<nsISupportsWeakReference> supportsweak =
                                                   do_QueryInterface(xpcom_obj);
            if (supportsweak) {
              nsWeakPtr weakref;
              supportsweak->GetWeakReference(getter_AddRefs(weakref));
              NS_RELEASE(xpcom_obj);
              xpcom_obj = weakref;
              NS_ADDREF(xpcom_obj);
            } else {
              xpcom_obj = nsnull;
            }
          }
        }
      } else {
        xpcom_obj = nsnull;
      }

      if (!aIsArrayElement) { // 'in' & 'inout'
        aVariant.val.p = xpcom_obj;
        aVariant.SetValIsInterface();
        if (aIsOut) { // 'inout'
          aVariant.ptr = &aVariant.val;
          aVariant.SetPtrIsData();
        }
      } else {  // 'array'
        NS_STATIC_CAST(nsISupports**, aVariant.val.p)[aIndex] = xpcom_obj;
      }
    }
    break;

    case nsXPTType::T_ASTRING:
    case nsXPTType::T_DOMSTRING:
    {
      LOG(("String\n"));
      jstring data = nsnull;
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        data = (jstring) aParam;
      } else if (aParam) {  // 'inout' & 'array'
        data = (jstring) env->GetObjectArrayElement((jobjectArray) aParam,
                                                    aIndex);
      }

      nsAString* str;
      if (data) {
        str = jstring_to_nsAString(env, data);
        if (!str) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }
      } else {
        str = nsnull;
      }

      if (!aIsArrayElement) { // 'in' & 'inout'
        aVariant.val.p = str;
        aVariant.SetValIsDOMString();
        if (aIsOut) { // 'inout'
          aVariant.ptr = &aVariant.val;
          aVariant.SetPtrIsData();
        }
      } else {  // 'array'
        NS_STATIC_CAST(nsAString**, aVariant.val.p)[aIndex] = str;
      }
    }
    break;

    case nsXPTType::T_UTF8STRING:
    case nsXPTType::T_CSTRING:
    {
      LOG(("StringUTF\n"));
      jstring data = nsnull;
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        data = (jstring) aParam;
      } else if (aParam) {  // 'inout' & 'array'
        data = (jstring) env->GetObjectArrayElement((jobjectArray) aParam,
                                                    aIndex);
      }

      nsACString* str;
      if (data) {
        str = jstring_to_nsACString(env, data);
        if (!str) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }
      } else {
        str = nsnull;
      }

      if (!aIsArrayElement) { // 'in' & 'inout'
        aVariant.val.p = str;
        if (aType == nsXPTType::T_CSTRING) {
          aVariant.SetValIsCString();
        } else {
          aVariant.SetValIsUTF8String();
        }
        if (aIsOut) { // 'inout'
          aVariant.ptr = &aVariant.val;
          aVariant.SetPtrIsData();
        }
      } else {  // 'array'
        NS_STATIC_CAST(nsACString**, aVariant.val.p)[aIndex] = str;
      }
    }
    break;

    // handle "void *" as an "int" in Java
    case nsXPTType::T_VOID:
    {
      LOG(("int (void*)\n"));
      if (!aIsOut && !aIsArrayElement) {  // 'in'
        aVariant.val.p = (void*) env->CallIntMethod(aParam, intValueMID);
      } else { // 'inout' & 'array'
        jint value;
        if (aParam) {
          env->GetIntArrayRegion((jintArray) aParam, aIndex, 1, &value);
        }

        if (aIsOut) { // 'inout'
          if (aParam) {
            aVariant.val.p = NS_REINTERPRET_CAST(void*, value);
            aVariant.ptr = &aVariant.val;
          } else {
            aVariant.ptr = nsnull;
          }
          aVariant.SetPtrIsData();
        } else {  // 'array'
          NS_STATIC_CAST(PRUint32*, aVariant.val.p)[aIndex] = value;
        }
      }
    }
    break;

    case nsXPTType::T_ARRAY:
    {
      jobject sourceArray = nsnull;
      if (!aIsOut) {  // 'in'
        sourceArray = aParam;
      } else if (aParam) {  // 'inout'
        jobjectArray array = NS_STATIC_CAST(jobjectArray, aParam);
        sourceArray = env->GetObjectArrayElement(array, 0);
      }

      if (sourceArray) {
        rv = CreateNativeArray(aArrayType, aArraySize, &aVariant.val.p);

        for (PRUint32 i = 0; i < aArraySize && NS_SUCCEEDED(rv); i++) {
          rv = SetupParams(env, sourceArray, aArrayType, PR_FALSE, aIID, 0, 0,
                           PR_TRUE, i, aVariant);
        }
      }

      if (aIsOut) { // 'inout'
        aVariant.ptr = &aVariant.val.p;
        aVariant.SetPtrIsData();
      }
    }
    break;

    default:
      NS_WARNING("unexpected parameter type");
      return NS_ERROR_UNEXPECTED;
  }

  return rv;
}

/**
 * Handles 'in', 'out', and 'inout' params.
 */
nsresult
FinalizeParams(JNIEnv *env, const jobject aParam, PRUint8 aType, PRBool aIsOut,
               const nsID& aIID, PRUint8 aArrayType, PRUint32 aArraySize,
               PRBool aIsArrayElement, PRUint32 aIndex, nsXPTCVariant &aVariant)
{
  nsresult rv = NS_OK;

  switch (aType)
  {
    case nsXPTType::T_I8:
    case nsXPTType::T_U8:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetByteArrayRegion((jbyteArray) aParam, aIndex, 1,
                                (jbyte*) &aVariant.val.u8);
      }
    }
    break;

    case nsXPTType::T_I16:
    case nsXPTType::T_U16:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetShortArrayRegion((jshortArray) aParam, aIndex, 1,
                                 (jshort*) &aVariant.val.u16);
      }
    }
    break;

    case nsXPTType::T_I32:
    case nsXPTType::T_U32:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetIntArrayRegion((jintArray) aParam, aIndex, 1,
                               (jint*) &aVariant.val.u32);
      }
    }
    break;

    case nsXPTType::T_I64:
    case nsXPTType::T_U64:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetLongArrayRegion((jlongArray) aParam, aIndex, 1,
                                (jlong*) &aVariant.val.u64);
      }
    }
    break;

    case nsXPTType::T_FLOAT:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetFloatArrayRegion((jfloatArray) aParam, aIndex, 1,
                                 (jfloat*) &aVariant.val.f);
      }
    }
    break;

    case nsXPTType::T_DOUBLE:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetDoubleArrayRegion((jdoubleArray) aParam, aIndex, 1,
                                  (jdouble*) &aVariant.val.d);
      }
    }
    break;

    case nsXPTType::T_BOOL:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetBooleanArrayRegion((jbooleanArray) aParam, aIndex, 1,
                                   (jboolean*) &aVariant.val.b);
      }
    }
    break;

    case nsXPTType::T_CHAR:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetCharArrayRegion((jcharArray) aParam, aIndex, 1,
                                (jchar*) &aVariant.val.c);
      }
    }
    break;

    case nsXPTType::T_WCHAR:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetCharArrayRegion((jcharArray) aParam, aIndex, 1,
                                (const jchar*) &aVariant.val.wc);
      }
    }
    break;

    case nsXPTType::T_CHAR_STR:
    case nsXPTType::T_WCHAR_STR:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        // create new string from data
        jstring str;
        if (aVariant.val.p) {
          if (aType == nsXPTType::T_CHAR_STR) {
            str = env->NewStringUTF((const char*) aVariant.val.p);
          } else {
            PRUint32 length = nsCRT::strlen((const PRUnichar*) aVariant.val.p);
            str = env->NewString((const jchar*) aVariant.val.p, length);
          }
          if (!str) {
            rv = NS_ERROR_OUT_OF_MEMORY;
            break;
          }
        } else {
          str = nsnull;
        }

        // put new string into output array
        env->SetObjectArrayElement((jobjectArray) aParam, aIndex, str);
      }

      // Delete for 'in', 'inout', 'out' and 'array'
      if (aVariant.val.p)
        nsMemory::Free(aVariant.val.p);
    }
    break;

    case nsXPTType::T_IID:
    {
      nsID* iid = (nsID*) aVariant.val.p;

      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        // Create the string from nsID
        jstring str = nsnull;
        if (iid) {
          char* iid_str = iid->ToString();
          if (iid_str) {
            str = env->NewStringUTF(iid_str);
          }
          if (!iid_str || !str) {
            rv = NS_ERROR_OUT_OF_MEMORY;
            break;
          }
          PR_Free(iid_str);
        }

        // put new string into output array
        env->SetObjectArrayElement((jobjectArray) aParam, aIndex, str);
      }

      // Ordinarily, we would delete 'iid' here.  But we cannot do that until
      // we've handled all of the params.  See comment in CallXPCOMMethod
    }
    break;

    case nsXPTType::T_INTERFACE:
    case nsXPTType::T_INTERFACE_IS:
    {
      nsISupports* xpcom_obj = NS_STATIC_CAST(nsISupports*, aVariant.val.p);

      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        jobject java_obj = nsnull;
        if (xpcom_obj) {
          // Get matching Java object for given xpcom object
          PRBool isNewProxy;
          rv = GetNewOrUsedJavaObject(env, xpcom_obj, aIID, &java_obj,
                                      &isNewProxy);
          if (NS_FAILED(rv))
            break;
          if (isNewProxy)
            NS_RELEASE(xpcom_obj);   // Java proxy owns ref to object
        }

        // put new Java object into output array
        env->SetObjectArrayElement((jobjectArray) aParam, aIndex, java_obj);
      }

      NS_IF_RELEASE(xpcom_obj);
    }
    break;

    case nsXPTType::T_ASTRING:
    case nsXPTType::T_DOMSTRING:
    {
      nsString* str = (nsString*) aVariant.val.p;

      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        // Create Java string from returned nsString
        jstring jstr;
        if (str) {
          jstr = env->NewString((const jchar*) str->get(), str->Length());
          if (!jstr) {
            rv = NS_ERROR_OUT_OF_MEMORY;
            break;
          }
        } else {
          jstr = nsnull;
        }

        // put new Java string into output array
        env->SetObjectArrayElement((jobjectArray) aParam, aIndex, jstr);
      }

      if (str) {
        delete str;
      }
    }
    break;

    case nsXPTType::T_UTF8STRING:
    case nsXPTType::T_CSTRING:
    {
      nsCString* str = (nsCString*) aVariant.val.p;

      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        // Create Java string from returned nsString
        jstring jstr;
        if (str) {
          jstr = env->NewStringUTF((const char*) str->get());
          if (!jstr) {
            rv = NS_ERROR_OUT_OF_MEMORY;
            break;
          }
        } else {
          jstr = nsnull;
        }

        // put new Java string into output array
        env->SetObjectArrayElement((jobjectArray) aParam, aIndex, jstr);
      }

      if (str) {
        delete str;
      }
    }
    break;

    case nsXPTType::T_VOID:
    {
      if ((aIsOut || aIsArrayElement) && aParam) { // 'inout', 'out' & 'array'
        env->SetIntArrayRegion((jintArray) aParam, aIndex, 1,
                               (jint*) &aVariant.val.p);
      }
    }
    break;

    case nsXPTType::T_ARRAY:
    {
      if (aIsOut && aParam) { // 'inout' & 'out'
        // Create Java array from returned native array
        jobject jarray = nsnull;
        if (aVariant.val.p) {
          rv = CreateJavaArray(env, aArrayType, aArraySize, aIID, &jarray);
          if (NS_FAILED(rv))
            break;

          nsXPTCVariant var;
          for (PRUint32 i = 0; i < aArraySize && NS_SUCCEEDED(rv); i++) {
            rv = GetNativeArrayElement(aArrayType, aVariant.val.p, i, &var);
            if (NS_SUCCEEDED(rv)) {
              rv = FinalizeParams(env, jarray, aArrayType, PR_FALSE, aIID, 0, 0,
                                  PR_TRUE, i, var);
            }
          }
        }

        // put new Java array into output array
        env->SetObjectArrayElement((jobjectArray) aParam, 0, jarray);
      }

      // cleanup
      PR_Free(aVariant.val.p);
    }
    break;

    default:
      NS_WARNING("unexpected parameter type");
      return NS_ERROR_UNEXPECTED;
  }

  // Check for Java exception, but don't overwrite pre-existing error code.
  if (NS_SUCCEEDED(rv) && env->ExceptionCheck())
    rv = NS_ERROR_FAILURE;

  return rv;
}

/**
 * Handles 'retval' and 'dipper' params.
 */
nsresult
SetRetval(JNIEnv *env, PRUint8 aType, nsXPTCVariant &aVariant, const nsID &aIID,
          PRUint8 aArrayType, PRUint32 aArraySize, PRBool aIsArrayElement,
          PRUint32 aIndex, jobject* aResult)
{
  nsresult rv = NS_OK;

  jobject obj = nsnull;
  switch (aType)
  {
    case nsXPTType::T_I8:
    case nsXPTType::T_U8:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(byteClass, byteInitMID, aVariant.val.u8);
      } else {
        jbyteArray array = NS_STATIC_CAST(jbyteArray, *aResult);
        env->SetByteArrayRegion(array, aIndex, 1,
                                (const jbyte*) &aVariant.val.u8);
      }
      break;

    case nsXPTType::T_I16:
    case nsXPTType::T_U16:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(shortClass, shortInitMID, aVariant.val.u16);
      } else {
        jshortArray array = NS_STATIC_CAST(jshortArray, *aResult);
        env->SetShortArrayRegion(array, aIndex, 1,
                                 (const jshort*) &aVariant.val.u16);
      }
      break;

    case nsXPTType::T_I32:
    case nsXPTType::T_U32:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(intClass, intInitMID, aVariant.val.u32);
      } else {
        jintArray array = NS_STATIC_CAST(jintArray, *aResult);
        env->SetIntArrayRegion(array, aIndex, 1,
                               (const jint*) &aVariant.val.u32);
      }
      break;

    case nsXPTType::T_I64:
    case nsXPTType::T_U64:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(longClass, longInitMID, aVariant.val.u64);
      } else {
        jlongArray array = NS_STATIC_CAST(jlongArray, *aResult);
        env->SetLongArrayRegion(array, aIndex, 1,
                                (const jlong*) &aVariant.val.u64);
      }
      break;

    case nsXPTType::T_FLOAT:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(floatClass, floatInitMID, aVariant.val.f);
      } else {
        jfloatArray array = NS_STATIC_CAST(jfloatArray, *aResult);
        env->SetFloatArrayRegion(array, aIndex, 1,
                                 (const jfloat*) &aVariant.val.f);
      }
      break;

    case nsXPTType::T_DOUBLE:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(doubleClass, doubleInitMID, aVariant.val.d);
      } else {
        jdoubleArray array = NS_STATIC_CAST(jdoubleArray, *aResult);
        env->SetDoubleArrayRegion(array, aIndex, 1,
                                  (const jdouble*) &aVariant.val.d);
      }
      break;

    case nsXPTType::T_BOOL:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(booleanClass, booleanInitMID, aVariant.val.b);
      } else {
        jbooleanArray array = NS_STATIC_CAST(jbooleanArray, *aResult);
        env->SetBooleanArrayRegion(array, aIndex, 1,
                                   (const jboolean*) &aVariant.val.b);
      }
      break;

    case nsXPTType::T_CHAR:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(charClass, charInitMID, aVariant.val.c);
      } else {
        jcharArray array = NS_STATIC_CAST(jcharArray, *aResult);
        env->SetCharArrayRegion(array, aIndex, 1,
                                (const jchar*) &aVariant.val.c);
      }
      break;

    case nsXPTType::T_WCHAR:
      if (!aIsArrayElement) {
        *aResult = env->NewObject(charClass, charInitMID, aVariant.val.wc);
      } else {
        jcharArray array = NS_STATIC_CAST(jcharArray, *aResult);
        env->SetCharArrayRegion(array, aIndex, 1,
                                (const jchar*) &aVariant.val.wc);
      }
      break;

    case nsXPTType::T_CHAR_STR:
    {
      char* str = NS_STATIC_CAST(char*, aVariant.val.p);
      if (str) {
        obj = env->NewStringUTF(str);
        if (obj == nsnull) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }
      }

      if (!aIsArrayElement) {
        *aResult = obj;
      } else {
        env->SetObjectArrayElement((jobjectArray) *aResult, aIndex, obj);
      }
    }
    break;

    case nsXPTType::T_WCHAR_STR:
    {
      PRUnichar* str = NS_STATIC_CAST(PRUnichar*, aVariant.val.p);
      if (str) {
        PRUint32 length = nsCRT::strlen(str);
        obj = env->NewString(str, length);
        if (obj == nsnull) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }
      }

      if (!aIsArrayElement) {
        *aResult = obj;
      } else {
        env->SetObjectArrayElement((jobjectArray) *aResult, aIndex, obj);
      }
    }
    break;

    case nsXPTType::T_IID:
    {
      nsID* iid = NS_STATIC_CAST(nsID*, aVariant.val.p);
      if (iid) {
        char* iid_str = iid->ToString();
        if (iid_str) {
          obj = env->NewStringUTF(iid_str);
        }
        if (iid_str == nsnull || obj == nsnull) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }
        PR_Free(iid_str);
      }

      if (!aIsArrayElement) {
        *aResult = obj;
      } else {
        env->SetObjectArrayElement((jobjectArray) *aResult, aIndex, obj);
      }
    }
    break;

    case nsXPTType::T_INTERFACE:
    case nsXPTType::T_INTERFACE_IS:
    {
      nsISupports* xpcom_obj = NS_STATIC_CAST(nsISupports*, aVariant.val.p);
      if (xpcom_obj) {
        // Get matching Java object for given xpcom object
        jobject java_obj;
        PRBool isNewProxy;
        rv = GetNewOrUsedJavaObject(env, xpcom_obj, aIID, &java_obj,
                                    &isNewProxy);
        if (NS_FAILED(rv))
          break;
        if (isNewProxy)
          xpcom_obj->Release();   // Java proxy owns ref to object

        // If returned object is an nsJavaXPTCStub, release it.
        nsJavaXPTCStub* stub = nsnull;
        xpcom_obj->QueryInterface(NS_GET_IID(nsJavaXPTCStub), (void**) &stub);
        if (stub) {
          NS_RELEASE(xpcom_obj);
          NS_RELEASE(stub);
        }

        obj = java_obj;

        if (!aIsArrayElement) {
          *aResult = obj;
        } else {
          env->SetObjectArrayElement((jobjectArray) *aResult, aIndex, obj);
        }
      }
    }
    break;

    case nsXPTType::T_ASTRING:
    case nsXPTType::T_DOMSTRING:
    {
      nsString* str = NS_STATIC_CAST(nsString*, aVariant.val.p);
      if (str) {
        obj = env->NewString(str->get(), str->Length());
        if (obj == nsnull) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }
        delete str;
      }

      if (!aIsArrayElement) {
        *aResult = obj;
      } else {
        env->SetObjectArrayElement((jobjectArray) *aResult, aIndex, obj);
      }
    }
    break;

    case nsXPTType::T_UTF8STRING:
    case nsXPTType::T_CSTRING:
    {
      nsCString* str = NS_STATIC_CAST(nsCString*, aVariant.val.p);
      if (str) {
        obj = env->NewStringUTF(str->get());
        if (obj == nsnull) {
          rv = NS_ERROR_OUT_OF_MEMORY;
          break;
        }
        delete str;
      }

      if (!aIsArrayElement) {
        *aResult = obj;
      } else {
        env->SetObjectArrayElement((jobjectArray) *aResult, aIndex, obj);
      }
    }
    break;

    case nsXPTType::T_VOID:
      // handle "void *" as an "int" in Java
      LOG((" returns int (void*)"));
      if (!aIsArrayElement) {
        *aResult = env->NewObject(intClass, intInitMID, aVariant.val.p);
      } else {
        jintArray array = NS_STATIC_CAST(jintArray, *aResult);
        env->SetIntArrayRegion(array, aIndex, 1,
                               (const jint*) &aVariant.val.p);
      }
      break;

    case nsXPTType::T_ARRAY:
    {
      jobject jarray = nsnull;
      if (aVariant.val.p) {
        rv = CreateJavaArray(env, aArrayType, aArraySize, aIID, &jarray);

        nsXPTCVariant var;
        for (PRUint32 i = 0; i < aArraySize && NS_SUCCEEDED(rv); i++) {
          rv = GetNativeArrayElement(aArrayType, aVariant.val.p, i, &var);
          if (NS_SUCCEEDED(rv)) {
            rv = SetRetval(env, aArrayType, var, aIID, 0, 0, PR_TRUE, i,
                           &jarray);
          }
        }
      }

      *aResult = jarray;
    }
    break;

    default:
      NS_WARNING("unexpected parameter type");
      return NS_ERROR_UNEXPECTED;
  }

  // Check for Java exception, but don't overwrite pre-existing error code.
  if (NS_SUCCEEDED(rv) && env->ExceptionCheck())
    rv = NS_ERROR_FAILURE;

  return rv;
}

/**
 * Given an interface info struct and a method name, returns the method info
 * and index, if that method exists.
 *
 * Most method names are lower case.  Unfortunately, the method names of some
 * interfaces (such as nsIAppShell) start with a capital letter.  This function
 * will try all of the permutations.
 */
nsresult
QueryMethodInfo(nsIInterfaceInfo* aIInfo, const char* aMethodName,
                PRUint16* aMethodIndex, const nsXPTMethodInfo** aMethodInfo)
{
  // The common case is that the method name is lower case, so we check
  // that first.
  nsresult rv;
  rv = aIInfo->GetMethodInfoForName(aMethodName, aMethodIndex, aMethodInfo);
  if (NS_SUCCEEDED(rv))
    return rv;

  // If there is no method called <aMethodName>, then maybe it is an
  // 'attribute'.  An 'attribute' will start with "get" or "set".  But first,
  // we check the length, in order to skip over method names that match exactly
  // "get" or "set".
  if (strlen(aMethodName) > 3) {
    if (strncmp("get", aMethodName, 3) == 0) {
      char* getterName = strdup(aMethodName + 3);
      getterName[0] = tolower(getterName[0]);
      rv = aIInfo->GetMethodInfoForName(getterName, aMethodIndex, aMethodInfo);
      free(getterName);
    } else if (strncmp("set", aMethodName, 3) == 0) {
      char* setterName = strdup(aMethodName + 3);
      setterName[0] = tolower(setterName[0]);
      rv = aIInfo->GetMethodInfoForName(setterName, aMethodIndex, aMethodInfo);
      if (NS_SUCCEEDED(rv)) {
        // If this succeeded, GetMethodInfoForName will have returned the
        // method info for the 'getter'.  We want the 'setter', so increase
        // method index by one ('setter' immediately follows the 'getter'),
        // and get its method info.
        (*aMethodIndex)++;
        rv = aIInfo->GetMethodInfo(*aMethodIndex, aMethodInfo);
        if (NS_SUCCEEDED(rv)) {
          // Double check that this methodInfo matches the given method.
          if (!(*aMethodInfo)->IsSetter() ||
              strcmp(setterName, (*aMethodInfo)->name) != 0) {
            rv = NS_ERROR_FAILURE;
          }
        }
      }
      free(setterName);
    }
  }
  if (NS_SUCCEEDED(rv))
    return rv;

  // If we get here, then maybe the method name is capitalized.
  char* methodName = strdup(aMethodName);
  methodName[0] = toupper(methodName[0]);
  rv = aIInfo->GetMethodInfoForName(methodName, aMethodIndex, aMethodInfo);
  free(methodName);
  if (NS_SUCCEEDED(rv))
    return rv;

  // If there is no method called <aMethodName>, then maybe it is an
  // 'attribute'.
  if (strlen(aMethodName) > 3) {
    if (strncmp("get", aMethodName, 3) == 0) {
      char* getterName = strdup(aMethodName + 3);
      rv = aIInfo->GetMethodInfoForName(getterName, aMethodIndex, aMethodInfo);
      free(getterName);
    } else if (strncmp("set", aMethodName, 3) == 0) {
      char* setterName = strdup(aMethodName + 3);
      rv = aIInfo->GetMethodInfoForName(setterName, aMethodIndex, aMethodInfo);
      if (NS_SUCCEEDED(rv)) {
        // If this succeeded, GetMethodInfoForName will have returned the
        // method info for the 'getter'.  We want the 'setter', so increase
        // method index by one ('setter' immediately follows the 'getter'),
        // and get its method info.
        (*aMethodIndex)++;
        rv = aIInfo->GetMethodInfo(*aMethodIndex, aMethodInfo);
        if (NS_SUCCEEDED(rv)) {
          // Double check that this methodInfo matches the given method.
          if (!(*aMethodInfo)->IsSetter() ||
              strcmp(setterName, (*aMethodInfo)->name) != 0) {
            rv = NS_ERROR_FAILURE;
          }
        }
      }
      free(setterName);
    }
  }

  return rv;
}

/**
 *  org.mozilla.xpcom.XPCOMJavaProxy.callXPCOMMethod
 */
extern "C" JX_EXPORT jobject JNICALL
JAVAPROXY_NATIVE(callXPCOMMethod) (JNIEnv *env, jclass that, jobject aJavaProxy,
                                   jstring aMethodName, jobjectArray aParams)
{
  nsresult rv;

  // Get native XPCOM instance
  void* xpcom_obj;
  rv = GetXPCOMInstFromProxy(env, aJavaProxy, &xpcom_obj);
  if (NS_FAILED(rv)) {
    ThrowException(env, 0, "Failed to get matching XPCOM object");
    return nsnull;
  }
  JavaXPCOMInstance* inst = NS_STATIC_CAST(JavaXPCOMInstance*, xpcom_obj);

  // Get method info
  PRUint16 methodIndex;
  const nsXPTMethodInfo* methodInfo;
  nsIInterfaceInfo* iinfo = inst->InterfaceInfo();
  const char* methodName = env->GetStringUTFChars(aMethodName, nsnull);
  rv = QueryMethodInfo(iinfo, methodName, &methodIndex, &methodInfo);
  env->ReleaseStringUTFChars(aMethodName, methodName);

  if (NS_FAILED(rv)) {
    ThrowException(env, rv, "GetMethodInfoForName failed");
    return nsnull;
  }

#ifdef DEBUG_JAVAXPCOM
  const char* ifaceName;
  iinfo->GetNameShared(&ifaceName);
  LOG(("===> (XPCOM) %s::%s()\n", ifaceName, methodInfo->GetName()));
#endif

  // Convert the Java params
  PRUint8 paramCount = methodInfo->GetParamCount();
  nsXPTCVariant* params = nsnull;
  if (paramCount)
  {
    params = new nsXPTCVariant[paramCount];
    if (!params) {
      ThrowException(env, NS_ERROR_OUT_OF_MEMORY, "Can't create params array");
      return nsnull;
    }
    memset(params, 0, paramCount * sizeof(nsXPTCVariant));

    for (PRUint8 i = 0; i < paramCount && NS_SUCCEEDED(rv); i++)
    {
      LOG(("\t Param %d: ", i));
      const nsXPTParamInfo &paramInfo = methodInfo->GetParam(i);

      if (paramInfo.IsIn() && !paramInfo.IsDipper()) {
        PRUint8 type = paramInfo.GetType().TagPart();

        // is paramater an array?
        PRUint8 arrayType;
        PRUint32 arraySize;
        if (type == nsXPTType::T_ARRAY) {
          // get array type
          nsXPTType xpttype;
          rv = iinfo->GetTypeForParam(methodIndex, &paramInfo, 1, &xpttype);
          if (NS_FAILED(rv))
            break;
          arrayType = xpttype.TagPart();

          // get size of array
          PRUint8 argnum;
          rv = iinfo->GetSizeIsArgNumberForParam(methodIndex, &paramInfo, 0,
                                                 &argnum);
          if (NS_FAILED(rv))
            break;
          arraySize = params[argnum].val.u32;
        }

        // get IID for interface params
        nsID iid;
        if (type == nsXPTType::T_INTERFACE ||
            type == nsXPTType::T_INTERFACE_IS ||
            type == nsXPTType::T_ARRAY &&
              (arrayType == nsXPTType::T_INTERFACE ||
               arrayType == nsXPTType::T_INTERFACE_IS))
        {
          PRUint8 paramType = type == nsXPTType::T_ARRAY ? arrayType : type;
          rv = GetIIDForMethodParam(iinfo, methodInfo, paramInfo, paramType,
                                    methodIndex, params, PR_TRUE, iid);
        }

        if (NS_SUCCEEDED(rv)) {
          rv = SetupParams(env, env->GetObjectArrayElement(aParams, i), type,
                           paramInfo.IsOut(), iid, arrayType, arraySize,
                           PR_FALSE, 0, params[i]);
        }
      } else if (paramInfo.IsDipper()) {
        LOG(("dipper\n"));
        const nsXPTType &type = paramInfo.GetType();
        switch (type.TagPart())
        {
          case nsXPTType::T_ASTRING:
          case nsXPTType::T_DOMSTRING:
          {
            params[i].val.p = new nsString();
            if (params[i].val.p == nsnull) {
              rv = NS_ERROR_OUT_OF_MEMORY;
              break;
            }
            params[i].type = type;
            params[i].flags = nsXPTCVariant::VAL_IS_DOMSTR;
            break;
          }

          case nsXPTType::T_UTF8STRING:
          case nsXPTType::T_CSTRING:
          {
            params[i].val.p = new nsCString();
            if (params[i].val.p == nsnull) {
              rv = NS_ERROR_OUT_OF_MEMORY;
              break;
            }
            params[i].type = type;
            params[i].flags = nsXPTCVariant::VAL_IS_CSTR;
            break;
          }

          default:
            LOG(("unhandled dipper type\n"));
            rv = NS_ERROR_UNEXPECTED;
        }
      } else {
        LOG(("out/retval\n"));
        params[i].ptr = &(params[i].val);
        params[i].type = paramInfo.GetType();
        params[i].flags = nsXPTCVariant::PTR_IS_DATA;
      }
    }
    if (NS_FAILED(rv)) {
      ThrowException(env, rv, "SetupParams failed");
      return nsnull;
    }
  }

  // Call the XPCOM method
  nsresult invokeResult;
  invokeResult = XPTC_InvokeByIndex(inst->GetInstance(), methodIndex,
                                    paramCount, params);

  // Clean up params
  jobject result = nsnull;
  for (PRUint8 i = 0; i < paramCount && NS_SUCCEEDED(rv); i++)
  {
    const nsXPTParamInfo &paramInfo = methodInfo->GetParam(i);
    PRUint8 type = paramInfo.GetType().TagPart();

    // is paramater an array?
    PRUint8 arrayType;
    PRUint32 arraySize;
    if (type == nsXPTType::T_ARRAY) {
      // get array type
      nsXPTType array_xpttype;
      rv = iinfo->GetTypeForParam(methodIndex, &paramInfo, 1, &array_xpttype);
      if (NS_FAILED(rv))
        break;
      arrayType = array_xpttype.TagPart();

      // get size of array
      PRUint8 argnum;
      rv = iinfo->GetSizeIsArgNumberForParam(methodIndex, &paramInfo, 0,
                                             &argnum);
      if (NS_FAILED(rv))
        break;
      arraySize = params[argnum].val.u32;
    }

    // get IID for interface params
    nsID iid;
    if (type == nsXPTType::T_INTERFACE || type == nsXPTType::T_INTERFACE_IS ||
        type == nsXPTType::T_ARRAY && (arrayType == nsXPTType::T_INTERFACE ||
                                       arrayType == nsXPTType::T_INTERFACE_IS))
    {
      PRUint8 paramType = type == nsXPTType::T_ARRAY ? arrayType : type;
      rv = GetIIDForMethodParam(iinfo, methodInfo, paramInfo, paramType,
                                methodIndex, params, PR_TRUE, iid);
      if (NS_FAILED(rv))
        break;
    }

    if (!paramInfo.IsRetval()) {
      rv = FinalizeParams(env, env->GetObjectArrayElement(aParams, i), type,
                          paramInfo.IsOut(), iid, arrayType, arraySize,
                          PR_FALSE, 0, params[i]);
    } else {
      rv = SetRetval(env, type, params[i], iid, arrayType, arraySize,
                     PR_FALSE, 0, &result);
    }
  }
  if (NS_FAILED(rv)) {
    ThrowException(env, rv, "FinalizeParams/SetRetval failed");
    return nsnull;
  }

  // Normally, we would delete any created nsID object in the above loop.
  // However, GetIIDForMethodParam may need some of the nsID params when it's
  // looking for the IID of an INTERFACE_IS.  Therefore, we can't delete it
  // until we've gone through the 'Finalize' loop once and created the result.
  for (PRUint8 j = 0; j < paramCount && NS_SUCCEEDED(rv); j++)
  {
    const nsXPTParamInfo &paramInfo = methodInfo->GetParam(j);
    const nsXPTType &type = paramInfo.GetType();
    if (type.TagPart() == nsXPTType::T_IID) {
      nsID* iid = (nsID*) params[j].val.p;
      delete iid;
    }
  }

  if (params) {
    delete params;
  }

  // If the XPCOM method invocation failed, we don't immediately throw an
  // exception and return so that we can clean up any parameters.
  if (NS_FAILED(invokeResult)) {
    nsCAutoString message("The function \"");
    message.AppendASCII(methodInfo->GetName());
    message.AppendLiteral("\" returned an error condition");
    ThrowException(env, invokeResult, message.get());
  }

  LOG(("<=== (XPCOM) %s::%s()\n", ifaceName, methodInfo->GetName()));
  return result;
}

nsresult
CreateJavaProxy(JNIEnv* env, nsISupports* aXPCOMObject, const nsIID& aIID,
                jobject* aResult)
{
  NS_PRECONDITION(aResult != nsnull, "null ptr");
  if (!aResult)
    return NS_ERROR_NULL_POINTER;

  nsCOMPtr<nsIInterfaceInfoManager> iim = XPTI_GetInterfaceInfoManager();
  NS_ASSERTION(iim != nsnull, "Failed to get InterfaceInfoManager");
  if (!iim)
    return NS_ERROR_FAILURE;

  // Get interface info for class
  nsCOMPtr<nsIInterfaceInfo> info;
  nsresult rv = iim->GetInfoForIID(&aIID, getter_AddRefs(info));
  if (NS_FAILED(rv))
    return rv;

  // Wrap XPCOM object
  JavaXPCOMInstance* inst = new JavaXPCOMInstance(aXPCOMObject, info);
  if (!inst)
    return NS_ERROR_OUT_OF_MEMORY;

  // Get interface name
  const char* iface_name;
  rv = info->GetNameShared(&iface_name);

  if (NS_SUCCEEDED(rv)) {
    jobject java_obj = nsnull;

    // Create proper Java interface name
    nsCAutoString class_name("org/mozilla/xpcom/");
    class_name.AppendASCII(iface_name);
    jclass ifaceClass = env->FindClass(class_name.get());

    if (ifaceClass) {
      java_obj = env->CallStaticObjectMethod(xpcomJavaProxyClass,
                                             createProxyMID, ifaceClass,
                                             NS_REINTERPRET_CAST(jlong, inst));
      if (env->ExceptionCheck())
        java_obj = nsnull;
    }

    if (java_obj) {
#ifdef DEBUG_JAVAXPCOM
      char* iid_str = aIID.ToString();
      LOG(("+ CreateJavaProxy (Java=%08x | XPCOM=%08x | IID=%s)\n",
           (PRUint32) env->CallIntMethod(java_obj, hashCodeMID),
           (PRUint32) aXPCOMObject, iid_str));
      PR_Free(iid_str);
#endif

      // Associate XPCOM object with Java proxy
      rv = gNativeToJavaProxyMap->Add(env, aXPCOMObject, aIID, java_obj);
      if (NS_SUCCEEDED(rv)) {
        *aResult = java_obj;
        return NS_OK;
      }
    } else {
      rv = NS_ERROR_FAILURE;
    }
  }

  // If there was an error, clean up.
  delete inst;
  return rv;
}

nsresult
GetXPCOMInstFromProxy(JNIEnv* env, jobject aJavaObject, void** aResult)
{
  NS_PRECONDITION(aResult != nsnull, "null ptr");
  if (!aResult)
    return NS_ERROR_NULL_POINTER;

  long xpcom_obj = env->CallStaticLongMethod(xpcomJavaProxyClass,
                                             getNativeXPCOMInstMID, aJavaObject);

  if (!xpcom_obj || env->ExceptionCheck()) {
    return NS_ERROR_FAILURE;
  }

  *aResult = NS_REINTERPRET_CAST(void*, xpcom_obj);
#ifdef DEBUG_JAVAXPCOM
  JavaXPCOMInstance* inst = NS_STATIC_CAST(JavaXPCOMInstance*, *aResult);
  nsIID* iid;
  inst->InterfaceInfo()->GetInterfaceIID(&iid);
  char* iid_str = iid->ToString();
  LOG(("< GetXPCOMInstFromProxy (Java=%08x | XPCOM=%08x | IID=%s)\n",
       (PRUint32) env->CallIntMethod(aJavaObject, hashCodeMID),
       (PRUint32) inst->GetInstance(), iid_str));
  PR_Free(iid_str);
  nsMemory::Free(iid);
#endif
  return NS_OK;
}

/**
 *  org.mozilla.xpcom.XPCOMJavaProxy.finalizeProxy
 */
extern "C" JX_EXPORT void JNICALL
JAVAPROXY_NATIVE(finalizeProxy) (JNIEnv *env, jclass that, jobject aJavaProxy)
{
#ifdef DEBUG_JAVAXPCOM
  PRUint32 xpcom_addr = 0;
#endif

  // Due to Java's garbage collection, this finalize statement may get called
  // after FreeJavaGlobals().  So check to make sure that everything is still
  // initialized.
  if (gJavaXPCOMLock) {
    nsAutoLock lock(gJavaXPCOMLock);

    // If may be possible for the lock to be acquired here when FreeGlobals is
    // in the middle of running.  If so, then this thread will sleep until
    // FreeGlobals releases its lock.  At that point, we resume this thread
    // here, but JavaXPCOM may no longer be initialized.  So we need to check
    // that everything is legit after acquiring the lock.
    if (gJavaXPCOMInitialized) {
      // Get native XPCOM instance
      void* xpcom_obj;
      nsresult rv = GetXPCOMInstFromProxy(env, aJavaProxy, &xpcom_obj);
      if (NS_SUCCEEDED(rv)) {
        JavaXPCOMInstance* inst = NS_STATIC_CAST(JavaXPCOMInstance*, xpcom_obj);
#ifdef DEBUG_JAVAXPCOM
        xpcom_addr = NS_REINTERPRET_CAST(PRUint32, inst->GetInstance());
#endif
        nsIID* iid;
        rv = inst->InterfaceInfo()->GetInterfaceIID(&iid);
        if (NS_SUCCEEDED(rv)) {
          rv = gNativeToJavaProxyMap->Remove(env, inst->GetInstance(), *iid);
          nsMemory::Free(iid);
        }
        NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to RemoveJavaProxy");
        delete inst;
      }
    }
  }

#ifdef DEBUG_JAVAXPCOM
  LOG(("- Finalize (Java=%08x | XPCOM=%08x)\n",
       (PRUint32) env->CallIntMethod(aJavaProxy, hashCodeMID), xpcom_addr));
#endif
}

