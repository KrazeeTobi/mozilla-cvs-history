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
 * The Original Code is Mozilla Communicator client code.
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

#include "nsSBCharSetProber.h"
/****************************************************************
255: Control characters that usually does not exist in any text
254: Carriage/Return
253: symbol (punctuation) that does not belong to word
252: 0 - 9

*****************************************************************/

//Character Mapping Table:
unsigned char Latin2_HungarianCharToOrderMap[] =
{
255,255,255,255,255,255,255,255,255,255,254,255,255,254,255,255,  //00
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  //10
+253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,  //20
252,252,252,252,252,252,252,252,252,252,253,253,253,253,253,253,  //30
253, 28, 40, 54, 45, 32, 50, 49, 38, 39, 53, 36, 41, 34, 35, 47,
 46, 71, 43, 33, 37, 57, 48, 64, 68, 55, 52,253,253,253,253,253,
253,  2, 18, 26, 17,  1, 27, 12, 20,  9, 22,  7,  6, 13,  4,  8,
 23, 67, 10,  5,  3, 21, 19, 65, 62, 16, 11,253,253,253,253,253,
159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,
175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,
191,192,193,194,195,196,197, 75,198,199,200,201,202,203,204,205,
 79,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
221, 51, 81,222, 78,223,224,225,226, 44,227,228,229, 61,230,231,
232,233,234, 58,235, 66, 59,236,237,238, 60, 69, 63,239,240,241,
 82, 14, 74,242, 70, 80,243, 72,244, 15, 83, 77, 84, 30, 76, 85,
245,246,247, 25, 73, 42, 24,248,249,250, 31, 56, 29,251,252,253,
};

unsigned char win1250HungarianCharToOrderMap[] =
{
255,255,255,255,255,255,255,255,255,255,254,255,255,254,255,255,  //00
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  //10
+253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,  //20
252,252,252,252,252,252,252,252,252,252,253,253,253,253,253,253,  //30
253, 28, 40, 54, 45, 32, 50, 49, 38, 39, 53, 36, 41, 34, 35, 47,
 46, 72, 43, 33, 37, 57, 48, 64, 68, 55, 52,253,253,253,253,253,
253,  2, 18, 26, 17,  1, 27, 12, 20,  9, 22,  7,  6, 13,  4,  8,
 23, 67, 10,  5,  3, 21, 19, 65, 62, 16, 11,253,253,253,253,253,
161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
177,178,179,180, 78,181, 69,182,183,184,185,186,187,188,189,190,
191,192,193,194,195,196,197, 76,198,199,200,201,202,203,204,205,
 81,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
221, 51, 83,222, 80,223,224,225,226, 44,227,228,229, 61,230,231,
232,233,234, 58,235, 66, 59,236,237,238, 60, 70, 63,239,240,241,
 84, 14, 75,242, 71, 82,243, 73,244, 15, 85, 79, 86, 30, 77, 87,
245,246,247, 25, 74, 42, 24,248,249,250, 31, 56, 29,251,252,253,
};

//Model Table: 
//total sequences: 100%
//first 512 sequences: 94.7368%
//first 1024 sequences:5.2623%
//rest  sequences:     0.8894%
//negative sequences:  0.0009% 
char HungarianLangModel[] = 
{
0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,2,2,3,3,1,1,2,2,2,2,2,1,2,
3,2,2,3,3,3,3,3,2,3,3,3,3,3,3,1,2,3,3,3,3,2,3,3,1,1,3,3,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,
3,2,1,3,3,3,3,3,2,3,3,3,3,3,1,1,2,3,3,3,3,3,3,3,1,1,3,2,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
3,3,3,3,3,3,3,3,3,3,3,1,1,2,3,3,3,1,3,3,3,3,3,1,3,3,2,2,0,3,2,3,
0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,
3,3,3,3,3,3,2,3,3,3,2,3,3,2,3,3,3,3,3,2,3,3,2,2,3,2,3,2,0,3,2,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,
3,3,3,3,3,3,2,3,3,3,3,3,2,3,3,3,1,2,3,2,2,3,1,2,3,3,2,2,0,3,3,3,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,3,3,3,3,3,3,3,2,2,3,3,3,3,3,3,2,3,3,3,3,2,3,3,3,3,0,2,3,2,
0,0,0,1,1,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,3,3,3,3,3,3,3,3,1,1,1,3,3,2,1,3,2,2,3,2,1,3,2,2,1,0,3,3,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,2,2,3,3,3,3,3,1,2,3,3,3,3,1,2,1,3,3,3,3,2,2,3,1,1,3,2,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
3,3,3,3,3,3,3,3,2,2,3,3,3,3,3,2,1,3,3,3,3,3,2,2,1,3,3,3,0,1,1,2,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,2,3,3,2,3,3,3,2,0,3,2,3,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,
3,3,3,3,3,3,2,3,3,3,2,3,2,3,3,3,1,3,2,2,2,3,1,1,3,3,1,1,0,3,3,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,3,3,3,3,2,3,3,3,2,3,2,3,3,3,2,3,3,3,3,3,1,2,3,2,2,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,2,2,2,3,1,3,3,2,2,1,3,3,3,1,1,3,1,2,3,2,3,2,2,2,1,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,
3,1,1,3,3,3,3,3,1,2,3,3,3,3,1,2,1,3,3,3,2,2,3,2,1,0,3,2,0,1,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,1,3,3,3,3,3,1,2,3,3,3,3,1,1,0,3,3,3,3,0,2,3,0,0,2,1,0,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,3,3,3,3,3,2,2,3,3,2,2,2,2,3,3,0,1,2,3,2,3,2,2,3,2,1,2,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,
3,3,3,3,3,3,1,2,3,3,3,2,1,2,3,3,2,2,2,3,2,3,3,1,3,3,1,1,0,2,3,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,1,2,2,2,2,3,3,3,1,1,1,3,3,1,1,3,1,1,3,2,1,2,3,1,1,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,2,1,2,1,1,3,3,1,1,1,1,3,3,1,1,2,2,1,2,1,1,2,2,1,1,0,2,2,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,1,1,2,1,1,3,3,1,0,1,1,3,3,2,0,1,1,2,3,1,0,2,2,1,0,0,1,3,2,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,2,1,3,3,3,3,3,1,2,3,2,3,3,2,1,1,3,2,3,2,1,2,2,0,1,2,1,0,0,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
3,3,3,3,2,2,2,2,3,1,2,2,1,1,3,3,0,3,2,1,2,3,2,1,3,3,1,1,0,2,1,3,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,2,2,2,3,2,3,3,3,2,1,1,3,3,1,1,1,2,2,3,2,3,2,2,2,1,0,2,2,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
1,0,0,3,3,3,3,3,0,0,3,3,2,3,0,0,0,2,3,3,1,0,1,2,0,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,2,3,3,3,3,3,1,2,3,3,2,2,1,1,0,3,3,2,2,1,2,2,1,0,2,2,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,3,2,2,1,3,1,2,3,3,2,2,1,1,2,2,1,1,1,1,3,2,1,1,1,1,2,1,0,1,2,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
2,3,3,1,1,1,1,1,3,3,3,0,1,1,3,3,1,1,1,1,1,2,2,0,3,1,1,2,0,2,1,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,1,0,1,2,1,2,2,0,1,2,3,1,2,0,0,0,2,1,1,1,1,1,2,0,0,1,1,0,0,0,0,
1,2,1,2,2,2,1,2,1,2,0,2,0,2,2,1,1,2,1,1,2,1,1,1,0,1,0,0,0,1,1,0,
1,1,1,2,3,2,3,3,0,1,2,2,3,1,0,1,0,2,1,2,2,0,1,1,0,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,0,3,3,2,2,1,0,0,3,2,3,2,0,0,0,1,1,3,0,0,1,1,0,0,2,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,1,2,2,3,3,1,0,1,3,2,3,1,1,1,0,1,1,1,1,1,3,1,0,0,2,2,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,1,1,2,2,2,1,0,1,2,3,3,2,0,0,0,2,1,1,1,2,1,1,1,0,1,1,1,0,0,0,
1,2,2,2,2,2,1,1,1,2,0,2,1,1,1,1,1,2,1,1,1,1,1,1,0,1,1,1,0,0,1,1,
3,2,2,1,0,0,1,1,2,2,0,3,0,1,2,1,1,0,0,1,1,1,0,1,1,1,1,0,2,1,1,1,
2,2,1,1,1,2,1,2,1,1,1,1,1,1,1,2,1,1,1,2,3,1,1,1,1,1,1,1,1,1,0,1,
2,3,3,0,1,0,0,0,3,3,1,0,0,1,2,2,1,0,0,0,0,2,0,0,1,1,1,0,2,1,1,1,
2,1,1,1,1,1,1,2,1,1,0,1,1,0,1,1,1,0,1,2,1,1,0,1,1,1,1,1,1,1,0,1,
2,3,3,0,1,0,0,0,2,2,0,0,0,0,1,2,2,0,0,0,0,1,0,0,1,1,0,0,2,0,1,0,
2,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,0,1,1,1,1,1,0,1,
3,2,2,0,1,0,1,0,2,3,2,0,0,1,2,2,1,0,0,1,1,1,0,0,2,1,0,1,2,2,1,1,
2,1,1,1,1,1,1,2,1,1,1,1,1,1,0,2,1,0,1,1,0,1,1,1,0,1,1,2,1,1,0,1,
2,2,2,0,0,1,0,0,2,2,1,1,0,0,2,1,1,0,0,0,1,2,0,0,2,1,0,0,2,1,1,1,
2,1,1,1,1,2,1,2,1,1,1,2,2,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,0,1,
1,2,3,0,0,0,1,0,3,2,1,0,0,1,2,1,1,0,0,0,0,2,1,0,1,1,0,0,2,1,2,1,
1,1,0,0,0,1,0,1,1,1,1,1,2,0,0,1,0,0,0,2,0,0,1,1,1,1,1,1,1,1,0,1,
3,0,0,2,1,2,2,1,0,0,2,1,2,2,0,0,0,2,1,1,1,0,1,1,0,0,1,1,2,0,0,0,
1,2,1,2,2,1,1,2,1,2,0,1,1,1,1,1,1,1,1,1,2,1,1,0,0,1,1,1,1,0,0,1,
1,3,2,0,0,0,1,0,2,2,2,0,0,0,2,2,1,0,0,0,0,3,1,1,1,1,0,0,2,1,1,1,
2,1,0,1,1,1,0,1,1,1,1,1,1,1,0,2,1,0,0,1,0,1,1,0,1,1,1,1,1,1,0,1,
2,3,2,0,0,0,1,0,2,2,0,0,0,0,2,1,1,0,0,0,0,2,1,0,1,1,0,0,2,1,1,0,
2,1,1,1,1,2,1,2,1,2,0,1,1,1,0,2,1,1,1,2,1,1,1,1,0,1,1,1,1,1,0,1,
3,1,1,2,2,2,3,2,1,1,2,2,1,1,0,1,0,2,2,1,1,1,1,1,0,0,1,1,0,1,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,2,2,0,0,0,0,0,2,2,0,0,0,0,2,2,1,0,0,0,1,1,0,0,1,2,0,0,2,1,1,1,
2,2,1,1,1,2,1,2,1,1,0,1,1,1,1,2,1,1,1,2,1,1,1,1,0,1,2,1,1,1,0,1,
1,0,0,1,2,3,2,1,0,0,2,0,1,1,0,0,0,1,1,1,1,0,1,1,0,0,1,0,0,0,0,0,
1,2,1,2,1,2,1,1,1,2,0,2,1,1,1,0,1,2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
2,3,2,0,0,0,0,0,1,1,2,1,0,0,1,1,1,0,0,0,0,2,0,0,1,1,0,0,2,1,1,1,
2,1,1,1,1,1,1,2,1,0,1,1,1,1,0,2,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,
1,2,2,0,1,1,1,0,2,2,2,0,0,0,3,2,1,0,0,0,1,1,0,0,1,1,0,1,1,1,0,0,
1,1,0,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,0,0,1,1,1,0,1,0,1,
2,1,0,2,1,1,2,2,1,1,2,1,1,1,0,0,0,1,1,0,1,1,1,1,0,0,1,1,1,0,0,0,
1,2,2,2,2,2,1,1,1,2,0,2,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,1,0,
1,2,3,0,0,0,1,0,2,2,0,0,0,0,2,2,0,0,0,0,0,1,0,0,1,0,0,0,2,0,1,0,
2,1,1,1,1,1,0,2,0,0,0,1,2,1,1,1,1,0,1,2,0,1,0,1,0,1,1,1,0,1,0,1,
2,2,2,0,0,0,1,0,2,1,2,0,0,0,1,1,2,0,0,0,0,1,0,0,1,1,0,0,2,1,0,1,
2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,1,1,1,1,1,0,1,
1,2,2,0,0,0,1,0,2,2,2,0,0,0,1,1,0,0,0,0,0,1,1,0,2,0,0,1,1,1,0,1,
1,0,1,1,1,1,1,1,0,1,1,1,1,0,0,1,0,0,1,1,0,1,0,1,1,1,1,1,0,0,0,1,
1,0,0,1,0,1,2,1,0,0,1,1,1,2,0,0,0,1,1,0,1,0,1,1,0,0,1,0,0,0,0,0,
0,2,1,2,1,1,1,1,1,2,0,2,0,1,1,0,1,2,1,0,1,1,1,0,0,0,0,0,0,1,0,0,
2,1,1,0,1,2,0,0,1,1,1,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,0,0,2,1,0,1,
2,2,1,1,1,1,1,2,1,1,0,1,1,1,1,2,1,1,1,2,1,1,0,1,0,1,1,1,1,1,0,1,
1,2,2,0,0,0,0,0,1,1,0,0,0,0,2,1,0,0,0,0,0,2,0,0,2,2,0,0,2,0,0,1,
2,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,
1,1,2,0,0,3,1,0,2,1,1,1,0,0,1,1,1,0,0,0,1,1,0,0,0,1,0,0,1,0,1,0,
1,2,1,0,1,1,1,2,1,1,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,1,0,0,
2,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,2,0,0,0,
2,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,2,1,1,0,0,1,1,1,1,1,0,1,
2,1,1,1,2,1,1,1,0,1,1,2,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,0,1,1,1,1,1,0,0,1,1,2,1,0,0,0,1,1,0,0,0,1,1,0,0,1,0,1,0,0,0,
1,2,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,1,0,0,
2,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,2,0,0,1,0,0,1,0,1,0,0,0,
0,1,1,1,1,1,1,1,1,2,0,1,1,1,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
1,0,0,1,1,1,1,1,0,0,2,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,
0,1,1,1,1,1,1,0,1,1,0,1,0,1,1,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
1,0,0,1,1,1,0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,
0,1,1,1,1,1,0,0,1,1,0,1,0,1,0,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,1,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,1,1,0,1,0,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
2,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,0,1,0,1,0,1,1,1,0,0,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,0,1,1,1,1,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,1,1,1,1,1,0,1,1,0,1,0,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
};

SequenceModel Latin2HungarianModel = 
{
  Latin2_HungarianCharToOrderMap,
  HungarianLangModel,
  PR_TRUE,
  "ISO-8859-2"
};

SequenceModel Win1250HungarianModel = 
{
  win1250HungarianCharToOrderMap,
  HungarianLangModel,
  PR_TRUE,
  "windows-1250"
};
