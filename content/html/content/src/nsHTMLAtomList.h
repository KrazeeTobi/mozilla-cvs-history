/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/******

  This file contains the list of all HTML atoms 
  See nsHTMLAtoms.h for access to the atoms

  It is designed to be used as inline input to nsHTMLAtoms.cpp *only*
  through the magic of C preprocessing.

  All entires must be enclosed in the macro HTML_ATOM which will have cruel
  and unusual things done to it

  The first argument to HTML_ATOM is the C++ name of the atom
  The second argument it HTML_ATOM is the string value of the atom

 ******/

HTML_ATOM(mozAnonymousBlock, ":-moz-anonymous-block")
HTML_ATOM(mozAnonymousPositionedBlock, ":-moz-anonymous-positioned-block")
HTML_ATOM(mozFirstLineFixup, ":-moz-first-line-fixup")
HTML_ATOM(mozLetterFrame, ":-moz-letter-frame")
HTML_ATOM(mozLineFrame, ":-moz-line-frame")
HTML_ATOM(mozListBulletPseudo, ":-moz-list-bullet")
HTML_ATOM(mozSingleLineTextControlFrame, ":-moz-singleline-textcontrol-frame")
HTML_ATOM(mozFocusInnerPseudo, ":-moz-focus-inner")
HTML_ATOM(mozFocusOuterPseudo, ":-moz-focus-outer")
HTML_ATOM(mozDisplayComboboxControlFrame, ":-moz-display-comboboxcontrol-frame")

HTML_ATOM(_baseHref, NS_HTML_BASE_HREF)
HTML_ATOM(_baseTarget, NS_HTML_BASE_TARGET)
HTML_ATOM(a, "a")
HTML_ATOM(abbr, "abbr")
HTML_ATOM(above, "above")
HTML_ATOM(accept, "accept")
HTML_ATOM(acceptcharset, "accept-charset")
HTML_ATOM(accesskey, "accesskey")
HTML_ATOM(action, "action")
HTML_ATOM(align, "align")
HTML_ATOM(alink, "alink")
HTML_ATOM(alt, "alt")
HTML_ATOM(applet, "applet")
HTML_ATOM(archive, "archive")
HTML_ATOM(area, "area")
HTML_ATOM(axis, "axis")
HTML_ATOM(background, "background")
HTML_ATOM(below, "below")
HTML_ATOM(bgcolor, "bgcolor")
HTML_ATOM(blockquote, "blockquote")
HTML_ATOM(body, "body")
HTML_ATOM(border, "border")
HTML_ATOM(bordercolor, "bordercolor")
HTML_ATOM(bottompadding, "bottompadding")
HTML_ATOM(br, "br")
HTML_ATOM(button, "button")
HTML_ATOM(buttonContentPseudo, ":button-content")
HTML_ATOM(caption, "caption")
HTML_ATOM(cellContentPseudo, ":cell-content")
HTML_ATOM(cellpadding, "cellpadding")
HTML_ATOM(cellspacing, "cellspacing")
HTML_ATOM(ch, "ch")
HTML_ATOM(_char, "char")
HTML_ATOM(charoff, "charoff")
HTML_ATOM(charset, "charset")
HTML_ATOM(checked, "checked")
HTML_ATOM(choff, "choff")
HTML_ATOM(cite, "cite")
HTML_ATOM(kClass, "class")
HTML_ATOM(classid, "classid")
HTML_ATOM(clear, "clear")
HTML_ATOM(clip, "clip")
HTML_ATOM(code, "code")
HTML_ATOM(codebase, "codebase")
HTML_ATOM(codetype, "codetype")
HTML_ATOM(color, "color")
HTML_ATOM(col, "col")
HTML_ATOM(colgroup, "colgroup")
HTML_ATOM(cols, "cols")
HTML_ATOM(colspan, "colspan")
HTML_ATOM(combobox, "combobox")
HTML_ATOM(columnPseudo, ":body-column")
HTML_ATOM(commentPseudo, ":-moz-comment")
HTML_ATOM(compact, "compact")
HTML_ATOM(content, "content")
HTML_ATOM(coords, "coords")
HTML_ATOM(dd, "dd")
HTML_ATOM(defaultchecked, "defaultchecked")
HTML_ATOM(defaultselected, "defaultselected")
HTML_ATOM(defaultvalue, "defaultvalue")
HTML_ATOM(declare, "declare")
HTML_ATOM(defer, "defer")
HTML_ATOM(dir, "dir")
HTML_ATOM(div, "div")
HTML_ATOM(disabled, "disabled")
HTML_ATOM(dl, "dl")
HTML_ATOM(dropDownListPseudo, ":-moz-dropdown-list")
HTML_ATOM(dt, "dt")
   
HTML_ATOM(datetime, "datetime")
HTML_ATOM(data, "data")
HTML_ATOM(embed, "embed")
HTML_ATOM(encoding, "encoding")
HTML_ATOM(enctype, "enctype")
HTML_ATOM(face, "face")
HTML_ATOM(fieldset, "fieldset")
HTML_ATOM(fieldsetContentPseudo, ":fieldset-content")
HTML_ATOM(firstLetterPseudo, ":first-letter")
HTML_ATOM(firstLinePseudo, ":first-line")
HTML_ATOM(font, "font")
HTML_ATOM(fontWeight, "font-weight")
HTML_ATOM(_for, "for")
HTML_ATOM(form, "form")
HTML_ATOM(frame, "frame")
HTML_ATOM(frameborder, "frameborder")
HTML_ATOM(frameset, "frameset")
HTML_ATOM(framesetBlankPseudo, ":frameset-blank")
HTML_ATOM(gutter, "gutter")
HTML_ATOM(h1, "h1")
HTML_ATOM(h2, "h2")
HTML_ATOM(h3, "h3")
HTML_ATOM(h4, "h4")
HTML_ATOM(h5, "h5")
HTML_ATOM(h6, "h6")
HTML_ATOM(head, "head")
HTML_ATOM(headerContentBase, "content-base")
HTML_ATOM(headerContentLanguage, "content-language")
HTML_ATOM(headerContentScriptType, "content-script-type")
HTML_ATOM(headerContentStyleType, "content-style-type")
HTML_ATOM(headerContentType, "content-type")
HTML_ATOM(headerDefaultStyle, "default-style")
HTML_ATOM(headerWindowTarget, "window-target")
HTML_ATOM(headers, "headers")
HTML_ATOM(height, "height")
HTML_ATOM(hidden, "hidden")
HTML_ATOM(horizontalFramesetBorderPseudo, ":hframeset-border")
HTML_ATOM(hr, "hr")
HTML_ATOM(href, "href")
HTML_ATOM(hreflang, "hreflang")
HTML_ATOM(hspace, "hspace")
HTML_ATOM(html, "html")
HTML_ATOM(httpEquiv, "http-equiv")
HTML_ATOM(ibPseudo, ":ib-pseudo")
HTML_ATOM(id, "id")
HTML_ATOM(iframe, "iframe")
HTML_ATOM(ilayer, "ilayer")
HTML_ATOM(img, "img")
HTML_ATOM(index, "index")
HTML_ATOM(input, "input")
HTML_ATOM(isindex, "isindex")
HTML_ATOM(ismap, "ismap")
HTML_ATOM(label, "label")
HTML_ATOM(labelContentPseudo, ":label-content")
HTML_ATOM(lang, "lang")
HTML_ATOM(layer, "layer")
HTML_ATOM(layout, "layout")
HTML_ATOM(li, "li")
HTML_ATOM(link, "link")
HTML_ATOM(left, "left")
HTML_ATOM(leftpadding, "leftpadding")
HTML_ATOM(legend, "legend")
HTML_ATOM(legendContentPseudo, ":legend-content")
HTML_ATOM(length, "length")
HTML_ATOM(longdesc, "longdesc")
HTML_ATOM(lowsrc, "lowsrc")
HTML_ATOM(marginheight, "marginheight")
HTML_ATOM(marginwidth, "marginwidth")
HTML_ATOM(maxlength, "maxlength")
HTML_ATOM(mayscript, "mayscript")
HTML_ATOM(media, "media")
HTML_ATOM(menu, "menu")
HTML_ATOM(meta, "meta")
HTML_ATOM(method, "method")
HTML_ATOM(multicol, "multicol")
HTML_ATOM(multiple, "multiple")
HTML_ATOM(name, "name")
HTML_ATOM(nohref, "nohref")
HTML_ATOM(noresize, "noresize")
HTML_ATOM(noshade, "noshade")
HTML_ATOM(nowrap, "nowrap")
HTML_ATOM(object, "object")
HTML_ATOM(ol, "ol")
HTML_ATOM(option, "option")
HTML_ATOM(overflow, "overflow")
HTML_ATOM(p, "p")
HTML_ATOM(pagex, "pagex")
HTML_ATOM(pagey, "pagey")
HTML_ATOM(param, "param")
HTML_ATOM(placeholderPseudo, ":placeholder-frame")
HTML_ATOM(pointSize, "point-size")
HTML_ATOM(pre, "pre")
HTML_ATOM(processingInstructionPseudo, ":-moz-pi")
HTML_ATOM(profile, "profile")
HTML_ATOM(prompt, "prompt")
HTML_ATOM(radioPseudo, ":-moz-radio")
HTML_ATOM(checkPseudo, ":-moz-checkbox")
HTML_ATOM(readonly, "readonly")
HTML_ATOM(rel, "rel")
HTML_ATOM(repeat, "repeat")
HTML_ATOM(rev, "rev")
HTML_ATOM(rightpadding, "rightpadding")
HTML_ATOM(rows, "rows")
HTML_ATOM(rowspan, "rowspan")
HTML_ATOM(rules, "rules")
HTML_ATOM(scheme, "scheme")
HTML_ATOM(scope, "scope")
HTML_ATOM(script, "script")
HTML_ATOM(scrolling, "scrolling")
HTML_ATOM(select, "select")
HTML_ATOM(selected, "selected")
HTML_ATOM(selectedindex, "selectedindex")
HTML_ATOM(shape, "shape")
HTML_ATOM(size, "size")
HTML_ATOM(spacer, "spacer")
HTML_ATOM(span, "span")
HTML_ATOM(src, "src")
HTML_ATOM(standby, "standby")
HTML_ATOM(start, "start")
HTML_ATOM(style, "style")
HTML_ATOM(summary, "summary")
HTML_ATOM(suppress, "suppress")
HTML_ATOM(tabindex, "tabindex")
HTML_ATOM(table, "table")
HTML_ATOM(tablePseudo, ":table")
HTML_ATOM(tableCellPseudo, ":table-cell")
HTML_ATOM(tableColGroupPseudo, ":table-column-group")
HTML_ATOM(tableColPseudo, ":table-column")
HTML_ATOM(tableOuterPseudo, ":table-outer")
HTML_ATOM(tableRowGroupPseudo, ":table-row-group")
HTML_ATOM(tableRowPseudo, ":table-row")
HTML_ATOM(tabstop, "tabstop")
HTML_ATOM(target, "target")
HTML_ATOM(tbody, "tbody")
HTML_ATOM(td, "td")
HTML_ATOM(tfoot, "tfoot")
HTML_ATOM(thead, "thead")
HTML_ATOM(text, "text")
HTML_ATOM(textarea, "textarea")
HTML_ATOM(textPseudo, ":-moz-text")
HTML_ATOM(th, "th")
HTML_ATOM(title, "title")
HTML_ATOM(top, "top")
HTML_ATOM(toppadding, "toppadding")
HTML_ATOM(tr, "tr")
HTML_ATOM(type, "type")
HTML_ATOM(ul, "ul")
HTML_ATOM(usemap, "usemap")
HTML_ATOM(valign, "valign")
HTML_ATOM(value, "value")
HTML_ATOM(valuetype, "valuetype")
HTML_ATOM(variable, "variable")
HTML_ATOM(vcard_name, "vcard_name")
HTML_ATOM(version, "version")
HTML_ATOM(verticalFramesetBorderPseudo, ":vframeset-border")
HTML_ATOM(visibility, "visibility")
HTML_ATOM(vlink, "vlink")
HTML_ATOM(vspace, "vspace")
HTML_ATOM(wbr, "wbr")
HTML_ATOM(width, "width")
HTML_ATOM(wrap, "wrap")
HTML_ATOM(wrappedFramePseudo, ":wrapped-frame")
HTML_ATOM(zindex, "zindex")
HTML_ATOM(z_index, "z-index")

HTML_ATOM(moz_tristate, "moz-tristate")
HTML_ATOM(moz_tristatevalue, "moz-tristatevalue")

#ifdef DEBUG
HTML_ATOM(iform, "IForm")
HTML_ATOM(form_control_list, "FormControlList")
#endif
