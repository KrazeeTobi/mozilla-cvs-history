/* 
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun Microsystems,
 * Inc. Portions created by Sun are
 * Copyright (C) 1999 Sun Microsystems, Inc. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
#ifndef __PlugletViewMotif_h__
#define __PlugletViewMotif_h__
#include "nsplugindefs.h"
#include "jni.h"
#include "PlugletView.h"

class PlugletViewMotif : public PlugletView {
 public:
    PlugletViewMotif(void);
    virtual jobject GetJObject(void);
    virtual PRBool SetWindow(nsPluginWindow* window);
 private:
    static  void Initialize(void);
    static  jclass clazz;
    static  jmethodID initMID;
    int     WindowID;
    jobject frame;
};
#endif /* __PlugletViewMotif_h__ */










