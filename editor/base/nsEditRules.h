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
 * Portions created by the Initial Developer are Copyright (C) 1998
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

#ifndef nsEditRules_h__
#define nsEditRules_h__

#define NS_IEDITRULES_IID \
{ /* a6cf911d-15b3-11d2-932e-00805f8add32 */ \
0xa6cf911d, 0x15b3, 0x11d2, \
{0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32} }

class nsPlaintextEditor;
class nsISelection;

/***************************************************************************
 * base for an object to encapsulate any additional info needed to be passed
 * to rules system by the editor
 */
class nsRulesInfo
{
  public:
  
  nsRulesInfo(int aAction) : action(aAction) {}
  virtual ~nsRulesInfo() {}
  
  int action;
};

/***************************************************************************
 * Interface of editing rules.
 *  
 */
class nsIEditRules : public nsISupports
{
public:
  static const nsIID& GetIID() { static nsIID iid = NS_IEDITRULES_IID; return iid; }
  
//Interfaces for addref and release and queryinterface
//NOTE: Use   NS_DECL_ISUPPORTS_INHERITED in any class inherited from nsIEditRules

  NS_IMETHOD Init(nsPlaintextEditor *aEditor, PRUint32 aFlags)=0;
  NS_IMETHOD BeforeEdit(PRInt32 action, nsIEditor::EDirection aDirection)=0;
  NS_IMETHOD AfterEdit(PRInt32 action, nsIEditor::EDirection aDirection)=0;
  NS_IMETHOD WillDoAction(nsISelection *aSelection, nsRulesInfo *aInfo, PRBool *aCancel, PRBool *aHandled)=0;
  NS_IMETHOD DidDoAction(nsISelection *aSelection, nsRulesInfo *aInfo, nsresult aResult)=0;
  NS_IMETHOD GetFlags(PRUint32 *aFlags)=0;
  NS_IMETHOD SetFlags(PRUint32 aFlags)=0;
  NS_IMETHOD DocumentIsEmpty(PRBool *aDocumentIsEmpty)=0;
};


#endif //nsEditRules_h__

