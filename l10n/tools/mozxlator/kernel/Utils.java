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

import java.awt.*;
/** 
 *
 * @author  Henrik Lynggaard Hansen
 * @version 4.0
 */
public class Utils extends Object {

  public static void placeFrameAtCenter(Component frame)
  {
    int x;
    int y;
    
    Dimension scr = Toolkit.getDefaultToolkit().getScreenSize();
    Dimension frm = frame.getSize();
    
    x = (int) ( (scr.getWidth()/2) - (frm.getWidth()/2) );
    y = (int) ( (scr.getHeight()/2) - (frm.getHeight()/2) );
    
    frame.setLocation(x,y);
  }
  
}