/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Josh Aas (josh@mozilla.com)
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

#import "StatusBarView.h"

#import "NSWorkspace+Utils.h"

@implementation StatusBarView

//
// -drawRect:
//
// Override to draw the top header of the status bar
//
- (void)drawRect:(NSRect)aRect
{
  [super drawRect:aRect];
  
  if (![NSWorkspace isLeopardOrHigher]) {
    // optimize drawing a bit so we're not *always* redrawing our top header. Only
    // draw if the area we're asked to draw overlaps with the top line.
    NSRect bounds = [self bounds];
    if (NSMaxY(bounds) <= NSMaxY(aRect)) {
      NSPoint leftPoint = NSMakePoint(bounds.origin.x, bounds.origin.y + bounds.size.height);
      NSPoint rightPoint = NSMakePoint(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height);
      [NSBezierPath strokeLineFromPoint:leftPoint toPoint:rightPoint];
    }
  }
}

@end
