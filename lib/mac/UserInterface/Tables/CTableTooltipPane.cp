/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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


#include "CTableToolTipPane.h"

#include "CStandardFlexTable.h"
#include "UGraphicGizmos.h"


//----------------------------------------------------------------------------------------
CTableToolTipAttachment::CTableToolTipAttachment(LStream* inStream)
//----------------------------------------------------------------------------------------
	: Inherited(inStream)
{
	mPaneMustBeActive = false;
}

#pragma mark -

//----------------------------------------------------------------------------------------
CTableToolTipPane::CTableToolTipPane(LStream* inStream)
	:	CToolTipPane(inStream)
//----------------------------------------------------------------------------------------
{
} // CTableToolTipPane::CTableToolTipPane

//----------------------------------------------------------------------------------------
void CTableToolTipPane::DrawSelf()
//----------------------------------------------------------------------------------------
{
	Rect theFrame;
	CalcLocalFrameRect(theFrame);
	::EraseRect(&theFrame);
	::FrameRect(&theFrame);
	UTextTraits::SetPortTextTraits(mTextDrawingStuff.mTextTraitsID);
	::TextFace(mTextDrawingStuff.style);
	::TextMode(mTextDrawingStuff.mode);
	::RGBForeColor(&mTextDrawingStuff.color);
	theFrame.left += 2;
	if (mTextDrawingStuff.encoding == CStandardFlexTable::TextDrawingStuff::eDefault)
		UGraphicGizmos::PlaceStringInRect(mTip, theFrame, teFlushDefault);
	else
		UGraphicGizmos::DrawUTF8TextString(mTip, nil, 0, theFrame, teFlushDefault);
} // CTableToolTipPane::DrawSelf
	
//----------------------------------------------------------------------------------------
Boolean CTableToolTipPane::WantsToCancel(Point inMousePort)
// Cancel if we detect that we're now in a new cell, because the tooltip has to be
// recalculated.
//----------------------------------------------------------------------------------------
{
	STableCell currentCell;
	if (GetTableAndCell(inMousePort, currentCell))
	{
		static STableCell sLastCell(0, 0);
		if (currentCell == sLastCell)
			return false;
		sLastCell = currentCell;
	}
	return true;
} //  CTableToolTipPane::WantsToCancel

//----------------------------------------------------------------------------------------
void CTableToolTipPane::CalcTipText(
	LWindow*			inOwningWindow,
	const EventRecord&	inMacEvent,
	StringPtr			outTipText)
//----------------------------------------------------------------------------------------
{
	STableCell cell;
	Point where = inMacEvent.where;
	inOwningWindow->GlobalToPortPoint(where);
	outTipText[0] = 0;
	CStandardFlexTable* table = GetTableAndCell(where, cell);
	if (!table)
		return;
	table->CalcToolTipText(cell, outTipText, mTextDrawingStuff, mTruncationOnly);
} // CTableToolTipPane::CalcTipText

//----------------------------------------------------------------------------------------
void CTableToolTipPane::CalcFrameWithRespectTo(
	LWindow*				inOwningWindow,
	const EventRecord&		inMacEvent,
	Rect& 					outPortFrame)
//----------------------------------------------------------------------------------------
{
	StTextState theTextSaver;
	UTextTraits::SetPortTextTraits(mTextDrawingStuff.mTextTraitsID);
	::TextFace(mTextDrawingStuff.style);
	::TextMode(mTextDrawingStuff.mode);
	::RGBForeColor(&mTextDrawingStuff.color);
	
	Int16 theTextHeight
		= mTextDrawingStuff.mTextFontInfo.ascent
		+ mTextDrawingStuff.mTextFontInfo.descent
		+ mTextDrawingStuff.mTextFontInfo.leading
		+ (2 * 2);
	// Int16 theTextWidth = ::StringWidth(mTip) + 4; //(2 * ::CharWidth(char_Space));
	Int16 theTextWidth ;

	if(mTextDrawingStuff.encoding == CStandardFlexTable::TextDrawingStuff::eDefault)
	{
		theTextWidth = ::StringWidth(mTip) + 4;
	}
	else
	{
		theTextWidth = UGraphicGizmos::GetUTF8TextWidth(mTip, mTip.Length()) + 4;
	}

	inOwningWindow->FocusDraw();	

	Rect theOwningPortFrame;
	mOwningPane->CalcPortFrameRect(theOwningPortFrame);

	// Find which cell the mouse is over
	STableCell cell;
	Point where = inMacEvent.where;
	inOwningWindow->GlobalToPortPoint(where);
	CStandardFlexTable* table = GetTableAndCell(where, cell);
	if (!table)
		return;
	// Get a rectangle for the text.  Normally, it's the cell rect.  But if it's the hilite cell,
	// make sure we don't cover over the icon (unless we're forced to to make it fit)
	table->FocusDraw();
	table->GetLocalCellRect(cell, outPortFrame);
	outPortFrame.right--; // that's what CStandardFlexTable::DrawCell does
	if (cell.col == table->GetHiliteColumn())
	{
		Rect hiliteRect;
		table->GetHiliteTextRect(cell.row, false, hiliteRect);
		outPortFrame.left = hiliteRect.left;
		outPortFrame.left -= 2; // looks better.
	}
		
	table->LocalToPortPoint(topLeft(outPortFrame));
	table->LocalToPortPoint(botRight(outPortFrame));
	
	Int16 textRight = outPortFrame.left + theTextWidth;
	if (mTruncationOnly && textRight <= outPortFrame.right)
	{
		outPortFrame.right = outPortFrame.left; // No tooltip
		return;
	}
	else
		outPortFrame.right = outPortFrame.left + theTextWidth;

	inOwningWindow->FocusDraw();	
	ForceInPortFrame(inOwningWindow, outPortFrame);	
} //  CTableToolTipPane::CalcFrameWithRespectTo

//----------------------------------------------------------------------------------------
CStandardFlexTable* CTableToolTipPane::GetTableAndCell(
	Point				inMousePort,
	STableCell&			outCell)
//----------------------------------------------------------------------------------------
{
	CStandardFlexTable* table = dynamic_cast<CStandardFlexTable*>(mOwningPane);
	Assert_(table);
	if (table)
	{
		table->FocusDraw();
		Point mouseLocal = inMousePort;
		table->PortToLocalPoint(mouseLocal);
		SPoint32 whereImage;
		table->LocalToImagePoint(mouseLocal, whereImage);
		table->GetCellHitBy(whereImage, outCell);
	}
	return table;
} // CTableToolTipPane::GetTableAndCell

