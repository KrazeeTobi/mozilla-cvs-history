/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * 
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
 * The Original Code is RaptorCanvas.
 *
 * The Initial Developer of the Original Code is Kirk Baker and
 * Ian Wilkinson. Portions created by Kirk Baker and Ian Wilkinson are
 * Copyright (C) 1999 Kirk Baker and Ian Wilkinson. All
 * Rights Reserved.
 *
 * Contributor(s):  Ashutosh Kulkarni <ashuk@eng.sun.com>
 *                  Ed Burns <edburns@acm.org>
 *
 */


package org.mozilla.webclient.wrapper_nonnative;

import org.mozilla.webclient.BrowserType;
import org.mozilla.webclient.WrapperFactory;
import org.mozilla.webclient.BrowserControl;

class BrowserTypeImpl extends BrowserType
{

public Object ICEbase = null;
public Object viewPort = null;

//
// Constructors and Initializers    
//

public BrowserTypeImpl(WrapperFactory yourFactory, 
                       BrowserControl yourBrowserControl)
{
    super(yourFactory, yourBrowserControl);
}

public void setICEProps(Object base, Object viewport)
{
    ICEbase = base;
    viewPort = viewport;
}

public Object getStormBase()
{
    return ICEbase;
}

public Object getViewPort()
{
    return viewPort;
}


public static void main(String [] args)
{
 //Unit Tests need to go here
}

} // end of class BrowserTypeImpl
