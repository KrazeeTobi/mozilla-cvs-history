/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */
#include "nsIHTMLContentSink.h"
#include "nsIStyleSheet.h"
#include "nsIUnicharInputStream.h"
#include "nsIHTMLContent.h"
#include "nsIURL.h"
#include "nsIHttpUrl.h"
#include "nsHTMLDocument.h"
#include "nsIPresShell.h"
#include "nsIPresContext.h"
#include "nsIViewManager.h"
#include "nsHTMLTokens.h" 
#include "nsHTMLEntities.h" 
#include "nsCRT.h"
#include "prtime.h"
#include "prlog.h"

#include "nsHTMLParts.h"
#include "nsITextContent.h"

#include "nsIDOMText.h"

#include "nsIDOMHTMLFormElement.h"
#include "nsIDOMHTMLTextAreaElement.h"
#include "nsIDOMHTMLOptionElement.h"
#include "nsIFormControl.h"
#include "nsIImageMap.h"

#include "nsRepository.h"

#include "nsIScrollableView.h"
#include "nsHTMLAtoms.h"
#include "nsIFrame.h"

#include "nsIWebShell.h"
#include "nsIHTMLDocument.h"

// XXX Go through a factory for this one
#include "nsICSSParser.h"

#include "nsIDOMHTMLTitleElement.h"

static NS_DEFINE_IID(kIDOMHTMLTitleElementIID, NS_IDOMHTMLTITLEELEMENT_IID);

#define XXX_ART_HACK 1

static NS_DEFINE_IID(kIContentIID, NS_ICONTENT_IID);
static NS_DEFINE_IID(kIHTMLContentIID, NS_IHTMLCONTENT_IID);
static NS_DEFINE_IID(kIDOMTextIID, NS_IDOMTEXT_IID);
static NS_DEFINE_IID(kIDOMHTMLFormElementIID, NS_IDOMHTMLFORMELEMENT_IID);
static NS_DEFINE_IID(kIDOMHTMLTextAreaElementIID, NS_IDOMHTMLTEXTAREAELEMENT_IID);
static NS_DEFINE_IID(kIDOMHTMLOptionElementIID, NS_IDOMHTMLOPTIONELEMENT_IID);
static NS_DEFINE_IID(kIFormControlIID, NS_IFORMCONTROL_IID);
static NS_DEFINE_IID(kIHTMLContentSinkIID, NS_IHTML_CONTENT_SINK_IID);
static NS_DEFINE_IID(kIHTTPUrlIID, NS_IHTTPURL_IID);
static NS_DEFINE_IID(kIScrollableViewIID, NS_ISCROLLABLEVIEW_IID);
static NS_DEFINE_IID(kIHTMLDocumentIID, NS_IHTMLDOCUMENT_IID);

//----------------------------------------------------------------------

#ifdef NS_DEBUG
static PRLogModuleInfo* gSinkLogModuleInfo;

#define SINK_TRACE_CALLS        0x1
#define SINK_TRACE_REFLOW       0x2

#define SINK_LOG_TEST(_lm,_bit) (PRIntn((_lm)->level) & (_bit))

#define SINK_TRACE(_bit,_args)                    \
  PR_BEGIN_MACRO                                  \
    if (SINK_LOG_TEST(gSinkLogModuleInfo,_bit)) { \
      PR_LogPrint _args;                          \
    }                                             \
  PR_END_MACRO

#define SINK_TRACE_NODE(_bit,_msg,_node) SinkTraceNode(_bit,_msg,_node,this)

static void
SinkTraceNode(PRUint32 aBit,
              const char* aMsg,
              const nsIParserNode& aNode,
              void* aThis)
{
  if (SINK_LOG_TEST(gSinkLogModuleInfo,aBit)) {
    char cbuf[40];
    const char* cp;
    PRInt32 nt = aNode.GetNodeType();
    if ((nt > PRInt32(eHTMLTag_unknown)) &&
        (nt < PRInt32(eHTMLTag_text))) {
      cp = NS_EnumToTag(nsHTMLTag(aNode.GetNodeType()));
    } else {
      aNode.GetText().ToCString(cbuf, sizeof(cbuf));
      cp = cbuf;
    }
    PR_LogPrint("%s: this=%p node='%s'", aMsg, aThis, cp);
  }
}

#else
#define SINK_TRACE(_bit,_args)
#define SINK_TRACE_NODE(_bit,_msg,_node)
#endif

//----------------------------------------------------------------------

class SinkContext;

class HTMLContentSink : public nsIHTMLContentSink {
public:
  void* operator new(size_t size) {
    void* rv = ::operator new(size);
    nsCRT::zero(rv, size);
    return (void*) rv;
  }

  HTMLContentSink();
  ~HTMLContentSink();

  nsresult Init(nsIDocument* aDoc,
                nsIURL* aURL,
                nsIWebShell* aContainer);

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIContentSink
  NS_IMETHOD WillBuildModel(void);
  NS_IMETHOD DidBuildModel(PRInt32 aQualityLevel);
  NS_IMETHOD WillInterrupt(void);
  NS_IMETHOD WillResume(void);

  // nsIHTMLContentSink
  NS_IMETHOD BeginContext(PRInt32 aID);
  NS_IMETHOD EndContext(PRInt32 aID);
  NS_IMETHOD SetContext(PRInt32 aID);
  NS_IMETHOD SetTitle(const nsString& aValue);
  NS_IMETHOD OpenHTML(const nsIParserNode& aNode);
  NS_IMETHOD CloseHTML(const nsIParserNode& aNode);
  NS_IMETHOD OpenHead(const nsIParserNode& aNode);
  NS_IMETHOD CloseHead(const nsIParserNode& aNode);
  NS_IMETHOD OpenBody(const nsIParserNode& aNode);
  NS_IMETHOD CloseBody(const nsIParserNode& aNode);
  NS_IMETHOD OpenForm(const nsIParserNode& aNode);
  NS_IMETHOD CloseForm(const nsIParserNode& aNode);
  NS_IMETHOD OpenFrameset(const nsIParserNode& aNode);
  NS_IMETHOD CloseFrameset(const nsIParserNode& aNode);
  NS_IMETHOD OpenMap(const nsIParserNode& aNode);
  NS_IMETHOD CloseMap(const nsIParserNode& aNode);
  NS_IMETHOD OpenContainer(const nsIParserNode& aNode);
  NS_IMETHOD CloseContainer(const nsIParserNode& aNode);
  NS_IMETHOD AddLeaf(const nsIParserNode& aNode);

  nsIDocument* mDocument;
  nsIScriptObjectOwner* mDocumentScript;
  nsIURL* mDocumentURL;
  nsIWebShell* mWebShell;

  nsIHTMLContent* mRoot;
  nsIHTMLContent* mBody;
  PRInt32         mBodyChildCount;
  nsIHTMLContent* mFrameset;
  nsIHTMLContent* mHead;
  nsString* mTitle;

  PRInt32 mInMonolithicContainer;
  PRBool mDirty;
  PRBool mLayoutStarted;
  nsIDOMHTMLFormElement* mCurrentForm;
  nsIImageMap* mCurrentMap;

  SinkContext** mContexts;
  PRInt32 mNumContexts;
  nsVoidArray mContextStack;
  SinkContext* mCurrentContext;
  SinkContext* mHeadContext;

  nsString* mRef;
  nsScrollPreference mOriginalScrollPreference;
  PRBool mNotAtRef;
  nsIHTMLContent* mRefContent;

  nsString mBaseHREF;
  nsString mBaseTarget;

  nsIStyleSheet* mStyleSheet;

  void StartLayout();

  void ScrollToRef();

  void AddBaseTagInfo(nsIHTMLContent* aContent);

  nsresult LoadStyleSheet(nsIURL* aURL,
                          nsIUnicharInputStream* aUIN,
                          PRBool aInline);

  // Routines for tags that require special handling
  nsresult ProcessATag(const nsIParserNode& aNode, nsIHTMLContent* aContent);
  nsresult ProcessAREATag(const nsIParserNode& aNode);
  nsresult ProcessBASETag(const nsIParserNode& aNode);
  nsresult ProcessLINKTag(const nsIParserNode& aNode);
  nsresult ProcessMETATag(const nsIParserNode& aNode);
  nsresult ProcessSCRIPTTag(const nsIParserNode& aNode);
  nsresult ProcessSTYLETag(const nsIParserNode& aNode);
};

class SinkContext {
public:
  SinkContext(HTMLContentSink* aSink);
  ~SinkContext();

  // Normally when OpenContainer's are done the container is not
  // appended to it's parent until the container is closed. By setting
  // pre-append to true, the container will be appended when it is
  // created.
  void SetPreAppend(PRBool aPreAppend) {
    mPreAppend = aPreAppend;
  }

  nsresult Begin(nsHTMLTag aNodeType, nsIHTMLContent* aRoot);
  nsresult OpenContainer(const nsIParserNode& aNode);
  nsresult CloseContainer(const nsIParserNode& aNode);
  nsresult AddLeaf(const nsIParserNode& aNode);
  nsresult End();

  nsresult GrowStack();
  nsresult AddText(const nsString& aText);
  nsresult FlushText(PRBool* aDidFlush = nsnull);

  void MaybeMarkSinkDirty();

  HTMLContentSink* mSink;
  PRBool mPreAppend;

  struct Node {
    nsHTMLTag mType;
    nsIHTMLContent* mContent;
    PRUint32 mFlags;
  };

// Node.mFlags
#define APPENDED 0x1

  Node* mStack;
  PRInt32 mStackSize;
  PRInt32 mStackPos;

  PRUnichar* mText;
  PRInt32 mTextLength;
  PRInt32 mTextSize;

};

//----------------------------------------------------------------------

// Temporary factory code to create content objects

static void
GetAttributeValueAt(const nsIParserNode& aNode,
                    PRInt32 aIndex,
                    nsString& aResult,
                    nsIScriptContextOwner* aScriptContextOwner)
{
  // Copy value
  const nsString& value = aNode.GetValueAt(aIndex);
  aResult.Truncate();
  aResult.Append(value);

  // Strip quotes if present
  PRUnichar first = aResult.First();
  if ((first == '"') || (first == '\'')) {
    if (aResult.Last() == first) {
      aResult.Cut(0, 1);
      PRInt32 pos = aResult.Length() - 1;
      if (pos >= 0) {
        aResult.Cut(pos, 1);
      }
    } else {
      // Mismatched quotes - leave them in
    }
  }

  // Reduce any entities
  // XXX Note: as coded today, this will only convert well formed
  // entities.  This may not be compatible enough.
  // XXX there is a table in navigator that translates some numeric entities
  // should we be doing that? If so then it needs to live in two places (bad)
  // so we should add a translate numeric entity method from the parser...
  char cbuf[100];
  PRInt32 index = 0;
  while (index < aResult.Length()) {
    // If we have the start of an entity (and it's not at the end of
    // our string) then translate the entity into it's unicode value.
    if ((aResult.CharAt(index++) == '&') && (index < aResult.Length())) {
      PRInt32 start = index - 1;
      PRUnichar e = aResult.CharAt(index);
      if (e == '#') {
        // Convert a numeric character reference
        index++;
        char* cp = cbuf;
        char* limit = cp + sizeof(cbuf) - 1;
        PRBool ok = PR_FALSE;
        PRInt32 slen = aResult.Length();
        while ((index < slen) && (cp < limit)) {
          PRUnichar e = aResult.CharAt(index);
          if (e == ';') {
            index++;
            ok = PR_TRUE;
            break;
          }
          if ((e >= '0') && (e <= '9')) {
            *cp++ = char(e);
            index++;
            continue;
          }
          break;
        }
        if (!ok || (cp == cbuf)) {
          continue;
        }
        *cp = '\0';
        if (cp - cbuf > 5) {
          continue;
        }
        PRInt32 ch = PRInt32( ::atoi(cbuf) );
        if (ch > 65535) {
          continue;
        }

        // Remove entity from string and replace it with the integer
        // value.
        aResult.Cut(start, index - start);
        aResult.Insert(PRUnichar(ch), start);
        index = start + 1;
      }
      else if (((e >= 'A') && (e <= 'Z')) ||
               ((e >= 'a') && (e <= 'z'))) {
        // Convert a named entity
        index++;
        char* cp = cbuf;
        char* limit = cp + sizeof(cbuf) - 1;
        *cp++ = char(e);
        PRBool ok = PR_FALSE;
        PRInt32 slen = aResult.Length();
        while ((index < slen) && (cp < limit)) {
          PRUnichar e = aResult.CharAt(index);
          if (e == ';') {
            index++;
            ok = PR_TRUE;
            break;
          }
          if (((e >= '0') && (e <= '9')) ||
              ((e >= 'A') && (e <= 'Z')) ||
              ((e >= 'a') && (e <= 'z'))) {
            *cp++ = char(e);
            index++;
            continue;
          }
          break;
        }
        if (!ok || (cp == cbuf)) {
          continue;
        }
        *cp = '\0';
        PRInt32 ch = NS_EntityToUnicode(cbuf);
        if (ch < 0) {
          continue;
        }

        // Remove entity from string and replace it with the integer
        // value.
        aResult.Cut(start, index - start);
        aResult.Insert(PRUnichar(ch), start);
        index = start + 1;
      }
      else if (e == '{') {
        // Convert a script entity
        // XXX write me!
      }
    }
  }
}

static PRBool
FindAttribute(const nsIParserNode& aNode,
              const nsString& aKeyName,
              nsString& aResult)
{
  PRInt32 ac = aNode.GetAttributeCount();
  for (PRInt32 i = 0; i < ac; i++) {
    const nsString& key = aNode.GetKeyAt(i);
    if (key.EqualsIgnoreCase(aKeyName)) {
      // Get value and remove mandatory quotes.

      // NOTE: we do <b>not</b> evaluate any script entities here
      // because to do so would result in double evaluations since
      // attribute values found here are not stored. The unfortunate
      // implication is that these attributes cannot support script
      // entities.
      GetAttributeValueAt(aNode, i, aResult, nsnull);

      return PR_TRUE;
    }
  }
  return PR_FALSE;
}

static nsresult
AddAttributes(const nsIParserNode& aNode,
              nsIHTMLContent* aContent,
              nsIScriptContextOwner* aScriptContextOwner)
{
  // Add tag attributes to the content attributes
  nsAutoString k, v;
  PRInt32 ac = aNode.GetAttributeCount();
  for (PRInt32 i = 0; i < ac; i++) {
    // Get upper-cased key
    const nsString& key = aNode.GetKeyAt(i);
    k.Truncate();
    k.Append(key);
    k.ToUpperCase();

    nsIAtom*  keyAtom = NS_NewAtom(k);
    nsHTMLValue value;
    
    if (NS_CONTENT_ATTR_NOT_THERE == 
        aContent->GetAttribute(keyAtom, value)) {
      // Get value and remove mandatory quotes
      GetAttributeValueAt(aNode, i, v, aScriptContextOwner);

      // Add attribute to content
      aContent->SetAttribute(keyAtom, v, PR_FALSE);
    }
    NS_RELEASE(keyAtom);
  }
  return NS_OK;
}

void SetForm(nsIHTMLContent* aContent, nsIDOMHTMLFormElement* aForm)
{
  nsIFormControl* formControl = nsnull;
  nsresult result = aContent->QueryInterface(kIFormControlIID, (void**)&formControl);
  if ((NS_OK == result) && formControl) {
    formControl->SetForm(aForm);
    NS_RELEASE(formControl);
  }
}

// XXX compare switch statement against nsHTMLTags.h's list
static nsresult
MakeContentObject(nsHTMLTag aNodeType,
                  nsIAtom* aAtom,
                  nsIDOMHTMLFormElement* aForm,
                  nsIWebShell* aWebShell,
                  nsIHTMLContent** aResult,
                  const nsString* aContent = nsnull)
{
  nsresult rv = NS_OK;
  switch (aNodeType) {
  default:
    rv = NS_NewHTMLSpanElement(aResult, aAtom);
    break;

  case eHTMLTag_a:
    rv = NS_NewHTMLAnchorElement(aResult, aAtom);
    break;
  case eHTMLTag_applet:
    rv = NS_NewHTMLAppletElement(aResult, aAtom);
    break;
  case eHTMLTag_area:
    rv = NS_NewHTMLAreaElement(aResult, aAtom);
    break;
  case eHTMLTag_base:
    rv = NS_NewHTMLBaseElement(aResult, aAtom);
    break;
  case eHTMLTag_basefont:
    rv = NS_NewHTMLBaseFontElement(aResult, aAtom);
    break;
  case eHTMLTag_blockquote:/* XXX need a real object??? how does type=cite work? */
    rv = NS_NewHTMLSpanElement(aResult, aAtom);
    break;
  case eHTMLTag_body:
    rv = NS_NewHTMLBodyElement(aResult, aAtom);
    break;
  case eHTMLTag_br:
    rv = NS_NewHTMLBRElement(aResult, aAtom);
    break;
  case eHTMLTag_caption:
    rv = NS_NewHTMLTableCaptionElement(aResult, aAtom);
    break;
  case eHTMLTag_col:
    rv = NS_NewHTMLTableColElement(aResult, aAtom);
    break;
  case eHTMLTag_colgroup:
    rv = NS_NewHTMLTableColGroupElement(aResult, aAtom);
    break;
  case eHTMLTag_dir:
    rv = NS_NewHTMLDirectoryElement(aResult, aAtom);
    break;
  case eHTMLTag_div:
    rv = NS_NewHTMLDivElement(aResult, aAtom);
    break;
  case eHTMLTag_dl:
    rv = NS_NewHTMLDListElement(aResult, aAtom);
    break;
  case eHTMLTag_embed:
    rv = NS_NewHTMLEmbedElement(aResult, aAtom);
    break;
  case eHTMLTag_font:
    rv = NS_NewHTMLFontElement(aResult, aAtom);
    break;
  case eHTMLTag_form:
    // the form was already created 
    *aResult = nsnull;
    if (aForm) {
      rv = aForm->QueryInterface(kIHTMLContentIID, (void**)aResult);
    }
    break;
  case eHTMLTag_frame:
    rv = NS_NewHTMLFrameElement(aResult, aAtom);
    break;
  case eHTMLTag_frameset:
    rv = NS_NewHTMLFrameSetElement(aResult, aAtom);
    break;
  case eHTMLTag_h1:
  case eHTMLTag_h2:
  case eHTMLTag_h3:
  case eHTMLTag_h4:
  case eHTMLTag_h5:
  case eHTMLTag_h6:
    rv = NS_NewHTMLHeadingElement(aResult, aAtom);
    break;
  case eHTMLTag_head:
    rv = NS_NewHTMLHeadElement(aResult, aAtom);
    break;
  case eHTMLTag_hr:
    rv = NS_NewHTMLHRElement(aResult, aAtom);
    break;
  case eHTMLTag_iframe:
    rv = NS_NewHTMLIFrameElement(aResult, aAtom);
    break;
  case eHTMLTag_img:
    rv = NS_NewHTMLImageElement(aResult, aAtom);
    break;
  case eHTMLTag_input:
    rv = NS_NewHTMLInputElement(aResult, aAtom);
    SetForm(*aResult, aForm);
    break;
  case eHTMLTag_isindex:
    rv = NS_NewHTMLIsIndexElement(aResult, aAtom);
    break;
  case eHTMLTag_label:
    rv = NS_NewHTMLLabelElement(aResult, aAtom);
    break;
  case eHTMLTag_legend:
    rv = NS_NewHTMLLegendElement(aResult, aAtom);
    break;
  case eHTMLTag_li:
    rv = NS_NewHTMLLIElement(aResult, aAtom);
    break;
  case eHTMLTag_link:
    rv = NS_NewHTMLLinkElement(aResult, aAtom);
    break;
  case eHTMLTag_map:
    rv = NS_NewHTMLMapElement(aResult, aAtom);
    break;
  case eHTMLTag_menu:
    rv = NS_NewHTMLMenuElement(aResult, aAtom);
    break;
  case eHTMLTag_meta:
    rv = NS_NewHTMLMetaElement(aResult, aAtom);
    break;
  case eHTMLTag_object:
    rv = NS_NewHTMLObjectElement(aResult, aAtom);
    break;
  case eHTMLTag_ol:
    rv = NS_NewHTMLOListElement(aResult, aAtom);
    break;
  case eHTMLTag_optgroup:
    rv = NS_NewHTMLOptGroupElement(aResult, aAtom);
    break;
  case eHTMLTag_option:
    rv = NS_NewHTMLOptionElement(aResult, aAtom);
    break;
  case eHTMLTag_p:
    rv = NS_NewHTMLParagraphElement(aResult, aAtom);
    break;
  case eHTMLTag_pre:
    rv = NS_NewHTMLPreElement(aResult, aAtom);
    break;
  case eHTMLTag_param:
    rv = NS_NewHTMLParamElement(aResult, aAtom);
    break;
  case eHTMLTag_q:
    rv = NS_NewHTMLQuoteElement(aResult, aAtom);
    break;
  case eHTMLTag_script:
    rv = NS_NewHTMLScriptElement(aResult, aAtom);
    break;
  case eHTMLTag_select:
    rv = NS_NewHTMLSelectElement(aResult, aAtom);
    SetForm(*aResult, aForm);
    break;
  case eHTMLTag_spacer:
    rv = NS_NewHTMLSpacerElement(aResult, aAtom);
    break;
  case eHTMLTag_style:
    rv = NS_NewHTMLStyleElement(aResult, aAtom);
    break;
  case eHTMLTag_table:
    rv = NS_NewHTMLTableElement(aResult, aAtom);
    break;
  case eHTMLTag_tbody:
  case eHTMLTag_thead:
  case eHTMLTag_tfoot:
    rv = NS_NewHTMLTableSectionElement(aResult, aAtom);
    break;
  case eHTMLTag_td:
  case eHTMLTag_th:
    rv = NS_NewHTMLTableCellElement(aResult, aAtom);
    break;
  case eHTMLTag_textarea:
    rv = NS_NewHTMLTextAreaElement(aResult, aAtom);
    // XXX why is textarea not a container. If it were, this code would not be necessary
    // If the text area has some content, set it 
    if (aContent && (aContent->Length() > 0)) {
      nsIDOMHTMLTextAreaElement* taElem;
      rv = (*aResult)->QueryInterface(kIDOMHTMLTextAreaElementIID, (void **)&taElem);
      if ((NS_OK == rv) && taElem) {
        taElem->SetDefaultValue(*aContent);
        NS_RELEASE(taElem);
      }
    }
    SetForm(*aResult, aForm);
    break;
  case eHTMLTag_title:
    rv = NS_NewHTMLTitleElement(aResult, aAtom);
    break;
  case eHTMLTag_tr:
    rv = NS_NewHTMLTableRowElement(aResult, aAtom);
    break;
  case eHTMLTag_ul:
    rv = NS_NewHTMLUListElement(aResult, aAtom);
    break;
  case eHTMLTag_wbr:
    rv = NS_NewHTMLWBRElement(aResult, aAtom);
    break;
  }
  return rv;
}

#if 0
// XXX is this logic needed by nsDOMHTMLOptionElement?
void
GetOptionText(const nsIParserNode& aNode, nsString& aText)
{
  aText.SetLength(0);
  switch (aNode.GetTokenType()) {
  case eToken_text:
  case eToken_whitespace:
  case eToken_newline:
    aText.Append(aNode.GetText());
    break;

  case eToken_entity:
    {
      nsAutoString tmp2("");
      PRInt32 unicode = aNode.TranslateToUnicodeStr(tmp2);
      if (unicode < 0) {
        aText.Append(aNode.GetText());
      } else {
        aText.Append(tmp2);
      }
    }
    break;
  }
    nsAutoString x;
    char* y = aText.ToNewCString();
    printf("foo");
}
#endif

/**
 * Factory subroutine to create all of the html content objects.
 */
static nsresult
CreateContentObject(const nsIParserNode& aNode,
                    nsHTMLTag aNodeType,
                    nsIDOMHTMLFormElement* aForm,
                    nsIWebShell* aWebShell,
                    nsIHTMLContent** aResult)
{
  // Find/create atom for the tag name
  nsAutoString tmp;
  if (eHTMLTag_userdefined == aNodeType) {
    tmp.Append(aNode.GetText());
    tmp.ToUpperCase();
  }
  else {
    tmp.Append(NS_EnumToTag(aNodeType));
  }
  nsIAtom* atom = NS_NewAtom(tmp);
  if (nsnull == atom) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  // Make the content object
  nsresult rv;
 
  // XXX right now this code is here because we need aNode to create
  // images, textareas and input form elements. As soon as all of the
  // generic content code is in use, it can be moved up into
  // MakeContentObject
  switch (aNodeType) {
  case eHTMLTag_img:
#ifdef XXX_ART_HACK
    {
      /* HACK - Jim Dunn 8/6
       * Check to see if this is an ART image type
       * If so then process it using the ART plugin
       * Otherwise treat it like a normal image
       */

      PRBool bArt = PR_FALSE;
      nsAutoString v;
      PRInt32 ac = aNode.GetAttributeCount();
      for (PRInt32 i = 0; i < ac; i++)    /* Go through all of this tag's attributes */
      {
        const nsString& key = aNode.GetKeyAt(i);

        if (!key.Compare("SRC", PR_TRUE))   /* Find the SRC (source) tag */
        {
          const nsString& key2 = aNode.GetValueAt(i);

          v.Truncate();
          v.Append(key2);
          v.ToLowerCase();
          if (-1 != v.Find(".art"))   /* See if it has an ART extension */
          {
            bArt = PR_TRUE;    /* Treat this like an embed */
            break;
          }
        }
        if (!key.Compare("TYPE", PR_TRUE))  /* Find the TYPE (mimetype) tag */
        {
          const nsString& key2 = aNode.GetValueAt(i);

          v.Truncate();
          v.Append(key2);
          v.ToLowerCase();
          if ((-1 != v.Find("image/x-art"))   /* See if it has an ART Mimetype */
              || (-1 != v.Find("image/art"))
              || (-1 != v.Find("image/x-jg")))
          {
            bArt = PR_TRUE;    /* Treat this like an embed */
            break;
          }
        }
      }
      if (bArt)
        rv = NS_NewHTMLEmbedElement(aResult, atom);
      else
        rv = NS_NewHTMLImageElement(aResult, atom);
    }
#else
    rv = NS_NewHTMLImageElement(aResult, atom);
#endif /* XXX */
    break;
  default:
    {
      // XXX why is textarea not a container?
      nsAutoString content;
      if (eHTMLTag_textarea == aNodeType) {
        content = aNode.GetSkippedContent();
      }
      rv = MakeContentObject(aNodeType, atom, aForm, aWebShell, aResult, &content);
      break;
    }
  }
  NS_RELEASE(atom);

  return rv;
}

nsresult
NS_CreateHTMLElement(nsIHTMLContent** aResult, const nsString& aTag)
{
  // Find tag in tag table
  nsAutoString tmp(aTag);
  tmp.ToUpperCase();
  char cbuf[20];
  tmp.ToCString(cbuf, sizeof(cbuf));
  nsHTMLTag id = NS_TagToEnum(cbuf);
  if (eHTMLTag_userdefined == id) {
    return NS_ERROR_NOT_AVAILABLE;
  }

  // Create atom for tag and then create content object
  nsIAtom* atom = NS_NewAtom(tmp);
  nsresult rv = MakeContentObject(id, atom, nsnull, nsnull, aResult);
  NS_RELEASE(atom);

  return rv;
}

//----------------------------------------------------------------------

SinkContext::SinkContext(HTMLContentSink* aSink)
{
  mSink = aSink;
  mPreAppend = PR_FALSE;
  mStack = nsnull;
  mStackSize = 0;
  mStackPos = 0;
  mText = nsnull;
  mTextLength = 0;
  mTextSize = 0;
}

SinkContext::~SinkContext()
{
  if (nsnull != mStack) {
    for (PRInt32 i = 0; i < mStackPos; i++) {
      NS_RELEASE(mStack[i].mContent);
    }
    delete [] mStack;
  }
  if (nsnull != mText) {
    delete [] mText;
  }
}

nsresult
SinkContext::Begin(nsHTMLTag aNodeType, nsIHTMLContent* aRoot)
{
  if (1 > mStackSize) {
    nsresult rv = GrowStack();
    if (NS_OK != rv) {
      return rv;
    }
  }

  mStack[0].mType = aNodeType;
  mStack[0].mContent = aRoot;
  mStack[0].mFlags = APPENDED;
  NS_ADDREF(aRoot);
  mStackPos = 1;
  mTextLength = 0;

  return NS_OK;
}

void
SinkContext::MaybeMarkSinkDirty()
{
  if (!mSink->mDirty &&
      (2 == mStackPos) &&
      (mSink->mBody == mStack[1].mContent)) {
    // We just finished adding something to the body
    mSink->mDirty = PR_TRUE;
  }
}

nsresult
SinkContext::OpenContainer(const nsIParserNode& aNode)
{
  FlushText();

  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "SinkContext::OpenContainer", aNode);

  nsresult rv;
  if (mStackPos + 1 > mStackSize) {
    rv = GrowStack();
    if (NS_OK != rv) {
      return rv;
    }
  }

  // Create new container content object
  nsHTMLTag nodeType = nsHTMLTag(aNode.GetNodeType());
  nsIHTMLContent* content;
  rv = CreateContentObject(aNode, nodeType,
                           mSink->mCurrentForm,
                           mSink->mFrameset ? mSink->mWebShell : nsnull,
                           &content);
  if (NS_OK != rv) {
    return rv;
  }
  mStack[mStackPos].mType = nodeType;
  mStack[mStackPos].mContent = content;
  mStack[mStackPos].mFlags = 0;
  content->SetDocument(mSink->mDocument);

  nsIScriptContextOwner* sco = mSink->mDocument->GetScriptContextOwner();
  rv = AddAttributes(aNode, content, sco);
  NS_IF_RELEASE(sco);

  if (mPreAppend) {
    NS_ASSERTION(mStackPos > 0, "container w/o parent");
    nsIHTMLContent* parent = mStack[mStackPos-1].mContent;
    parent->AppendChildTo(content, PR_FALSE);
    mStack[mStackPos].mFlags |= APPENDED;
  }
  mStackPos++;
  if (NS_OK != rv) {
    return rv;
  }

  // Special handling for certain tags
  switch (nodeType) {
  case eHTMLTag_a:
    mSink->ProcessATag(aNode, content);
    break;
  case eHTMLTag_table:
    mSink->mInMonolithicContainer++;
    break;
  }

  return NS_OK;
}

nsresult
SinkContext::CloseContainer(const nsIParserNode& aNode)
{
  FlushText();

  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "SinkContext::CloseContainer", aNode);

  --mStackPos;
  nsHTMLTag nodeType = mStack[mStackPos].mType;
  nsIHTMLContent* content = mStack[mStackPos].mContent;
  content->Compact();

  // Add container to its parent if we haven't already done it
  if (0 == (mStack[mStackPos].mFlags & APPENDED)) {
    NS_ASSERTION(mStackPos > 0, "container w/o parent");
    nsIHTMLContent* parent = mStack[mStackPos-1].mContent;
    parent->AppendChildTo(content, PR_FALSE);
  }
  NS_IF_RELEASE(content);

  // Special handling for certain tags
  switch (nodeType) {
  case eHTMLTag_table:
    mSink->mInMonolithicContainer--;
    break;
  }

  // Mark sink dirty if it can safely reflow something
  MaybeMarkSinkDirty();

  return NS_OK;
}

nsresult
SinkContext::AddLeaf(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "SinkContext::AddLeaf", aNode);

  nsresult rv = NS_OK;

  switch (aNode.GetTokenType()) {
  case eToken_start:
    {
      FlushText();

      // Create new leaf content object
      nsHTMLTag nodeType = nsHTMLTag(aNode.GetNodeType());
      nsIHTMLContent* content;
      rv = CreateContentObject(aNode, nodeType,
                               mSink->mCurrentForm, mSink->mWebShell,
                               &content);
      if (NS_OK != rv) {
        return rv;
      }

      // Set the content's document
      content->SetDocument(mSink->mDocument);

      nsIScriptContextOwner* sco = mSink->mDocument->GetScriptContextOwner();
      rv = AddAttributes(aNode, content, sco);
      NS_IF_RELEASE(sco);
      if (NS_OK != rv) {
        NS_RELEASE(content);
        return rv;
      }
      switch (nodeType) {
      case eHTMLTag_img:
        mSink->AddBaseTagInfo(content);
        break;
      }

      // Add new leaf to its parent
      NS_ASSERTION(mStackPos > 0, "leaf w/o container");
      nsIHTMLContent* parent = mStack[mStackPos-1].mContent;
      parent->AppendChildTo(content, PR_FALSE);
      NS_RELEASE(content);

      // Mark sink dirty if it can safely reflow something
      MaybeMarkSinkDirty();
    }
    break;

  case eToken_text:
  case eToken_whitespace:
  case eToken_newline:
    return AddText(aNode.GetText());

  case eToken_entity:
    {
      nsAutoString tmp;
      PRInt32 unicode = aNode.TranslateToUnicodeStr(tmp);
      if (unicode < 0) {
        return AddText(aNode.GetText());
      }
      return AddText(tmp);
    }

  case eToken_skippedcontent:
    break;
  }
  return rv;
}

nsresult
SinkContext::End()
{
  NS_ASSERTION(mStackPos == 1, "insufficient close container calls");

  for (PRInt32 i = 0; i < mStackPos; i++) {
    NS_RELEASE(mStack[i].mContent);
  }
  mStackPos = 0;
  mTextLength = 0;

  return NS_OK;
}

nsresult
SinkContext::GrowStack()
{
  PRInt32 newSize = mStackSize * 2;
  if (0 == newSize) {
    newSize = 32;
  }
  Node* stack = new Node[newSize];
  if (nsnull == stack) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  if (0 != mStackPos) {
    memcpy(stack, mStack, sizeof(Node) * mStackPos);
    delete [] mStack;
  }
  mStack = stack;
  mStackSize = newSize;
  return NS_OK;
}

/**
 * Add textual content to the current running text buffer. If the text buffer
 * overflows, flush out the text by creating a text content object and adding
 * it to the content tree.
 */
// XXX If we get a giant string grow the buffer instead of chopping it up???
nsresult
SinkContext::AddText(const nsString& aText)
{
  PRInt32 addLen = aText.Length();
  if (0 == addLen) {
    return NS_OK;
  }

  // Create buffer when we first need it
  if (0 == mTextSize) {
    mText = new PRUnichar[4096];
    if (nsnull == mText) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    mTextSize = 4096;
  }
//  else if (mTextLength + addLen > mTextSize) {
//  }

  // Copy data from string into our buffer; flush buffer when it fills up
  PRInt32 offset = 0;
  while (0 != addLen) {
    PRInt32 amount = mTextSize - mTextLength;
    if (amount > addLen) {
      amount = addLen;
    }
    if (0 == amount) {
      nsresult rv = FlushText();
      if (NS_OK != rv) {
        return rv;
      }
    }
    memcpy(&mText[mTextLength], aText.GetUnicode() + offset,
           sizeof(PRUnichar) * amount);
    mTextLength += amount;
    offset += amount;
    addLen -= amount;
  }

  return NS_OK;
}

/**
 * Flush any buffered text out by creating a text content object and
 * adding it to the content.
 */
nsresult
SinkContext::FlushText(PRBool* aDidFlush)
{
  nsresult rv = NS_OK;
  PRBool didFlush = PR_FALSE;
  if (0 != mTextLength) {
    nsIHTMLContent* content;
    rv = NS_NewTextNode(&content);
    if (NS_OK == rv) {
      // Set the content's document
      content->SetDocument(mSink->mDocument);

      // Set the text in the text node
      static NS_DEFINE_IID(kITextContentIID, NS_ITEXT_CONTENT_IID);
      nsITextContent* text = nsnull;
      content->QueryInterface(kITextContentIID, (void**) &text);
      text->SetText(mText, mTextLength, PR_FALSE);
      NS_RELEASE(text);

      // Add text to its parent
      NS_ASSERTION(mStackPos > 0, "leaf w/o container");
      nsIHTMLContent* parent = mStack[mStackPos - 1].mContent;
      parent->AppendChildTo(content, PR_FALSE);
      NS_RELEASE(content);

      // Mark sink dirty if it can safely reflow something
      MaybeMarkSinkDirty();
    }
    mTextLength = 0;
    didFlush = PR_TRUE;
  }
  if (nsnull != aDidFlush) {
    *aDidFlush = didFlush;
  }
  return rv;
}


nsresult
NS_NewHTMLContentSink(nsIHTMLContentSink** aResult,
                      nsIDocument* aDoc,
                      nsIURL* aURL,
                      nsIWebShell* aWebShell)
{
  NS_PRECONDITION(nsnull != aResult, "null ptr");
  if (nsnull == aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  HTMLContentSink* it;
  NS_NEWXPCOM(it, HTMLContentSink);
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  nsresult rv = it->Init(aDoc, aURL, aWebShell);
  if (NS_OK != rv) {
    delete it;
    return rv;
  }
  return it->QueryInterface(kIHTMLContentSinkIID, (void **)aResult);
}

HTMLContentSink::HTMLContentSink()
{
#ifdef NS_DEBUG
  if (nsnull == gSinkLogModuleInfo) {
    gSinkLogModuleInfo = PR_NewLogModule("htmlcontentsink");
  }
#endif
  mNotAtRef = PR_TRUE;
}

HTMLContentSink::~HTMLContentSink()
{
  NS_IF_RELEASE(mHead);
  NS_IF_RELEASE(mBody);
  NS_IF_RELEASE(mFrameset);
  NS_IF_RELEASE(mRoot);

  NS_IF_RELEASE(mDocument);
  NS_IF_RELEASE(mDocumentURL);
  NS_IF_RELEASE(mWebShell);

  NS_IF_RELEASE(mStyleSheet);
  NS_IF_RELEASE(mCurrentForm);
  NS_IF_RELEASE(mCurrentMap);
  NS_IF_RELEASE(mRefContent);

  for (PRInt32 i = 0; i < mNumContexts; i++) {
    SinkContext* sc = mContexts[i];
    sc->End();
    delete sc;
    if (sc == mCurrentContext) {
      mCurrentContext = nsnull;
    }
  }
  if (nsnull != mContexts) {
    delete [] mContexts;
  }
  if (mCurrentContext == mHeadContext) {
    mCurrentContext = nsnull;
  }
  if (nsnull != mCurrentContext) {
    delete mCurrentContext;
  }
  if (nsnull != mHeadContext) {
    delete mHeadContext;
  }
  if (nsnull != mTitle) {
    delete mTitle;
  }
  if (nsnull != mRef) {
    delete mRef;
  }
}

NS_IMPL_ISUPPORTS(HTMLContentSink, kIHTMLContentSinkIID)

nsresult
HTMLContentSink::Init(nsIDocument* aDoc,
                      nsIURL* aURL,
                      nsIWebShell* aContainer)
{
  NS_PRECONDITION(nsnull != aDoc, "null ptr");
  NS_PRECONDITION(nsnull != aURL, "null ptr");
  NS_PRECONDITION(nsnull != aContainer, "null ptr");
  if ((nsnull == aDoc) || (nsnull == aURL) || (nsnull == aContainer)) {
    return NS_ERROR_NULL_POINTER;
  }

  mDocument = aDoc;
  NS_ADDREF(aDoc);
  mDocumentURL = aURL;
  NS_ADDREF(aURL);
  mWebShell = aContainer;
  NS_ADDREF(aContainer);

  // Make root part
  nsresult rv = NS_NewHTMLHtmlElement(&mRoot, nsHTMLAtoms::html);
  if (NS_OK != rv) {
    return rv;
  }
  mRoot->SetDocument(mDocument);
  mDocument->SetRootContent(mRoot);

  // Make head part
  nsIAtom* atom = NS_NewAtom("HEAD");
  if (nsnull == atom) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  rv = NS_NewHTMLHeadElement(&mHead, atom);
  NS_RELEASE(atom);
  if (NS_OK != rv) {
    return rv;
  }
  mRoot->AppendChildTo(mHead, PR_FALSE);

  mCurrentContext = new SinkContext(this);
  mCurrentContext->Begin(eHTMLTag_html, mRoot);
  mContextStack.AppendElement(mCurrentContext);

  SINK_TRACE(SINK_TRACE_CALLS,
             ("HTMLContentSink::Init: this=%p url='%s'",
              this, aURL->GetSpec()));

  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::WillBuildModel(void)
{
  // Notify document that the load is beginning
  mDocument->BeginLoad();
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::DidBuildModel(PRInt32 aQualityLevel)
{
  if (nsnull == mTitle) {
    ((nsHTMLDocument*)mDocument)->SetTitle("");
  }

  // XXX this is silly; who cares?
  PRInt32 i, ns = mDocument->GetNumberOfShells();
  for (i = 0; i < ns; i++) {
    nsIPresShell* shell = mDocument->GetShellAt(i);
    if (nsnull != shell) {
      nsIViewManager* vm = shell->GetViewManager();
      if(vm) {
        vm->SetQuality(nsContentQuality(aQualityLevel));
      }
      NS_RELEASE(vm);
      NS_RELEASE(shell);
    }
  }

  SINK_TRACE(SINK_TRACE_REFLOW,
             ("HTMLContentSink::DidBuildModel: layout final content"));

  // Reflow the last batch of content
  if (nsnull != mBody) {
    mDocument->ContentAppended(mBody, mBodyChildCount);
    mBody->ChildCount(mBodyChildCount);
  }
  ScrollToRef();

  mDocument->EndLoad();
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::WillInterrupt()
{
  SINK_TRACE(SINK_TRACE_CALLS,
             ("HTMLContentSink::WillInterrupt: this=%p", this));
  if (mDirty) {
    if (nsnull != mBody) {
      mDocument->ContentAppended(mBody, mBodyChildCount);
      mBody->ChildCount(mBodyChildCount);
    }
    mDirty = PR_FALSE;
  }
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::WillResume()
{
  SINK_TRACE(SINK_TRACE_CALLS,
             ("HTMLContentSink::WillResume: this=%p", this));
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::BeginContext(PRInt32 aID)
{
  NS_PRECONDITION(aID >= 0, "bad context ID");

  // Create new context
  SinkContext* sc = new SinkContext(this);
  if (nsnull == sc) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  // Put it into our context array. Grow the array if necessary.
  if (aID >= mNumContexts) {
    PRInt32 newSize = aID + 10;
    SinkContext** contexts = new SinkContext*[newSize];
    if (nsnull == contexts) {
      delete sc;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    memset(contexts, 0, sizeof(SinkContext*) * newSize);
    if (0 != mNumContexts) {
      memcpy(contexts, mContexts, sizeof(SinkContext*) * mNumContexts);
      delete [] mContexts;
    }
    mNumContexts = newSize;
    mContexts = contexts;
  }
  mContexts[aID] = sc;

  // Push the old current context and make the new context the current one
  mContextStack.AppendElement(mCurrentContext);
  mCurrentContext = sc;

  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::EndContext(PRInt32 aID)
{
  NS_PRECONDITION((aID >= 0) && (aID < mNumContexts) &&
                  (mCurrentContext == mContexts[aID]), "bad context ID");

  // Destroy the context
  SinkContext* sc = mCurrentContext;
  sc->End();
  delete sc;
  mContexts[aID] = nsnull;

  // Pop off the previous context and make it our current context
  PRInt32 n = mContextStack.Count() - 1;
  mCurrentContext = (SinkContext*) mContextStack.ElementAt(n);
  mContextStack.RemoveElementAt(n);

  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::SetContext(PRInt32 aID)
{
  NS_PRECONDITION((aID >= 0) && (aID < mNumContexts), "bad context ID");
  SinkContext* sc = mContexts[aID];
  mCurrentContext = sc;
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::SetTitle(const nsString& aValue)
{
  NS_ASSERTION(mCurrentContext == mHeadContext, "SetTitle not in head");

  if (nsnull == mTitle) {
    mTitle = new nsString(aValue);
  }
  else {
    *mTitle = aValue;
  }
  mTitle->CompressWhitespace(PR_TRUE, PR_TRUE);
  ((nsHTMLDocument*)mDocument)->SetTitle(*mTitle);

  nsIAtom* atom = NS_NewAtom("TITLE");
  nsIHTMLContent* it = nsnull;
  nsresult rv = NS_NewHTMLTitleElement(&it, atom);
  if (NS_OK == rv) {
    nsIDOMHTMLTitleElement* domtitle = nsnull;
    it->QueryInterface(kIDOMHTMLTitleElementIID, (void**) &domtitle);
    if (nsnull != domtitle) {
      domtitle->SetText(aValue);
      NS_RELEASE(domtitle);
    }
    mHead->AppendChildTo(it, PR_FALSE);
    NS_RELEASE(it);
  }
  NS_RELEASE(atom);
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::OpenHTML(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::OpenHTML", aNode);
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::CloseHTML(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::CloseHTML", aNode);
  if (nsnull != mHeadContext) {
    mHeadContext->End();
    delete mHeadContext;
    mHeadContext = nsnull;
  }
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::OpenHead(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::OpenHead", aNode);
  if (nsnull == mHeadContext) {
    mHeadContext = new SinkContext(this);
    if (nsnull == mHeadContext) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    mHeadContext->SetPreAppend(PR_TRUE);
    nsresult rv = mHeadContext->Begin(eHTMLTag_head, mHead);
    if (NS_OK != rv) {
      return rv;
    }
  }
  mContextStack.AppendElement(mCurrentContext);
  mCurrentContext = mHeadContext;
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::CloseHead(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::CloseHead", aNode);
  PRInt32 n = mContextStack.Count() - 1;
  mCurrentContext = (SinkContext*) mContextStack.ElementAt(n);
  mContextStack.RemoveElementAt(n);
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::OpenBody(const nsIParserNode& aNode)
{
  NS_PRECONDITION(nsnull == mBody, "parser called OpenBody twice");

  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::OpenBody", aNode);

  // Open body. Note that we pre-append the body to the root so that
  // incremental reflow during document loading will work properly.
  mCurrentContext->SetPreAppend(PR_TRUE);
  nsresult rv = mCurrentContext->OpenContainer(aNode);
  mCurrentContext->SetPreAppend(PR_FALSE);
  if (NS_OK != rv) {
    return rv;
  }
  mBody = mCurrentContext->mStack[mCurrentContext->mStackPos - 1].mContent;
  mBodyChildCount = 0;
  NS_ADDREF(mBody);

  StartLayout();
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::CloseBody(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::CloseBody", aNode);

  PRBool didFlush;
  nsresult rv = mCurrentContext->FlushText(&didFlush);
  if (NS_OK != rv) {
    return rv;
  }
  mCurrentContext->CloseContainer(aNode);

  if (didFlush) {
    // Trigger a reflow for the flushed text
    mDocument->ContentAppended(mBody, mBodyChildCount);
    mBody->ChildCount(mBodyChildCount);
  }

  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::OpenForm(const nsIParserNode& aNode)
{
  mCurrentContext->FlushText();

  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::OpenForm", aNode);

  // Close out previous form if it's there
  NS_IF_RELEASE(mCurrentForm);

  // set the current form
  nsAutoString tmp("FORM");
  nsIAtom* atom = NS_NewAtom(tmp);
  nsIHTMLContent* iContent = nsnull;
  nsresult rv = NS_NewHTMLFormElement(&iContent, atom);
  if ((NS_OK == rv) && iContent) {
    iContent->QueryInterface(kIDOMHTMLFormElementIID, (void**)&mCurrentForm);
    NS_RELEASE(iContent);
  }
  NS_RELEASE(atom);

  AddLeaf(aNode);

  // add the form to the document
  //nsIHTMLDocument* htmlDoc = nsnull;
  //if (mDocument && mCurrentForm) {
  //  rv = mDocument->QueryInterface(kIHTMLDocumentIID, (void**)&htmlDoc);
  //  if ((NS_OK == rv) && htmlDoc) {
  //    htmlDoc->AddForm(mCurrentForm);
  //    NS_RELEASE(htmlDoc);
  //  }
  //}

  return NS_OK;
}

// XXX MAYBE add code to place close form tag into the content model
// for navigator layout compatability.
NS_IMETHODIMP
HTMLContentSink::CloseForm(const nsIParserNode& aNode)
{
  mCurrentContext->FlushText();
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::CloseForm", aNode);
  NS_IF_RELEASE(mCurrentForm);
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::OpenFrameset(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::OpenFrameset", aNode);

  nsresult rv = mCurrentContext->OpenContainer(aNode);
  if ((NS_OK == rv) && (nsnull == mFrameset)) {
    mFrameset = mCurrentContext->mStack[mCurrentContext->mStackPos-1].mContent;
    NS_ADDREF(mFrameset);
  }
  mInMonolithicContainer++;
  return rv;
}

NS_IMETHODIMP
HTMLContentSink::CloseFrameset(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::CloseFrameset", aNode);

  SinkContext* sc = mCurrentContext;
  nsIHTMLContent* fs = sc->mStack[sc->mStackPos-1].mContent;
  PRBool done = fs == mFrameset;
  nsresult rv = sc->CloseContainer(aNode);
  if (done) {
    StartLayout();
  }
  return rv;
}

NS_IMETHODIMP
HTMLContentSink::OpenMap(const nsIParserNode& aNode)
{
  mCurrentContext->FlushText();

  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::OpenMap", aNode);

  // Close out previous map if it's there
  NS_IF_RELEASE(mCurrentMap);

  nsAutoString tmp("MAP");
  nsIAtom* atom = NS_NewAtom(tmp);
  nsresult rv = NS_NewImageMap(&mCurrentMap, atom);
  NS_RELEASE(atom);
  if (NS_OK == rv) {
    // XXX rewrite to use AddAttributes and don't use FindAttribute
    // Look for name attribute and set the map name
    nsAutoString name;
    if (FindAttribute(aNode, "name", name)) {
      // XXX leading, trailing, interior non=-space ws is removed
      name.StripWhitespace();
      mCurrentMap->SetName(name);
    }

    // Add the map to the document
    ((nsHTMLDocument*)mDocument)->AddImageMap(mCurrentMap);

    // XXX Add a content object for the map too
  }
  return rv;
}

NS_IMETHODIMP
HTMLContentSink::CloseMap(const nsIParserNode& aNode)
{
  SINK_TRACE_NODE(SINK_TRACE_CALLS,
                  "HTMLContentSink::CloseMap", aNode);
  NS_IF_RELEASE(mCurrentMap);
  return NS_OK;
}

NS_IMETHODIMP
HTMLContentSink::OpenContainer(const nsIParserNode& aNode)
{
  // XXX work around parser bug
  if (eHTMLTag_frameset == aNode.GetNodeType()) {
    return OpenFrameset(aNode);
  }
  return mCurrentContext->OpenContainer(aNode);
}

NS_IMETHODIMP
HTMLContentSink::CloseContainer(const nsIParserNode& aNode)
{
  // XXX work around parser bug
  if (eHTMLTag_frameset == aNode.GetNodeType()) {
    return CloseFrameset(aNode);
  }
  return mCurrentContext->CloseContainer(aNode);
}

NS_IMETHODIMP
HTMLContentSink::AddLeaf(const nsIParserNode& aNode)
{
  nsresult rv;

  nsHTMLTag nodeType = nsHTMLTag(aNode.GetNodeType());
  switch (nodeType) {
  case eHTMLTag_area:
    mCurrentContext->FlushText();
    rv = ProcessAREATag(aNode);
    break;

  case eHTMLTag_base:
    mCurrentContext->FlushText();
    rv = ProcessBASETag(aNode);
    break;

  case eHTMLTag_link:
    mCurrentContext->FlushText();
    rv = ProcessLINKTag(aNode);
    break;

  case eHTMLTag_meta:
    mCurrentContext->FlushText();
    rv = ProcessMETATag(aNode);
    break;

  case eHTMLTag_style:
    mCurrentContext->FlushText();
    rv = ProcessSTYLETag(aNode);
    break;

  case eHTMLTag_script:
    mCurrentContext->FlushText();
    rv = ProcessSCRIPTTag(aNode);
    break;

  default:
    rv = mCurrentContext->AddLeaf(aNode);
    break;
  }
  return rv;
}

void
HTMLContentSink::StartLayout()
{
  if (mLayoutStarted) {
    return;
  }
  mLayoutStarted = PR_TRUE;

  PRInt32 i, ns = mDocument->GetNumberOfShells();
  for (i = 0; i < ns; i++) {
    nsIPresShell* shell = mDocument->GetShellAt(i);
    if (nsnull != shell) {
      // Make shell an observer for next time
      shell->BeginObservingDocument();

      // Resize-reflow this time
      nsIPresContext* cx = shell->GetPresContext();
      nsRect r;
      cx->GetVisibleArea(r);
      shell->InitialReflow(r.width, r.height);
      NS_RELEASE(cx);

      // Now trigger a refresh
      nsIViewManager* vm = shell->GetViewManager();
      if (nsnull != vm) {
        vm->EnableRefresh();
        NS_RELEASE(vm);
      }

      NS_RELEASE(shell);
    }
  }

  // If the document we are loading has a reference or it is a top level
  // frameset document, disable the scroll bars on the views.
  const char* ref = mDocumentURL->GetRef();
  if (nsnull != ref) {
    mRef = new nsString(ref);
  }
  PRBool topLevelFrameset = PR_FALSE;
  if (mFrameset && mWebShell) {
    nsIWebShell* rootWebShell;
    mWebShell->GetRootWebShell(rootWebShell);
    if (mWebShell == rootWebShell) {
      topLevelFrameset = PR_TRUE;
    }
    NS_IF_RELEASE(rootWebShell);
  }

  if ((nsnull != ref) || topLevelFrameset) {
    // XXX support more than one presentation-shell here

    // Get initial scroll preference and save it away; disable the
    // scroll bars.
    PRInt32 i, ns = mDocument->GetNumberOfShells();
    for (i = 0; i < ns; i++) {
      nsIPresShell* shell = mDocument->GetShellAt(i);
      if (nsnull != shell) {
        nsIViewManager* vm = shell->GetViewManager();
        if (nsnull != vm) {
          nsIView* rootView = nsnull;
          vm->GetRootView(rootView);
          if (nsnull != rootView) {
            nsIScrollableView* sview = nsnull;
            rootView->QueryInterface(kIScrollableViewIID, (void**) &sview);
            if (nsnull != sview) {
              if (topLevelFrameset)
                mOriginalScrollPreference = nsScrollPreference_kNeverScroll;
              else
                sview->GetScrollPreference(mOriginalScrollPreference);
              sview->SetScrollPreference(nsScrollPreference_kNeverScroll);
            }
          }
          NS_RELEASE(vm);
        }
        NS_RELEASE(shell);
      }
    }
  }
}

void
HTMLContentSink::ScrollToRef()
{
  if (mNotAtRef && (nsnull != mRef) && (nsnull != mRefContent)) {
    // See if the ref content has been reflowed by finding it's frame
    PRInt32 i, ns = mDocument->GetNumberOfShells();
    for (i = 0; i < ns; i++) {
      nsIPresShell* shell = mDocument->GetShellAt(i);
      if (nsnull != shell) {
        nsIFrame* frame = shell->FindFrameWithContent(mRefContent);
        if (nsnull != frame) {
          nsIViewManager* vm = shell->GetViewManager();
          if (nsnull != vm) {
            nsIView* rootView = nsnull;
            vm->GetRootView(rootView);
            if (nsnull != rootView) {
              nsIScrollableView* sview = nsnull;
              rootView->QueryInterface(kIScrollableViewIID, (void**) &sview);
              if (nsnull != sview) {
                // Determine the x,y scroll offsets for the given
                // frame. The offsets are relative to the
                // ScrollableView's upper left corner so we need frame
                // coordinates that are relative to that.
                nsPoint offset;
                nsIView* view;
                frame->GetOffsetFromView(offset, view);
                if (view == rootView) {
// XXX write me!
// printf("view==rootView ");
                }
                nscoord x = 0;
                nscoord y = offset.y;
#if 0
nsIPresContext* cx = shell->GetPresContext();
float t2p = cx->GetTwipsToPixels();
printf("x=%d y=%d\n", NSTwipsToIntPixels(x, t2p), NSTwipsToIntPixels(y, t2p));
NS_RELEASE(cx);
#endif
                sview->SetScrollPreference(mOriginalScrollPreference);
                sview->ScrollTo(x, y, NS_VMREFRESH_IMMEDIATE);

                // Note that we did this so that we don't bother doing it again
                mNotAtRef = PR_FALSE;
              }
            }
            NS_RELEASE(vm);
          }
        }
        NS_RELEASE(shell);
      }
    }
  }
}

void
HTMLContentSink::AddBaseTagInfo(nsIHTMLContent* aContent)
{
  if (mBaseHREF.Length() > 0) {
    nsHTMLValue value(mBaseHREF);
    aContent->SetAttribute(nsHTMLAtoms::_baseHref, value, PR_FALSE);
  }
  if (mBaseTarget.Length() > 0) {
    nsHTMLValue value(mBaseTarget);
    aContent->SetAttribute(nsHTMLAtoms::_baseTarget, value, PR_FALSE);
  }
}

nsresult
HTMLContentSink::ProcessATag(const nsIParserNode& aNode,
                             nsIHTMLContent* aContent)
{
  AddBaseTagInfo(aContent);
  if ((nsnull != mRef) && (nsnull == mRefContent)) {
    nsHTMLValue value;
    aContent->GetAttribute(nsHTMLAtoms::name, value);
    if (eHTMLUnit_String == value.GetUnit()) {
      nsAutoString tmp;
      value.GetStringValue(tmp);
      if (mRef->EqualsIgnoreCase(tmp)) {
        // Winner. We just found the content that is the named anchor
        mRefContent = aContent;
        NS_ADDREF(aContent);
      }
    }
  }
  return NS_OK;
}

// XXX add area content object to map content object instead
nsresult
HTMLContentSink::ProcessAREATag(const nsIParserNode& aNode)
{
  if (nsnull != mCurrentMap) {
    nsAutoString shape, coords, href, target(mBaseTarget), alt;
    PRInt32 ac = aNode.GetAttributeCount();
    PRBool suppress = PR_FALSE;
    nsIScriptContextOwner* sco = mDocument->GetScriptContextOwner();
    for (PRInt32 i = 0; i < ac; i++) {
      // Get upper-cased key
      const nsString& key = aNode.GetKeyAt(i);
      if (key.EqualsIgnoreCase("shape")) {
        GetAttributeValueAt(aNode, i, shape, sco);
      }
      else if (key.EqualsIgnoreCase("coords")) {
        GetAttributeValueAt(aNode, i, coords, sco);
      }
      else if (key.EqualsIgnoreCase("href")) {
        GetAttributeValueAt(aNode, i, href, sco);
        href.StripWhitespace();
      }
      else if (key.EqualsIgnoreCase("target")) {
        GetAttributeValueAt(aNode, i, target, sco);
      }
      else if (key.EqualsIgnoreCase("alt")) {
        GetAttributeValueAt(aNode, i, alt, sco);
      }
      else if (key.EqualsIgnoreCase("suppress")) {
        suppress = PR_TRUE;
      }
    }
    NS_RELEASE(sco);
    mCurrentMap->AddArea(mBaseHREF, shape, coords, href, target, alt,
                         suppress);
  }
  return NS_OK;
}

nsresult
HTMLContentSink::ProcessBASETag(const nsIParserNode& aNode)
{
  nsIScriptContextOwner* sco = mDocument->GetScriptContextOwner();
  PRInt32 ac = aNode.GetAttributeCount();
  for (PRInt32 i = 0; i < ac; i++) {
    const nsString& key = aNode.GetKeyAt(i);
    if (key.EqualsIgnoreCase("href")) {
      GetAttributeValueAt(aNode, i, mBaseHREF, sco);
    } else if (key.EqualsIgnoreCase("target")) {
      GetAttributeValueAt(aNode, i, mBaseTarget, sco);
    }
  }
  NS_RELEASE(sco);
  return NS_OK;
}

nsresult
HTMLContentSink::ProcessLINKTag(const nsIParserNode& aNode)
{
  nsresult  result = NS_OK;
  PRInt32   index;
  PRInt32   count = aNode.GetAttributeCount();

  nsAutoString href;
  nsAutoString rel; 
  nsAutoString title; 
  nsAutoString type; 

  nsIScriptContextOwner* sco = mDocument->GetScriptContextOwner();
  for (index = 0; index < count; index++) {
    const nsString& key = aNode.GetKeyAt(index);
    if (key.EqualsIgnoreCase("href")) {
      GetAttributeValueAt(aNode, index, href, sco);
      href.StripWhitespace();
    }
    else if (key.EqualsIgnoreCase("rel")) {
      GetAttributeValueAt(aNode, index, rel, sco);
      rel.CompressWhitespace();
    }
    else if (key.EqualsIgnoreCase("title")) {
      GetAttributeValueAt(aNode, index, title, sco);
      title.CompressWhitespace();
    }
    else if (key.EqualsIgnoreCase("type")) {
      GetAttributeValueAt(aNode, index, type, sco);
      type.StripWhitespace();
    }
  }
  NS_RELEASE(sco);

  if (rel.EqualsIgnoreCase("stylesheet")) {
    if (type.EqualsIgnoreCase("text/css")) {
      nsIURL* url = nsnull;
      nsIUnicharInputStream* uin = nsnull;
      nsAutoString absURL;
      nsIURL* docURL = mDocument->GetDocumentURL();
      result = NS_MakeAbsoluteURL(docURL, mBaseHREF, href, absURL);
      if (NS_OK != result) {
        return result;
      }
      NS_RELEASE(docURL);
      result = NS_NewURL(&url, nsnull, absURL);
      if (NS_OK != result) {
        return result;
      }
      PRInt32 ec;
      nsIInputStream* iin = url->Open(&ec);
      if (nsnull == iin) {
        NS_RELEASE(url);
        return (nsresult) ec;/* XXX fix url->Open */
      }
      result = NS_NewConverterStream(&uin, nsnull, iin);
      NS_RELEASE(iin);
      if (NS_OK != result) {
        NS_RELEASE(url);
        return result;
      }

      result = LoadStyleSheet(url, uin, PR_FALSE);
      NS_RELEASE(uin);
      NS_RELEASE(url);
    }
  }

  return result;
}

nsresult
HTMLContentSink::ProcessMETATag(const nsIParserNode& aNode)
{
  if (nsnull != mHead) {
    // Create content object
    nsAutoString tmp("META");
    nsIAtom* atom = NS_NewAtom(tmp);
    if (nsnull == atom) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    nsIHTMLContent* it;
    nsresult rv = NS_NewHTMLMetaElement(&it, atom);
    NS_RELEASE(atom);
    if (NS_OK == rv) {
      // Add in the attributes and add the meta content object to the
      // head container.
      nsIScriptContextOwner* sco = mDocument->GetScriptContextOwner();
      rv = AddAttributes(aNode, it, sco);
      NS_IF_RELEASE(sco);
      if (NS_OK != rv) {
        NS_RELEASE(it);
        return rv;
      }
      mHead->AppendChildTo(it, PR_FALSE);

      // If we are processing an HTTP url, handle the Refresh Case. We
      // could also handle the other http-equiv cases here such as
      // "Content-type".
      nsIHttpUrl* httpUrl = nsnull;
      rv = mDocumentURL->QueryInterface(kIHTTPUrlIID, (void **)&httpUrl);
      if (NS_OK == rv) {
        nsAutoString result;
        it->GetAttribute(nsHTMLAtoms::httpEquiv, result);
        if (result.EqualsIgnoreCase("REFRESH")) {
          it->GetAttribute(nsHTMLAtoms::content, result);
          if (result.Length() > 0) {
            char* value = result.ToNewCString();
            if (!value) {
              NS_RELEASE(it);
              return NS_ERROR_OUT_OF_MEMORY;
            }
            httpUrl->AddMimeHeader("REFRESH", value);
            delete value;
          }
        }
        NS_RELEASE(httpUrl);
      }

      NS_RELEASE(it);
    }
  }

  return NS_OK;
}

#define SCRIPT_BUF_SIZE 1024

nsresult
HTMLContentSink::ProcessSCRIPTTag(const nsIParserNode& aNode)
{
  nsresult rv = NS_OK;
  PRInt32 i, ac = aNode.GetAttributeCount();

  // Look for SRC attribute
  nsAutoString src;
  for (i = 0; i < ac; i++) {
    const nsString& key = aNode.GetKeyAt(i);
    if (key.EqualsIgnoreCase("src")) {
      src = aNode.GetValueAt(i);
      src.Trim("\"", PR_TRUE, PR_TRUE); 
    }
  }

  nsAutoString script;

  // If there is a SRC attribute, (for now) read from the
  // stream synchronously and hold the data in a string.
  if (src != "") {
    // Use the SRC attribute value to open a blocking stream
    nsIURL* url = nsnull;
    nsAutoString absURL;
    nsIURL* docURL = mDocument->GetDocumentURL();
    rv = NS_MakeAbsoluteURL(docURL, mBaseHREF, src, absURL);
    if (NS_OK != rv) {
      return rv;
    }
    NS_RELEASE(docURL);
    rv = NS_NewURL(&url, nsnull, absURL);
    if (NS_OK != rv) {
      return rv;
    }
    PRInt32 ec;
    nsIInputStream* iin = url->Open(&ec);
    if (nsnull == iin) {
      NS_RELEASE(url);
      return (nsresult) ec;/* XXX fix url->Open */
    }

    // Drain the stream by reading from it a chunk at a time
    PRInt32 nb;
    nsresult err;
    do {
      char buf[SCRIPT_BUF_SIZE];
      
      err = iin->Read(buf, 0, SCRIPT_BUF_SIZE, &nb);
      if (NS_OK == err) {
        script.Append((const char *)buf, nb);
      }
    } while (err == NS_OK);

    if (NS_BASE_STREAM_EOF != err) {
      rv = NS_ERROR_FAILURE;
    }

    NS_RELEASE(iin);
    NS_RELEASE(url);
  }
  else {
    // Otherwise, get the text content of the script tag
    script = aNode.GetSkippedContent();
  }

  if (script != "") {
    nsIScriptContextOwner *owner;
    nsIScriptContext *context;
    owner = mDocument->GetScriptContextOwner();
    if (nsnull != owner) {
    
      rv = owner->GetScriptContext(&context);
      if (rv != NS_OK) {
        NS_RELEASE(owner);
        return rv;
      }
      
      jsval val;
      nsIURL* mDocURL = mDocument->GetDocumentURL();
      const char* mURL;
      if (mDocURL) {
        mURL = mDocURL->GetSpec();
      }
      PRUint32 mLineNo = (PRUint32)aNode.GetSourceLineNumber();

      PRBool result = context->EvaluateString(script, mURL, mLineNo, &val);
      
      NS_IF_RELEASE(mDocURL);

      NS_RELEASE(context);
      NS_RELEASE(owner);
    }
  }

  return rv;
}

// 3 ways to load a style sheet: inline, style src=, link tag
// XXX What does nav do if we have SRC= and some style data inline?
// XXX This code and ProcessSCRIPTTag share alot in common; clean that up!

nsresult
HTMLContentSink::ProcessSTYLETag(const nsIParserNode& aNode)
{
  nsresult rv = NS_OK;
  PRInt32 i, ac = aNode.GetAttributeCount();

  nsString* src = nsnull;
  PRBool    isInline;

  for (i = 0; i < ac; i++) {
    const nsString& key = aNode.GetKeyAt(i);
    if (key.EqualsIgnoreCase("src")) {
      src = new nsString(aNode.GetValueAt(i));
    }
  }

  // The skipped content contains the inline style data
  const nsString& content = aNode.GetSkippedContent();

  nsIURL* url = nsnull;
  nsIUnicharInputStream* uin = nsnull;
  if (nsnull == src) {
    // Create a string to hold the data and wrap it up in a unicode
    // input stream.
    rv = NS_NewStringUnicharInputStream(&uin, new nsString(content));
    if (NS_OK != rv) {
      return rv;
    }

    // Use the document's url since the style data came from there
    url = mDocumentURL;
    NS_IF_ADDREF(url);
    isInline = PR_TRUE;
  } else {
    // src with immediate style data doesn't add up
    // XXX what does nav do?
    nsAutoString absURL;
    nsIURL* docURL = mDocument->GetDocumentURL();
    rv = NS_MakeAbsoluteURL(docURL, mBaseHREF, *src, absURL);
    if (NS_OK != rv) {
      return rv;
    }
    NS_RELEASE(docURL);
    rv = NS_NewURL(&url, nsnull, absURL);
    delete src;
    if (NS_OK != rv) {
      return rv;
    }
    PRInt32 ec;
    nsIInputStream* iin = url->Open(&ec);
    if (nsnull == iin) {
      NS_RELEASE(url);
      return (nsresult) ec;/* XXX fix url->Open */
    }
    rv = NS_NewConverterStream(&uin, nsnull, iin);
    NS_RELEASE(iin);
    if (NS_OK != rv) {
      NS_RELEASE(url);
      return rv;
    }
    isInline = PR_FALSE;
  }

  // Now that we have a url and a unicode input stream, parse the
  // style sheet.
  rv = LoadStyleSheet(url, uin, isInline);
  NS_RELEASE(uin);
  NS_RELEASE(url);

  return rv;
}

nsresult
HTMLContentSink::LoadStyleSheet(nsIURL* aURL,
                                nsIUnicharInputStream* aUIN,
                                PRBool aInline)
{
  /* XXX use repository */
  nsICSSParser* parser;
  nsresult rv = NS_NewCSSParser(&parser);
  if (NS_OK == rv) {
    if (aInline && (nsnull != mStyleSheet)) {
      parser->SetStyleSheet(mStyleSheet);
      // XXX we do probably need to trigger a style change reflow
      // when we are finished if this is adding data to the same sheet
    }
    nsIStyleSheet* sheet = nsnull;
    // XXX note: we are ignoring rv until the error code stuff in the
    // input routines is converted to use nsresult's
    parser->Parse(aUIN, aURL, sheet);
    if (nsnull != sheet) {
      if (aInline) {
        if (nsnull == mStyleSheet) {
          // Add in the sheet the first time; if we update the sheet
          // with new data (mutliple style tags in the same document)
          // then the sheet will be updated by the css parser and
          // therefore we don't need to add it to the document)
          mDocument->AddStyleSheet(sheet);
          mStyleSheet = sheet;
        }
      }
      else {
        mDocument->AddStyleSheet(sheet);
      }
      rv = NS_OK;
    } else {
      rv = NS_ERROR_OUT_OF_MEMORY;/* XXX */
    }
    NS_RELEASE(parser);
  }
  return rv;
}
