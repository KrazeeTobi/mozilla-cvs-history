/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/******

  This file contains the list of all HTML tags 
  See nsHTMLTags.h for access to the enum values for tags

  It is designed to be used as inline input to nsHTMLTags.cpp and
  nsHTMLContentSink *only* through the magic of C preprocessing.

  All entires must be enclosed in the macro HTML_TAG which will have cruel
  and unusual things done to it

  It is recommended (but not strictly necessary) to keep all entries
  in alphabetical order

  The first argument to HTML_TAG is both the enum identifier of the
  property and the string value. The second argument is the "creator"
  method of the form NS_New$TAGNAMEElement, that will be used by
  nsHTMLContentSink.cpp to create a content object for a tag of that
  type. Use NOTUSED, if the particular tag has a non-standard creator.

  The HTML_OTHER macro is for values in the nsHTMLTag enum that are
  not strictly tags.

  Entries *must* use only lowercase characters.

  ** Break these invarient and bad things will happen. **    

 ******/
HTML_TAG(a, Anchor)
HTML_TAG(abbr, Span)
HTML_TAG(acronym, Span)
HTML_TAG(address, Span)
HTML_TAG(applet, Applet)
HTML_TAG(area, Area)
HTML_TAG(b, Span)
HTML_TAG(base, Base)
HTML_TAG(basefont, BaseFont)
HTML_TAG(bdo, Span)
HTML_TAG(bgsound, Span)
HTML_TAG(big, Span)
HTML_TAG(blink, Span)
HTML_TAG(blockquote, Quote)
HTML_TAG(body, Body)
HTML_TAG(br, BR)
HTML_TAG(button, Button)
HTML_TAG(caption, TableCaption)
HTML_TAG(center, Span)
HTML_TAG(cite, Span)
HTML_TAG(code, Span)
HTML_TAG(col, TableCol)
HTML_TAG(colgroup, TableColGroup)
HTML_TAG(counter, Span)
HTML_TAG(dd, Span)
HTML_TAG(del, Del)
HTML_TAG(dfn, Span)
HTML_TAG(dir, Directory)
HTML_TAG(div, Div)
HTML_TAG(dl, DList)
HTML_TAG(dt, Span)
HTML_TAG(em, Span)
HTML_TAG(embed, Embed)
HTML_TAG(endnote, Span)
HTML_TAG(fieldset, FieldSet)
HTML_TAG(font, Font)
HTML_TAG(form, NOTUSED)
HTML_TAG(frame, Frame)
HTML_TAG(frameset, FrameSet)
HTML_TAG(h1, Heading)
HTML_TAG(h2, Heading)
HTML_TAG(h3, Heading)
HTML_TAG(h4, Heading)
HTML_TAG(h5, Heading)
HTML_TAG(h6, Heading)
HTML_TAG(head, Head)
HTML_TAG(hr, HR)
HTML_TAG(html, Html)
HTML_TAG(i, Span)
HTML_TAG(iframe, IFrame)
HTML_TAG(image, Span)
HTML_TAG(img, Image)
HTML_TAG(input, NOTUSED)
HTML_TAG(ins, Ins)
HTML_TAG(isindex, IsIndex)
HTML_TAG(kbd, Span)
HTML_TAG(keygen, Span)
HTML_TAG(label, Label)
HTML_TAG(legend, Legend)
HTML_TAG(li, LI)
HTML_TAG(link, Link)
HTML_TAG(listing, Span)
HTML_TAG(map, Map)
HTML_TAG(marquee, Div)
HTML_TAG(menu, Menu)
HTML_TAG(meta, Meta)
HTML_TAG(multicol, Span)
HTML_TAG(nobr, Span)
HTML_TAG(noembed, Div)
HTML_TAG(noframes, Div)
HTML_TAG(noscript, Div)
HTML_TAG(object, Object)
HTML_TAG(ol, OList)
HTML_TAG(optgroup, OptGroup)
HTML_TAG(option, Option)
HTML_TAG(p, Paragraph)
HTML_TAG(param, Param)
HTML_TAG(parsererror, Div)
HTML_TAG(plaintext, Span)
HTML_TAG(pre, Pre)
HTML_TAG(q, Quote)
HTML_TAG(s, Span)
HTML_TAG(samp, Span)
HTML_TAG(script, Script)
HTML_TAG(select, NOTUSED)
HTML_TAG(server, Span)
HTML_TAG(small, Span)
HTML_TAG(sound, Span)
HTML_TAG(sourcetext, Div)
HTML_TAG(spacer, Spacer)
HTML_TAG(span, Span)
HTML_TAG(strike, Span)
HTML_TAG(strong, Span)
HTML_TAG(style, Style)
HTML_TAG(sub, Span)
HTML_TAG(sup, Span)
HTML_TAG(table, Table)
HTML_TAG(tbody, TableSection)
HTML_TAG(td, TableCell)
HTML_TAG(textarea, TextArea)
HTML_TAG(tfoot, TableSection)
HTML_TAG(th, TableCell)
HTML_TAG(thead, TableSection)
HTML_TAG(title, Title)
HTML_TAG(tr, TableRow)
HTML_TAG(tt, Span)
HTML_TAG(u, Span)
HTML_TAG(ul, UList)
HTML_TAG(var, Span)
HTML_TAG(wbr, WBR)
HTML_TAG(xmp, Span)


/* These are not for tags. But they will be included in the nsHTMLTag
   enum anyway */

/* XXX: The second parameters in some of the following entries look
   like they are just wrong. They should really be NOTUSED. For now,
   I'm just emulating what nsHTMLContentSink has done all along.
*/
HTML_OTHER(text, Span)
HTML_OTHER(whitespace, Span)
HTML_OTHER(newline, Span)
HTML_OTHER(comment, Span)
HTML_OTHER(entity, Span)
HTML_OTHER(doctypeDecl, Span)
HTML_OTHER(markupDecl, Span)
HTML_OTHER(instruction, Span)
HTML_OTHER(userdefined, Unknown)
