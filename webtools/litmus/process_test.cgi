#!/usr/bin/perl -w
# -*- mode: cperl; c-basic-offset: 8; indent-tabs-mode: nil; -*-

# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1
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
# The Original Code is Litmus.
#
# The Initial Developer of the Original Code is
# the Mozilla Corporation.
# Portions created by the Initial Developer are Copyright (C) 2006
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Chris Cooper <ccooper@deadsquid.com>
#   Zach Lipton <zach@zachlipton.com>
#
# ***** END LICENSE BLOCK *****

use strict;

use Time::HiRes qw( gettimeofday tv_interval );
our $t0 = [gettimeofday];

use Litmus;
use Litmus::Error;
use Litmus::DB::Product;
use Litmus::UserAgentDetect;
use Litmus::SysConfig;
use Litmus::Auth;
use Litmus::Utils;
use Litmus::DB::Resultbug;
use Litmus::XML;

use CGI;
use Date::Manip;

Litmus->init();
my $c = Litmus->cgi(); 

if ($c->param('data')) {
  # we're getting XML result data from an automated testing provider, 
  # so pass that off to XML.pm for processing
  my $x = Litmus::XML->new();
  $x->processResults($c->param('data'));

  # return whatever response was generated:
  print $c->header('text/plain');
  print $x->response();
  exit; # that's all folks!
}

Litmus::Auth::requireLogin("process_test.cgi");
my $user || Litmus::Auth::getCurrentUser();

print $c->header();

my $sysconfig;
my $branch_id;
my ($test_run_id, $test_run);
if ($c->param("singleResult")) {
  $sysconfig = &verifySysConfig($c);
  $branch_id = $c->param("testcaseBranchId");
} else {
  $test_run_id = $c->param("test_run_id");
  if (!$test_run_id) {
    invalidInputError("No Test Run selected!");
    exit 1;
  }

  $test_run = Litmus::DB::TestRun->getTestRunWithRefs($test_run_id);
  $sysconfig = Litmus::SysConfig->getCookieByTestRunId($test_run_id);
 
  if (!$sysconfig) {
    invalidInputError("No system configuration information found!");
    exit 1;
  };

  $c->storeCookie($sysconfig->setCookie());
  $test_run->flagCriteriaInUse($sysconfig);

  $branch_id = $test_run->branch_id;
}

my $locale = Litmus::DB::Locale->retrieve($sysconfig->{'locale'});

my @names = $c->param();

# find all the test numbers contained in this result submission
my @tests;
foreach my $curname (@names) {
  if ($curname =~ /testresult_(\d*)/) {
    push(@tests, $1);
  }
}

if (scalar @tests < 1) {
  invalidInputError("No test results found!");
}

my $testcount;
my %resultcounts;
my $product;
my $testcase_id;
foreach my $curtestid (@tests) {
  unless ($c->param("testresult_".$curtestid)) {
    # user didn't submit a result for this test so just skip 
    # it and move on...
    next; 
  }
  
  $testcase_id = $curtestid;
  my $curtest = Litmus::DB::Testcase->retrieve($curtestid);
  unless ($curtest) {
    # oddly enough, the test doesn't exist
    Litmus::Error::logError("No testcase found for ID: $curtestid",
                            caller(0));
    next;
  }
  
  $testcount++;
  
  my $ua = Litmus::UserAgentDetect->new();
  
  my $result_status = Litmus::DB::ResultStatus->retrieve($c->param("testresult_".$curtestid));
  $resultcounts{$result_status->name()}++;
  
  my $note = $c->param("comment_".$curtestid);
  my $bugs = $c->param("bugs_".$curtestid);
  
  my $time = &Date::Manip::UnixDate("now","%q");
  $user = $user || Litmus::Auth::getCookie()->user_id();  

  my $tr = Litmus::DB::Testresult->create({
                                           user_id       => $user,
                                           testcase      => $curtest,
                                           timestamp     => $time,
                                           last_updated  => $time,
                                           useragent     => $ua,
                                           result_status => $result_status,
                                           opsys_id      => $sysconfig->{'opsys_id'},
                                           branch_id     => $branch_id,
                                           build_id      => $sysconfig->{'build_id'},
                                           locale        => $locale,
                                          });
  
  # if there's a note, create an entry in the comments table for it
  if ($note and
      $note ne '') {
    Litmus::DB::Comment->create({
                                 testresult      => $tr,
                                 submission_time => $time,
                                 last_updated    => $time,
                                 user            => $user,
                                 comment         => $note
                                });
  }

  if ($bugs and
      $bugs ne '') {
    $bugs =~ s/[^0-9,]//g;
    my @new_bugs = split(/,/,$bugs);
    foreach my $new_bug (@new_bugs) {
      next if ($new_bug eq '0');
      my $bug = Litmus::DB::Resultbug->create({
                                               testresult => $tr,
                                               last_updated => $time,
                                               submission_time => $time,
                                               user => $user,
                                               bug_id => $new_bug,
                                              });
    }
  }

}

if (! $testcount) {
  invalidInputError("No results submitted.");
}

my $vars;
if ($c->param("singleResult")) {
  $vars->{"singleResult"} = 1;
  $vars->{"testcase_id"} = $testcase_id;
} else {
  $vars->{"test_runs"} = [$test_run];
  $vars->{"sysconfig"} = $sysconfig;
}
  
my $testgroup;
if ($c->param("testgroup")) {
  $testgroup = Litmus::DB::Testgroup->retrieve($c->param("testgroup"));
} 

$vars->{'title'} = 'Run Tests';

my $cookie =  Litmus::Auth::getCookie();
$vars->{"defaultemail"} = $cookie;
$vars->{"show_admin"} = Litmus::Auth::istrusted($cookie);
$vars->{'testcount'} = $testcount;
$vars->{'resultcounts'} = \%resultcounts || undef;
$vars->{'testgroup'} = $testgroup || undef;
$vars->{'return'} = $c->param("return") || undef;

Litmus->template()->process("process/process.html.tmpl", $vars) ||
  internalError(Litmus->template()->error());    

if ($Litmus::Config::DEBUG) {
  my $elapsed = tv_interval ( $t0 );
  printf  "<div id='pageload'>Page took %f seconds to load.</div>", $elapsed;
}

exit 0;

# END

#########################################################################
sub redirectToRunTests() {
  my $c = Litmus->cgi();
  print $c->redirect("/run_tests.cgi");
}

#########################################################################
sub verifySysConfig($) {
  my $c = shift; 
  # Use the criterion value as the basis for our check, since disabled form
  # controls don't submit their values anyway.
  my ($row_num,$build_id,$platform_id,$opsys_id);
  if ($c->param("criterion")) {
    ($row_num,$build_id,$platform_id,$opsys_id) = split(/\|/,$c->param("criterion"),4);
    if (!$build_id) {
      $build_id = $c->param("build_id_${row_num}");
    }
    if (!$platform_id) {
      $platform_id = $c->param("platform_${row_num}");
    }
    if (!$opsys_id) {
      $opsys_id = $c->param("opsys_${row_num}");
    }
  }
  my $locale = $c->param("locale");

  # Make sure we have a complete set of criteria.
  if (!$build_id or
      !$platform_id or
      !$opsys_id or
      !$locale) {
    return undef;
  }
  
  my $sysconfig = Litmus::SysConfig::new(
                                         0,
                                         $build_id,
                                         $platform_id,
                                         $opsys_id,
                                         $locale
                                         );
  return $sysconfig;
}
