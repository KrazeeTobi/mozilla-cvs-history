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
 * The Original Code is RaptorCanvas.
 *
 * The Initial Developer of the Original Code is Kirk Baker and
 * Ian Wilkinson. Portions created by Kirk Baker and Ian Wilkinson are
 * Copyright (C) 1999 Kirk Baker and Ian Wilkinson. All
 * Rights Reserved.
 *
 * Contributor(s):  Ed Burns <edburns@acm.org>
 *                  Ashutosh Kulkarni <ashuk@eng.sun.com>
 *      Jason Mawdsley <jason@macadamian.com>
 *      Louis-Philippe Gagnon <louisphilippe@macadamian.com>
 */

package org.mozilla.webclient.impl.wrapper_native;

import org.mozilla.util.Assert;
import org.mozilla.util.Log;
import org.mozilla.util.ParameterCheck;

import java.util.Vector;
import java.util.Enumeration;

import java.util.Stack;

import org.mozilla.webclient.BrowserControlCanvas;
import org.mozilla.webclient.DocumentLoadEvent;
import org.mozilla.webclient.DocumentLoadListener;
import org.mozilla.webclient.NewWindowEvent;
import org.mozilla.webclient.NewWindowListener;
import java.awt.event.MouseListener;
import org.mozilla.webclient.WebclientEvent;
import org.mozilla.webclient.WebclientEventListener;
import org.mozilla.webclient.UnimplementedException;

import org.mozilla.webclient.impl.WrapperFactory;

public class NativeEventThread extends Thread {

    //
    // Class variables
    //
    
    //
    // Attribute ivars
    //

    /**
     * store the exception property, set when running a Runnable causes
     * an exception.
     */

    private Exception exception;
    
    //
    // Relationship ivars
    //
    
    private WrapperFactory wrapperFactory;
    private int nativeWrapperFactory;
    
    private BrowserControlCanvas browserControlCanvas;
    
    private Stack runnablesWithNotify;
    private Stack runnables;
    
    
    //
    // Attribute ivars
    //
    
    //
    // Constructors
    //
    
    public NativeEventThread(String threadName, 
			     WrapperFactory yourFactory,
			     int yourNativeWrapperFactory) {
	super(threadName);
	ParameterCheck.nonNull(yourFactory);
	
	wrapperFactory = yourFactory;
	nativeWrapperFactory = yourNativeWrapperFactory;
	
	runnablesWithNotify = new Stack();
	runnables = new Stack();
    }
    
    /**
     *
     * This is a very delicate method, and possibly subject to race
     * condition problems.  To combat this, our first step is to set our
     * wrapperFactory to null, within a synchronized block which
     * synchronizes on the same object used in the run() method's event
     * loop.  By setting the wrapperFactory ivar to null, we cause
     * the run method to return.
     */
    
    public void delete() {
	synchronized(this) {
	    // this has to be inside the synchronized block!
	    wrapperFactory = null;
	    try {
		wait();
	    }
	    catch (Exception e) {
		System.out.println("NativeEventThread.delete: interrupted while waiting\n\t for NativeEventThread to notify() after destruction of initContext: " + e +
				   " " + e.getMessage());
	    }
	}
	wrapperFactory = null;
    }

    public Exception getAndClearException() {
	synchronized (this) {
	    Exception result = exception;
	    exception = null;
	}
	return exception;
    }

    public void setException(Exception e) {
	synchronized (this) {
	    exception = e;
	}
    }
    
    //
    // Methods from Thread
    //
    
/**

 * This method is the heart of webclient.  It is called from
 * {@link WrapperFactoryImpl#getNativeEventThread}.  It calls
 * nativeStartup, which does the per-window initialization, including
 * creating the native event queue which corresponds to this instance,
 * then enters into an infinite loop where processes native events, then
 * checks to see if there are any listeners to add, and adds them if
 * necessary.

 * @see nativeProcessEvents

 * @see nativeAddListener

 */

public void run()
{
    // our owner must have put an event in the queue 
    Assert.assert_it(!runnablesWithNotify.empty());

    //
    // Execute the event-loop.
    // 
    
    while (true) {
        try {
            Thread.sleep(1);
        }
        catch (Exception e) {
            System.out.println("NativeEventThread.run(): Exception: " + e +
                               " while sleeping: " + e.getMessage());
        }
        synchronized (this) {
	    // if we are have been told to delete ourselves
            if (null == this.wrapperFactory) {
                try {
                    notify();
                }
                catch (Exception e) {
                    System.out.println("NativeEventThread.run: Exception: trying to send notify() to this during delete: " + e + " " + e.getMessage());
                }
                return;
            }
	    
	    if (!runnables.empty()) {
		((Runnable)runnables.pop()).run();
	    }
	    if (!runnablesWithNotify.empty()) {
		((Runnable)runnablesWithNotify.pop()).run();
		synchronized(wrapperFactory) {
		    try {
			wrapperFactory.notify();
		    }
		    catch (Exception e) {
			System.out.println("NativeEventThread.run: Exception: trying to send notify() to wrapperFactory: " + e + " " + e.getMessage());
		    }
		}
	    }
	    nativeProcessEvents(nativeWrapperFactory);
        }
    }
}

//
// private methods
//

//
// Package methods
//

    void pushRunnable(Runnable toInvoke) {
	synchronized (this) {
	    runnables.push(toInvoke);
	}
    }
    
    void pushNotifyRunnable(Runnable toInvoke) {
	synchronized (this) {
	    runnablesWithNotify.push(toInvoke);
	}
    }

/**

 * This method is called from native code when an event occurrs.  This
 * method relies on the fact that all events types that the client can
 * observe descend from WebclientEventListener.  I use instanceOf to
 * determine what kind of WebclientEvent subclass to create.

 */

void nativeEventOccurred(WebclientEventListener target,
                         String targetClassName, long eventType,
                         Object eventData)
{
    ParameterCheck.nonNull(target);
    ParameterCheck.nonNull(targetClassName);

    Assert.assert_it(-1 != nativeWrapperFactory);

    WebclientEvent event = null;

    if (DocumentLoadListener.class.getName().equals(targetClassName)) {
        event = new DocumentLoadEvent(this, eventType, eventData);
    }
    else if (MouseListener.class.getName().equals(targetClassName)) {
        Assert.assert_it(target instanceof WCMouseListenerImpl);

        // We create a plain vanilla WebclientEvent, which the
        // WCMouseListenerImpl target knows how to deal with.

        // Also, the source happens to be the browserControlCanvas
        // to satisfy the java.awt.event.MouseEvent's requirement
        // that the source be a java.awt.Component subclass.

        event = new WebclientEvent(browserControlCanvas, eventType, eventData);
    }
    else if (NewWindowListener.class.getName().equals(targetClassName)) {
        event = new NewWindowEvent(this, eventType, eventData);
    }
    // else...

    // PENDING(edburns): maybe we need to put this in some sort of
    // event queue?
    target.eventDispatched(event);
}

//
// Native methods
//


/**

 * Called from java to allow the native code to process any pending
 * events, such as: painting the UI, processing mouse and key events,
 * etc.

 */

public native void nativeProcessEvents(int webShellPtr);

} // end of class NativeEventThread
