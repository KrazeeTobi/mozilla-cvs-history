/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/

 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is MozillaTranslator (Mozilla Localization Tool)
 *
 * The Initial Developer of the Original Code is Henrik Lynggaard Hansen
 *
 * Portions created by Henrik Lynggard Hansen are
 * Copyright (C) Henrik Lynggaard Hansen.
 * All Rights Reserved.
 *
 * Contributor(s):
 * Henrik Lynggaard Hansen (Initial Code)
 *
 */
package org.mozilla.translator.kernel;

import org.mozilla.translator.gui.*;
import org.mozilla.translator.gui.models.*;

import org.mozilla.translator.datamodel.*;
/** This class is the main class which is run by the user
 * @author Henrik Lynggaard Hansen
 * @version 4.01
 */
public class Program extends Object {

  /** The method that is run on startup
     * @param args the command line arguments
     */
  public static void main (String args[]) 
  {
   
    Settings.init("MozillaTranslator.properties");
    Log.init();
    MainWindow.init();

    if (args.length==0)
    {
        MainWindow.getDefaultInstance().setVisible(true);
    }
    
    ComplexColumn.init();
    Glossary.init();
    
    if (args.length>0)
    {
      BatchControl.runBatch(args);
    }
    
  }

}