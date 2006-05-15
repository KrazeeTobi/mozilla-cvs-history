#!/usr/bin/perl 
#
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
# The Original Code is mozilla.org code.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1999
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

# If you run this script because the Unicode standard has been updated,
# check xpcom/string/public/nsCharTraits.h to see whether
# nsCharTraits<PRUnichar>::ASCIIToLower needs to be updated. It only
# needs to be update if the Unicode consortium adds (or removes)
# a Unicode character whose lowercase form is an ASCII character.
# Currently there are only two such characters: KELVIN SIGN and
# LATIN CAPITAL LETTER I WITH DOT ABOVE.

######################################################################
#
# Initial global variable
#
######################################################################
%utot = ();
$ui=0;
$li=0;

######################################################################
#
# Open the unicode database file
#
######################################################################
open ( UNICODATA , "< UnicodeData-Latest.txt") 
   || die "cannot find UnicodeData-Latest.txt";

######################################################################
#
# Open the output file
#
######################################################################
open ( OUT , "> ../src/casetable.h") 
  || die "cannot open output ../src/casetable.h file";

######################################################################
#
# Generate license and header
#
######################################################################
$npl = <<END_OF_NPL;
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
/* 
    DO NOT EDIT THIS DOCUMENT !!! THIS DOCUMENT IS GENERATED BY
    mozilla/intl/unicharutil/tools/gencasetable.pl
 */
END_OF_NPL
print OUT $npl;

print OUT "#include \"nscore.h\" \n\n";

######################################################################
#
# Process the file line by line
#
######################################################################
while(<UNICODATA>) {
   chop;
   ######################################################################
   #
   # Get value from fields
   #
   ######################################################################
   @f = split(/;/ , $_); 
   $c = $f[0];   # The unicode value
   $u = $f[12];  # The upper case
   $l = $f[13];  # The lower case
   $t = $f[14];  # The title case

   #
   # print $c . " | " . $u . " | " . $l . " | " . $t . "\n";
   # 

   ######################################################################
   #
   # Process title case for this entry
   #
   ######################################################################
   #
   # if upper case is not equal to title case , store into
   # %utot hash
   #
 
   if(($t ne "") && ($u ne "") && ( $u ne $t )) { 
     #
     # print $c . " | " . $u . " | " . $l . " | " . $t . "\n";
     #
     $utot{$u} = $t;
   }
   
   $cv = hex($c);     # convert the Unicode value into integer

   ######################################################################
   #
   # Process upper case for this entry
   #
   ######################################################################
   if( $u ne "") {    # if upper case exist
      $uvalue = hex($u); # convert the upper case value into integer

      ######################################################################
      # store related information into arrays
      # @ucv     - unicode value
      # @uv      - upper case value (debug only)
      # @ud      - difference between unicode and upper case 
      # @ulastd  - difference between unicode and last unicode in the entry
      ######################################################################

      $ucv[$ui] = $cv;    
      $uv[$ui] = $uvalue;
      $ud[$ui] = 0x0000FFFF & ($uvalue - $cv);

      if( $ui ne 0) { 
          $ulastd[$ui] = $cv - $ucv[$ui-1]; 
      }
      $ui++;
   }

   ######################################################################
   #
   # Process lower case for this entry
   #
   ######################################################################
   if( $l ne "") {    # if lower case exist
      $lvalue = hex($l); # convert the lower case value into integer

      ######################################################################
      # store related information into arrays
      # @lcv     - unicode value
      # @lv      - lower case value (debug only)
      # @ld      - difference between unicode and lower case 
      # @llastd  - difference between unicode and last unicode in the entry
      ######################################################################

      $lcv[$li] = $cv;    
      $lv[$li] = $lvalue;
      $ld[$li] = 0x0000FFFF & ($lvalue - $cv);

      if( $li ne 0) { 
          $llastd[$li] = $cv - $lcv[$li-1]; 
      }
      $li++;
   }
   if(( $l ne "") || ($u ne "")) {    # if lower or upper case exist
      $idx = $cv >> 13;
      $bits = 1 << (($cv >> 8) & 0x1F) ;
      $blk[$idx] |= $bits;
   }

}


######################################################################
#
# Print out all the tables
#
######################################################################

######################################################################
#
# Print out upper to title case mapping
#
######################################################################

$ttotal = 0;
print OUT "static const PRUnichar gUpperToTitle[] = { \n";
while(($upper, $title) = each(%utot)) {
   print OUT "   0x" . $upper . ",    0x" . $utot{$upper} . ", \n";
   $ttotal++;
}
print OUT "};\n\n";
print OUT "static const PRUint32 gUpperToTitleItems = $ttotal;\n\n";

######################################################################
#
# Print out gToUpper table
#
######################################################################
print OUT "static const PRUint16 gToUpper[] = \n";
print OUT "{ /*   From    To             Every   Diff   */ \n";
$utotal=0;
$ufrom = 0;   # remember the start of the output item
for ($i = 0; $i <= $#ucv; $i++)
{
  if(0 eq $i) {
     ######################################################################
     #
     # Print the first item in the array
     #
     ######################################################################
     $ufrom = $ucv[0];
     printf OUT "       0x%04x, " ,  $ucv[0];
  } else {
     ######################################################################
     #
     # Print all the item except the first and last one
     #    only print if the upper case difference is different from the 
     #    and the difference between last entry changed
     #
     ######################################################################
     if(($ud[$i] ne $ud[$i-1])  || 
        (($ufrom ne $ucv[$i-1]) && ($ulastd[$i] ne $ulastd[$i-1]))) {
 
         $every = 0;
         if($ufrom ne $ucv[$i-1])
         {
             $every = $ulastd[$i-1];
         }

         printf OUT "((0x%02x << 8) | 0x%02x), 0x%04x  ,\n",
                ($ucv[$i-1] - $ufrom), $every, $ud[$i-1];

         if((($ucv[$i-1] - $ufrom) > 255) || ($every > 255)) {
             print "WARNING!!! cannot handle block > 255 chars (Upper)\n\n";
             printf "0x%04X, 0x%04x,  0x%04x), 0x%04x \n",
                $ufrom, $ucv[$i-1], $every, $ud[$i-1];
         }

         $ufrom = $ucv[$i];                  # update the start of the item
         printf OUT "       0x%04x, " , $ufrom;
         $utotal++;
     }
  }
  if( $i eq $#ucv) {
     ######################################################################
     #
     # Print the last item in the array
     #
     ######################################################################
     printf OUT "((0x%02x << 8) | 0x%02x), 0x%04x   \n};\n\n",
            ($ucv[$i] - $ufrom), $ulastd[$i], $ud[$i];
     $utotal++;
     print OUT "static const PRUint32 gToUpperItems = $utotal;\n\n";
  }
  #
  #  printf "%4x - %4x - %4x - %4x\n", $ucv[$i], $uv[$i], $ud[$i], $ulastd[$i];
  #
}

######################################################################
#
# Print out gToLower table
#
######################################################################
print OUT "static const PRUint16 gToLower[] = \n";
print OUT "{ /*   From    To             Every   Diff   */ \n";
$ltotal=0;
$lfrom = 0;   # remember the start of the output item
for ($i = 0; $i <= $#lcv; $i++)
{
  if(0 eq $i) {
     ######################################################################
     #
     # Print the first item in the array
     #
     ######################################################################
     $lfrom = $lcv[0];
     printf OUT "       0x%04x, " ,  $lcv[0];
  } else {
     ######################################################################
     #
     # Print all the item except the first and last one
     #    only print if the lower case difference is different from the 
     #    and the difference between last entry changed
     #
     ######################################################################
     if(($ld[$i] ne $ld[$i-1])  || 
        (($lfrom ne $lcv[$i-1]) && ($llastd[$i] ne $llastd[$i-1]))) {
 
         $every = 0;
         if($lfrom ne $lcv[$i-1])
         {
             $every = $llastd[$i-1];
         }

         printf OUT "((0x%02x << 8) | 0x%02x), 0x%04x  ,\n",
                ($lcv[$i-1] - $lfrom) , $every, $ld[$i-1];

         if((($lcv[$i-1] - $lfrom) > 255) || ($every > 255)) {
             print "WARNING!!! cannot handle block > 255 chars (Lower)\n\n";
             printf "0x%04X, 0x%04x,  0x%04x, 0x%04x \n",
                $lfrom, $lcv[$i-1], $every, $ld[$i-1];
         }
         $lfrom = $lcv[$i];                  # update the start of the item
         printf OUT "       0x%04x, " , $lfrom;
         $ltotal++;
     }
  }
  if( $i eq $#lcv) {
     ######################################################################
     #
     # Print the last item in the array
     #
     ######################################################################
     printf OUT "((0x%02x << 8) | 0x%02x), 0x%04x   \n};\n\n",
            ($lcv[$i] - $lfrom), $llastd[$i], $ld[$i];
     $ltotal++;
     print OUT "static const PRUint32 gToLowerItems = $ltotal;\n\n";
  }
  #
  #  printf "%4x - %4x - %4x - %4x\n", $lcv[$i], $lv[$i], $ld[$i], $llastd[$i];
  #
}

print OUT "static const PRUint32 gCaseBlocks [8] = {\n";
for($idx=0;$idx<8;$idx++)
{
   printf OUT "0x%08X", $blk[$idx];
   if($idx != 7) {
      printf OUT ",\n";
   } else {
      printf OUT "\n";
   }
}
print OUT "};\n";


######################################################################
#
# Close files
#
######################################################################
close(UNIDATA);
close(OUT);

