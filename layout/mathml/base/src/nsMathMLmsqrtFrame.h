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
 * The Original Code is Mozilla MathML Project.
 * 
 * The Initial Developer of the Original Code is The University Of 
 * Queensland.  Portions created by The University Of Queensland are
 * Copyright (C) 1999 The University Of Queensland.  All Rights Reserved.
 * 
 * Contributor(s): 
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *   David J. Fiddes <D.J.Fiddes@hw.ac.uk>
 *   Vilya Harvey <vilya@nag.co.uk>
 */

#ifndef nsMathMLmsqrtFrame_h___
#define nsMathMLmsqrtFrame_h___

#include "nsCOMPtr.h"
#include "nsMathMLContainerFrame.h"

//
// <msqrt> -- form a fraction from two subexpressions 
//

/*
The MathML REC describes:

The <msqrt> element is used to display square roots.
The syntax for <msqrt> is:
  <msqrt> base </msqrt>

Attributes of <msqrt> and <mroot>:

None (except the attributes allowed for all MathML elements, listed in Section
2.3.4). 

The <mroot> element increments scriptlevel by 2, and sets displaystyle to
"false", within index, but leaves both attributes unchanged within base. The
<msqrt> element leaves both attributes unchanged within all its arguments.
These attributes are inherited by every element from its rendering environment,
but can be set explicitly only on <mstyle>. (See Section 3.3.4.) 
*/

/*
TODO:
*/

class nsMathMLmsqrtFrame : public nsMathMLContainerFrame {
public:
  friend nsresult NS_NewMathMLmsqrtFrame(nsIFrame** aNewFrame);

  NS_IMETHOD
  Init(nsIPresContext*  aPresContext,
       nsIContent*      aContent,
       nsIFrame*        aParent,
       nsIStyleContext* aContext,
       nsIFrame*        aPrevInFlow);

  NS_IMETHOD
  Reflow(nsIPresContext*          aPresContext,
         nsHTMLReflowMetrics&     aDesiredSize,
         const nsHTMLReflowState& aReflowState,
         nsReflowStatus&          aStatus);

  NS_IMETHOD 
  Paint(nsIPresContext*      aPresContext,
        nsIRenderingContext& aRenderingContext,
        const nsRect&        aDirtyRect,
        nsFramePaintLayer    aWhichLayer);

protected:
  nsMathMLmsqrtFrame();
  virtual ~nsMathMLmsqrtFrame();
  
  virtual PRIntn GetSkipSides() const { return 0; }

  nsMathMLChar mSqrChar, mBarChar;
};

#endif /* nsMathMLmsqrtFrame_h___ */

