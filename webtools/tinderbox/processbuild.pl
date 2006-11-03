#!/usr/bonsaitools/bin/perl --
# -*- Mode: perl; indent-tabs-mode: nil -*-
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
# The Original Code is the Tinderbox build tool.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): 

use Compress::Zlib;
use Compress::Bzip2;
use Getopt::Long;
use Time::Local;
use File::Copy;
use File::Basename;
use lib "@TINDERBOX_DIR@";
require 'tbglobals.pl'; # for $gzip
#use strict;

umask 002;

# setuid globals
$ENV{'PATH'} = "@SETUID_PATH@";
$tinderboxdir = "@TINDERBOX_DIR@";

# globals
my ($only_check_mail);
my @changed_trees=();
my %scraped_trees;
my $debug = 0;
my $rejected_mail_dir = "$data_dir/bad";

chdir $tinderboxdir or die "Couldn't chdir to $tinderboxdir"; 

# parse args
GetOptions("check-mail" => \$only_check_mail) or die ("Error parsing args.");

# Acquire a lock first so that we don't step on ourselves
my $lockfile = "$data_dir/processbuild.sem";
my $lock = &lock_datafile($lockfile);
opendir(DIR, $data_dir) or die("Can't opendir($data_dir): $!");
my @datafiles = 
    sort(grep { /^tbx\.\d+\.\d+$/ && -f "$data_dir/$_" } readdir(DIR));
closedir(DIR);
print "Files: @datafiles\n" if ($debug && $#datafiles > 0);
for my $file (@datafiles) {
    &process_mailfile("$data_dir/$file");
}
&unlock_datafile($lock);
unlink($lockfile);

require 'showbuilds.pl';
for my $t (@changed_trees) {
    $tree = $t;
    print "Tree: $t\n" if ($debug);
    # Static pages - For Sidebar flash and tinderbox panels.
    $rel_path = ''; 
    &tb_build_static();
    # Who data
    $err = system("./buildwho.pl", "$t");
    if ($err) {
        warn "buildwho.pl returned an error for tree $t\n";
    }
}
exit(0);
# end of main
######################################################################


sub process_mailfile($) {
    my ($mail_file) = @_;
    my $err = 0;

    print "process_mailfile($mail_file)\n" if ($debug);

    %MAIL_HEADER = ();
    %tinderbox = ();

    # Scan the logfile once to get mail header and build variables
    #
    print "Parsing: begin\n" if ($debug);
    unless (open(LOG, "<", $mail_file)) {
        warn "Can't open $mail_file: $!";
        return;
    }
    parse_mail_header(*LOG, \%MAIL_HEADER);
    parse_log_variables(*LOG, \%tinderbox);
    close LOG;

    print "Parsing: end\n" if ($debug);

    # If the mail does not contain any tinderbox header info, just drop it.
    @tbkeys = keys %tinderbox;
    if ($#tbkeys == -1) {
        print "Dropping spam mail: $mail_file\n" if ($debug);
        unlink $mail_file;
        return;
    }

    # Make sure variables are defined correctly
    #
    if (&check_required_variables(\%tinderbox, \%MAIL_HEADER)) {
        my $rejected_mail = $rejected_mail_dir . "/" . basename($mail_file);
        print "Moving corrupt logfile, $mail_file , to $rejected_mail .\n";
        move($mail_file, $rejected_mail_dir);
        return;
    }

    if ($only_check_mail) {
        warn "Mail variables passed the test\n";
        return;
    }

    # Write data to "build.dat"
    #
    print "Write build.dat: $tinderbox{build}\n" if ($debug);
    $tinderbox{logfile} = "$tinderbox{builddate}.$$.gz";
    write_build_data(\%tinderbox);

    # Add tree to changed trees list to later rebuild who.dat
    # 
    if (!grep(/$tinderbox{tree}/, @changed_trees)) {
        push @changed_trees, $tinderbox{tree};
    }

    # Compress the build log and put it in the tree
    #
    print "Compress\n" if ($debug);
    if ($tinderbox{status} =~ /building/) {
        unlink $mail_file;
        print "process_mailfile($mail_file) Building: END\n" if ($debug);
        return;
    } else {
        return if (&compress_log_file(\%tinderbox, $mail_file));
    }

    # Warnings
    #   Compare the name with $warning_buildnames_pat which is defined in
    #   $tinderbox{tree}/treedata.pl if at all.
    print "Warnings($tinderbox{tree}/$tinderbox{logfile})\n" if ($debug);
    undef $warning_buildnames_pat;
    open(TD, "$tinderbox{tree}/treedata.pl");
    while ($line=<TD>) {
        if ($line =~ m/^\$warning_build_names_pat\s*=.*;$/) {
            eval($line);
        }
    }
    close(TD);
    if (defined $warning_buildnames_pat
        and $tinderbox{build} =~ /^$warning_buildnames_pat$/
        and $tinderbox{status} ne 'failed') {
        $err = system("./warnings.pl", "$tinderbox{tree}/$tinderbox{logfile}");
        warn "warnings.pl($tinderbox{tree}/$tinderbox{logfile} returned an error\n" if ($err);
        undef $warning_buildnames_pat;
    }

    # Scrape data
    #   Look for build name in scrapedata.pl.
    print "Scrape($tinderbox{tree},$tinderbox{logfile})\n" if ($debug);
    undef $scrape_builds;
    require "$tinderbox{tree}/scrapebuilds.pl"
        if -r "$tinderbox{tree}/scrapebuilds.pl";
    # required files are only loaded once so preserve scraped_builds value
    if (defined($scrape_builds)) {
        $scraped_trees{$tinderbox{tree}} = $scrape_builds;
    }
    $sb = $scraped_trees{$tinderbox{tree}};
    if (defined($sb) and $sb->{$tinderbox{build}}) {
        $err = system("./scrape.pl", "$tinderbox{tree}", "$tinderbox{logfile}");
        warn "scrape.pl($tinderbox{tree},$tinderbox{logfile}) returned an error\n" if ($err);
    }
    print "process_mailfile($mail_file) END\n" if ($debug);
}

# This routine will scan through log looking for 'tinderbox:' variables
#
sub parse_log_variables {
  my ($fh, $tbx) = @_;
  local $_;

  while (<$fh>) {
    chomp;
    if (/^tinderbox:.*:/) {
      last if /^tinderbox: END/;
      my ($key, $value) = (split /:\s*/, $_, 3)[1..2];
      $value =~ s/\s*$//;
      $tbx->{$key} = &trick_taint($value);
    }
  }
}

sub parse_mail_header {
  my ($fh, $mail_ref) = @_;
  local $_;
  my $name = '';
  
  while(<$fh>) {
    chomp;
    last if $line eq '';
    
    if (/([^ :]*)\:[ \t]+([^\n]*)/) {
      $name = $1;
      $name =~ tr/A-Z/a-z/;
      $mail_ref{$name} = $2;
    }
    elsif ($name ne '') {
      $mail_ref{$name} .= $2;
    }
  }
}

sub check_required_variables {
  my ($tbx, $mail_header) = @_;
  my $err_string = '';

  if ($tbx->{tree} eq '') {
    $err_string .= "Variable 'tinderbox:tree' not set.\n";
  }
  elsif (not -r $tbx->{tree}) {
    $err_string .= "Variable 'tinderbox:tree' not set to a valid tree.\n";
  }
  elsif (($mail_header->{'to'} =~ /external/i or
          $mail_header->{'cc'} =~ /external/i) and
         $tbx->{tree} !~ /external/i) {
    $err_string .= "Data from an external source didn't specify an 'external' tree.";
  }
  if ($tbx->{build} eq '') {
    $err_string .= "Variable 'tinderbox:build' not set.\n";
  }
  if ($tbx->{errorparser} eq '') {
    $err_string .= "Variable 'tinderbox:errorparser' not set.\n";
  }

  # Grab the date in the form of mm/dd/yy hh:mm:ss
  #
  # Or a GMT unix date
  #
  if ($tbx->{builddate} eq '') {
    $err_string .= "Variable 'tinderbox:builddate' not set.\n";
  }
  else {
    if ($tbx->{builddate} =~ 
        /([0-9]*)\/([0-9]*)\/([0-9]*)[ \t]*([0-9]*)\:([0-9]*)\:([0-9]*)/) {
      $tbx->{builddate} = timelocal($6,$5,$4,$2,$1-1,$3);
    }
    elsif ($tbx->{builddate} < 7000000) {
      $err_string .= "Variable 'tinderbox:builddate' not of the form MM/DD/YY HH:MM:SS or unix date\n";
    }
  }

  # Build Status
  #
  if ($tbx->{status} eq '') {
    $err_string .= "Variable 'tinderbox:status' not set.\n";
  }
  elsif (not $tbx->{status} =~ /success|busted|building|testfailed/) {
    $err_string .= "Variable 'tinderbox:status' must be 'success', 'busted', 'testfailed', or 'building'\n";
  }

  # Log compression
  #
  if ($tbx->{logcompression} !~ /^(bzip2|gzip)?$/) {
    $err_string .= "Variable 'tinderbox:logcompression' must be '', 'bzip2' or 'gzip'\n";
  }

  # Log encoding
  if ($tbx->{logencoding} !~ /^(base64|uuencode)?$/) {
    $err_string .= "Variable 'tinderbox:logencoding' must be '', 'base64' or 'uuencode'\n";
  }

  # Report errors
  #
  if ($err_string eq '') {
      return 0;
  } else {
      warn $err_string;
      return 1;
  }
}

sub write_build_data {
  my $tbx = $_[0];
  $process_time = time;
  my $lockfile = "$tbx->{tree}/builddat.sem";
  my $lock = &lock_datafile($lockfile);
  unless (open(BUILDDATA, ">>", "$tbx->{tree}/build.dat")) {
      warn "can't open $tbx->{tree}/build.dat for writing: $!";
      &unlock_datafile($lock);
      return;
  }
  print BUILDDATA "$process_time|$tbx->{builddate}|$tbx->{build}|$tbx->{errorparser}|$tbx->{status}|$tbx->{logfile}|$tbx->{binaryurl}\n";
  close BUILDDATA;
  &unlock_datafile($lock);
  unlink($lockfile);
}

sub compress_log_file {
  my ($tbx, $maillog) = @_;
  local *LOG2;
  my $err = 0;

  open(LOG2, "<", $maillog) or $err++;
  if ($err) {
    warn "Can't open $maillog: $!";
    return 1;
  }

  # Skip past the the RFC822.HEADER
  #
  while (<LOG2>) {
    chomp;
    last if /^$/;
  }

  my $logfile = "$tbx->{tree}/$tbx->{logfile}";
  my $gz = gzopen($logfile,"wb") or $err++;
  if ($err) {
    warn "gzopen($logfile): $!\n";
    return 1;
  }

  # If this log is compressed, we need to decode it and decompress
  # it before storing its contents into ZIPLOG.
  if($tbx->{logcompression} ne '') {

    # tinderbox variables are not compressed
    # write them directly to the gzip'd log
    while(<LOG2>) {
      $gz->gzwrite($_);
      last if(m/^tinderbox: END/);
    }

    # Decode the log using the logencoding variable to determine
    # the type of encoding.
    my $decoded = "$tbx->{tree}/$tbx->{logfile}.uncomp";
    if ($tbx->{logencoding} eq 'base64') {
      eval "use MIME::Base64 ();";
      open(DECODED, ">", $decoded) or $err++;
      if ($err) {
        warn "Can't open $decoded for writing: $!";
        close(LOG2);
        $gz->gzclose();
        unlink $logfile;
        return 1;
      }
      while (<LOG2>) {
        print DECODED MIME::Base64::decode($_);
      }
      close DECODED;
    }
    elsif ($tbx->{logencoding} eq 'uuencode') {
      open(DECODED, ">", $decoded) or $err++;
      if ($err) {
        close(LOG2);
        $gz->gzclose();
        unlink $logfile;
        return 1;
      }
      while (<LOG2>) {
        print DECODED unpack("u*", $_);
      }
      close DECODED;
    }

    # Decompress the log using the logcompression variable to determine
    # the type of compression used.
    if ($tbx->{logcompression} eq 'gzip') {
      my $comp_gz = gzopen($decoded, "rb") or $err++;
      if ($err) {
        warn ("$decoded: $!\n");
        close(LOG2);
        $gz->gzclose();
        unlink $logfile;
        unlink $decoded;
        return 1;
      }
      my ($bytesread, $line);
      while (($bytesread = $comp_gz->gzread($line)) > 0) {
          $gz->gzwrite($line);
      }
      $comp_gz->gzclose();
    } elsif ($tbx->{logcompression} eq 'bzip2') {
      my $comp_bz = bzopen($decoded, "rb") or $err++;
      if ($err) {
        warn ("$decoded: $!\n");
        close(LOG2);
        $gz->gzclose();
        unlink $logfile;
        unlink $decoded;
        return 1;
      }
      my ($bytesread, $line);
      while (($bytesread = $comp_bz->bzread($line)) > 0) {
          $gz->gzwrite($line);
      }
      $comp_bz->bzclose();
    }
    # Remove our temporary decoded file
    unlink($decoded) if -f $decoded;
  }
  # This log is not compressed/encoded so we can simply write out
  # it's contents to the gzip'd log file.
  else {
    while (<LOG2>) {
      $gz->gzwrite($_);
    }
  }
  $gz->gzclose();
  close LOG2;
  unlink $maillog;
  return 0;
}
