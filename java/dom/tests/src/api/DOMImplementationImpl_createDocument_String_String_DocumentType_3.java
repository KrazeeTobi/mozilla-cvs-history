
/*
 The contents of this file are subject to the Mozilla Public
 License Version 1.1 (the "License"); you may not use this file
 except in compliance with the License. You may obtain a copy of
 the License at http://www.mozilla.org/MPL/

 Software distributed under the License is distributed on an "AS
 IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 implied. See the License for the specific language governing
 rights and limitations under the License.

 The Original Code is mozilla.org code.

 The Initial Developer of the Original Code is Sun Microsystems,
 Inc. Portions created by Sun are
 Copyright (C) 1999 Sun Microsystems, Inc. All
 Rights Reserved.

 Contributor(s):
*/

package org.mozilla.dom.test;

import java.util.*;
import java.io.*;
import org.mozilla.dom.test.*;
import org.mozilla.dom.*;
import org.w3c.dom.*;

public class DOMImplementationImpl_createDocument_String_String_DocumentType_3 extends BWBaseTest implements Execution
{

   /**
    *
    ***********************************************************
    *  Constructor
    ***********************************************************
    *
    */
   public DOMImplementationImpl_createDocument_String_String_DocumentType_3()
   {
   }


   /**
    *
    ***********************************************************
    *  Starting point of application
    *
    *  @param   args    Array of command line arguments
    *
    ***********************************************************
    */
   public static void main(String[] args)
   {
   }

   /**
    ***********************************************************
    *
    *  Execute Method 
    *
    *  @param   tobj    Object reference (Node/Document/...)
    *  @return          true or false  depending on whether test passed or failed.
    *
    ***********************************************************
    */
   public boolean execute(Object tobj)
   {
      if (tobj == null)  {
           TestLoader.logErrPrint("Object is NULL...");
           return BWBaseTest.FAILED;
      }

      String os = System.getProperty("OS");
      osRoutine(os);

      Document d = (Document)tobj;
        if (d != null)
        {
             DOMImplementationImpl di = (DOMImplementationImpl)d.getImplementation();
	     if (di == null) {
                TestLoader.logErrPrint("Document DomImplementation is  NULL..");
                return BWBaseTest.FAILED;
             } else {
               try {
		 DocumentType dt = di.createDocumentType("edi:price", "publicID", "systemID");
		if (dt == null) {
	                TestLoader.logErrPrint("DocumentType is  NULL..");
        	        return BWBaseTest.FAILED;
		 }
		 String namespaceURI = "edi='http://ecommerce.org/schema'";
		 String qualifiedName = "edi:price";
                 if (di.createDocument(null, qualifiedName, dt) == null) {
                    System.out.println("DomImplementation 'createDocument' returns null ...");
		    TestLoader.logErrPrint("DomImplementation 'createDocument' returns null ...");
                    return BWBaseTest.FAILED;
                  } else {
		     TestLoader.logErrPrint("createDocumetn didn't throw exception ...");
                     return BWBaseTest.FAILED;
		}
               } catch (DOMException de) {
                     TestLoader.logErrPrint("DOMException : " + de);
                     return BWBaseTest.PASSED;
               } catch (Exception e) {
 		     TestLoader.logErrPrint("Exception: "+e);
                     return BWBaseTest.PASSED; 
		}
             }
        } else {
             System.out.println("Document is  NULL..");
             return BWBaseTest.FAILED;
        }
   }

   /**
    *
    ***********************************************************
    *  Routine where OS specific checks are made. 
    *
    *  @param   os      OS Name (SunOS/Linus/MacOS/...)
    ***********************************************************
    *
    */
   private void osRoutine(String os)
   {
     if(os == null) return;

     os = os.trim();
     if(os.compareTo("SunOS") == 0) {}
     else if(os.compareTo("Linux") == 0) {}
     else if(os.compareTo("Windows") == 0) {}
     else if(os.compareTo("MacOS") == 0) {}
     else {}
   }
}
