#!c:\perl\bin\perl
# 
# The contents of this file are subject to the Netscape Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/NPL/
#  
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#  
# The Original Code is Mozilla Communicator client code, released
# March 31, 1998.
# 
# The Initial Developer of the Original Code is Netscape
# Communications Corporation. Portions created by Netscape are
# Copyright (C) 1998-1999 Netscape Communications Corporation. All
# Rights Reserved.
# 
# Contributor(s): 
# Sean Su <ssu@netscape.com>
# 

#
# This perl script parses the input file for special variables
# in the format of $Variable$ and replace it with the appropriate
# value(s).
#
# Input: .it file
#             - which is a .ini template
#
#        version
#             - version to display on the blue background
#
#        UserAgent
#             - user agent to use in the windows registry.  should be the same as the one
#               built into the browser (ie "6.0b2 (en)")
#
#        Path to staging area
#             - path on where the seamonkey built bits are staged to
#
#        xpi path
#             - path on where xpi files will be located at
#
#        redirect file url
#             - url to where the redirect.ini file will be staged at.
#               Either ftp:// or http:// can be used
#               ie: ftp://ftp.netscape.com/pub/seamonkey
#
#        xpi url
#             - url to where the .xpi files will be staged at.
#               Either ftp:// or http:// can be used
#               ie: ftp://ftp.netscape.com/pub/seamonkey/xpi
#
#   ie: perl makecfgini.pl config.it 5.0.0.1999120608 "5.0b1 (en)" k:\windows\32bit\5.0 d:\builds\mozilla\dist\win32_o.obj\install\xpi ftp://ftp.netscape.com/pub/seamonkey/windows/32bit/x86/1999-09-13-10-M10 ftp://ftp.netscape.com/pub/seamonkey/windows/32bit/x86/1999-09-13-10-M10/xpi
#
#

# Make sure there are at least two arguments
if($#ARGV < 6)
{
  die "usage: $0 <.it file> <version> <UserAgent> <staging path> <.xpi path> <redirect file url> <xpi url>

       .it file      : input ini template file

       version       : version to be shown in setup.  Typically the same version
                       as show in mozilla.exe.

       UserAgent     : user agent to use in the windows registry.  should be the same as the one
                       built into the browser (ie \"6.0b2 (en)\")

       staging path  : path to where the components are staged at

       .xpi path     : path to where the .xpi files have been built to
                       ie: d:\\builds\\mozilla\\dist\\win32_o.obj\\install\\xpi

       redirect file : url to where the redirect.ini file will be staged at.
       url             Either ftp:// or http:// can be used
                       ie: ftp://ftp.netscape.com/pub/seamonkey
       xpi url       : url to where the .xpi files will be staged at.
                       Either ftp:// or http:// can be used
                       ie: ftp://ftp.netscape.com/pub/seamonkey/xpi
       \n";
}

$inItFile         = $ARGV[0];
$inVersion        = $ARGV[1];
$inUserAgent      = $ARGV[2];
$inStagePath      = $ARGV[3];
$inXpiPath        = $ARGV[4];
$inRedirIniUrl    = $ARGV[5];
$inUrl            = $ARGV[6];

$inDomain;
$inServerPath;

($inDomain, $inServerPath) = ParseDomainAndPath($inUrl);

# Get the name of the file replacing the .it extension with a .ini extension
@inItFileSplit    = split(/\./,$inItFile);
$outIniFile       = $inItFileSplit[0];
$outIniFile      .= ".ini";

# Open the input file
open(fpInIt, $inItFile) || die "\ncould not open $ARGV[0]: $!\n";

# Open the output file
open(fpOutIni, ">$outIniFile") || die "\nCould not open $outIniFile: $!\n";

print "\n Making $outIniFile...\n";

# While loop to read each line from input file
while($line = <fpInIt>)
{
  # For each line read, search and replace $InstallSize$ with the calculated size
  if($line =~ /\$InstallSize\$/i)
  {
    $installSize          = 0;
    $installSizeSystem    = 0;

    # split read line by ":" deliminator
    @colonSplit = split(/:/, $line);
    if($#colonSplit >= 0)
    {
      $componentName    = $colonSplit[1];
      chop($componentName);

      $installSize      = OutputInstallSize("$inStagePath\\$componentName");

      # special oji consideration here.  Since it's an installer that 
      # seamonkey installer will be calling, the disk space allocation
      # needs to be adjusted by an expansion factor of 3.62.
      if($componentName =~ /oji/i)
      {
        $installSize = int($installSize * 3.62);
      }
    }

    # Read the next line to calculate for the "Install Size System="
    if($line = <fpInIt>)
    {
      if($line =~ /\$InstallSizeSystem\$/i)
      {
        $installSizeSystem = OutputInstallSizeSystem($line, "$inStagePath\\$componentName");
      }
    }

    $installSize -= $installSizeSystem;
    print fpOutIni "Install Size=$installSize\n";
    print fpOutIni "Install Size System=$installSizeSystem\n";
  }
  elsif($line =~ /\$InstallSizeArchive\$/i)
  {
    $installSizeArchive = 0;

    # split read line by ":" deliminator
    @colonSplit = split(/:/, $line);
    if($#colonSplit >= 0)
    {
      $componentName = $colonSplit[1];
      chop($componentName);

      $installSizeArchive = OutputInstallSizeArchive("$inXpiPath\\$componentName");
    }

    print fpOutIni "Install Size Archive=$installSizeArchive\n";
  }
  else
  {
    # For each line read, search and replace $Version$ with the version passed in
    $line =~ s/\$Version\$/$inVersion/i;
    $line =~ s/\$Domain\$/$inDomain/i;
    $line =~ s/\$ServerPath\$/$inServerPath/i;
    $line =~ s/\$RedirIniUrl\$/$inRedirIniUrl/i;
    $line =~ s/\$UserAgent\$/$inUserAgent/i;
    print fpOutIni $line;
  }
}

print " done!\n";

# end of script
exit(0);

sub ParseDomainAndPath()
{
  my($aUrl) = @_;
  my($aDomain, $aServerPath);

  @slashSplit = split(/\//, $aUrl);
  if($#slashSplit >= 0)
  {
    for($i = 0; $i <= $#slashSplit; $i++)
    {
      if($i <= 2)
      {
        if($aDomain eq "")
        {
          $aDomain = "$slashSplit[$i]";
        }
        else
        {
          $aDomain = "$aDomain/$slashSplit[$i]";
        }
      }
      else
      {
        if($aServerPath eq "")
        {
          $aServerPath = "/$slashSplit[$i]";
        }
        else
        {
          $aServerPath = "$aServerPath/$slashSplit[$i]";
        }
      }
    }
  }

  return($aDomain, $aServerPath);
}

sub OutputInstallSize()
{
  my($inPath) = @_;
  my($installSize);

  print "   calculating size for $inPath\n";
  $installSize    = `ds32.exe /D /L0 /A /S /C 32768 $inPath`;
  $installSize   += 32768; # take into account install.js
  $installSize    = int($installSize / 1024);
  $installSize   += 1;
  return($installSize);
}

sub OutputInstallSizeArchive()
{
  my($inPath) = @_;
  my($installSizeArchive);
  my($dev, $ino, $mode, $nlink, $uid, $gui, $rdev, $size, $atime, $mtime, $ctime, $blksize, $blocks);

  print "   calculating size for $inPath\n";
  ($dev, $ino, $mode, $nlink, $uid, $gui, $rdev, $size, $atime, $mtime, $ctime, $blksize, $blocks) = stat $inPath;
  $installSizeArchive   += 32768; # take into account install.js
  $installSizeArchive    = int($size / 1024);
  $installSizeArchive   += 1;
  return($installSizeArchive);
}

sub OutputInstallSizeSystem()
{
  my($inLine, $inPath) = @_;
  my($installSizeSystem) = 0;

  # split read line by ":" deliminator
  @colonSplit = split(/:/, $inLine);
  if($#colonSplit >= 0)
  {
    # split line by "," deliminator
    @commaSplit = split(/\,/, $colonSplit[1]);
    if($#commaSplit >= 0)
    {
      foreach(@commaSplit)
      {
        # calculate the size of component installed using ds32.exe in Kbytes
        print "   calculating size for $inPath\\$_";
        $installSizeSystem += `ds32.exe /D /L0 /A /S /C 32768 $inPath\\$_`;
      }
    }
  }

  $installSizeSystem  = int($installSizeSystem / 1024);
  $installSizeSystem += 1;
  return($installSizeSystem);
}

