#!/usr/bonsaitools/bin/perl -wT
# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Bugzilla Bug Tracking System.
#
# Contributor(s): Jacob Steenhagen <jake@acutex.net>

use strict;
use diagnostics;

use lib ".";
require "CGI.pl";

# Shut up "Used Only Once" errors
use vars qw { $anyvotesallowed };

ConnectToDatabase();

# Use the template toolkit (http://www.template-toolkit.org/) to generate
# the user interface (HTML pages and mail messages) using templates in the
# "template/" subdirectory.
use Template;

# Create the global template object that processes templates and specify
# configuration parameters that apply to all templates processed in this script.
my $template = Template->new(
  {
    # Colon-separated list of directories containing templates.
    INCLUDE_PATH => "template/custom:template/default",
    # Allow templates to be specified with relative paths.
    RELATIVE => 1,
    POST_CHOMP =>1,
  }
);

# Define the global variables and functions that will be passed to the UI 
# template.  Individual functions add their own values to this hash before
# sending them to the templates they process.
my $vars = 
  {
    # Function for retrieving global parameters.
    'Param' => \&Param ,

    # Function that tells us if the logged in user is in a specific group.
    'UserInGroup' => \&UserInGroup ,
  };


# Needed for $::anyvotesallowed
GetVersionTable();

# Check to see if the user has logged in yet.
quietly_check_login();

###############################################################################
# Main Body Execution
###############################################################################

$vars->{'username'} = $::COOKIE{'Bugzilla_login'} || '';
$vars->{'anyvotesallowed'} = $::anyvotesallowed;

if (defined $::COOKIE{'Bugzilla_login'}) {
    SendSQL("SELECT mybugslink, userid, blessgroupset FROM profiles " .
            "WHERE login_name = " . SqlQuote($::COOKIE{'Bugzilla_login'}));
    my ($mybugslink, $userid, $blessgroupset) = (FetchSQLData());
    $vars->{'userid'} = $userid;
    $vars->{'blessgroupset'} = $blessgroupset;
    if ($mybugslink) {
        my $mybugstemplate = Param("mybugstemplate");
        my %substs = ( 'userid' => url_quote($::COOKIE{'Bugzilla_login'}) );
        $vars->{'mybugsurl'} = PerformSubsts($mybugstemplate, \%substs);
    }
    SendSQL("SELECT name FROM namedqueries WHERE userid = $userid AND linkinfooter");
    while (MoreSQLData()) {
        my ($name) = FetchSQLData();
        push(@{$vars->{'namedqueries'}}, $name);
    }
}

# This sidebar is currently for use with Mozilla based web browsers.
# Internet Explorer 6 is supposed to have a similar feature, but it
# most likely won't support XUL ;)  When that does come out, this
# can be expanded to output normal HTML for IE.  Until then, I like
# the way Scott's sidebar looks so I'm using that as the base for
# this file.
# http://bugzilla.mozilla.org/show_bug.cgi?id=37339

my $useragent = $ENV{HTTP_USER_AGENT};
if ($useragent =~ m:Mozilla/([1-9][0-9]*):i && $1 >= 5 && $useragent !~ m/compatible/i) {
    print "Content-type: application/vnd.mozilla.xul+xml\n\n";
    # Generate and return the XUL from the appropriate template.
    $template->process("sidebar/xul.tmpl", $vars)
      || DisplayError("Template process failed: " . $template->error())
      && exit;
} else {
    DisplayError("sidebar.cgi currently only supports Mozilla based web browsers");
    exit;
}



