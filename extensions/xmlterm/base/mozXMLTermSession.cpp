/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "MPL"); you may not use this file
 * except in compliance with the MPL. You may obtain a copy of
 * the MPL at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the MPL is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the MPL for the specific language governing
 * rights and limitations under the MPL.
 * 
 * The Original Code is XMLterm.
 * 
 * The Initial Developer of the Original Code is Ramalingam Saravanan.
 * Portions created by Ramalingam Saravanan <svn@xmlterm.org> are
 * Copyright (C) 1999 Ramalingam Saravanan. All Rights Reserved.
 * 
 * Contributor(s):
 */

// mozXMLTermSession.cpp: implementation of mozXMLTermSession class

#include "nscore.h"
#include "prlog.h"

#include "nsCOMPtr.h"
#include "nsString.h"

#include "nsIAllocator.h"

#include "nsIDocumentViewer.h"

#include "nsICaret.h"
#include "nsITextContent.h"

#include "nsIDOMElement.h"
#include "nsIDOMSelection.h"
#include "nsIDOMText.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMRange.h"
#include "nsIDOMCharacterData.h"

#include "nsIDOMHTMLDocument.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIDOMNSRange.h"

#include "nsIHTMLContent.h"

#include "mozXMLT.h"
#include "mozILineTermAux.h"
#include "mozIXMLTerminal.h"
#include "mozXMLTermUtils.h"
#include "mozXMLTermSession.h"

/////////////////////////////////////////////////////////////////////////
// mozXMLTermSession definition
/////////////////////////////////////////////////////////////////////////
static const char* kWhitespace=" \b\t\r\n";

const char* const mozXMLTermSession::sessionElementNames[] = {
  "session",
  "entry",
  "input",
  "output",
  "prompt",
  "command",
  "stdin",
  "stdout",
  "stderr"
};

const char* const mozXMLTermSession::sessionEventNames[] = {
  "click",
  "dblclick"
};

const char* const mozXMLTermSession::metaCommandNames[] = {
  "",
  "",
  "http",
  "js",
  "tree",
  "ls"
};

const char* const mozXMLTermSession::fileTypeNames[] = {
  "plainfile",
  "directory",
  "executable"
};

const char* const mozXMLTermSession::treeActionNames[] = {
  "^",
  "v",
  "<",
  ">",
  "A",
  "H"
};

mozXMLTermSession::mozXMLTermSession() :
  mInitialized(PR_FALSE),
  mXMLTerminal(nsnull),
  mPresShell(nsnull),
  mDOMDocument(nsnull),

  mBodyNode(nsnull),
  mSessionNode(nsnull),
  mCurrentDebugNode(nsnull),

  mStartEntryNode(nsnull),
  mCurrentEntryNode(nsnull),

  mMaxHistory(50),
  mStartEntryNumber(0),
  mCurrentEntryNumber(0),

  mEntryHasOutput(false),

  mPromptSpanNode(nsnull),
  mCommandSpanNode(nsnull),
  mInputTextNode(nsnull),

  mOutputBlockNode(nsnull),
  mOutputDisplayNode(nsnull),
  mOutputTextNode(nsnull),

  mXMLTermStream(nsnull),

  mMetaCommandType(NO_META_COMMAND),

  mOutputDisplayType(NO_NODE),
  mOutputMarkupType(PLAIN_TEXT),

  mAutoDetect(FIRST_LINE),

  mLineBreakNeeded(false),
  mFirstOutputLine(false),

  mPreTextIncomplete(""),
  mPreTextBuffered(""),
  mPreTextDisplayed(""),

  mRestoreInputEcho(false),

  mShellPrompt(""),
  mPromptHTML(""),
  mFragmentBuffer("")

{
}


mozXMLTermSession::~mozXMLTermSession()
{
  if (mInitialized) {
    Finalize();
  }
}


// Initialize XMLTermSession
NS_IMETHODIMP mozXMLTermSession::Init(mozIXMLTerminal* aXMLTerminal,
                                      nsIPresShell* aPresShell,
                                      nsIDOMDocument* aDOMDocument)
{
  XMLT_LOG(mozXMLTermSession::Init,30,("\n"));

  if (mInitialized)
    return NS_ERROR_ALREADY_INITIALIZED;

  if (!aXMLTerminal || !aPresShell || !aDOMDocument)
      return NS_ERROR_NULL_POINTER;

  mXMLTerminal = aXMLTerminal;    // containing XMLTerminal; no addref
  mPresShell = aPresShell;        // presentation shell; no addref
  mDOMDocument = aDOMDocument;    // DOM document; no addref

  nsresult result = NS_OK;

  // Show the caret
  nsCOMPtr<nsICaret> caret;
  if (NS_SUCCEEDED(mPresShell->GetCaret(getter_AddRefs(caret)))) {
  	caret->SetCaretVisible(PR_TRUE);
  	caret->SetCaretReadOnly(PR_FALSE);
  }

  mPresShell->SetCaretEnabled(PR_TRUE);

  nsCOMPtr<nsIDOMHTMLDocument> vDOMHTMLDocument
                                             (do_QueryInterface(mDOMDocument));
  if (!vDOMHTMLDocument)
    return NS_ERROR_FAILURE;

  // Locate document body node
  nsCOMPtr<nsIDOMNodeList> nodeList;
  nsAutoString bodyTag = "body";
  result = vDOMHTMLDocument->GetElementsByTagName(bodyTag,
                                                  getter_AddRefs(nodeList));
  if (NS_FAILED(result) || !nodeList)
    return NS_ERROR_FAILURE;

  PRUint32 count;
  nodeList->GetLength(&count);
  PR_ASSERT(count==1);

  result = nodeList->Item(0, getter_AddRefs(mBodyNode));
  if (NS_FAILED(result) || !mBodyNode)
    return NS_ERROR_FAILURE;

  // Use body node as session node by default
  mSessionNode = mBodyNode;

  nsCOMPtr<nsIDOMElement> sessionElement;
  nsAutoString sessionID = sessionElementNames[SESSION_ELEMENT];
  result = vDOMHTMLDocument->GetElementById(sessionID,
                                            getter_AddRefs(sessionElement));

  if (NS_SUCCEEDED(result) && sessionElement) {
    // Specific session node
    mSessionNode = do_QueryInterface(sessionElement);
  }

  mCurrentDebugNode = mSessionNode;

  // Create preface element to display initial output
  result = NewPreface();
  if (NS_FAILED(result))
    return NS_ERROR_FAILURE;

#if 0
  nsAutoString prefaceText ("Preface");
  nsAutoString nullStyle ("");
  result = AppendOutput(prefaceText, nullStyle, true);
#endif

  mInitialized = PR_TRUE;

  XMLT_LOG(mozXMLTermSession::Init,31,("exiting\n"));
  return result;
}


// De-initialize XMLTermSession
NS_IMETHODIMP mozXMLTermSession::Finalize(void)
{

  mOutputBlockNode = nsnull;
  mOutputDisplayNode = nsnull;
  mOutputTextNode = nsnull;

  mXMLTermStream = nsnull;

  mPromptSpanNode = nsnull;
  mCommandSpanNode = nsnull;
  mInputTextNode = nsnull;

  mStartEntryNode = nsnull;
  mCurrentEntryNode = nsnull;

  mBodyNode = nsnull;
  mSessionNode = nsnull;
  mCurrentDebugNode = nsnull;

  mXMLTerminal = nsnull;
  mPresShell = nsnull;
  mDOMDocument = nsnull;

  mInitialized = PR_FALSE;

  return NS_OK;
}


/** Preprocesses user input before it is transmitted to LineTerm
 * @param aString (inout) input data to be preprocessed
 * @param consumed (output) true if input data has been consumed
 */
NS_IMETHODIMP mozXMLTermSession::Preprocess(const nsString& aString,
                                            PRBool& consumed)
{

  consumed = false;

  if (mMetaCommandType == TREE_META_COMMAND) {
    if (aString.Length() == 1) {
      // Navigate the DOM tree from keyboard
      PRUnichar uch = aString.CharAt(0);

      XMLT_LOG(mozXMLTermSession::Preprocess,60,("char=0x%x\n", uch));

      consumed = true;
      switch (uch) {
      case U_CTL_B:
        TraverseDOMTree(stderr, mBodyNode, mCurrentDebugNode,
                        TREE_MOVE_LEFT);
        break;

      case U_CTL_F:
        TraverseDOMTree(stderr, mBodyNode, mCurrentDebugNode,
                        TREE_MOVE_RIGHT);
        break;

      case U_CTL_N:
        TraverseDOMTree(stderr, mBodyNode, mCurrentDebugNode,
                        TREE_MOVE_DOWN);
        break;

      case U_CTL_P:
        TraverseDOMTree(stderr, mBodyNode, mCurrentDebugNode,
                        TREE_MOVE_UP);
        break;

      case U_A_CHAR:
      case U_a_CHAR:
        TraverseDOMTree(stderr, mBodyNode, mCurrentDebugNode,
                        TREE_PRINT_ATTS);
        break;

      case U_H_CHAR:
      case U_h_CHAR:
        TraverseDOMTree(stderr, mBodyNode, mCurrentDebugNode,
                        TREE_PRINT_HTML);
        break;

      case U_Q_CHAR:
      case U_q_CHAR:
      case U_CTL_C:
        // End of keyboard command sequence; reset debug node to session node
        mCurrentDebugNode = mSessionNode;
        mMetaCommandType = NO_META_COMMAND;
        break;

      default:
        break;
      }
    }
  }

  return NS_OK;
}


/** Reads all available data from LineTerm and displays it;
 * returns when no more data is available.
 * @param lineTermAux LineTermAux object to read data from
 * @param processedData (output) true if any data was processed
 */
NS_IMETHODIMP mozXMLTermSession::ReadAll(mozILineTermAux* lineTermAux,
                                         PRBool& processedData)
{
  PRInt32 opcodes, opvals, buf_row, buf_col;
  PRUnichar *buf_str, *buf_style;
  PRBool newline, streamData;
  nsAutoString bufString, bufStyle;

  XMLT_LOG(mozXMLTermSession::ReadAll,60,("\n"));

  processedData = false;

  if (lineTermAux == nsnull)
    return NS_ERROR_FAILURE;

  nsresult result = NS_OK;
  PRBool flushOutput = false;

  PRBool metaNextCommand = false;

  for (;;) {
    // NOTE: Remember to de-allocate buf_str and buf_style
    //       using nsAllocator::Free, if opcodes != 0
    result = lineTermAux->ReadAux(&opcodes, &opvals, &buf_row, &buf_col,
                                  &buf_str, &buf_style);
    if (NS_FAILED(result))
      break;

    XMLT_LOG(mozXMLTermSession::ReadAll,62,
           ("opcodes=0x%x, mMetaCommandType=%d, mEntryHasOutput=%d\n",
            opcodes, mMetaCommandType, mEntryHasOutput));

    if (opcodes == 0) break;

    processedData = true;

    streamData = (opcodes & LTERM_STREAMDATA_CODE);
    newline = (opcodes & LTERM_NEWLINE_CODE);

    // Copy character/style strings
    bufString = buf_str;
    bufStyle = buf_style;

    // De-allocate buf_str, buf_style using nsAllocator::Free
    nsAllocator::Free(buf_str);
    nsAllocator::Free(buf_style);

    if (streamData) {
      // Process stream data
      if (mMetaCommandType == NO_META_COMMAND) {
        mMetaCommandType = STREAM_META_COMMAND;

        // Disable input echo
        lineTermAux->SetEchoFlag(false);
        mRestoreInputEcho = true;

        // Determine effective stream URL and default markup type
        nsAutoString streamURL;
        OutputMarkupType streamMarkupType;
        PRBool streamIsSecure = (opcodes & LTERM_COOKIESTR_CODE);

        if (streamIsSecure) {
          // Secure stream, i.e., prefixed with cookie; fragments allowed
          streamURL = "chrome://xmlterm/content/xmltblank.html";

          if (opcodes & LTERM_JSSTREAM_CODE) {
            // Javascript stream 
            streamMarkupType = JS_FRAGMENT;

          } else {
            // HTML/XML stream
            streamMarkupType = HTML_FRAGMENT;
          }

        } else {
          // Insecure stream; treat as text fragment
          streamURL = "http://in.sec.ure";
          streamMarkupType = TEXT_FRAGMENT;
        }

        if (!(opcodes & LTERM_JSSTREAM_CODE) &&
            (opcodes & LTERM_DOCSTREAM_CODE)) {
          // Stream contains complete document (not Javascript)

          if (opcodes & LTERM_XMLSTREAM_CODE) {
            streamMarkupType = XML_DOCUMENT;
          } else {
            streamMarkupType = HTML_DOCUMENT;
          }
        }

        // Initialize stream output
        result = InitStream(streamURL, streamMarkupType, streamIsSecure);
        if (NS_FAILED(result))
          return result;
      }

      // Process stream output
      bufStyle = "";
      result = ProcessOutput(bufString, bufStyle, false, true);
      if (NS_FAILED(result))
        return result;

      if (newline) {
        if (!mEntryHasOutput) {
          // Start of command output
          mEntryHasOutput = true;
        }

        // Break stream output display
        result = BreakOutput();
        if (NS_FAILED(result))
          return result;

        mMetaCommandType = NO_META_COMMAND;
        flushOutput = true;
      }

    } else {
      // Process non-stream data
      PRBool promptLine, inputLine, metaCommand, completionRequested;

      flushOutput = true;

      inputLine = (opcodes & LTERM_INPUT_CODE);
      promptLine = (opcodes & LTERM_PROMPT_CODE);
      metaCommand = (opcodes & LTERM_META_CODE);
      completionRequested = (opcodes & LTERM_COMPLETION_CODE);

      nsAutoString promptStr ("");
      PRInt32 promptLength = 0;

      if (promptLine) {
        // Count prompt characters
        const PRUnichar *styleVals = bufStyle.GetUnicode();
        const PRInt32 bufLength = bufStyle.Length();

        for (promptLength=0; promptLength<bufLength; promptLength++) {
          if (styleVals[promptLength] != LTERM_PROMPT_STYLE)
            break;
        }

        XMLT_LOG(mozXMLTermSession::ReadAll,62,
                 ("bufLength=%d, promptLength=%d, styleVals[0]=0x%x\n",
                  bufLength, promptLength, styleVals[0]));

        PR_ASSERT(promptLength > 0);

        // Extract prompt string
        bufString.Left(promptStr, promptLength);

        if ( (promptLength < bufLength) &&
             !inputLine &&
             !promptStr.Equals(mShellPrompt) ) {
          // Ignore the mismatched prompt in the output line
          int j;
          promptLine = 0;

          for (j=0; j<promptLength; j++)
            bufStyle.SetCharAt((UNICHAR) LTERM_STDOUT_STYLE, j);

        } else {
          // Remove prompt chars/style from buffer strings
          bufString.Cut(0, promptLength);
          bufStyle.Cut(0, promptLength);

          // Save prompt string
          mShellPrompt = promptStr;
        }
      }

      if (!metaCommand && inputLine) {
        if (metaNextCommand) {
          // Echo of transmitted meta command
          metaNextCommand = false;

        } else {
          // No meta command; enable input echo
          mMetaCommandType = NO_META_COMMAND;

          if (mRestoreInputEcho) {
            lineTermAux->SetEchoFlag(true);
            mRestoreInputEcho = false;
          }
        }
      }

      if (metaCommand && !completionRequested) {
        // Identify meta command type

        // Eliminate leading spaces/TABs
        nsAutoString metaLine = bufString;
        metaLine.Trim(kWhitespace, PR_TRUE, PR_FALSE);

        int delimOffset = metaLine.FindChar((PRUnichar) ':');
        PR_ASSERT(delimOffset >= 0);

        XMLT_LOG(mozXMLTermSession::ReadAll,62, ("delimOffset=%d\n", delimOffset));

        if (delimOffset == 0) {
          // Default to HTTP protocol
          mMetaCommandType = HTTP_META_COMMAND;

        } else {
          // Identify meta command type
          mMetaCommandType = NO_META_COMMAND;

          nsAutoString temString;
          metaLine.Left(temString, delimOffset);

          PRInt32 j;
          for (j=NO_META_COMMAND+1; j<META_COMMAND_TYPES; j++) {
            if (temString.Equals(metaCommandNames[j])) {
              mMetaCommandType = (MetaCommandType) j;
              break;
            }
          }
        }

        XMLT_LOG(mozXMLTermSession::ReadAll,62,("mMetaCommandType=%d\n",
                                               mMetaCommandType));

        // Extract command arguments
        int argChars = metaLine.Length() - delimOffset - 1;
        nsAutoString commandArgs;
        metaLine.Right(commandArgs, argChars);

        // Eliminate leading spaces/TABs
        commandArgs.Trim(kWhitespace, PR_TRUE, PR_FALSE);

        // Display meta command
        if (mEntryHasOutput) {
          // Break previous output display
          result = BreakOutput();

          // Create new entry block
          result = NewEntry(promptStr);
          if (NS_FAILED(result))
            return result;
        }

        // Display input and position cursor
        PRInt32 cursorCol = 0;
        result = lineTermAux->GetCursorColumn(&cursorCol);

        // Remove prompt offset
        cursorCol -= promptLength;
        if (cursorCol < 0) cursorCol = 0;

        XMLT_LOG(mozXMLTermSession::ReadAll,62,("cursorCol=%d\n", cursorCol));

        result = DisplayInput(bufString, bufStyle, cursorCol);
        if (NS_FAILED(result))
          return NS_ERROR_FAILURE;

        if (newline && mXMLTerminal) {
          // Complete meta command; XMLterm instantiated
          nsAutoString metaCommandOutput = "";

          switch (mMetaCommandType) {

          case HTTP_META_COMMAND:
            {
              // Display URL using IFRAME
              nsAutoString url = "http:";
              url.Append(commandArgs);
              nsAutoString width = "100%";
              nsAutoString height = "100";
              result = NewIFrame(mOutputBlockNode, mCurrentEntryNumber,
                                 2, url, width, height);
              if (NS_FAILED(result))
                return result;

            }
            break;

          case JS_META_COMMAND:
            {
              // Execute JavaScript command
              result = mozXMLTermUtils::ExecuteScript(mDOMDocument,
                                                      commandArgs,
                                                      metaCommandOutput);
              if (NS_FAILED(result))
                metaCommandOutput = "Error in executing JavaScript command\n";

              nsCAutoString cstrout = metaCommandOutput;
              printf("mozXMLTermSession::ReadAll, JS output=%s\n",
                     cstrout.GetBuffer());

            }
            break;

          case TREE_META_COMMAND:
            XMLT_WARNING("\nTraverseDOMTree: use arrow keys; A for attributes; H for HTML; Q to quit\n");
            break;

          case LS_META_COMMAND:
            {
              // Disable input echo and transmit command
              lineTermAux->SetEchoFlag(false);
              nsAutoString lsCommand ("");

              if (commandArgs.Length() > 0) {
                lsCommand += "cd ";
                lsCommand += commandArgs;
                lsCommand += ";";
              }

              lsCommand += "ls -dF `pwd`/*\n";

              //mXMLTerminal->SendText(lsCommand);

              /* Set flag to recognize transmitted command */
              metaNextCommand = true;
              mRestoreInputEcho = true;
            }
            break;

          default:
            break;
          }

          if (mMetaCommandType == JS_META_COMMAND) {
            // Display metacommand output
            mEntryHasOutput = true;

            XMLT_LOG(mozXMLTermSession::ReadAll,62,("metaCommandOutput\n"));
            // Check metacommand output for markup (secure)
            result = AutoDetectMarkup(metaCommandOutput, true, true);
            if (NS_FAILED(result))
              return result;

            nsAutoString nullStyle ("");
            result = ProcessOutput(metaCommandOutput, nullStyle, true,
                                   mOutputMarkupType != PLAIN_TEXT);
            if (NS_FAILED(result))
              return result;

            // Break metacommand output display
            result = BreakOutput();
          }

          // Reset newline flag
          newline = false;
        }

        // Clear the meta command from the string nuffer
        bufString = "";
        bufStyle = "";
      }

      if (promptLine) {
        // Prompt line
        if (mEntryHasOutput) {
          // Break previous output display
          result = BreakOutput();

          // Create new entry block
          result = NewEntry(promptStr);
          if (NS_FAILED(result))
            return result;
        }

        // Display input and position cursor
        PRInt32 cursorCol = 0;
        result = lineTermAux->GetCursorColumn(&cursorCol);

        // Remove prompt offset
        cursorCol -= promptLength;
        if (cursorCol < 0) cursorCol = 0;

        XMLT_LOG(mozXMLTermSession::ReadAll,62,("cursorCol=%d\n", cursorCol));

        result = DisplayInput(bufString, bufStyle, cursorCol);
        if (NS_FAILED(result))
          return NS_ERROR_FAILURE;

        if (newline) {
          // Start of command output
          // (this is needed to properly handle commands with no output!)
          mEntryHasOutput = true;
          mFirstOutputLine = true;

        }

      } else {
        // Not prompt line
        if (!mEntryHasOutput) {
          // Start of command output
          mEntryHasOutput = true;
          mFirstOutputLine = true;
        }

        if (newline) {
          // Complete line; check for markup (insecure)
          result = AutoDetectMarkup(bufString, mFirstOutputLine, false);
          if (NS_FAILED(result))
            return result;

          // Not first output line anymore
          mFirstOutputLine = false;
        }

        if (mOutputMarkupType == PLAIN_TEXT) {
          // Display plain text output
          result = ProcessOutput(bufString, bufStyle, newline, false);
          if (NS_FAILED(result))
            return result;

        } else if (newline) {
          // Process autodetected stream output (complete lines only)
          bufStyle = "";
          result = ProcessOutput(bufString, bufStyle, true, true);
          if (NS_FAILED(result))
            return result;
        }
      }
    }
  }

  XMLT_LOG(mozXMLTermSession::ReadAll,62,(" result=%d\n", result));

  if (NS_FAILED(result)) {
    // Close LineTerm
    lineTermAux->CloseAux();
    return NS_ERROR_FAILURE;
  }

  if (flushOutput) {
    // Flush output, splitting off incomplete line
    result = FlushOutput(SPLIT_INCOMPLETE_FLUSH);

    // printf("mozXMLTermSession::ReadAll (flush)\n");
    result = mPresShell->ScrollSelectionIntoView(SELECTION_NORMAL,
                                                 SELECTION_FOCUS_REGION);

    // Scroll frame (ignore result)
    //    result = ScrollBottomLeft();
  }

  return NS_OK;
}


/** Displays ("echoes") input text string with style and positions cursor
 * @param aString string to be displayed
 * @param aStyle style values for string (see lineterm.h)
 * @param cursorCol cursor column
 */
NS_IMETHODIMP mozXMLTermSession::DisplayInput(const nsString& aString,
                                              const nsString& aStyle,
                                              PRInt32 cursorCol)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::DisplayInput,70,("cursorCol=%d\n", cursorCol));

  result = SetDOMText(mInputTextNode, aString);
  if (NS_FAILED(result))
    return NS_ERROR_FAILURE;

  char* temCString = aString.ToNewCString();
  XMLT_LOG(mozXMLTermSession::DisplayInput,72,
           ("aString=%s\n", temCString));
  nsCRT::free(temCString);

  // Collapse selection and position cursor
  nsCOMPtr<nsIDOMSelection> selection;

  result = mPresShell->GetSelection(SELECTION_NORMAL,
                                    getter_AddRefs(selection));
  if (NS_FAILED(result) || !selection)
    return NS_ERROR_FAILURE;

  if ((cursorCol > 0) || (mPromptHTML.Length() > 0)) {
    result = selection->Collapse(mInputTextNode, cursorCol);

  } else {
    // WORKAROUND for cursor positioning at end of prompt
    nsCOMPtr<nsIDOMNode> promptTextNode;
    result = mPromptSpanNode->GetFirstChild(getter_AddRefs(promptTextNode));

    if (NS_SUCCEEDED(result)) {
      nsCOMPtr<nsIDOMText> domText (do_QueryInterface(promptTextNode));

      if (domText) {
        PRUint32 promptLength;
        result = domText->GetLength(&promptLength);
        if (NS_SUCCEEDED(result)) {
          XMLT_LOG(mozXMLTermSession::DisplayInput,72,
                   ("promptLength=%d\n", promptLength));
          result = selection->Collapse(promptTextNode, promptLength);
        }
      }
    }
  }

  NS_ASSERTION((NS_SUCCEEDED(result)),
                 "selection could not be collapsed after insert.");

  return NS_OK;
}


/** Autodetects markup in current output line
 * @param aString string to be displayed
 * @param firstOutputLine true if this is the first output line
 * @param secure true if output data is secure
 *               (usually true for metacommand output only)
 */
NS_IMETHODIMP mozXMLTermSession::AutoDetectMarkup(const nsString& aString,
                                                  PRBool firstOutputLine,
                                                  PRBool secure)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::AutoDetectMarkup,70,("firstOutputLine=0x%x\n",
                                                   firstOutputLine));

  // If autodetect disabled or not plain text, do nothing
  if ((mAutoDetect == NO_MARKUP) ||
      ((mAutoDetect == FIRST_LINE) && !firstOutputLine) ||
      (mOutputMarkupType != PLAIN_TEXT))
    return NS_OK;

  OutputMarkupType newMarkupType = PLAIN_TEXT;

  // Copy string and trim leading spaces/backspaces/tabs
  nsAutoString str = aString;
  
  str.Trim(kWhitespace, PR_TRUE, PR_FALSE);

  if (str.First() == U_LESSTHAN) {
    // Markup tag detected
    str.CompressWhitespace();
    str.Append(" ");

    if ( (str.Find("<!DOCTYPE HTML",PR_TRUE) == 0) ||
         (str.Find("<BASE ",PR_TRUE) == 0) ||
         (str.Find("<HTML>",PR_TRUE) == 0) ) {
      // HTML document
      newMarkupType = HTML_DOCUMENT;

    } else if (str.Find("<?xml ",PR_FALSE) == 0) {
      // XML document
      newMarkupType = XML_DOCUMENT;

    } else {
      // HTML fragment
      if (secure) {
        // Secure HTML fragment
        newMarkupType = HTML_FRAGMENT;
      } else {
        // Insecure; treat as text fragment for security reasons
        newMarkupType = TEXT_FRAGMENT;
      }
    }


  } else if (firstOutputLine && str.Find("Content-Type",PR_TRUE) == 0) {
    // Possible MIME content type header
    str.StripWhitespace();
    if (str.Find("Content-Type:text/html",PR_TRUE) == 0) {
      // MIME content type header for HTML document
      newMarkupType = HTML_DOCUMENT;
    }
  }

  if (newMarkupType != PLAIN_TEXT) {
    // Markup found; initialize (insecure) stream
    nsAutoString streamURL = "http://in.sec.ure";
    result = InitStream(streamURL, newMarkupType, false);
    if (NS_FAILED(result))
      return result;

  } else {
    // No markup found; assume rest of output is plain text
    mOutputMarkupType = PLAIN_TEXT;
  }

  XMLT_LOG(mozXMLTermSession::AutoDetectMarkup,71,("mOutputMarkupType=%d\n",
                                                   mOutputMarkupType));

  return NS_OK;
}


/** Initializes display of stream output with specified markup type
 * @param streamURL effective URL of stream output
 * @param streamMarkupType stream markup type
 * @param streamIsSecure true if stream is secure
 */
NS_IMETHODIMP mozXMLTermSession::InitStream(const nsString& streamURL,
                                            OutputMarkupType streamMarkupType,
                                            PRBool streamIsSecure)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::InitStream,70,("streamMarkupType=%d\n",
                                             streamMarkupType));

  // Break previous output display
  result = BreakOutput();
  if (NS_FAILED(result))
    return result;

  if ((streamMarkupType == TEXT_FRAGMENT) ||
      (streamMarkupType == JS_FRAGMENT) ||
      (streamMarkupType == HTML_FRAGMENT)) {
    // Initialize fragment buffer
    mFragmentBuffer = "";

  } else {
    // Create IFRAME to display stream document
    nsAutoString src = "about:blank";
    nsAutoString width = "100%";
    nsAutoString height = "10";
    PRInt32 frameBorder = 0;

    if (!streamIsSecure)
      frameBorder = 2;

    result = NewIFrame(mOutputBlockNode, mCurrentEntryNumber,
                       frameBorder, src, width, height);

    if (NS_FAILED(result))
      return result;

    result = NS_NewXMLTermStream(getter_AddRefs(mXMLTermStream));
    if (NS_FAILED(result))
      return result;


    nsCOMPtr<nsIWebShell> webShell;
    result = mXMLTerminal->GetWebShell(getter_AddRefs(webShell));
    if (NS_FAILED(result) || !webShell)
      return NS_ERROR_FAILURE;

    nsCOMPtr<nsIDOMWindow> outerDOMWindow;
    result = mozXMLTermUtils::ConvertWebShellToDOMWindow(webShell,
                                              getter_AddRefs(outerDOMWindow));

    if (NS_FAILED(result) || !outerDOMWindow) {
      fprintf(stderr,
                "mozXMLTermSession::InitStream: Failed to convert webshell\n");
      return NS_ERROR_FAILURE;
    }

    // Initialize markup handling
    nsCAutoString iframeName = "iframet";
    //iframeName.Append(mCurrentEntryNumber,10);

    nsCAutoString contentType;
    switch (streamMarkupType) {

    case HTML_DOCUMENT:
      contentType = "text/html";
      break;

    case XML_DOCUMENT:
      contentType = "text/xml";
      break;

    default:
      PR_ASSERT(0);
      break;
    }

    nsCAutoString url ( streamURL );
    result = mXMLTermStream->Open(outerDOMWindow, iframeName.GetBuffer(),
                                  url.GetBuffer(),
                                  contentType.GetBuffer(), 800);
    if (NS_FAILED(result)) {
      fprintf(stderr, "mozXMLTerminal::Activate: Failed to open stream\n");
      return result;
    }

  }

  mOutputMarkupType = streamMarkupType;

  return NS_OK;
}


/** Breaks output display by flushing and deleting incomplete lines */
NS_IMETHODIMP mozXMLTermSession::BreakOutput(void)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::BreakOutput,70,("mOutputMarkupType=%d\n",
                                              mOutputMarkupType));

  if (!mEntryHasOutput)
    return NS_OK;

  switch (mOutputMarkupType) {

  case TEXT_FRAGMENT:
    {
      // Display text fragment using new SPAN node
      nsCOMPtr<nsIDOMNode> spanNode, textNode;
      nsAutoString tagName = "span";
      nsAutoString elementName = "stream";
      result = NewElementWithText(tagName, elementName, -1,
                                  mOutputBlockNode, spanNode, textNode);

      if (NS_FAILED(result) || !spanNode || !textNode)
        return NS_ERROR_FAILURE;

      // Append node
      nsCOMPtr<nsIDOMNode> resultNode;
      result = mOutputBlockNode->AppendChild(spanNode,
                                             getter_AddRefs(resultNode));

      // Display text
      result = SetDOMText(textNode, mFragmentBuffer);
      if (NS_FAILED(result))
        return result;

      mFragmentBuffer = "";
      break;
    }

  case JS_FRAGMENT:
    {
      // Execute JS fragment
      nsAutoString jsOutput = "";
      result = mozXMLTermUtils::ExecuteScript(mDOMDocument,
                                              mFragmentBuffer,
                                              jsOutput);
      if (NS_FAILED(result))
        jsOutput = "Error in JavaScript execution\n";

      mFragmentBuffer = "";

      if (jsOutput.Length() > 0) {
        // Display JS output as HTML fragment
        result = InsertFragment(jsOutput, mOutputBlockNode,
                                mCurrentEntryNumber);
        if (NS_FAILED(result))
          return result;
      }
    }

    break;

  case HTML_FRAGMENT:
    // Display HTML fragment
    result = InsertFragment(mFragmentBuffer, mOutputBlockNode,
                            mCurrentEntryNumber);
    if (NS_FAILED(result))
      return result;

    mFragmentBuffer = "";
    break;

  case HTML_DOCUMENT:
  case XML_DOCUMENT:
    // Close HTML/XML document
    result = mXMLTermStream->Close();
    if (NS_FAILED(result)) {
      fprintf(stderr, "mozXMLTermSession::BreakOutput: Failed to close stream\n");
      return result;
    }
    mXMLTermStream = nsnull;
    break;

  default:
    // Flush plain text output, clearing any incomplete input line
    result = FlushOutput(CLEAR_INCOMPLETE_FLUSH);
    if (NS_FAILED(result))
      return result;

    mPreTextBuffered = "";
    mPreTextDisplayed = "";
    mOutputDisplayNode = nsnull;
    mOutputDisplayType = NO_NODE;
    mOutputTextNode = nsnull;
    break;
  }

  // Revert to plain text type
  mOutputMarkupType = PLAIN_TEXT;

  return NS_OK;
}


/** Processes output string with specified style
 * @param aString string to be processed
 * @param aStyle style values for string (see lineterm.h)
 *               (if it is a null string, STDOUT style is assumed)
 * @param newline true if this is a complete line of output
 * @param streamOutput true if string represents stream output
 */
NS_IMETHODIMP mozXMLTermSession::ProcessOutput(const nsString& aString,
                                               const nsString& aStyle,
                                               PRBool newline,
                                               PRBool streamOutput)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::ProcessOutput,70,
           ("newline=%d, streamOutput=%d\n", newline, streamOutput));

  if ((mMetaCommandType == LS_META_COMMAND) && newline) {
    // Display hypertext directory listing
    result = AppendLineLS(aString, aStyle);
    if (NS_FAILED(result))
      return NS_ERROR_FAILURE;

    return NS_OK;

  } else {
    // Not meta command

    switch (mOutputMarkupType) {

    case TEXT_FRAGMENT:
    case JS_FRAGMENT:
    case HTML_FRAGMENT:
      // Append complete lines to fragment buffer
      if (newline || streamOutput) {
        mFragmentBuffer += aString;
        if (newline)
          mFragmentBuffer += '\n';
      }

      break;

    case HTML_DOCUMENT:
    case XML_DOCUMENT:
      // Write complete lines to document stream

      if (newline || streamOutput) {
        nsAutoString str = aString;
        if (newline)
          str.Append("\n");

        result = mXMLTermStream->Write(str.GetUnicode());
        if (NS_FAILED(result)) {
          fprintf(stderr, "mozXMLTermSession::ProcessOutput: Failed to write to stream\n");
          return result;
        }
      }
      break;

    default:
      // Display plain text output, complete or incomplete lines
      PR_ASSERT(!streamOutput);
      result = AppendOutput(aString, aStyle, newline);
      if (NS_FAILED(result))
        return NS_ERROR_FAILURE;
      break;
    }

    return NS_OK;
  }
}


/** Appends text string to output buffer
 *  (appended text may need to be flushed for it to be actually displayed)
 * @param aString string to be processed
 * @param aStyle style values for string (see lineterm.h)
 *               (if it is a null string, STDOUT style is assumed)
 * @param newline true if this is a complete line of output
 */
NS_IMETHODIMP mozXMLTermSession::AppendOutput(const nsString& aString,
                                              const nsString& aStyle,
                                              PRBool newline)
{
  nsresult result;

  const PRInt32   strLength   = aString.Length();
  const PRInt32   styleLength = aStyle.Length();
  const PRUnichar *strStyle   = aStyle.GetUnicode();

  XMLT_LOG(mozXMLTermSession::AppendOutput,70,("\n"));

  // Check if line has uniform style
  PRUnichar allStyles = LTERM_STDOUT_STYLE;
  PRUnichar uniformStyle = LTERM_STDOUT_STYLE;

  if (styleLength > 0) {
    PRInt32 j;
    allStyles = strStyle[0];
    uniformStyle = strStyle[0];

    for (j=1; j<strLength; j++) {
      allStyles |= strStyle[j];
      if (strStyle[j] != strStyle[0]) {
        uniformStyle = 0;
      }
    }
  }

  XMLT_LOG(mozXMLTermSession::AppendOutput,72,
           ("mOutputDisplayType=%d, uniformStyle=0x%x, newline=%d\n",
            mOutputDisplayType, uniformStyle, newline));

  char* temCString = aString.ToNewCString();
  XMLT_LOG(mozXMLTermSession::AppendOutput,72,
           ("aString=%s\n", temCString));
  nsCRT::free(temCString);

#ifdef NO_WORKAROUND
  // Do not use PRE text
  if (0) {
#else
  if ((uniformStyle == LTERM_STDOUT_STYLE) ||
      (uniformStyle == LTERM_STDERR_STYLE)) {
#endif
    // Pure STDOUT/STDERR data; display as preformatted block
    OutputDisplayType preDisplayType;
    nsAutoString elementName = "";

    if (uniformStyle == LTERM_STDERR_STYLE) {
      preDisplayType = PRE_STDERR_NODE;
      elementName = sessionElementNames[STDERR_ELEMENT];
      XMLT_LOG(mozXMLTermSession::AppendOutput,72, ("PRE_STDERR_NODE\n"));

    } else {
      preDisplayType = PRE_STDOUT_NODE;
      elementName = sessionElementNames[STDOUT_ELEMENT];
      XMLT_LOG(mozXMLTermSession::AppendOutput,72, ("PRE_STDOUT_NODE\n"));
    }

    if (mOutputDisplayType != preDisplayType) {
      // Flush incomplete line
      result = FlushOutput(CLEAR_INCOMPLETE_FLUSH);

      // Create PRE display node
      nsCOMPtr<nsIDOMNode> preNode, textNode;
      nsAutoString tagName = "pre";

      result = NewElementWithText(tagName, elementName, -1,
                                  mOutputBlockNode, preNode, textNode);

      if (NS_FAILED(result) || !preNode || !textNode)
        return NS_ERROR_FAILURE;

      XMLT_LOG(mozXMLTermSession::AppendOutput,72,
               ("Creating new PRE node\n"));

      // Append node
      nsCOMPtr<nsIDOMNode> resultNode;
      result = mOutputBlockNode->AppendChild(preNode,
                                             getter_AddRefs(resultNode));

      mOutputDisplayType = preDisplayType;
      mOutputDisplayNode = preNode;
      mOutputTextNode = textNode;

      // Display incomplete line
      result = SetDOMText(mOutputTextNode, aString);
      if (NS_FAILED(result))
        return NS_ERROR_FAILURE;

      // Initialize PRE text string buffers
      mPreTextDisplayed = aString;
      mPreTextBuffered = "";
    }

    // Save incomplete line
    mPreTextIncomplete = aString;

    if (newline) {
      // Complete line; append to buffer
      if (mPreTextBuffered.Length() > 0) {
        mPreTextBuffered += '\n';
      }
      mPreTextBuffered += mPreTextIncomplete;
      mPreTextIncomplete = "";
    }

    mLineBreakNeeded = false;

    XMLT_LOG(mozXMLTermSession::AppendOutput,72,
             ("mPreTextDisplayed.Length()=%d, mPreTextBuffered.Length()=%d\n",
              mPreTextDisplayed.Length(), mPreTextBuffered.Length()));

  } else {
    // Create uniform style SPAN display node
    OutputDisplayType spanDisplayType;
    nsAutoString elementName = "";

    if (uniformStyle == LTERM_STDERR_STYLE) {
      // STDERR style
      spanDisplayType = SPAN_STDERR_NODE;
      elementName = sessionElementNames[STDERR_ELEMENT];
      XMLT_LOG(mozXMLTermSession::AppendOutput,72,("SPAN_STDERR_NODE\n"));

    } else {
      // STDOUT style (the default output style)
      spanDisplayType = SPAN_STDOUT_NODE;
      elementName = sessionElementNames[STDOUT_ELEMENT];
      XMLT_LOG(mozXMLTermSession::AppendOutput,72,("SPAN_STDOUT_NODE\n"));
    }

    PRBool preDisplay = (mOutputDisplayType == PRE_STDOUT_NODE) ||
                        (mOutputDisplayType == PRE_STDERR_NODE);

    if (preDisplay) {
      // Flush buffer, clearing incomplete line
      result = FlushOutput(CLEAR_INCOMPLETE_FLUSH);
    }

    if (mLineBreakNeeded) {
      // Insert line break element
      result = NewBreak(mOutputBlockNode);
      mLineBreakNeeded = false;
    }

    if (elementName.Length() > 0) {
      // Create new SPAN node
      nsCOMPtr<nsIDOMNode> spanNode, textNode;
      nsAutoString tagName = "span";
      result = NewElementWithText(tagName, elementName, -1,
                                  mOutputBlockNode, spanNode, textNode);

      if (NS_FAILED(result) || !spanNode || !textNode)
        return NS_ERROR_FAILURE;

      XMLT_LOG(mozXMLTermSession::AppendOutput,72,
               ("Creating new SPAN node\n"));

      if (mOutputDisplayNode != nsnull) {
        // Replace node
        nsCOMPtr<nsIDOMNode> resultNode;
        result = mOutputBlockNode->ReplaceChild(spanNode, mOutputDisplayNode,
                                                getter_AddRefs(resultNode));
      } else {
        // Append node
        nsCOMPtr<nsIDOMNode> resultNode;
        result = mOutputBlockNode->AppendChild(spanNode,
                                                getter_AddRefs(resultNode));
      }

      mOutputDisplayType = spanDisplayType;
      mOutputDisplayNode = spanNode;
      mOutputTextNode = textNode;
    }

    // Display line
    result = SetDOMText(mOutputTextNode, aString);
    if (NS_FAILED(result))
      return NS_ERROR_FAILURE;

    if (newline) {
      mOutputDisplayType = NO_NODE;
      mOutputDisplayNode = nsnull;
      mOutputTextNode = nsnull;

      mLineBreakNeeded = true;
    }
  }

  return NS_OK;
}


/** Adds markup to LS output (TEMPORARY)
 * @param aString string to be processed
 * @param aStyle style values for string (see lineterm.h)
 *               (if it is a null string, STDOUT style is assumed)
 */
NS_IMETHODIMP mozXMLTermSession::AppendLineLS(const nsString& aString,
                                              const nsString& aStyle)
{
  nsresult result;

  const PRInt32   strLength   = aString.Length();
  const PRInt32   styleLength = aStyle.Length();
  const PRUnichar *strStyle   = aStyle.GetUnicode();

  // Check if line has uniform style
  PRUnichar allStyles = LTERM_STDOUT_STYLE;
  PRUnichar uniformStyle = LTERM_STDOUT_STYLE;

  if (styleLength > 0) {
    PRInt32 j;
    allStyles = strStyle[0];
    uniformStyle = strStyle[0];

    for (j=1; j<strLength; j++) {
      allStyles |= strStyle[j];
      if (strStyle[j] != strStyle[0]) {
        uniformStyle = 0;
      }
    }
  }

  XMLT_LOG(mozXMLTermSession::AppendLineLS,60,
           ("mOutputDisplayType=%d, uniformStyle=0x%x\n",
            mOutputDisplayType, uniformStyle));

  if (uniformStyle != LTERM_STDOUT_STYLE) {
    return AppendOutput(aString, aStyle, PR_TRUE);
  }

  char* temCString = aString.ToNewCString();
  XMLT_LOG(mozXMLTermSession::AppendLineLS,62,("aString=%s\n", temCString));
  nsCRT::free(temCString);

  // Add markup to directory listing
  nsAutoString markupString = "";
  PRInt32 lineLength = aString.Length();
  PRInt32 wordBegin = 0;

  while (wordBegin < lineLength) {
    // Consume any leading spaces
    while ( (wordBegin < lineLength) &&
            ((aString[wordBegin] == U_SPACE) ||
             (aString[wordBegin] == U_TAB)) ) {
      markupString += aString[wordBegin];
      wordBegin++;
    }
    if (wordBegin >= lineLength) break;

    // Locate end of word (non-space character)
    PRInt32 wordEnd = aString.FindCharInSet(kWhitespace, wordBegin);
    if (wordEnd < 0) {
      wordEnd = lineLength-1;
    } else {
      wordEnd--;
    }

    PR_ASSERT(wordEnd >= wordBegin);

    // Locate pure filename, with possible type suffix
    PRInt32 nameBegin;
    if (wordEnd > wordBegin) {
      nameBegin = aString.RFindChar(U_SLASH, PR_FALSE, wordEnd-1);
      if (nameBegin >= wordBegin) {
        nameBegin++;
      } else {
        nameBegin = wordBegin;
      }
    } else {
      nameBegin = wordBegin;
    }

    nsAutoString filename;
    aString.Mid(filename, nameBegin, wordEnd-nameBegin+1);

    FileType fileType = PLAIN_FILE;
    PRUint32 dropSuffix = 0;

    if (wordEnd > wordBegin) {
      // Determine file type from suffix character
      switch (aString[wordEnd]) {
      case U_SLASH:
        fileType = DIRECTORY_FILE;
        break;
      case U_STAR:
        fileType = EXECUTABLE_FILE;
        break;
      default:
        break;
      }

      // Discard any type suffix
      if (fileType != PLAIN_FILE)
        dropSuffix = 1;
    }

    // Extract full pathname (minus any type suffix)
    nsAutoString pathname;
    aString.Mid(pathname, wordBegin, wordEnd-wordBegin+1-dropSuffix);

    // Append to markup string
    markupString += "<span class=\"";
    markupString += fileTypeNames[fileType];
    markupString += "\"";

    int j;
    for (j=0; j<SESSION_EVENT_TYPES; j++) {
      markupString += " on";
      markupString += sessionEventNames[j];
      markupString += "=\"return ";
      markupString += sessionEventNames[j];
      markupString += "XMLTerm('";
      markupString += fileTypeNames[fileType];
      markupString += "',-1,'";
      markupString += pathname;
      markupString += "');\"";
    }

    markupString += ">";
    markupString += filename;
    markupString += "</span>";

    // Search for new word
    wordBegin = wordEnd+1;
  }

  if (mOutputDisplayType != PRE_STDOUT_NODE) {
    // Create PRE block
    nsAutoString nullString("");
    result = AppendOutput(nullString, nullString, PR_FALSE);
  }

  PR_ASSERT(mOutputDisplayNode != nsnull);
  PR_ASSERT(mOutputTextNode != nsnull);

  result = InsertFragment(markupString, mOutputDisplayNode,
                          mCurrentEntryNumber, mOutputTextNode.get());

  // Insert text node containing newline only
  nsCOMPtr<nsIDOMText> newText;
  nsAutoString newlineStr ("\n");
  result = mDOMDocument->CreateTextNode(newlineStr, getter_AddRefs(newText));
  if (NS_FAILED(result) || !newText)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMNode> newTextNode = do_QueryInterface(newText);
  nsCOMPtr<nsIDOMNode> resultNode;
  result = mOutputDisplayNode->InsertBefore(newTextNode, mOutputTextNode,
                                            getter_AddRefs(resultNode));
  if (NS_FAILED(result))
    return NS_ERROR_FAILURE;

  XMLT_LOG(mozXMLTermSession::AppendLineLS,61,("exiting\n"));

#if 0
  mCurrentDebugNode = mOutputDisplayNode;
  mMetaCommandType = TREE_META_COMMAND;
  XMLT_LOG(mozXMLTermSession::AppendLineLS,0,("tree:\n"));
#endif  /* 0 */

  return NS_OK;
}


/** Inserts HTML fragment string as child of parentNode, before specified
 * child node, or after the last child node
 * @param aString HTML fragment string to be inserted
 * @param parentNode parent node for HTML fragment
 * @param entryNumber entry number (default value = -1)
 *                   (if entryNumber >= 0, all '#' characters in aString
 *                    are substituted by entryNumber)
 * @param beforeNode child node before which to insert fragment;
 *                   if null, insert after last child node
 *                   (default value is null)
 * @param replace if true, replace beforeNode with inserted fragment
 *                (default value is false)
 */
 NS_IMETHODIMP mozXMLTermSession::InsertFragment(const nsString& aString,
                                              nsIDOMNode* parentNode,
                                              PRInt32 entryNumber,
                                              nsIDOMNode* beforeNode,
                                              PRBool replace)
{
  nsresult result;

  char* temCString = aString.ToNewCString();
  XMLT_LOG(mozXMLTermSession::InsertFragment,70,("aString=%s\n", temCString));
  nsCRT::free(temCString);

  // Get selection
  nsCOMPtr<nsIDOMSelection> selection;

  result = mPresShell->GetSelection(SELECTION_NORMAL,
                                    getter_AddRefs(selection));
  if (NS_FAILED(result) || !selection)
    return NS_ERROR_FAILURE;

  PRUint32 insertOffset = 0;

  nsCOMPtr<nsIDOMNodeList> childNodes;
  result = parentNode->GetChildNodes(getter_AddRefs(childNodes));

  if (NS_SUCCEEDED(result) && childNodes) {
    PRUint32 nChildren = 0;
    childNodes->GetLength(&nChildren);

    if(!beforeNode) {
      // Append child
      insertOffset = nChildren;

    } else {
      // Determine offset of before node
      int j;
      PRInt32 nNodes = nChildren;

      for (j=0; j<nNodes; j++) {
        nsCOMPtr<nsIDOMNode> childNode;
        result = childNodes->Item(j, getter_AddRefs(childNode));
        if ((NS_SUCCEEDED(result)) && childNode) {
          if (childNode.get() == beforeNode) {
            insertOffset = j;
            break;
          }
        }
      }
    }
  }

  // Collapse selection to insertion point
  result = selection->Collapse(parentNode, insertOffset);
  if (NS_FAILED(result))
    return result;

  // Get the first range in the selection
  nsCOMPtr<nsIDOMRange> firstRange;
  result = selection->GetRangeAt(0, getter_AddRefs(firstRange));
  if (NS_FAILED(result) || !firstRange)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMNSRange> nsrange (do_QueryInterface(firstRange));
  if (!nsrange)
    return NS_ERROR_FAILURE;

  XMLT_LOG(mozXMLTermSession::InsertFragment,62,("Creating Fragment\n"));

  nsCOMPtr<nsIDOMDocumentFragment> docfrag;
  result = nsrange->CreateContextualFragment(aString, getter_AddRefs(docfrag));
  if (NS_FAILED(result) || !docfrag)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMNode> docfragNode (do_QueryInterface(docfrag));
  if (!docfragNode)
    return NS_ERROR_FAILURE;

  // Insert child nodes of document fragment before PRE text node
  nsCOMPtr<nsIDOMNode> childNode;
  result = docfragNode->GetFirstChild(getter_AddRefs(childNode));
  if (NS_FAILED(result) || !childNode)
    return NS_ERROR_FAILURE;

  while (childNode) {
    // Get next sibling prior to insertion
    nsCOMPtr<nsIDOMNode> nextChild;
    result = childNode->GetNextSibling(getter_AddRefs(nextChild));

    XMLT_LOG(mozXMLTermSession::InsertFragment,72,("Inserting child node ...\n"));

    //  nsCOMPtr<nsIContent> childContent (do_QueryInterface(childNode));
    //    if (childContent) childContent->List(stderr);

    // Insert child node
    nsCOMPtr<nsIDOMNode> resultNode;

    PRBool replaceTem = replace;
    if (beforeNode) {
      if (replaceTem) {
        // Replace before node
        result = parentNode->ReplaceChild(childNode, beforeNode,
                                          getter_AddRefs(resultNode));

        beforeNode = nsnull;

        nsCOMPtr<nsIDOMNode> newBeforeNode;
        result = resultNode->GetNextSibling(getter_AddRefs(newBeforeNode));

        if (NS_SUCCEEDED(result) && newBeforeNode) {
          beforeNode = newBeforeNode.get();
          replaceTem = false;
        }

      } else {
        // Insert before specified node
        result = parentNode->InsertBefore(childNode, beforeNode,
                                          getter_AddRefs(resultNode));
      }
    } else {
      // Append child
      result = parentNode->AppendChild(childNode, getter_AddRefs(resultNode));
    }
    if (NS_FAILED(result))
      return result;

    // Refresh attributes of inserted child node (deep)
    DeepRefreshAttributes(resultNode, entryNumber);

    childNode = nextChild;
  }

  return NS_OK;

}


/** Deep refresh of selected attributes for DOM elements
 * (WORKAROUND for inserting HTML fragments properly)
 * @param domNode DOM node of branch to be refreshed
 * @param entryNumber entry number (default value = -1)
 *                   (if entryNumber >= 0, all '#' characters in event
 *                    handler scripts are substituted by entryNumber)
 */
NS_IMETHODIMP mozXMLTermSession::DeepRefreshAttributes(
                                  nsCOMPtr<nsIDOMNode>& domNode,
                                  PRInt32 entryNumber)
{
  static const PRInt32 REFRESH_ATTRIBUTE_NAMES = 3;
  static const char* const refreshAttributeNames[] = {
    "id",
    "onclick",
    "ondblclick"
  };

  nsresult result;
  nsAutoString numberString = "";

  if (entryNumber >= 0)
    numberString.Append(entryNumber,10);

  nsCOMPtr<nsIDOMElement> domElement = do_QueryInterface(domNode);
  if (!domElement)
    return NS_OK;

  int j;
  for (j=0; j<REFRESH_ATTRIBUTE_NAMES; j++) {
    nsAutoString attName = refreshAttributeNames[j];
    nsAutoString attValue;

    result = domElement->GetAttribute(attName, attValue);
    if (NS_SUCCEEDED(result) && (attValue.Length() > 0)) {

      if (entryNumber >= 0) {
        // Search for number character
        PRInt32 numberOffset = attValue.FindChar((PRUnichar) '#');

        if (numberOffset >= 0) {
          // Substitute number sign with supplied number
          attValue.Cut(numberOffset,1);
          attValue.Insert(numberString, numberOffset);
        }
      }

      // Refresh attribute value
      domElement->SetAttribute(attName, attValue);

      XMLT_LOG(mozXMLTermSession::DeepRefreshEventAttributes,82,
               ("Refreshing on%s attribute\n",sessionEventNames[j] ));
    }
  }

  // Iterate over all child nodes for deep refresh
  nsCOMPtr<nsIDOMNode> child;
  result = domNode->GetFirstChild(getter_AddRefs(child));
  if (NS_FAILED(result))
    return NS_OK;

  while (child) {
    DeepRefreshAttributes(child, entryNumber);

    nsCOMPtr<nsIDOMNode> temp = child;
    result = temp->GetNextSibling(getter_AddRefs(child));
    if (NS_FAILED(result))
      break;
  }

  return NS_OK;
}


/** Forces display of data in output buffer
 * @param flushAction type of flush action: display, split-off, clear, or
 *                                          close incomplete lines
 */
NS_IMETHODIMP mozXMLTermSession::FlushOutput(FlushActionType flushAction)
{
  nsresult result;

  if (!mEntryHasOutput)
    return NS_OK;

  XMLT_LOG(mozXMLTermSession::FlushOutput,70,
          ("flushAction=%d, mOutputDisplayType=%d\n",
           flushAction, mOutputDisplayType));

  PRBool preDisplay = (mOutputDisplayType == PRE_STDOUT_NODE) ||
                      (mOutputDisplayType == PRE_STDERR_NODE);

  if (preDisplay) {
    // PRE text display
    OutputDisplayType preDisplayType = mOutputDisplayType;
    nsAutoString preTextSplit = "";

    if (flushAction != DISPLAY_INCOMPLETE_FLUSH) {
      // Split/clear/close incomplete line

      if (flushAction == SPLIT_INCOMPLETE_FLUSH) {
        // Move incomplete text to new PRE element
        preTextSplit = mPreTextIncomplete;

      } else if (flushAction == CLOSE_INCOMPLETE_FLUSH) {
        // Move incomplete text into buffer
        mPreTextBuffered += mPreTextIncomplete;
      }

      // Clear incomplete PRE text
      mPreTextIncomplete = "";

      if (mPreTextBuffered.Length() == 0) {
        // Remove last text node
        nsCOMPtr<nsIDOMNode> resultNode;
        result = mOutputDisplayNode->RemoveChild(mOutputTextNode,
                                             getter_AddRefs(resultNode));

        // Check if PRE node has any child nodes
        PRBool hasChildNodes = true;
        result = mOutputDisplayNode->HasChildNodes(&hasChildNodes);

        if (!hasChildNodes) {
          // No child nodes left; Delete PRE node itself
          nsCOMPtr<nsIDOMNode> resultNode2;
          result = mOutputBlockNode->RemoveChild(mOutputDisplayNode,
                                                 getter_AddRefs(resultNode));
        }

        mOutputDisplayNode = nsnull;
        mOutputDisplayType = NO_NODE;
        mOutputTextNode = nsnull;
      }
    }

    if (mOutputDisplayNode != nsnull) {
      // Update displayed PRE text
      nsAutoString outString = mPreTextBuffered;
      outString += mPreTextIncomplete;

      if (outString != mPreTextDisplayed) {
        // Display updated buffer
        mPreTextDisplayed = outString;

        XMLT_LOG(mozXMLTermSession::FlushOutput,72,
                 ("mOutputTextNode=%d\n", (mOutputTextNode != nsnull)));

        result = SetDOMText(mOutputTextNode, mPreTextDisplayed);
        if (NS_FAILED(result))
          return NS_ERROR_FAILURE;
      }

      if (flushAction != DISPLAY_INCOMPLETE_FLUSH) {
        // Split/clear/close incomplete line
        mOutputDisplayNode = nsnull;
        mOutputDisplayType = NO_NODE;
        mOutputTextNode = nsnull;

        if ( (flushAction == SPLIT_INCOMPLETE_FLUSH) &&
             (preTextSplit.Length() > 0) ) {
          // Create new PRE element with incomplete text
          nsAutoString styleStr = "";

          if (preDisplayType == PRE_STDERR_NODE) {
            styleStr += (PRUnichar) LTERM_STDERR_STYLE;
          } else {
            styleStr += (PRUnichar) LTERM_STDOUT_STYLE;
          }

          XMLT_LOG(mozXMLTermSession::FlushOutput,72,("splitting\n"));

          AppendOutput(preTextSplit, styleStr, false);
        }
      }
    }

  } else if (mOutputDisplayNode != nsnull) {
    // Non-PRE node
    if (flushAction == CLEAR_INCOMPLETE_FLUSH) {
      // Clear incomplete line info
      nsCOMPtr<nsIDOMNode> resultNode;
      result = mOutputBlockNode->RemoveChild(mOutputDisplayNode,
                                             getter_AddRefs(resultNode));
      mOutputDisplayNode = nsnull;
      mOutputDisplayType = NO_NODE;
      mOutputTextNode = nsnull;

    } else if (flushAction == CLOSE_INCOMPLETE_FLUSH) {
      mOutputDisplayNode = nsnull;
      mOutputDisplayType = NO_NODE;
      mOutputTextNode = nsnull;
    }
  }

  XMLT_LOG(mozXMLTermSession::FlushOutput,71,("returning\n"));

  return NS_OK;
}


/** Scrolls document to align bottom and left margin with screen */
NS_IMETHODIMP mozXMLTermSession::ScrollBottomLeft(void)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::ScrollBottomLeft,70,("\n"));

  if (!mCurrentEntryNode)
    return NS_ERROR_FAILURE;

  // Scroll primary frame to bottom of view
  nsCOMPtr<nsIContent> blockContent (do_QueryInterface(mCurrentEntryNode));
  if (!blockContent)
    return NS_ERROR_FAILURE;

  nsIFrame* primaryFrame;

  result = mPresShell->GetPrimaryFrameFor(blockContent, &primaryFrame);
  if (NS_FAILED(result)) {
    return NS_ERROR_FAILURE;
  }

  result = mPresShell->ScrollFrameIntoView(primaryFrame,
                                           NS_PRESSHELL_SCROLL_BOTTOM,
                                           NS_PRESSHELL_SCROLL_LEFT);
  if (NS_FAILED(result)) {
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


/** Gets current entry (command) number
 * @param aNumber (output) current entry number
 */
NS_IMETHODIMP mozXMLTermSession::GetCurrentEntryNumber(PRInt32 *aNumber)
{
  *aNumber = mCurrentEntryNumber;
  return NS_OK;
}


// Get size of entry history buffer
NS_IMETHODIMP mozXMLTermSession::GetHistory(PRInt32 *aHistory)
{
  *aHistory = mMaxHistory;
  return NS_OK;
}


// Set size of entry history buffer
NS_IMETHODIMP mozXMLTermSession::SetHistory(PRInt32 aHistory)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::SetHistory,30,("\n"));

  if (aHistory < 1)
    aHistory = 1;

  if (mInitialized && mStartEntryNode && (aHistory < mMaxHistory)) {
    // Delete any extra entry blocks
    PRInt32 delEntries = (mCurrentEntryNumber-mStartEntryNumber)
                         - aHistory;
    PRInt32 j;
    for (j=0; j<delEntries; j++) {
      nsCOMPtr<nsIDOMNode> newStartNode;
      result = mStartEntryNode->GetNextSibling(getter_AddRefs(newStartNode));
      if (NS_FAILED(result) || !newStartNode) {
        return NS_ERROR_FAILURE;
      }

      nsCOMPtr<nsIDOMNode> resultNode;
      result = mSessionNode->RemoveChild(mStartEntryNode,
                                        getter_AddRefs(resultNode));

      if (NS_FAILED(result)) {
        return NS_ERROR_FAILURE;
      }

      mStartEntryNode = newStartNode;
      mStartEntryNumber++;
    }
  }

  mMaxHistory = aHistory;

  return NS_OK;
}


// Get HTML prompt string
NS_IMETHODIMP mozXMLTermSession::GetPrompt(PRUnichar **_aPrompt)
{
  // NOTE: Need to be sure that this may be freed by nsAllocator::Free
  *_aPrompt = mPromptHTML.ToNewUnicode();
  return NS_OK;
}


// Set HTML prompt string
NS_IMETHODIMP mozXMLTermSession::SetPrompt(const PRUnichar* aPrompt)
{
  mPromptHTML = aPrompt;
  return NS_OK;
}


/** Create a DIV element with attributes NAME="preface", CLASS="preface",
 * and ID="preface0", containing an empty text node, and append it as a
 * child of the main BODY element. Also make it the current display element.
 */
NS_IMETHODIMP mozXMLTermSession::NewPreface(void)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::NewPreface,40,("\n"));

  // Create preface element and append as child of session element
  nsCOMPtr<nsIDOMNode> divNode;
  nsAutoString tagName = "div";
  nsAutoString name = "preface";
  result = NewElement(tagName, name, 0,
                      mSessionNode, divNode);

  if (NS_FAILED(result) || !divNode)
    return NS_ERROR_FAILURE;

  mOutputBlockNode = divNode;

  mOutputDisplayType = NO_NODE;
  mOutputDisplayNode = nsnull;
  mOutputTextNode = nsnull;

  // Command output being processed
  mEntryHasOutput = true;

  return NS_OK;
}


/** Create and append a new DIV element with attributes NAME="entry",
 * CLASS="entry", and ID="entry#" as the last child of the main BODY element,
 * where "#" denotes the new entry number obtained by incrementing the
 * current entry number.
 * Inside the entry element, create a DIV element with attributes
 * NAME="input", CLASS="input", and ID="input#" containing two elements,
 * named "prompt" and "command", each containing a text node.
 * Insert the supplied prompt string into the prompt element's text node.
 * @param aPrompt prompt string to be inserted into prompt element
 */
NS_IMETHODIMP mozXMLTermSession::NewEntry(const nsString& aPrompt)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::NewEntry,50,("\n"));

  if (mCurrentEntryNumber == 0) {
    // First entry
    mCurrentEntryNumber = 1;
    mStartEntryNumber = 1;

  } else {
    // Not first entry

    // Add event attributes to current command element
    result = SetEventAttributes(sessionElementNames[COMMAND_ELEMENT],
                                mCurrentEntryNumber,
                                mCommandSpanNode);
    if (NS_FAILED(result))
      return NS_ERROR_FAILURE;

    // Increment entry number
    mCurrentEntryNumber++;

    if ((mCurrentEntryNumber - mStartEntryNumber) > mMaxHistory) {
      // Delete oldest displayed entry element

      nsCOMPtr<nsIDOMNode> newStartNode;
      result = mStartEntryNode->GetNextSibling(getter_AddRefs(newStartNode));
      if (NS_FAILED(result) || !newStartNode) {
        return NS_ERROR_FAILURE;
      }

      nsCOMPtr<nsIDOMNode> resultNode;
      result = mSessionNode->RemoveChild(mStartEntryNode,
                                      getter_AddRefs(resultNode));

      if (NS_FAILED(result)) {
        return NS_ERROR_FAILURE;
      }

      mStartEntryNode = newStartNode;
      mStartEntryNumber++;
    }
  }

  XMLT_LOG(mozXMLTermSession::NewEntry,50,
           ("%d (start=%d)\n", mCurrentEntryNumber, mStartEntryNumber));

  nsAutoString tagName, name;

  // Create "entry" element
  nsCOMPtr<nsIDOMNode> entryNode;
  tagName = "div";
  name = sessionElementNames[ENTRY_ELEMENT];
  result = NewElement(tagName, name, mCurrentEntryNumber,
                      mSessionNode, entryNode);
  if (NS_FAILED(result) || !entryNode) {
    return NS_ERROR_FAILURE;
  }

  mCurrentEntryNode = entryNode;

  if (mCurrentEntryNumber == 1) {
    mStartEntryNode = mCurrentEntryNode;
  }

  // Create "input" element containing "prompt" and "command" elements
  nsCOMPtr<nsIDOMNode> inputNode;
  tagName = "div";
  name = sessionElementNames[INPUT_ELEMENT];
  result = NewElement(tagName, name, mCurrentEntryNumber,
                      mCurrentEntryNode, inputNode);
  if (NS_FAILED(result) || !inputNode) {
    return NS_ERROR_FAILURE;
  }

  nsAutoString classAttribute;

  // Create prompt element
  nsCOMPtr<nsIDOMNode> newPromptSpanNode;
  tagName = "span";
  name = sessionElementNames[PROMPT_ELEMENT];
  result = NewElement(tagName, name, mCurrentEntryNumber,
                      inputNode, newPromptSpanNode);
  if (NS_FAILED(result) || !newPromptSpanNode) {
    return NS_ERROR_FAILURE;
  }

  mPromptSpanNode = newPromptSpanNode;

  // Add event attributes to prompt element
  result = SetEventAttributes(name, mCurrentEntryNumber,
                                mPromptSpanNode);

  if (mPromptHTML.Length() == 0) {
    // Create text node as child of prompt element
    nsCOMPtr<nsIDOMNode> textNode;
    result = NewTextNode(mPromptSpanNode, textNode);

    if (NS_FAILED(result) || !textNode)
      return NS_ERROR_FAILURE;

    // Set prompt text
    result = SetDOMText(textNode, aPrompt);
    if (NS_FAILED(result))
      return NS_ERROR_FAILURE;

  } else {
    // User-specified HTML prompt
    result = InsertFragment(mPromptHTML, mPromptSpanNode,
                            mCurrentEntryNumber);
  }

  // Create command element
  nsCOMPtr<nsIDOMNode> newCommandSpanNode;
  tagName = "span";
  name = sessionElementNames[COMMAND_ELEMENT];
  result = NewElement(tagName, name, mCurrentEntryNumber,
                      inputNode, newCommandSpanNode);
  if (NS_FAILED(result) || !newCommandSpanNode) {
    return NS_ERROR_FAILURE;
  }

  mCommandSpanNode = newCommandSpanNode;

  // Create text node as child of command element
  nsCOMPtr<nsIDOMNode> textNode2;
  result = NewTextNode(mCommandSpanNode, textNode2);

  if (NS_FAILED(result) || !textNode2)
    return NS_ERROR_FAILURE;

  mInputTextNode = textNode2;

  // Create output element and append as child of current entry element
  nsCOMPtr<nsIDOMNode> divNode;
  tagName = "div";
  name = sessionElementNames[OUTPUT_ELEMENT];
  result = NewElement(tagName, name, mCurrentEntryNumber,
                      mCurrentEntryNode, divNode);

  if (NS_FAILED(result) || !divNode)
    return NS_ERROR_FAILURE;

  mOutputBlockNode = divNode;

  mOutputDisplayType = NO_NODE;
  mOutputDisplayNode = nsnull;
  mOutputTextNode = nsnull;

  // No command output processed yet
  mEntryHasOutput = false;

  return NS_OK;
}


/** Append a BR element as the next child of specified parent.
 * @param parentNode parent node for BR element
 */
NS_IMETHODIMP mozXMLTermSession::NewBreak(nsIDOMNode* parentNode)
{
  nsresult result;
  nsAutoString tagName = "br";

  XMLT_LOG(mozXMLTermSession::NewBreak,60,("\n"));

  // Create "br" element and append as child of specified parent
  nsCOMPtr<nsIDOMNode> brNode;
  nsAutoString name = "";
  result = NewElement(tagName, name, -1, parentNode, brNode);

  if (NS_FAILED(result) || !brNode)
    return NS_ERROR_FAILURE;

  return NS_OK;
}


/** Create an empty block element with tag name tagName with attributes
 * NAME="name", CLASS="name", and ID="name#", and appends it as a child of
 * the specified parent. ("#" denotes the specified number)
 * Also create an empty text node inside the new block element.
 * @param tagName tag name of element
 * @param name name and class of element
 *             (If zero-length string, then no attributes are set)
 * @param number numeric suffix for element ID
 *             (If < 0, no ID attribute is defined)
 * @param parentNode parent node for element
 * @param blockNode (output) block-level DOM node for created element
 * @param textNode (output) child text DOM node of element
 */
NS_IMETHODIMP mozXMLTermSession::NewElementWithText(const nsString& tagName,
                                      const nsString& name, PRInt32 number,
                                      nsIDOMNode* parentNode,
                                      nsCOMPtr<nsIDOMNode>& blockNode,
                                      nsCOMPtr<nsIDOMNode>& textNode)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::NewElementWithText,80,("\n"));

  // Create block element
  result = NewElement(tagName, name, number, parentNode, blockNode);
  if (NS_FAILED(result) || !blockNode)
    return NS_ERROR_FAILURE;

  // Create text node as child of block element
  result = NewTextNode(blockNode, textNode);

  if (NS_FAILED(result) || !textNode)
    return NS_ERROR_FAILURE;

  return NS_OK;
}


/** Creates an empty anchor (A) element with tag name tagName with attributes
 * CLASS="classAttribute", and ID="classAttribute#", and appends it as a
 * child of the specified parent. ("#" denotes the specified number)
 * @param classAttribute class attribute of anchor element
 *             (If zero-length string, then no attributes are set)
 * @param number numeric suffix for element ID
 *             (If < 0, no ID attribute is defined)
 * @param parentNode parent node for element
 * @param anchorNode (output) DOM node for created anchor element
 */
NS_IMETHODIMP mozXMLTermSession::NewAnchor(const nsString& classAttribute,
                                           PRInt32 number,
                                           nsIDOMNode* parentNode,
                                           nsCOMPtr<nsIDOMNode>& anchorNode)
{
  nsresult result;
  nsAutoString tagName("a");

  XMLT_LOG(mozXMLTermSession::NewAnchor,80,("\n"));

  // Create anchor
  nsCOMPtr<nsIDOMElement> newElement;
  result = mDOMDocument->CreateElement(tagName, getter_AddRefs(newElement));
  if (NS_FAILED(result) || !newElement)
    return NS_ERROR_FAILURE;

  // Set element attributes
  nsAutoString hrefAtt("href");
  nsAutoString hrefVal("#");
  newElement->SetAttribute(hrefAtt, hrefVal);

  if (classAttribute.Length() > 0) {
    nsAutoString classStr("class");
    newElement->SetAttribute(classStr, classAttribute);

    if (number >= 0) {
      nsAutoString idAtt("id");
      nsAutoString idVal(classAttribute);
      idVal.Append(number,10);
      newElement->SetAttribute(idAtt, idVal);
    }
  }

  // Append child to parent
  nsCOMPtr<nsIDOMNode> newBlockNode = do_QueryInterface(newElement);
  result = parentNode->AppendChild(newBlockNode, getter_AddRefs(anchorNode));
  if (NS_FAILED(result) || !anchorNode)
    return NS_ERROR_FAILURE;

  return NS_OK;
}


/** Creates an empty block element with tag name tagName with attributes
 * NAME="name", CLASS="name", and ID="name#", and appends it as a child of
 * the specified parent. ("#" denotes the specified number)
 * @param tagName tag name of element
 * @param name name and class of element
 *             (If zero-length string, then no attributes are set)
 * @param number numeric suffix for element ID
 *             (If < 0, no ID attribute is defined)
 * @param parentNode parent node for element
 * @param blockNode (output) block-level DOM node for created element
 */
NS_IMETHODIMP mozXMLTermSession::NewElement(const nsString& tagName,
                                     const nsString& name, PRInt32 number,
                                     nsIDOMNode* parentNode,
                                     nsCOMPtr<nsIDOMNode>& blockNode)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::NewElement,80,("\n"));

  // Create element
  nsCOMPtr<nsIDOMElement> newElement;
  result = mDOMDocument->CreateElement(tagName, getter_AddRefs(newElement));
  if (NS_FAILED(result) || !newElement)
    return NS_ERROR_FAILURE;

  if (name.Length() > 0) {
    // Set attributes
    nsAutoString classAtt("class");
    nsAutoString classVal(name);
    newElement->SetAttribute(classAtt, classVal);

    nsAutoString nameAtt("name");
    nsAutoString nameVal(name);
    newElement->SetAttribute(nameAtt, nameVal);

    if (number >= 0) {
      nsAutoString idAtt("id");
      nsAutoString idVal(name);
      idVal.Append(number,10);
      newElement->SetAttribute(idAtt, idVal);
    }
  }

  // Append child to parent
  nsCOMPtr<nsIDOMNode> newBlockNode = do_QueryInterface(newElement);
  result = parentNode->AppendChild(newBlockNode, getter_AddRefs(blockNode));
  if (NS_FAILED(result) || !blockNode)
    return NS_ERROR_FAILURE;

  return NS_OK;
}


/** Creates a new DOM text node, and appends it as a child of the
 * specified parent.
 * @param parentNode parent node for element
 * @param textNode (output) created text DOM node
 */
NS_IMETHODIMP mozXMLTermSession::NewTextNode( nsIDOMNode* parentNode,
                                       nsCOMPtr<nsIDOMNode>& textNode)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::NewTextNode,80,("\n"));

  // Create text node
  nsCOMPtr<nsIDOMText> newText;
  nsAutoString nullStr("");
  result = mDOMDocument->CreateTextNode(nullStr, getter_AddRefs(newText));
  if (NS_FAILED(result) || !newText)
    return NS_ERROR_FAILURE;

  // Append child to parent
  nsCOMPtr<nsIDOMNode> newTextNode = do_QueryInterface(newText);
  result = parentNode->AppendChild(newTextNode, getter_AddRefs(textNode));
  if (NS_FAILED(result))
    return NS_ERROR_FAILURE;

  return NS_OK;
}


/** Creates a new IFRAME element with attribute NAME="iframe#",
 * and appends it as a child of the specified parent.
 * ("#" denotes the specified number)
 * @param parentNode parent node for element
 * @param number numeric suffix for element ID
 *             (If < 0, no name attribute is defined)
 * @param frameBorder IFRAME FRAMEBORDER attribute
 * @param src IFRAME SRC attribute
 * @param width IFRAME width attribute
 * @param height IFRAME height attribute
 */
NS_IMETHODIMP mozXMLTermSession::NewIFrame(nsIDOMNode* parentNode,
                                           PRInt32 number,
                                           PRInt32 frameBorder,
                                           const nsString& src,
                                           const nsString& width,
                                           const nsString& height)
                                           
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::NewIFrame,80,("\n"));

#if 0
  nsAutoString iframeFrag = "<iframe name='iframe";
  iframeFrag.Append(number,10);
  iframeFrag.Append("' frameborder=")
  iframeFrag.Append(frameBorder,10);
  iframeFrag.Append(" src='");
  iframeFrag.Append(src)
  iframeFrag.Append("'> </iframe>\n");
  result = InsertFragment(iframeFrag, parentNode, number);
  if (NS_FAILED(result))
    return result;

  return NS_OK;
#else
  // Create IFRAME element
  nsCOMPtr<nsIDOMElement> newElement;
  nsAutoString tagName = "iframe";
  result = mDOMDocument->CreateElement(tagName, getter_AddRefs(newElement));
  if (NS_FAILED(result) || !newElement)
    return NS_ERROR_FAILURE;

  nsAutoString attName, attValue;

  // Set attributes
  if (number >= 0) {
    attName = "name";
    attValue = "iframe";
    attValue.Append(number,10);
    newElement->SetAttribute(attName, attValue);
  }

  attName = "frameborder";
  attValue = "";
  attValue.Append(frameBorder,10);
  newElement->SetAttribute(attName, attValue);

  if (src.Length() > 0) {
    // Set SRC attribute
    attName = "src";
    newElement->SetAttribute(attName, src);
  }

  if (width.Length() > 0) {
    // Set WIDTH attribute
    attName = "width";
    newElement->SetAttribute(attName, width);
  }

  if (height.Length() > 0) {
    // Set HEIGHT attribute
    attName = "height";
    newElement->SetAttribute(attName, height);
  }

  // Append child to parent
  nsCOMPtr<nsIDOMNode> iframeNode;
  nsCOMPtr<nsIDOMNode> newNode = do_QueryInterface(newElement);
  result = parentNode->AppendChild(newNode, getter_AddRefs(iframeNode));
  if (NS_FAILED(result) || !iframeNode)
    return NS_ERROR_FAILURE;

  return NS_OK;
#endif
}


/** Add event attributes (onclick, ondblclick, ...) to DOM node
 * @param name name of DOM node (supplied as argument to the event handler)
 * @param number entry number (supplied as argument to the event handler)
 * @param domNode DOM node to be modified 
 */
NS_IMETHODIMP mozXMLTermSession::SetEventAttributes(const nsString& name,
                                                    PRInt32 number,
                                             nsCOMPtr<nsIDOMNode>& domNode)
{
  nsresult result;

  nsCOMPtr <nsIDOMElement> domElement = do_QueryInterface(domNode);
  if (!domElement)
    return NS_ERROR_FAILURE;

  int j;
  for (j=0; j<SESSION_EVENT_TYPES; j++) {
    nsAutoString attName("on");
    attName += sessionEventNames[j];

    nsAutoString attValue("return ");
    attValue += sessionEventNames[j];
    attValue += "XMLTerm('";
    attValue += name;
    attValue += "','";
    attValue.Append(number,10);
    attValue += "','');";

    result = domElement->SetAttribute(attName, attValue);
    if (NS_FAILED(result))
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


/** Sets text content of a DOM node to supplied string
 * @param textNode DOM text node to be modified
 * @param aString string to be inserted
 */
NS_IMETHODIMP mozXMLTermSession::SetDOMText(nsCOMPtr<nsIDOMNode>& textNode,
                                            const nsString& aString)
{
  nsresult result;

  nsCOMPtr<nsIDOMText> domText (do_QueryInterface(textNode));
  if (!domText)
    return NS_ERROR_FAILURE;

  result = domText->SetData(aString);

  return result;
}


/** Checks if node is a text node
 * @param aNode DOM node to be checked
 * @return true if node is a text node
 */
PRBool mozXMLTermSession::IsTextNode(nsIDOMNode *aNode)
{
  if (!aNode) {
    NS_NOTREACHED("null node passed to IsTextNode()");
    return PR_FALSE;
  }

  XMLT_LOG(mozXMLTermSession::IsTextNode,90,("\n"));

  PRUint16 nodeType;
  aNode->GetNodeType(&nodeType);
  if (nodeType == nsIDOMNode::TEXT_NODE)
    return PR_TRUE;
    
  return PR_FALSE;
}


/** Checks if node is a text, span, or anchor node
 * (i.e., allowed inside a PRE element)
 * @param aNode DOM node to be checked
 * @return true if node is a text, span or anchor node
 */
PRBool mozXMLTermSession::IsPREInlineNode(nsIDOMNode* aNode)
{
  nsresult result;
  PRBool isPREInlineNode = false;

  nsCOMPtr<nsIDOMText> domText = do_QueryInterface(aNode);

  if (domText) {
    isPREInlineNode = true;

  } else {
    nsCOMPtr<nsIDOMElement> domElement = do_QueryInterface(aNode);

    if (domElement) {
      nsAutoString tagName = "";
      result = domElement->GetTagName(tagName);
      if (NS_SUCCEEDED(result)) {
        isPREInlineNode = tagName.EqualsIgnoreCase("span") ||
                          tagName.EqualsIgnoreCase("a");
      }
    }
  }

  return isPREInlineNode;
}


/** Serializes DOM node and its content as an HTML fragment string
 * @param aNode DOM node to be serialized
 * @param indentString indentation prefix string
 * @param htmlString (output) serialized HTML fragment
 * @param deepContent if true, serialize children of node as well
 *                    (defaults to false)
 * @param insidePREnode set to true if aNode is embedded inside a PRE node
 *                      control formatting
 *                      (defaults to false)
 */
NS_IMETHODIMP mozXMLTermSession::ToHTMLString(nsIDOMNode* aNode,
                                              nsString& indentString,
                                              nsString& htmlString,
                                              PRBool deepContent,
                                              PRBool insidePRENode)
{
  nsresult result;

  XMLT_LOG(mozXMLTermSession::ToHTMLString,80,("\n"));

  nsAutoString newIndentString (indentString);
  newIndentString += "  ";

  htmlString = "";

  nsCOMPtr<nsIDOMText> domText( do_QueryInterface(aNode) );

  if (domText) {
    // Text node
    nsCOMPtr<nsIHTMLContent> htmlContent ( do_QueryInterface(aNode) );

    if (htmlContent) {
      htmlContent->ToHTMLString(htmlString);
      XMLT_LOG(mozXMLTermSession::ToHTMLString,82,("htmlContent\n"));
    } else {
      domText->GetData(htmlString);
    }

  } else {
    nsCOMPtr<nsIDOMElement> domElement = do_QueryInterface(aNode);

    if (domElement) {
      nsAutoString tagName = "";
      domElement->GetTagName(tagName);

      if (!insidePRENode) {
        htmlString += indentString;
      }
      htmlString += "<";
      htmlString += tagName;

      PRBool isPRENode = tagName.EqualsIgnoreCase("pre");

      nsCOMPtr<nsIDOMNamedNodeMap> namedNodeMap(nsnull);
      result = aNode->GetAttributes(getter_AddRefs(namedNodeMap));

      if (NS_SUCCEEDED(result) && namedNodeMap) {
        // Print all attributes
        PRUint32 nodeCount, j;
        result = namedNodeMap->GetLength(&nodeCount);

        if (NS_SUCCEEDED(result)) {
          nsCOMPtr<nsIDOMNode> attrNode;

          for (j=0; j<nodeCount; j++) {
            result = namedNodeMap->Item(j, getter_AddRefs(attrNode));

            if (NS_SUCCEEDED(result)) {
              nsCOMPtr<nsIDOMAttr> attr = do_QueryInterface(attrNode);

              if (attr) {
                nsAutoString attrName = "";
                nsAutoString attrValue = "";

                result = attr->GetName(attrName);
                if (NS_SUCCEEDED(result)) {
                  htmlString += " ";
                  htmlString += attrName;
                }

                result = attr->GetValue(attrValue);
                if (NS_SUCCEEDED(result) && (attrName.Length() > 0)) {
                  htmlString += "=\"";
                  htmlString += attrValue;
                  htmlString += "\"";
                }
              }
            }
          }
        }
      }

      if (!deepContent) {
        htmlString += ">";

      } else {
        // Iterate over all child nodes to generate deep content
        nsCOMPtr<nsIDOMNode> child;
        result = aNode->GetFirstChild(getter_AddRefs(child));

        nsAutoString htmlInner ("");
        while (child) {
          nsAutoString innerString;
          ToHTMLString(child, newIndentString, innerString, deepContent,
                       isPRENode);

          htmlInner += innerString;

          nsCOMPtr<nsIDOMNode> temp = child;
          result = temp->GetNextSibling(getter_AddRefs(child));
          if (NS_FAILED(result))
            break;
        }

        if (htmlInner.Length() > 0) {
          if (insidePRENode)
            htmlString += "\n>";
          else
            htmlString += ">\n";

          htmlString += htmlInner;

          if (!insidePRENode)
            htmlString += indentString;
        } else {
          htmlString += ">";
        }

        htmlString += "</";
        htmlString += tagName;

        if (insidePRENode)
          htmlString += "\n";
        htmlString += ">";

        if (!insidePRENode)
          htmlString += "\n";
      }
    }
  }

  return NS_OK;
}


/** Implements the "tree:" meta command to traverse DOM tree
 * @param fileStream file stream for displaying tree traversal output
 * @param rootNode root node of DOM tree
 * @param currentNode current node for traversal
 * @param treeActionCode traversal action type
 */
void mozXMLTermSession::TraverseDOMTree(FILE* fileStream,
                                 nsIDOMNode* rootNode,
                                 nsCOMPtr<nsIDOMNode>& currentNode,
                                 TreeActionCode treeActionCode)
{
  static const PRInt32 NODE_TYPE_NAMES = 12;

  static const char* const nodeTypeNames[NODE_TYPE_NAMES] = {
    "ELEMENT",
    "ATTRIBUTE",
    "TEXT",
    "CDATA_SECTION",
    "ENTITY_REFERENCE",
    "ENTITY_NODE",
    "PROCESSING_INSTRUCTION",
    "COMMENT",
    "DOCUMENT",
    "DOCUMENT_TYPE",
    "DOCUMENT_FRAGMENT",
    "NOTATION_NODE"
  };

  static const PRInt32 PRINT_ATTRIBUTE_NAMES = 2;

  static const char* const printAttributeNames[PRINT_ATTRIBUTE_NAMES] = {
    "class",
    "id"
  };

  nsresult result = NS_ERROR_FAILURE;
  nsCOMPtr<nsIDOMNode> moveNode(nsnull);
  nsCOMPtr<nsIDOMNamedNodeMap> namedNodeMap(nsnull);

  switch (treeActionCode) {
  case TREE_MOVE_UP:
    if (currentNode.get() != rootNode) {
      result = currentNode->GetParentNode(getter_AddRefs(moveNode));

      if (NS_SUCCEEDED(result) && moveNode) {
        // Move up to parent node
        currentNode = moveNode;
      }

    } else {
      fprintf(fileStream, "TraverseDOMTree: already at the root node \n");
    }
    break;

  case TREE_MOVE_DOWN:
    result = currentNode->GetFirstChild(getter_AddRefs(moveNode));

    if (NS_SUCCEEDED(result) && moveNode) {
      // Move down to child node
      currentNode = moveNode;
    } else {
      fprintf(fileStream, "TraverseDOMTree: already at a leaf node\n");
    }
    break;

  case TREE_MOVE_LEFT:
    if (currentNode.get() != rootNode) {
      result = currentNode->GetPreviousSibling(getter_AddRefs(moveNode));

      if (NS_SUCCEEDED(result) && moveNode) {
        // Move to previous sibling node
        currentNode = moveNode;
      } else {
        fprintf(fileStream, "TraverseDOMTree: already at leftmost node\n");
      }
    } else {
      fprintf(fileStream, "TraverseDOMTree: already at the root node \n");
    }
    break;

  case TREE_MOVE_RIGHT:
    if (currentNode.get() != rootNode) {
      result = currentNode->GetNextSibling(getter_AddRefs(moveNode));

      if (NS_SUCCEEDED(result) && moveNode) {
        // Move to next sibling node
        currentNode = moveNode;
      } else {
        fprintf(fileStream, "TraverseDOMTree: already at rightmost node\n");
      }
    } else {
      fprintf(fileStream, "TraverseDOMTree: already at the root node \n");
    }
    break;

  case TREE_PRINT_ATTS:
  case TREE_PRINT_HTML:
    if (true) {
      nsAutoString indentString ("");
      nsAutoString htmlString;
      ToHTMLString(currentNode, indentString, htmlString,
                   (PRBool) (treeActionCode == TREE_PRINT_HTML) );

      fprintf(fileStream, "%s:\n", treeActionNames[treeActionCode-1]);

      char* htmlCString = htmlString.ToNewCString();
      fprintf(fileStream, "%s", htmlCString);
      nsCRT::free(htmlCString);

      fprintf(fileStream, "\n");
    }
    break;

  default:
    fprintf(fileStream, "mozXMLTermSession::TraverseDOMTree - unknown action %d\n",
            treeActionCode);
  }

  if (NS_SUCCEEDED(result) && moveNode) {
    PRUint16 nodeType = 0;

    moveNode->GetNodeType(&nodeType);
    fprintf(fileStream, "%s%s: ", treeActionNames[treeActionCode-1],
                                  nodeTypeNames[nodeType-1]);

    nsCOMPtr<nsIDOMElement> domElement;
    domElement = do_QueryInterface(moveNode);
    if (domElement) {
      nsAutoString tagName = "";

      result = domElement->GetTagName(tagName);
      if (NS_SUCCEEDED(result)) {
        char* tagCString = tagName.ToNewCString();
        fprintf(fileStream, "%s", tagCString);
        nsCRT::free(tagCString);

        // Print selected attribute values
        int j;
        for (j=0; j<PRINT_ATTRIBUTE_NAMES; j++) {
          nsAutoString attName (printAttributeNames[j]);
          nsAutoString attValue;

          result = domElement->GetAttribute(attName, attValue);
          if (NS_SUCCEEDED(result) && (attValue.Length() > 0)) {
            // Print attribute value
            char* tagCString2 = attValue.ToNewCString();
            fprintf(fileStream, " %s=%s", printAttributeNames[j], tagCString2);
            nsCRT::free(tagCString2);
          }
        }
      }
    }
    fprintf(fileStream, "\n");
  }
}
