# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the SeaMonkey Installer code.
#
# The Initial Developer of the Original Code is
#   Mark Banner <bugzilla@standard8.demon.co.uk>
#
# Portions created by the Initial Developer are Copyright (C) 2006
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

!macro checkSuiteComponents
  ; If DOMi/Palm Sync/DebugQA aren't available skip the components page
  ${Unless} ${FileExists} "$EXEDIR\optional\extensions\inspector@mozilla.org"
  ${AndUnless} ${FileExists} "$EXEDIR\optional\extensions\debugQA@mozilla.org"
  ${AndUnless} ${FileExists} "$EXEDIR\optional\extensions\p@m"
    Abort
  ${EndUnless}
!macroend

!macro createSuiteComponentsIni
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Type   "label"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Text   "$(OPTIONAL_COMPONENTS_LABEL)"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Left   "0"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Right  "-1"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Top    "5"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Bottom "15"

  StrCpy $R1 2
  ; Top of checkbox
  StrCpy $R2 20
  ; Bottom of checkbox
  StrCpy $R3 30
  ; Seperation between titles/text
  StrCpy $R4 35

  ${If} ${FileExists} "$EXEDIR\optional\extensions\inspector@mozilla.org"
    ; Set the details for DOMI
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Type   "checkbox"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Text   "$(DOMI_TITLE)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Left   "15"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Top    "$R2"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Bottom "$R3"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" State  "1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Flags  "GROUP"
    ${GetSize} "$EXEDIR\optional\extensions\inspector@mozilla.org" "/S=0K" $0 $8 $9
    SectionSetSize 1 $0
    IntOp $R1 $R1 + 1
    IntOp $R2 $R2 + $R4
    IntOp $R3 $R3 + $R4
  ${Else}
    ; Hide DOMi in the components page if it isn't available.
    SectionSetText 1 ""
  ${EndIf}

  ${If} ${FileExists} "$EXEDIR\optional\extensions\debugQA@mozilla.org"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Type   "checkbox"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Text   "$(DEBUGQA_TITLE)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Left   "15"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Top    "$R2"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Bottom "$R3"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" State  "1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Flags  "GROUP"
    ${GetSize} "$EXEDIR\optional\extensions\debugQA@mozilla.org" "/S=0K" $0 $8 $9
    SectionSetSize 2 $0
    IntOp $R1 $R1 + 1
    IntOp $R2 $R2 + $R4
    IntOp $R3 $R3 + $R4
  ${Else}
    ; Hide debugQA in the components page if it isn't available.
    SectionSetText 2 ""
  ${EndIf}

  ${If} ${FileExists} "$EXEDIR\optional\extensions\p@m"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Type   "checkbox"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Text   "$(PALMSYNC_TITLE)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Left   "15"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Top    "$R2"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Bottom "$R3"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" State  "1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Flags  "GROUP"
    ${GetSize} "$EXEDIR\optional\extensions\p@m" "/S=0K" $0 $8 $9
    SectionSetSize 3 $0
    IntOp $R1 $R1 + 1
    IntOp $R2 $R2 + $Separation
    IntOp $R3 $R3 + $R4
  ${Else}
    ; Hide debugQA in the components page if it isn't available.
    SectionSetText 3 ""
  ${EndIf}

  ; Set new values for the top and bottom of labels
  ; Top of label box
  StrCpy $R2 32
  ; Bottom of label box
  StrCpy $R3 52

  ${If} ${FileExists} "$EXEDIR\optional\extensions\inspector@mozilla.org"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Type   "label"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Text   "$(DOMI_TEXT)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Left   "30"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Top    "$R2"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Bottom "$R3"
    IntOp $R1 $R1 + 1
    IntOp $R2 $R2 + $R4
    IntOp $R3 $R3 + $R4
  ${EndIf}

  ${If} ${FileExists} "$EXEDIR\optional\extensions\debugQA@mozilla.org"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Type   "label"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Text   "$(DEBUGQA_TEXT)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Left   "30"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Top    "$R2"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Bottom "$R3"
    IntOp $R1 $R1 + 1
    IntOp $R2 $R2 + $R4
    IntOp $R3 $R3 + $R4
  ${EndIf}

  ${If} ${FileExists} "$EXEDIR\optional\extensions\p@m"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Type   "label"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Text   "$(PALMSYNC_TEXT)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Left   "30"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Top    "$R2"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field $R1" Bottom "$R3"
    IntOp $R2 $R2 + $R4
    IntOp $R3 $R3 + $R4
  ${EndIf}

  WriteINIStr "$PLUGINSDIR\components.ini" "Settings" NumFields "$R1"

!macroend
