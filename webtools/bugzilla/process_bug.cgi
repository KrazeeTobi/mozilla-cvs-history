#!/usr/bin/perl -wT
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
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Terry Weissman <terry@mozilla.org>
#                 Dan Mosedale <dmose@mozilla.org>
#                 Dave Miller <justdave@syndicomm.com>
#                 Christopher Aillon <christopher@aillon.com>
#                 Myk Melez <myk@mozilla.org>
#                 Jeff Hedlund <jeff.hedlund@matrixsi.com>
#                 Fr�d�ric Buclin <LpSolit@gmail.com>

use strict;

my $UserInEditGroupSet = -1;
my $UserInCanConfirmGroupSet = -1;
my $PrivilegesRequired = 0;

use lib qw(.);

use Bugzilla;
use Bugzilla::Constants;
require "CGI.pl";

use Bugzilla::Bug;
use Bugzilla::User;
use Bugzilla::Util;

# Use the Flag module to modify flag data if the user set flags.
use Bugzilla::Flag;

# Shut up misguided -w warnings about "used only once":

use vars qw(%versions
          %components
          %legal_opsys
          %legal_platform
          %legal_priority
          %settable_resolution
          %target_milestone
          %legal_severity
           );

my $user = Bugzilla->login(LOGIN_REQUIRED);
my $whoid = $user->id;

my $cgi = Bugzilla->cgi;
my $dbh = Bugzilla->dbh;

my $requiremilestone = 0;

use vars qw($template $vars);

######################################################################
# Begin Data/Security Validation
######################################################################

# Create a list of IDs of all bugs being modified in this request.
# This list will either consist of a single bug number from the "id"
# form/URL field or a series of numbers from multiple form/URL fields
# named "id_x" where "x" is the bug number.
# For each bug being modified, make sure its ID is a valid bug number 
# representing an existing bug that the user is authorized to access.
my @idlist;
if (defined $::FORM{'id'}) {
    ValidateBugID($::FORM{'id'});
    push @idlist, $::FORM{'id'};
} else {
    foreach my $i (keys %::FORM) {
        if ($i =~ /^id_([1-9][0-9]*)/) {
            my $id = $1;
            ValidateBugID($id);
            push @idlist, $id;
        }
    }
}

# Make sure there are bugs to process.
scalar(@idlist) || ThrowUserError("no_bugs_chosen");

$::FORM{'dontchange'} = '' unless exists $::FORM{'dontchange'};

# Validate all timetracking fields
foreach my $field ("estimated_time", "work_time", "remaining_time") {
    if (defined $::FORM{$field}) {
        my $er_time = trim($::FORM{$field});
        if ($er_time ne $::FORM{'dontchange'}) {
            Bugzilla::Bug::ValidateTime($er_time, $field);
        }
    }
}

if (UserInGroup(Param('timetrackinggroup'))) {
    my $wk_time = $::FORM{'work_time'};
    if ($::FORM{'comment'} =~ /^\s*$/ && $wk_time && $wk_time != 0) {
        ThrowUserError('comment_required');
    }
}

ValidateComment($::FORM{'comment'});

# If the bug(s) being modified have dependencies, validate them
# and rebuild the list with the validated values.  This is important
# because there are situations where validation changes the value
# instead of throwing an error, f.e. when one or more of the values
# is a bug alias that gets converted to its corresponding bug ID
# during validation.
foreach my $field ("dependson", "blocked") {
    if (defined($::FORM{$field}) && $::FORM{$field} ne "") {
        my @validvalues;
        foreach my $id (split(/[\s,]+/, $::FORM{$field})) {
            next unless $id;
            ValidateBugID($id, $field);
            push(@validvalues, $id);
        }
        $::FORM{$field} = join(",", @validvalues);
    }
}

# do a match on the fields if applicable

# The order of these function calls is important, as both Flag::validate
# and FlagType::validate assume User::match_field has ensured that the values
# in the requestee fields are legitimate user email addresses.
&Bugzilla::User::match_field({
    'qa_contact'                => { 'type' => 'single' },
    'newcc'                     => { 'type' => 'multi'  },
    'masscc'                    => { 'type' => 'multi'  },
    'assigned_to'               => { 'type' => 'single' },
    '^requestee(_type)?-(\d+)$' => { 'type' => 'single' },
});
# Validate flags, but only if the user is changing a single bug,
# since the multi-change form doesn't include flag changes.
if (defined $::FORM{'id'}) {
    Bugzilla::Flag::validate(\%::FORM, $::FORM{'id'});
    Bugzilla::FlagType::validate(\%::FORM, $::FORM{'id'});
}

######################################################################
# End Data/Security Validation
######################################################################

print $cgi->header();
$vars->{'title_tag'} = "bug_processed";

# Set the title if we can see a mid-air coming. This test may have false
# negatives, but never false positives, and should catch the majority of cases.
# It only works at all in the single bug case.
if (defined($::FORM{'id'})) {
    SendSQL("SELECT delta_ts FROM bugs WHERE bug_id = $::FORM{'id'}");
    my $delta_ts = FetchOneColumn();
    
    if (defined $::FORM{'delta_ts'} && $delta_ts && 
        $::FORM{'delta_ts'} ne $delta_ts) 
    {
        $vars->{'title_tag'} = "mid_air";
    }
}

# Set up the vars for nagiavtional <link> elements
my $next_bug;
if ($cgi->cookie("BUGLIST") && $::FORM{'id'}) {
    my @bug_list = split(/:/, $cgi->cookie("BUGLIST"));
    $vars->{'bug_list'} = \@bug_list;
    my $cur = lsearch(\@bug_list, $::FORM{"id"});
    if ($cur >= 0 && $cur < $#bug_list) {
        $next_bug = $bug_list[$cur + 1];

        # Note that we only bother with the bug_id here, and get
        # the full bug object at the end, before showing the edit
        # page. If you change this, remember that we have not
        # done the security checks on the next bug yet
        $vars->{'bug'} = { bug_id => $next_bug };
    }
}

GetVersionTable();

CheckFormFieldDefined(\%::FORM, 'product');
CheckFormFieldDefined(\%::FORM, 'version');
CheckFormFieldDefined(\%::FORM, 'component');


# This function checks if there is a comment required for a specific
# function and tests, if the comment was given.
# If comments are required for functions is defined by params.
#
sub CheckonComment( $ ) {
    my ($function) = (@_);
    
    # Param is 1 if comment should be added !
    my $ret = Param( "commenton" . $function );

    # Allow without comment in case of undefined Params.
    $ret = 0 unless ( defined( $ret ));

    if( $ret ) {
        if (!defined $::FORM{'comment'} || $::FORM{'comment'} =~ /^\s*$/) {
            # No comment - sorry, action not allowed !
            ThrowUserError("comment_required");
        } else {
            $ret = 0;
        }
    }
    return( ! $ret ); # Return val has to be inverted
}

# Figure out whether or not the user is trying to change the product
# (either the "product" variable is not set to "don't change" or the
# user is changing a single bug and has changed the bug's product),
# and make the user verify the version, component, target milestone,
# and bug groups if so.
if ( $::FORM{'id'} ) {
    SendSQL("SELECT name FROM products, bugs " .
            "WHERE products.id = bugs.product_id AND bug_id = $::FORM{'id'}");
    $::oldproduct = FetchSQLData();
}
if ((($::FORM{'id'} && $::FORM{'product'} ne $::oldproduct) 
     || (!$::FORM{'id'} && $::FORM{'product'} ne $::FORM{'dontchange'}))
    && CheckonComment( "reassignbycomponent" ))
{
    # Check to make sure they actually have the right to change the product
    if (!CheckCanChangeField('product', $::FORM{'id'}, $::oldproduct, $::FORM{'product'})) {
        $vars->{'oldvalue'} = $::oldproduct;
        $vars->{'newvalue'} = $::FORM{'product'};
        $vars->{'field'} = 'product';
        $vars->{'privs'} = $PrivilegesRequired;
        ThrowUserError("illegal_change", $vars);
    }

    CheckFormField(\%::FORM, 'product', \@::legal_product);
    my $prod = $::FORM{'product'};

    # note that when this script is called from buglist.cgi (rather
    # than show_bug.cgi), it's possible that the product will be changed
    # but that the version and/or component will be set to 
    # "--dont_change--" but still happen to be correct.  in this case,
    # the if statement will incorrectly trigger anyway.  this is a 
    # pretty weird case, and not terribly unreasonable behavior, but 
    # worthy of a comment, perhaps.
    #
    my $vok = lsearch($::versions{$prod}, $::FORM{'version'}) >= 0;
    my $cok = lsearch($::components{$prod}, $::FORM{'component'}) >= 0;

    my $mok = 1;   # so it won't affect the 'if' statement if milestones aren't used
    if ( Param("usetargetmilestone") ) {
       CheckFormFieldDefined(\%::FORM, 'target_milestone');
       $mok = lsearch($::target_milestone{$prod}, $::FORM{'target_milestone'}) >= 0;
    }

    # If the product-specific fields need to be verified, or we need to verify
    # whether or not to add the bugs to their new product's group, display
    # a verification form.
    if (!$vok || !$cok || !$mok || (AnyDefaultGroups() && !defined($::FORM{'addtonewgroup'}))) {
        $vars->{'form'} = \%::FORM;
        $vars->{'mform'} = \%::MFORM;
        
        if (!$vok || !$cok || !$mok) {
            $vars->{'verify_fields'} = 1;
            my %defaults;
            # We set the defaults to these fields to the old value,
            # if its a valid option, otherwise we use the default where
            # thats appropriate
            $vars->{'versions'} = $::versions{$prod};
            if ($vok) {
                $defaults{'version'} = $::FORM{'version'};
            }
            $vars->{'components'} = $::components{$prod};
            if ($cok) {
                $defaults{'component'} = $::FORM{'component'};
            }
            if (Param("usetargetmilestone")) {
                $vars->{'use_target_milestone'} = 1;
                $vars->{'milestones'} = $::target_milestone{$prod};
                if ($mok) {
                    $defaults{'target_milestone'} = $::FORM{'target_milestone'};
                } else {
                    SendSQL("SELECT defaultmilestone FROM products " .
                            "WHERE name = " . SqlQuote($prod));
                    $defaults{'target_milestone'} = FetchOneColumn();
                }
            }
            else {
                $vars->{'use_target_milestone'} = 0;
            }
            $vars->{'defaults'} = \%defaults;
        }
        else {
            $vars->{'verify_fields'} = 0;
        }
        
        $vars->{'verify_bug_group'} = (AnyDefaultGroups() 
                                       && !defined($::FORM{'addtonewgroup'}));
        
        $template->process("bug/process/verify-new-product.html.tmpl", $vars)
          || ThrowTemplateError($template->error());
        exit;
    }
}


# Checks that the user is allowed to change the given field.  Actually, right
# now, the rules are pretty simple, and don't look at the field itself very
# much, but that could be enhanced.

my $lastbugid = 0;
my $ownerid;
my $reporterid;
my $qacontactid;

################################################################################
# CheckCanChangeField() defines what users are allowed to change what bugs. You
# can add code here for site-specific policy changes, according to the 
# instructions given in the Bugzilla Guide and below. Note that you may also
# have to update the Bugzilla::Bug::user() function to give people access to the
# options that they are permitted to change.
#
# CheckCanChangeField() should return true if the user is allowed to change this
# field, and false if they are not.
#
# The parameters to this function are as follows:
# $field    - name of the field in the bugs table the user is trying to change
# $bugid    - the ID of the bug they are changing
# $oldvalue - what they are changing it from
# $newvalue - what they are changing it to
#
# Note that this function is currently not called for dependency changes 
# (bug 141593) or CC changes, which means anyone can change those fields.
#
# Do not change the sections between START DO_NOT_CHANGE and END DO_NOT_CHANGE.
################################################################################
sub CheckCanChangeField {
    # START DO_NOT_CHANGE
    my ($field, $bugid, $oldvalue, $newvalue) = (@_);

    # Return true if they haven't changed this field at all.
    if ($oldvalue eq $newvalue) {
        return 1;
    } elsif (trim($oldvalue) eq trim($newvalue)) {
        return 1;
    # numeric fields need to be compared using == 
    } elsif (($field eq "estimated_time" || $field eq "remaining_time")
             && $oldvalue == $newvalue)
    {
        return 1;
    }

    # A resolution change is always accompanied by a status change. So, we 
    # always OK resolution changes; if they really can't do this, we will 
    # notice it when status is checked. 
    if ($field eq "resolution") { 
        return 1;             
    }
    # END DO_NOT_CHANGE

    # Allow anyone to change comments.
    if ($field =~ /^longdesc/) {
        return 1;
    }

    # Ignore the assigned_to field if the bug is not being reassigned
    if ($field eq "assigned_to"
        && $::FORM{'knob'} ne "reassignbycomponent"
        && $::FORM{'knob'} ne "reassign")
    {
        return 1;
    }

    # START DO_NOT_CHANGE
    # Find out whether the user is a member of the "editbugs" and/or
    # "canconfirm" groups. $UserIn*GroupSet are caches of the return value of 
    # the UserInGroup calls.
    if ($UserInEditGroupSet < 0) {
        $UserInEditGroupSet = UserInGroup("editbugs");
    }
    
    if ($UserInCanConfirmGroupSet < 0) {
        $UserInCanConfirmGroupSet = UserInGroup("canconfirm");
    }
    # END DO_NOT_CHANGE

    # If the user isn't allowed to change a field, we must tell him who can.
    # We store the required permission set into the $PrivilegesRequired
    # variable which gets passed to the error template.
    #
    # $PrivilegesRequired = 0 : no privileges required;
    # $PrivilegesRequired = 1 : the reporter, owner or an empowered user;
    # $PrivilegesRequired = 2 : the owner or an empowered user;
    # $PrivilegesRequired = 3 : an empowered user.

    # Allow anyone with "editbugs" privs to change anything.
    if ($UserInEditGroupSet) {
        return 1;
    }

    # *Only* users with "canconfirm" privs can confirm bugs.
    if ($field eq "canconfirm"
        || ($field eq "bug_status"
            && $oldvalue eq 'UNCONFIRMED'
            && IsOpenedState($newvalue)))
    {
        $PrivilegesRequired = 3;
        return $UserInCanConfirmGroupSet;
    }

    # START DO_NOT_CHANGE
    # $reporterid, $ownerid and $qacontactid are caches of the results of
    # the call to find out the owner, reporter and qacontact of the current bug.
    if ($lastbugid != $bugid) {
        SendSQL("SELECT reporter, assigned_to, qa_contact FROM bugs
                 WHERE bug_id = $bugid");
        ($reporterid, $ownerid, $qacontactid) = (FetchSQLData());
        $lastbugid = $bugid;
    }    
    # END DO_NOT_CHANGE

    # Allow the owner to change anything else.
    if ($ownerid == $whoid) {
        return 1;
    }
    
    # Allow the QA contact to change anything else.
    if (Param('useqacontact') && ($qacontactid == $whoid)) {
        return 1;
    }
    
    # At this point, the user is either the reporter or an
    # unprivileged user. We first check for fields the reporter
    # is not allowed to change.

    # The reporter may not:
    # - reassign bugs, unless the bugs are assigned to him;
    #   in that case we will have already returned 1 above
    #   when checking for the owner of the bug.
    if ($field eq "assigned_to") {
        $PrivilegesRequired = 2;
        return 0;
    }
    # - change the QA contact
    if ($field eq "qa_contact") {
        $PrivilegesRequired = 2;
        return 0;
    }
    # - change the target milestone
    if ($field eq "target_milestone") {
        $PrivilegesRequired = 2;
        return 0;
    }
    # - change the priority (unless he could have set it originally)
    if ($field eq "priority"
        && !Param('letsubmitterchoosepriority'))
    {
        $PrivilegesRequired = 2;
        return 0;
    }

    # The reporter is allowed to change anything else.
    if ($reporterid == $whoid) {
        return 1;
    }

    # If we haven't returned by this point, then the user doesn't
    # have the necessary permissions to change this field.
    $PrivilegesRequired = 1;
    return 0;
}

# Confirm that the reporter of the current bug can access the bug we are duping to.
sub DuplicateUserConfirm {
    # if we've already been through here, then exit
    if (defined $::FORM{'confirm_add_duplicate'}) {
        return;
    }

    my $dupe = $::FORM{'id'};
    my $original = $::FORM{'dup_id'};
    
    SendSQL("SELECT reporter FROM bugs WHERE bug_id = " . SqlQuote($dupe));
    my $reporter = FetchOneColumn();
    my $rep_user = Bugzilla::User->new($reporter);

    if ($rep_user->can_see_bug($original)) {
        $::FORM{'confirm_add_duplicate'} = "1";
        return;
    }

    SendSQL("SELECT cclist_accessible FROM bugs WHERE bug_id = $original");
    $vars->{'cclist_accessible'} = FetchOneColumn();
    
    # Once in this part of the subroutine, the user has not been auto-validated
    # and the duper has not chosen whether or not to add to CC list, so let's
    # ask the duper what he/she wants to do.
    
    $vars->{'form'} = \%::FORM;
    $vars->{'mform'} = \%::MFORM;
    $vars->{'original_bug_id'} = $original;
    $vars->{'duplicate_bug_id'} = $dupe;
    
    # Confirm whether or not to add the reporter to the cc: list
    # of the original bug (the one this bug is being duped against).
    print Bugzilla->cgi->header();
    $template->process("bug/process/confirm-duplicate.html.tmpl", $vars)
      || ThrowTemplateError($template->error());
    exit;
}

if (defined $::FORM{'id'}) {
    # since this means that we were called from show_bug.cgi, now is a good
    # time to do a whole bunch of error checking that can't easily happen when
    # we've been called from buglist.cgi, because buglist.cgi only tweaks
    # values that have been changed instead of submitting all the new values.
    # (XXX those error checks need to happen too, but implementing them 
    # is more work in the current architecture of this script...)
    #
    CheckFormField(\%::FORM, 'product', \@::legal_product);
    CheckFormField(\%::FORM, 'component', 
                   \@{$::components{$::FORM{'product'}}});
    CheckFormField(\%::FORM, 'version', 
                   \@{$::versions{$::FORM{'product'}}});
    if ( Param("usetargetmilestone") ) {
        CheckFormField(\%::FORM, 'target_milestone', 
                       \@{$::target_milestone{$::FORM{'product'}}});
    }
    CheckFormField(\%::FORM, 'rep_platform', \@::legal_platform);
    CheckFormField(\%::FORM, 'op_sys', \@::legal_opsys);
    CheckFormField(\%::FORM, 'priority', \@::legal_priority);
    CheckFormField(\%::FORM, 'bug_severity', \@::legal_severity);
    CheckFormFieldDefined(\%::FORM, 'bug_file_loc');
    CheckFormFieldDefined(\%::FORM, 'short_desc');
    CheckFormFieldDefined(\%::FORM, 'longdesclength');

    if (trim($::FORM{'short_desc'}) eq "") {
        ThrowUserError("require_summary");
    }
}

my $action = '';
if (defined $::FORM{action}) {
  $action = trim($::FORM{action});
}
if (Param("move-enabled") && $action eq Param("move-button-text")) {
  $cgi->param('buglist', join (":", @idlist));
  do "move.pl" || die "Error executing move.cgi: $!";
  PutFooter();
  exit;
}


$::query = "update bugs\nset";
$::comma = "";
umask(0);

sub _remove_remaining_time {
    if (UserInGroup(Param('timetrackinggroup'))) {
        if ( defined $::FORM{'remaining_time'} 
             && $::FORM{'remaining_time'} > 0 )
        {
            $::FORM{'remaining_time'} = 0;
            $vars->{'message'} = "remaining_time_zeroed";
        }
    }
    else {
        DoComma();
        $::query .= "remaining_time = 0";
    }
}

sub DoComma {
    $::query .= "$::comma\n    ";
    $::comma = ",";
}

sub DoConfirm {
    if (CheckCanChangeField("canconfirm", $::FORM{'id'}, 0, 1)) {
        DoComma();
        $::query .= "everconfirmed = 1";
    }
}


sub ChangeStatus {
    my ($str) = (@_);
    if (!$::FORM{'dontchange'} ||
       ($str ne $::FORM{'dontchange'})) {
        DoComma();
        if ($::FORM{knob} eq 'reopen') {
            # When reopening, we need to check whether the bug was ever
            # confirmed or not
            $::query .= "bug_status = CASE WHEN everconfirmed = 1 THEN " .
                         SqlQuote($str) . " ELSE 'UNCONFIRMED' END";
        } elsif (IsOpenedState($str)) {
            # Note that we cannot combine this with the above branch - here we
            # need to check if bugs.bug_status is open, (since we don't want to
            # reopen closed bugs when reassigning), while above the whole point
            # is to reopen a closed bug.
            # Currently, the UI doesn't permit a user to reassign a closed bug
            # from the single bug page (only during a mass change), but they
            # could still hack the submit, so don't restrict this extended
            # check to the mass change page for safety/sanity/consistency
            # purposes.

            # The logic for this block is:
            # If the new state is open:
            #   If the old state was open
            #     If the bug was confirmed
            #       - move it to the new state
            #     Else
            #       - Set the state to unconfirmed
            #   Else
            #     - leave it as it was

            # This is valid only because 'reopen' is the only thing which moves
            # from closed to open, and its handled above
            # This also relies on the fact that confirming and accepting have
            # already called DoConfirm before this is called

            my @open_state = map(SqlQuote($_), OpenStates());
            my $open_state = join(", ", @open_state);
            $::query .= "bug_status = CASE WHEN bug_status IN($open_state) THEN " .
                                        "(CASE WHEN everconfirmed = 1 THEN " .
                                            SqlQuote($str) . " ELSE " .
                                            " 'UNCONFIRMED' END) ELSE " .
                                        "bug_status END";
        } else {
            $::query .= "bug_status = " . SqlQuote($str);
        }
        # If bugs are reassigned and their status is "UNCONFIRMED", they
        # should keep this status instead of "NEW" as suggested here.
        # This point is checked for each bug later in the code.
        $::FORM{'bug_status'} = $str;
    }
}

sub ChangeResolution {
    my ($str) = (@_);
    if (!$::FORM{'dontchange'}
        || $str ne $::FORM{'dontchange'})
    {
        DoComma();
        $::query .= "resolution = " . SqlQuote($str);
        # We define this variable here so that customized installations
        # may set rules based on the resolution in CheckCanChangeField.
        $::FORM{'resolution'} = $str;
    }
}

# Changing this so that it will process groups from checkboxes instead of
# select lists.  This means that instead of looking for the bit-X values in
# the form, we need to loop through all the bug groups this user has access
# to, and for each one, see if it's selected.
# If the form element isn't present, or the user isn't in the group, leave
# it as-is

my @groupAdd = ();
my @groupDel = ();

SendSQL("SELECT groups.id, isactive FROM groups, user_group_map WHERE " .
        "groups.id = user_group_map.group_id AND " .
        "user_group_map.user_id = $whoid AND " .
        "isbless = 0 AND isbuggroup = 1");
while (my ($b, $isactive) = FetchSQLData()) {
    # The multiple change page may not show all groups a bug is in
    # (eg product groups when listing more than one product)
    # Only consider groups which were present on the form. We can't do this
    # for single bug changes because non-checked checkboxes aren't present.
    # All the checkboxes should be shown in that case, though, so its not
    # an issue there
    if ($::FORM{'id'} || exists $::FORM{"bit-$b"}) {
        if (!$::FORM{"bit-$b"}) {
            push(@groupDel, $b);
        } elsif ($::FORM{"bit-$b"} == 1 && $isactive) {
            push(@groupAdd, $b);
        }
    }
}

foreach my $field ("rep_platform", "priority", "bug_severity",
                   "bug_file_loc", "short_desc", "version", "op_sys",
                   "target_milestone", "status_whiteboard") {
    if (defined $::FORM{$field}) {
        if (!$::FORM{'dontchange'}
            || $::FORM{$field} ne $::FORM{'dontchange'}) {
            DoComma();
            $::query .= "$field = " . SqlQuote(trim($::FORM{$field}));
        }
    }
}

my $prod_id; # Remember, can't use this for mass changes
if ($::FORM{'product'} ne $::FORM{'dontchange'}) {
    $prod_id = get_product_id($::FORM{'product'});
    $prod_id ||
      ThrowUserError("invalid_product_name", {product => $::FORM{'product'}});
      
    DoComma();
    $::query .= "product_id = $prod_id";
} else {
    SendSQL("SELECT DISTINCT product_id FROM bugs WHERE bug_id IN (" .
            join(',', @idlist) . ") " . $dbh->sql_limit(2));
    $prod_id = FetchOneColumn();
    $prod_id = undef if (FetchOneColumn());
}

my $comp_id; # Remember, can't use this for mass changes
if ($::FORM{'component'} ne $::FORM{'dontchange'}) {
    if (!defined $prod_id) {
        ThrowUserError("no_component_change_for_multiple_products");
    }
    $comp_id = get_component_id($prod_id,
                                $::FORM{'component'});
    $comp_id || ThrowCodeError("invalid_component", 
                               {name => $::FORM{'component'},
                                product => $::FORM{'product'}});
    
    DoComma();
    $::query .= "component_id = $comp_id";
}

# If this installation uses bug aliases, and the user is changing the alias,
# add this change to the query.
if (Param("usebugaliases") && defined($::FORM{'alias'})) {
    my $alias = trim($::FORM{'alias'});
    
    # Since aliases are unique (like bug numbers), they can only be changed
    # for one bug at a time, so ignore the alias change unless only a single
    # bug is being changed.
    if (scalar(@idlist) == 1) {
        # Validate the alias if the user entered one.
        if ($alias ne "") {
            # Make sure the alias isn't too long.
            if (length($alias) > 20) {
                ThrowUserError("alias_too_long");
            }

            # Make sure the alias is unique.
            my $escaped_alias = SqlQuote($alias);
            my $vars = { alias => $alias };
            
            SendSQL("SELECT bug_id FROM bugs WHERE alias = $escaped_alias " . 
                    "AND bug_id != $idlist[0]");
            my $id = FetchOneColumn();
            
            if ($id) {
                $vars->{'bug_link'} = GetBugLink($id, "Bug $id");
                ThrowUserError("alias_in_use", $vars);
            }

            # Make sure the alias isn't just a number.
            if ($alias =~ /^\d+$/) {
                ThrowUserError("alias_is_numeric", $vars);
            }

            # Make sure the alias has no commas or spaces.
            if ($alias =~ /[, ]/) {
                ThrowUserError("alias_has_comma_or_space", $vars);
            }
        }
        
        # Add the alias change to the query.  If the field contains the blank 
        # value, make the field be NULL to indicate that the bug has no alias.
        # Otherwise, if the field contains a value, update the record 
        # with that value.
        DoComma();
        $::query .= "alias = ";
        if ($alias eq "") {
            $::query .= "NULL";
        } else {
            $::query .= SqlQuote($alias);
        }
    }
}

# If the user is submitting changes from show_bug.cgi for a single bug,
# and that bug is restricted to a group, process the checkboxes that
# allowed the user to set whether or not the reporter
# and cc list can see the bug even if they are not members of all groups 
# to which the bug is restricted.
if ( $::FORM{'id'} ) {
    SendSQL("SELECT group_id FROM bug_group_map WHERE bug_id = $::FORM{'id'}");
    my ($havegroup) = FetchSQLData();
    if ( $havegroup ) {
        DoComma();
        $::FORM{'reporter_accessible'} = $::FORM{'reporter_accessible'} ? '1' : '0';
        $::query .= "reporter_accessible = $::FORM{'reporter_accessible'}";

        DoComma();
        $::FORM{'cclist_accessible'} = $::FORM{'cclist_accessible'} ? '1' : '0';
        $::query .= "cclist_accessible = $::FORM{'cclist_accessible'}";
    }
}

if ($::FORM{'id'} && 
    (Param("insidergroup") && UserInGroup(Param("insidergroup")))) {
    detaint_natural($::FORM{'id'});
    foreach my $field (keys %::FORM) {
        if ($field =~ /when-([0-9]+)/) {
            my $sequence = $1;
            my $private = $::FORM{"isprivate-$sequence"} ? 1 : 0 ;
            if ($private != $::FORM{"oisprivate-$sequence"}) {
                detaint_natural($::FORM{"$field"});
                SendSQL("UPDATE longdescs SET isprivate = $private 
                    WHERE bug_id = $::FORM{'id'} AND bug_when = " . $::FORM{"$field"});
            }
        }

    }
}

my $duplicate = 0;

# We need to check the addresses involved in a CC change before we touch any bugs.
# What we'll do here is formulate the CC data into two hashes of ID's involved
# in this CC change.  Then those hashes can be used later on for the actual change.
my (%cc_add, %cc_remove);
if (defined $::FORM{newcc} || defined $::FORM{'addselfcc'} || defined $::FORM{removecc} || defined $::FORM{masscc}) {
    # If masscc is defined, then we came from buglist and need to either add or
    # remove cc's... otherwise, we came from bugform and may need to do both.
    my ($cc_add, $cc_remove) = "";
    if (defined $::FORM{masscc}) {
        if ($::FORM{ccaction} eq 'add') {
            $cc_add = $::FORM{masscc};
        } elsif ($::FORM{ccaction} eq 'remove') {
            $cc_remove = $::FORM{masscc};
        }
    } else {
        $cc_add = $::FORM{newcc};
        # We came from bug_form which uses a select box to determine what cc's
        # need to be removed...
        if (defined $::FORM{removecc} && $::FORM{cc}) {
            $cc_remove = join (",", @{$::MFORM{cc}});
        }
    }

    if ($cc_add) {
        $cc_add =~ s/[\s,]+/ /g; # Change all delimiters to a single space
        foreach my $person ( split(" ", $cc_add) ) {
            my $pid = DBNameToIdAndCheck($person);
            $cc_add{$pid} = $person;
        }
    }
    if ($::FORM{'addselfcc'}) {
        $cc_add{$whoid} = $user->login;
    }
    if ($cc_remove) {
        $cc_remove =~ s/[\s,]+/ /g; # Change all delimiters to a single space
        foreach my $person ( split(" ", $cc_remove) ) {
            my $pid = DBNameToIdAndCheck($person);
            $cc_remove{$pid} = $person;
        }
    }
}

# Store the new assignee and QA contact IDs (if any). This is the
# only way to keep these informations when bugs are reassigned by
# component as $::FORM{'assigned_to'} and $::FORM{'qa_contact'}
# are not the right fields to look at.

my $assignee;
my $qacontact;

if (defined $::FORM{'qa_contact'}
    && $::FORM{'knob'} ne "reassignbycomponent")
{
    $qacontact = 0;
    my $name = trim($::FORM{'qa_contact'});
    # The QA contact cannot be deleted from show_bug.cgi for a single bug!
    if ($name ne $::FORM{'dontchange'}) {
        $qacontact = DBNameToIdAndCheck($name) if ($name ne "");
        DoComma();
        $::query .= "qa_contact = $qacontact";
    }
}

CheckFormFieldDefined(\%::FORM, 'knob');
SWITCH: for ($::FORM{'knob'}) {
    /^none$/ && do {
        last SWITCH;
    };
    /^confirm$/ && CheckonComment( "confirm" ) && do {
        DoConfirm();
        ChangeStatus('NEW');
        last SWITCH;
    };
    /^accept$/ && CheckonComment( "accept" ) && do {
        DoConfirm();
        ChangeStatus('ASSIGNED');
        if (Param("usetargetmilestone") && Param("musthavemilestoneonaccept")) {
            $requiremilestone = 1;
        }
        last SWITCH;
    };
    /^clearresolution$/ && CheckonComment( "clearresolution" ) && do {
        ChangeResolution('');
        last SWITCH;
    };
    /^resolve$/ && CheckonComment( "resolve" ) && do {
        # Check here, because its the only place we require the resolution
        CheckFormField(\%::FORM, 'resolution', \@::settable_resolution);

        # don't resolve as fixed while still unresolved blocking bugs
        if (Param("noresolveonopenblockers")
            && $::FORM{'resolution'} eq 'FIXED')
        {
            my @dependencies = Bugzilla::Bug::CountOpenDependencies(@idlist);
            if (scalar @dependencies > 0) {
                ThrowUserError("still_unresolved_bugs",
                               { dependencies     => \@dependencies,
                                 dependency_count => scalar @dependencies });
            }
        }

        # RESOLVED bugs should have no time remaining;
        # more time can be added for the VERIFY step, if needed.
        _remove_remaining_time();

        ChangeStatus('RESOLVED');
        ChangeResolution($::FORM{'resolution'});
        last SWITCH;
    };
    /^reassign$/ && CheckonComment( "reassign" ) && do {
        if ($::FORM{'andconfirm'}) {
            DoConfirm();
        }
        ChangeStatus('NEW');
        DoComma();
        if (!defined $::FORM{'assigned_to'}
            || trim($::FORM{'assigned_to'}) eq "")
        {
            ThrowUserError("reassign_to_empty");
        }
        $assignee = DBNameToIdAndCheck(trim($::FORM{'assigned_to'}));
        $::query .= "assigned_to = $assignee";
        last SWITCH;
    };
    /^reassignbycomponent$/  && CheckonComment( "reassignbycomponent" ) && do {
        if ($::FORM{'product'} eq $::FORM{'dontchange'}) {
            ThrowUserError("need_product");
        }
        if ($::FORM{'component'} eq $::FORM{'dontchange'}) {
            ThrowUserError("need_component");
        }
        if ($::FORM{'compconfirm'}) {
            DoConfirm();
        }
        ChangeStatus('NEW');
        SendSQL("SELECT initialowner FROM components " .
                "WHERE components.id = $comp_id");
        $assignee = FetchOneColumn();
        DoComma();
        $::query .= "assigned_to = $assignee";
        if (Param("useqacontact")) {
            SendSQL("SELECT initialqacontact FROM components " .
                    "WHERE components.id = $comp_id");
            $qacontact = FetchOneColumn() || 0;
            DoComma();
            $::query .= "qa_contact = $qacontact";
        }
        last SWITCH;
    };   
    /^reopen$/  && CheckonComment( "reopen" ) && do {
        ChangeStatus('REOPENED');
        ChangeResolution('');
        last SWITCH;
    };
    /^verify$/ && CheckonComment( "verify" ) && do {
        ChangeStatus('VERIFIED');
        last SWITCH;
    };
    /^close$/ && CheckonComment( "close" ) && do {
        # CLOSED bugs should have no time remaining.
        _remove_remaining_time();

        ChangeStatus('CLOSED');
        last SWITCH;
    };
    /^duplicate$/ && CheckonComment( "duplicate" ) && do {
        # Make sure we can change the original bug (issue A on bug 96085)
        CheckFormFieldDefined(\%::FORM, 'dup_id');
        ValidateBugID($::FORM{'dup_id'}, 'dup_id');

        # Also, let's see if the reporter has authorization to see
        # the bug to which we are duping. If not we need to prompt.
        DuplicateUserConfirm();

        $duplicate = $::FORM{'dup_id'};
        if (!defined($::FORM{'id'}) || $duplicate == $::FORM{'id'}) {
            ThrowUserError("dupe_of_self_disallowed");
        }

        # DUPLICATE bugs should have no time remaining.
        _remove_remaining_time();

        ChangeStatus('RESOLVED');
        ChangeResolution('DUPLICATE');
        $::FORM{'comment'} .= "\n\n*** This bug has been marked " .
                              "as a duplicate of $duplicate ***";
        last SWITCH;
    };

    ThrowCodeError("unknown_action", { action => $::FORM{'knob'} });
}


if ($#idlist < 0) {
    ThrowUserError("no_bugs_chosen");
}


my @keywordlist;
my %keywordseen;

if ($::FORM{'keywords'}) {
    foreach my $keyword (split(/[\s,]+/, $::FORM{'keywords'})) {
        if ($keyword eq '') {
            next;
        }
        my $i = GetKeywordIdFromName($keyword);
        if (!$i) {
            ThrowUserError("unknown_keyword",
                           { keyword => $keyword });
        }
        if (!$keywordseen{$i}) {
            push(@keywordlist, $i);
            $keywordseen{$i} = 1;
        }
    }
}

my $keywordaction = $::FORM{'keywordaction'} || "makeexact";
if (!grep($keywordaction eq $_, qw(add delete makeexact))) {
    $keywordaction = "makeexact";
}

if ($::comma eq ""
    && (! @groupAdd) && (! @groupDel)
    && (! @::legal_keywords || (0 == @keywordlist && $keywordaction ne "makeexact"))
    && defined $::FORM{'masscc'} && ! $::FORM{'masscc'}
    ) {
    if (!defined $::FORM{'comment'} || $::FORM{'comment'} =~ /^\s*$/) {
        ThrowUserError("bugs_not_changed");
    }
}

# Process data for Time Tracking fields
if (UserInGroup(Param('timetrackinggroup'))) {
    foreach my $field ("estimated_time", "remaining_time") {
        if (defined $::FORM{$field}) {
            my $er_time = trim($::FORM{$field});
            if ($er_time ne $::FORM{'dontchange'}) {
                DoComma();
                $::query .= "$field = " . SqlQuote($er_time);
            }
        }
    }

    if (defined $::FORM{'deadline'}) {
        DoComma();
        $::query .= "deadline = ";
        if ($::FORM{'deadline'}) {
            Bugzilla::Util::ValidateDate($::FORM{'deadline'}, 'YYYY-MM-DD');
            $::query .= SqlQuote($::FORM{'deadline'});
        } else {
            $::query .= "NULL" ;
        }
    }
}

my $basequery = $::query;
my $delta_ts;


sub SnapShotBug {
    my ($id) = (@_);
    SendSQL("select delta_ts, " . join(',', @::log_columns) .
            " from bugs where bug_id = $id");
    my @row = FetchSQLData();
    $delta_ts = shift @row;

    return @row;
}


sub SnapShotDeps {
    my ($i, $target, $me) = (@_);
    SendSQL("select $target from dependencies where $me = $i order by $target");
    my @list;
    while (MoreSQLData()) {
        push(@list, FetchOneColumn());
    }
    return join(',', @list);
}


my $timestamp;
my $bug_changed;

sub FindWrapPoint {
    my ($string, $startpos) = @_;
    if (!$string) { return 0 }
    if (length($string) < $startpos) { return length($string) }
    my $wrappoint = rindex($string, ",", $startpos); # look for comma
    if ($wrappoint < 0) {  # can't find comma
        $wrappoint = rindex($string, " ", $startpos); # look for space
        if ($wrappoint < 0) {  # can't find space
            $wrappoint = rindex($string, "-", $startpos); # look for hyphen
            if ($wrappoint < 0) {  # can't find hyphen
                $wrappoint = $startpos;  # just truncate it
            } else {
                $wrappoint++; # leave hyphen on the left side
            }
        }
    }
    return $wrappoint;
}

sub LogDependencyActivity {
    my ($i, $oldstr, $target, $me, $timestamp) = (@_);
    my $sql_timestamp = SqlQuote($timestamp);
    my $newstr = SnapShotDeps($i, $target, $me);
    if ($oldstr ne $newstr) {
        # Figure out what's really different...
        my ($removed, $added) = diff_strings($oldstr, $newstr);
        LogActivityEntry($i,$target,$removed,$added,$whoid,$timestamp);
        # update timestamp on target bug so midairs will be triggered
        SendSQL("UPDATE bugs SET delta_ts = $sql_timestamp WHERE bug_id = $i");
        $bug_changed = 1;
        return 1;
    }
    return 0;
}

# this loop iterates once for each bug to be processed (eg when this script
# is called with multiple bugs selected from buglist.cgi instead of
# show_bug.cgi).
#
foreach my $id (@idlist) {
    my %dependencychanged;
    $bug_changed = 0;
    my $write = "WRITE";        # Might want to make a param to control
                                # whether we do LOW_PRIORITY ...
    $dbh->bz_lock_tables("bugs $write", "bugs_activity $write",
            "cc $write", "cc AS selectVisible_cc $write",
            "profiles $write", "dependencies $write", "votes $write",
            "products READ", "components READ",
            "keywords $write", "longdescs $write", "fielddefs $write",
            "bug_group_map $write", "flags $write", "duplicates $write",
            # user_group_map would be a READ lock except that Flag::process
            # may call Flag::notify, which creates a new user object,
            # which might call derive_groups, which wants a WRITE lock on that
            # table. group_group_map is in here at all because derive_groups
            # needs it.
            "user_group_map $write", "group_group_map READ", "flagtypes READ",
            "flaginclusions AS i READ", "flagexclusions AS e READ",
            "keyworddefs READ", "groups READ", "attachments READ",
            "group_control_map AS oldcontrolmap READ",
            "group_control_map AS newcontrolmap READ",
            "group_control_map READ");
    # Fun hack.  @::log_columns only contains the component_id,
    # not the name (since bug 43600 got fixed).  So, we need to have
    # this id ready for the loop below, otherwise anybody can
    # change the component of a bug (we checked product above).
    # http://bugzilla.mozilla.org/show_bug.cgi?id=180545
    my $product_id = get_product_id($::FORM{'product'});
    
    if ($::FORM{'component'} ne $::FORM{'dontchange'}) {
        $::FORM{'component_id'} = 
                            get_component_id($product_id, $::FORM{'component'});
    }

    # It may sound crazy to set %formhash for each bug as $::FORM{}
    # will not change, but %formhash is modified below and we prefer
    # to set it again.
    my $i = 0;
    my @oldvalues = SnapShotBug($id);
    my %oldhash;
    my %formhash;
    foreach my $col (@::log_columns) {
        # Consider NULL db entries to be equivalent to the empty string
        $oldvalues[$i] = '' unless defined $oldvalues[$i];
        $oldhash{$col} = $oldvalues[$i];
        $formhash{$col} = $::FORM{$col} if defined $::FORM{$col};
        $i++;
    }
    # If the user is reassigning bugs, we need to:
    # - convert $newhash{'assigned_to'} and $newhash{'qa_contact'}
    #   email addresses into their corresponding IDs;
    # - update $newhash{'bug_status'} to its real state if the bug
    #   is in the unconfirmed state.
    $formhash{'qa_contact'} = $qacontact if Param('useqacontact');
    if ($::FORM{'knob'} eq 'reassignbycomponent'
        || $::FORM{'knob'} eq 'reassign')
    {
        $formhash{'assigned_to'} = $assignee;
        if ($oldhash{'bug_status'} eq 'UNCONFIRMED') {
            $formhash{'bug_status'} = $oldhash{'bug_status'};
        }
    }
    foreach my $col (@::log_columns) {
        if (exists $formhash{$col}
            && !CheckCanChangeField($col, $id, $oldhash{$col}, $formhash{$col}))
        {
            my $vars;
            if ($col eq 'component_id') {
                # Display the component name
                $vars->{'oldvalue'} = get_component_name($oldhash{$col});
                $vars->{'newvalue'} = $::FORM{'component'};
                $vars->{'field'} = 'component';
            } elsif ($col eq 'assigned_to' || $col eq 'qa_contact') {
                # Display the assignee or QA contact email address
                $vars->{'oldvalue'} = DBID_to_name($oldhash{$col});
                $vars->{'newvalue'} = DBID_to_name($formhash{$col});
                $vars->{'field'} = $col;
            } else {
                $vars->{'oldvalue'} = $oldhash{$col};
                $vars->{'newvalue'} = $formhash{$col};
                $vars->{'field'} = $col;
            }
            $vars->{'privs'} = $PrivilegesRequired;
            ThrowUserError("illegal_change", $vars);
        }
    }

    # When editing multiple bugs, users can specify a list of keywords to delete
    # from bugs.  If the list matches the current set of keywords on those bugs,
    # CheckCanChangeField above will fail to check permissions because it thinks
    # the list hasn't changed.  To fix that, we have to call CheckCanChangeField
    # again with old!=new if the keyword action is "delete" and old=new.
    if ($keywordaction eq "delete"
        && exists $::FORM{keywords}
        && length(@keywordlist) > 0
        && $::FORM{keywords} eq $oldhash{keywords}
        && !CheckCanChangeField("keywords", $id, "old is not", "equal to new"))
    {
        $vars->{'oldvalue'} = $oldhash{keywords};
        $vars->{'newvalue'} = "no keywords";
        $vars->{'field'} = "keywords";
        $vars->{'privs'} = $PrivilegesRequired;
        ThrowUserError("illegal_change", $vars);
    }

    $oldhash{'product'} = get_product_name($oldhash{'product_id'});
    if (!CanEditProductId($oldhash{'product_id'})) {
        ThrowUserError("product_edit_denied",
                      { product => $oldhash{'product'} });
    }

    if (defined $::FORM{'product'} 
        && $::FORM{'product'} ne $::FORM{'dontchange'} 
        && $::FORM{'product'} ne $oldhash{'product'}
        && !CanEnterProduct($::FORM{'product'})) {
        ThrowUserError("entry_access_denied",
                       { product => $::FORM{'product'} });
    }
    if ($requiremilestone) {
        # musthavemilestoneonaccept applies only if at least two
        # target milestones are defined for the current product.
        my $nb_milestones = scalar(@{$::target_milestone{$oldhash{'product'}}});
        if ($nb_milestones > 1) {
            my $value = $cgi->param('target_milestone');
            if (!defined $value || $value eq $cgi->param('dontchange')) {
                $value = $oldhash{'target_milestone'};
            }
            my $defaultmilestone =
                $dbh->selectrow_array("SELECT defaultmilestone
                                       FROM products WHERE id = ?",
                                       undef, $oldhash{'product_id'});
            # if musthavemilestoneonaccept == 1, then the target
            # milestone must be different from the default one.
            if ($value eq $defaultmilestone) {
                ThrowUserError("milestone_required", { bug_id => $id });
            }
        }
    }   
    if (defined $::FORM{'delta_ts'} && $::FORM{'delta_ts'} ne $delta_ts) {
        ($vars->{'operations'}) = GetBugActivity($::FORM{'id'}, $::FORM{'delta_ts'});

        $vars->{'start_at'} = $::FORM{'longdesclength'};
        $vars->{'comments'} = Bugzilla::Bug::GetComments($id);

        $::FORM{'delta_ts'} = $delta_ts;
        $vars->{'form'} = \%::FORM;
        $vars->{'mform'} = \%::MFORM;
        
        $vars->{'bug_id'} = $id;
        
        $dbh->bz_unlock_tables(UNLOCK_ABORT);
        
        # Warn the user about the mid-air collision and ask them what to do.
        $template->process("bug/process/midair.html.tmpl", $vars)
          || ThrowTemplateError($template->error());
        exit;
    }
        
    my %deps;
    if (defined $::FORM{'dependson'}) {
        my $me = "blocked";
        my $target = "dependson";
        my %deptree;
        for (1..2) {
            $deptree{$target} = [];
            my %seen;
            foreach my $i (split('[\s,]+', $::FORM{$target})) {
                next if $i eq "";
                
                if ($id eq $i) {
                    ThrowUserError("dependency_loop_single");
                }
                if (!exists $seen{$i}) {
                    push(@{$deptree{$target}}, $i);
                    $seen{$i} = 1;
                }
            }
            # populate $deps{$target} as first-level deps only.
            # and find remainder of dependency tree in $deptree{$target}
            @{$deps{$target}} = @{$deptree{$target}};
            my @stack = @{$deps{$target}};
            while (@stack) {
                my $i = shift @stack;
                SendSQL("select $target from dependencies where $me = " .
                        SqlQuote($i));
                while (MoreSQLData()) {
                    my $t = FetchOneColumn();
                    # ignore any _current_ dependencies involving this bug,
                    # as they will be overwritten with data from the form.
                    if ($t != $id && !exists $seen{$t}) {
                        push(@{$deptree{$target}}, $t);
                        push @stack, $t;
                        $seen{$t} = 1;
                    }
                }
            }

            if ($me eq 'dependson') {
                my @deps   =  @{$deptree{'dependson'}};
                my @blocks =  @{$deptree{'blocked'}};
                my @union = ();
                my @isect = ();
                my %union = ();
                my %isect = ();
                foreach my $b (@deps, @blocks) { $union{$b}++ && $isect{$b}++ }
                @union = keys %union;
                @isect = keys %isect;
                if (@isect > 0) {
                    my $both;
                    foreach my $i (@isect) {
                       $both = $both . GetBugLink($i, "#" . $i) . " ";
                    }

                    ThrowUserError("dependency_loop_multi",
                                   { both => $both });
                }
            }
            my $tmp = $me;
            $me = $target;
            $target = $tmp;
        }
    }

    #
    # Start updating the relevant database entries
    #

    SendSQL("select now()");
    $timestamp = FetchOneColumn();
    my $sql_timestamp = SqlQuote($timestamp);

    my $work_time;
    if (UserInGroup(Param('timetrackinggroup'))) {
        $work_time = $::FORM{'work_time'};
        if ($work_time) {
            # AppendComment (called below) can in theory raise an error,
            # but because we've already validated work_time here it's
            # safe to log the entry before adding the comment.
            LogActivityEntry($id, "work_time", "", $::FORM{'work_time'},
                             $whoid, $timestamp);
        }
    }

    if ($::FORM{'comment'} || $work_time) {
        AppendComment($id, Bugzilla->user->login, $::FORM{'comment'},
                      $::FORM{'commentprivacy'}, $timestamp, $work_time);
        $bug_changed = 1;
    }

    if (@::legal_keywords && exists $::FORM{keywords}) {
        # There are three kinds of "keywordsaction": makeexact, add, delete.
        # For makeexact, we delete everything, and then add our things.
        # For add, we delete things we're adding (to make sure we don't
        # end up having them twice), and then we add them.
        # For delete, we just delete things on the list.
        my $changed = 0;
        if ($keywordaction eq "makeexact") {
            SendSQL("DELETE FROM keywords WHERE bug_id = $id");
            $changed = 1;
        }
        foreach my $keyword (@keywordlist) {
            if ($keywordaction ne "makeexact") {
                SendSQL("DELETE FROM keywords
                         WHERE bug_id = $id AND keywordid = $keyword");
                $changed = 1;
            }
            if ($keywordaction ne "delete") {
                SendSQL("INSERT INTO keywords 
                         (bug_id, keywordid) VALUES ($id, $keyword)");
                $changed = 1;
            }
        }
        if ($changed) {
            SendSQL("SELECT keyworddefs.name 
                     FROM keyworddefs, keywords
                     WHERE keywords.bug_id = $id
                         AND keyworddefs.id = keywords.keywordid
                     ORDER BY keyworddefs.name");
            my @list;
            while (MoreSQLData()) {
                push(@list, FetchOneColumn());
            }
            SendSQL("UPDATE bugs SET delta_ts = $sql_timestamp, keywords = " .
                    SqlQuote(join(', ', @list)) .
                    " WHERE bug_id = $id");
        }
    }
    my $query = "$basequery\nwhere bug_id = $id";
    
    if ($::comma ne "") {
        SendSQL($query);
    }

    # Check for duplicates if the bug is [re]open
    SendSQL("SELECT resolution FROM bugs WHERE bug_id = $id");
    my $resolution = FetchOneColumn();
    if ($resolution eq '') {
        SendSQL("DELETE FROM duplicates WHERE dupe = $id");
    }
    
    my $newproduct_id = $oldhash{'product_id'};
    if ((defined $::FORM{'product'})
        && ($::FORM{'product'} ne $::FORM{'dontchange'})) {
        my $newproduct_id = get_product_id($::FORM{'product'});
    }

    my %groupsrequired = ();
    my %groupsforbidden = ();
    SendSQL("SELECT id, membercontrol 
             FROM groups LEFT JOIN group_control_map
             ON id = group_id
             AND product_id = $newproduct_id WHERE isactive != 0");
    while (MoreSQLData()) {
        my ($group, $control) = FetchSQLData();
        $control ||= 0;
        unless ($control > &::CONTROLMAPNA)  {
            $groupsforbidden{$group} = 1;
        }
        if ($control == &::CONTROLMAPMANDATORY) {
            $groupsrequired{$group} = 1;
        }
    }

    my @groupAddNames = ();
    my @groupAddNamesAll = ();
    foreach my $grouptoadd (@groupAdd, keys %groupsrequired) {
        next if $groupsforbidden{$grouptoadd};
        push(@groupAddNamesAll, GroupIdToName($grouptoadd));
        if (!BugInGroupId($id, $grouptoadd)) {
            push(@groupAddNames, GroupIdToName($grouptoadd));
            SendSQL("INSERT INTO bug_group_map (bug_id, group_id) 
                     VALUES ($id, $grouptoadd)");
        }
    }
    my @groupDelNames = ();
    my @groupDelNamesAll = ();
    foreach my $grouptodel (@groupDel, keys %groupsforbidden) {
        push(@groupDelNamesAll, GroupIdToName($grouptodel));
        next if $groupsrequired{$grouptodel};
        if (BugInGroupId($id, $grouptodel)) {
            push(@groupDelNames, GroupIdToName($grouptodel));
        }
        SendSQL("DELETE FROM bug_group_map 
                 WHERE bug_id = $id AND group_id = $grouptodel");
    }

    my $groupDelNames = join(',', @groupDelNames);
    my $groupAddNames = join(',', @groupAddNames);

    if ($groupDelNames ne $groupAddNames) {
        LogActivityEntry($id, "bug_group", $groupDelNames, $groupAddNames,
                         $whoid, $timestamp); 
        $bug_changed = 1;
    }

    my @ccRemoved = (); 
    if (defined $::FORM{newcc} || defined $::FORM{'addselfcc'} || defined $::FORM{removecc} || defined $::FORM{masscc}) {
        # Get the current CC list for this bug
        my %oncc;
        SendSQL("SELECT who FROM cc WHERE bug_id = $id");
        while (MoreSQLData()) {
            $oncc{FetchOneColumn()} = 1;
        }

        my (@added, @removed) = ();
        foreach my $pid (keys %cc_add) {
            # If this person isn't already on the cc list, add them
            if (! $oncc{$pid}) {
                SendSQL("INSERT INTO cc (bug_id, who) VALUES ($id, $pid)");
                push (@added, $cc_add{$pid});
                $oncc{$pid} = 1;
            }
        }
        foreach my $pid (keys %cc_remove) {
            # If the person is on the cc list, remove them
            if ($oncc{$pid}) {
                SendSQL("DELETE FROM cc WHERE bug_id = $id AND who = $pid");
                push (@removed, $cc_remove{$pid});
                $oncc{$pid} = 0;
            }
        }

        # If any changes were found, record it in the activity log
        if (scalar(@removed) || scalar(@added)) {
            my $removed = join(", ", @removed);
            my $added = join(", ", @added);
            LogActivityEntry($id,"cc",$removed,$added,$whoid,$timestamp);
            $bug_changed = 1;
        }
        @ccRemoved = @removed;
    }

    # We need to send mail for dependson/blocked bugs if the dependencies
    # change or the status or resolution change. This var keeps track of that.
    my $check_dep_bugs = 0;

    if (defined $::FORM{'dependson'}) {
        my $me = "blocked";
        my $target = "dependson";
        for (1..2) {
            SendSQL("select $target from dependencies where $me = $id order by $target");
            my %snapshot;
            my @oldlist;
            while (MoreSQLData()) {
                push(@oldlist, FetchOneColumn());
            }
            my @newlist = sort {$a <=> $b} @{$deps{$target}};
            @dependencychanged{@oldlist} = 1;
            @dependencychanged{@newlist} = 1;

            while (0 < @oldlist || 0 < @newlist) {
                if (@oldlist == 0 || (@newlist > 0 &&
                                      $oldlist[0] > $newlist[0])) {
                    $snapshot{$newlist[0]} = SnapShotDeps($newlist[0], $me,
                                                          $target);
                    shift @newlist;
                } elsif (@newlist == 0 || (@oldlist > 0 &&
                                           $newlist[0] > $oldlist[0])) {
                    $snapshot{$oldlist[0]} = SnapShotDeps($oldlist[0], $me,
                                                          $target);
                    shift @oldlist;
                } else {
                    if ($oldlist[0] != $newlist[0]) {
                        $dbh->bz_unlock_tables(UNLOCK_ABORT);
                        die "Error in list comparing code";
                    }
                    shift @oldlist;
                    shift @newlist;
                }
            }
            my @keys = keys(%snapshot);
            if (@keys) {
                my $oldsnap = SnapShotDeps($id, $target, $me);
                SendSQL("delete from dependencies where $me = $id");
                foreach my $i (@{$deps{$target}}) {
                    SendSQL("insert into dependencies ($me, $target) values ($id, $i)");
                }
                foreach my $k (@keys) {
                    LogDependencyActivity($k, $snapshot{$k}, $me, $target, $timestamp);
                }
                LogDependencyActivity($id, $oldsnap, $target, $me, $timestamp);
                $check_dep_bugs = 1;
            }

            my $tmp = $me;
            $me = $target;
            $target = $tmp;
        }
    }

    # When a bug changes products and the old or new product is associated
    # with a bug group, it may be necessary to remove the bug from the old
    # group or add it to the new one.  There are a very specific series of
    # conditions under which these activities take place, more information
    # about which can be found in comments within the conditionals below.
    # Check if the user has changed the product to which the bug belongs;
    if ( 
      defined $::FORM{'product'} 
        && $::FORM{'product'} ne $::FORM{'dontchange'} 
          && $::FORM{'product'} ne $oldhash{'product'} 
    ) {
        my $newproduct_id = get_product_id($::FORM{'product'});
        # Depending on the "addtonewgroup" variable, groups with
        # defaults will change.
        #
        # For each group, determine
        # - The group id and if it is active
        # - The control map value for the old product and this group
        # - The control map value for the new product and this group
        # - Is the user in this group?
        # - Is the bug in this group?
        SendSQL("SELECT DISTINCT groups.id, isactive, " .
                "oldcontrolmap.membercontrol, newcontrolmap.membercontrol, " .
                "user_group_map.user_id IS NOT NULL, " .
                "bug_group_map.group_id IS NOT NULL " .
                "FROM groups " .
                "LEFT JOIN group_control_map AS oldcontrolmap " .
                "ON oldcontrolmap.group_id = groups.id " .
                "AND oldcontrolmap.product_id = " . $oldhash{'product_id'} .
                " LEFT JOIN group_control_map AS newcontrolmap " .
                "ON newcontrolmap.group_id = groups.id " .
                "AND newcontrolmap.product_id = $newproduct_id " .
                "LEFT JOIN user_group_map " .
                "ON user_group_map.group_id = groups.id " .
                "AND user_group_map.user_id = $whoid " .
                "AND user_group_map.isbless = 0 " .
                "LEFT JOIN bug_group_map " .
                "ON bug_group_map.group_id = groups.id " .
                "AND bug_group_map.bug_id = $id "
            );
        my @groupstoremove = ();
        my @groupstoadd = ();
        my @defaultstoremove = ();
        my @defaultstoadd = ();
        my @allgroups = ();
        my $buginanydefault = 0;
        my $buginanychangingdefault = 0;
        while (MoreSQLData()) {
            my ($groupid, $isactive, $oldcontrol, $newcontrol, 
            $useringroup, $bugingroup) = FetchSQLData();
            # An undefined newcontrol is none.
            $newcontrol = CONTROLMAPNA unless $newcontrol;
            $oldcontrol = CONTROLMAPNA unless $oldcontrol;
            push(@allgroups, $groupid);
            if (($bugingroup) && ($isactive)
                && ($oldcontrol == CONTROLMAPDEFAULT)) {
                # Bug was in a default group.
                $buginanydefault = 1;
                if (($newcontrol != CONTROLMAPDEFAULT)
                    && ($newcontrol != CONTROLMAPMANDATORY)) {
                    # Bug was in a default group that no longer is.
                    $buginanychangingdefault = 1;
                    push (@defaultstoremove, $groupid);
                }
            }
            if (($isactive) && (!$bugingroup)
                && ($newcontrol == CONTROLMAPDEFAULT)
                && ($useringroup)) {
                push (@defaultstoadd, $groupid);
            }
            if (($bugingroup) && ($isactive) && ($newcontrol == CONTROLMAPNA)) {
                # Group is no longer permitted.
                push(@groupstoremove, $groupid);
            }
            if ((!$bugingroup) && ($isactive) 
                && ($newcontrol == CONTROLMAPMANDATORY)) {
                # Group is now required.
                push(@groupstoadd, $groupid);
            }
        }
        # If addtonewgroups = "yes", old default groups will be removed
        # and new default groups will be added.
        # If addtonewgroups = "yesifinold", old default groups will be removed
        # and new default groups will be added only if the bug was in ANY
        # of the old default groups.
        # If addtonewgroups = "no", old default groups will be removed and not
        # replaced.
        push(@groupstoremove, @defaultstoremove);
        if (AnyDefaultGroups()
            && (($::FORM{'addtonewgroup'} eq 'yes')
            || (($::FORM{'addtonewgroup'} eq 'yesifinold') 
            && ($buginanydefault)))) {
            push(@groupstoadd, @defaultstoadd);
        }

        # Now actually update the bug_group_map.
        my @DefGroupsAdded = ();
        my @DefGroupsRemoved = ();
        foreach my $groupid (@allgroups) {
            my $thisadd = grep( ($_ == $groupid), @groupstoadd);
            my $thisdel = grep( ($_ == $groupid), @groupstoremove);
            if ($thisadd) {
                push(@DefGroupsAdded, GroupIdToName($groupid));
                SendSQL("INSERT INTO bug_group_map (bug_id, group_id) VALUES " .
                        "($id, $groupid)");
            } elsif ($thisdel) {
                push(@DefGroupsRemoved, GroupIdToName($groupid));
                SendSQL("DELETE FROM bug_group_map WHERE bug_id = $id " .
                        "AND group_id = $groupid");
            }
        }
        if ((@DefGroupsAdded) || (@DefGroupsRemoved)) {
            LogActivityEntry($id, "bug_group",
                join(', ', @DefGroupsRemoved),
                join(', ', @DefGroupsAdded),
                     $whoid, $timestamp); 
        }
    }
  
    # get a snapshot of the newly set values out of the database, 
    # and then generate any necessary bug activity entries by seeing 
    # what has changed since before we wrote out the new values.
    #
    my @newvalues = SnapShotBug($id);
    my %newhash;
    $i = 0;
    foreach my $col (@::log_columns) {
        # Consider NULL db entries to be equivalent to the empty string
        $newvalues[$i] ||= '';
        $newhash{$col} = $newvalues[$i];
        $i++;
    }
    # for passing to Bugzilla::BugMail to ensure that when someone is removed
    # from one of these fields, they get notified of that fact (if desired)
    #
    my $origOwner = "";
    my $origQaContact = "";
    
    foreach my $c (@::log_columns) {
        my $col = $c;           # We modify it, don't want to modify array
                                # values in place.
        my $old = shift @oldvalues;
        my $new = shift @newvalues;
        if ($old ne $new) {

            # Products and components are now stored in the DB using ID's
            # We need to translate this to English before logging it
            if ($col eq 'product_id') {
                $old = get_product_name($old);
                $new = get_product_name($new);
                $col = 'product';
            }
            if ($col eq 'component_id') {
                $old = get_component_name($old);
                $new = get_component_name($new);
                $col = 'component';
            }

            # save off the old value for passing to Bugzilla::BugMail so
            # the old owner can be notified
            #
            if ($col eq 'assigned_to') {
                $old = ($old) ? DBID_to_name($old) : "";
                $new = ($new) ? DBID_to_name($new) : "";
                $origOwner = $old;
            }

            # ditto for the old qa contact
            #
            if ($col eq 'qa_contact') {
                $old = ($old) ? DBID_to_name($old) : "";
                $new = ($new) ? DBID_to_name($new) : "";
                $origQaContact = $old;
            }

            # If this is the keyword field, only record the changes, not everything.
            if ($col eq 'keywords') {
                ($old, $new) = diff_strings($old, $new);
            }

            if ($col eq 'product') {
                RemoveVotes($id, 0,
                            "This bug has been moved to a different product");
            }
            
            if ($col eq 'bug_status' 
                && IsOpenedState($old) ne IsOpenedState($new))
            {
                $check_dep_bugs = 1;
            }

            # Convert deadlines to the YYYY-MM-DD format. We use an
            # intermediate $xxxtime to prevent errors in the web
            # server log file when str2time($xxx) is undefined.
            if ($col eq 'deadline') {
                my $oldtime = str2time($old);
                $old = ($oldtime) ? time2str("%Y-%m-%d", $oldtime) : '';
                my $newtime = str2time($new);
                $new = ($newtime) ? time2str("%Y-%m-%d", $newtime) : '';
            }

            LogActivityEntry($id,$col,$old,$new,$whoid,$timestamp);
            $bug_changed = 1;
        }
    }
    # Set and update flags.
    if ($UserInEditGroupSet) {
        my $target = Bugzilla::Flag::GetTarget($id);
        Bugzilla::Flag::process($target, $timestamp, \%::FORM);
    }
    if ($bug_changed) {
        SendSQL("UPDATE bugs SET delta_ts = $sql_timestamp WHERE bug_id = $id");
    }
    $dbh->bz_unlock_tables();

    $vars->{'mailrecipients'} = { 'cc' => \@ccRemoved,
                                  'owner' => $origOwner,
                                  'qacontact' => $origQaContact,
                                  'changer' => Bugzilla->user->login };

    $vars->{'id'} = $id;
    
    # Let the user know the bug was changed and who did and didn't
    # receive email about the change.
    $template->process("bug/process/results.html.tmpl", $vars)
      || ThrowTemplateError($template->error());
    $vars->{'header_done'} = 1;
    
    if ($duplicate) {
        # Check to see if Reporter of this bug is reporter of Dupe 
        SendSQL("SELECT reporter FROM bugs WHERE bug_id = " . SqlQuote($::FORM{'id'}));
        my $reporter = FetchOneColumn();
        SendSQL("SELECT reporter FROM bugs WHERE bug_id = " . SqlQuote($duplicate) . " and reporter = $reporter");
        my $isreporter = FetchOneColumn();
        SendSQL("SELECT who FROM cc WHERE bug_id = " . SqlQuote($duplicate) . " and who = $reporter");
        my $isoncc = FetchOneColumn();
        unless ($isreporter || $isoncc || ! $::FORM{'confirm_add_duplicate'}) {
            # The reporter is oblivious to the existence of the new bug and is permitted access
            # ... add 'em to the cc (and record activity)
            LogActivityEntry($duplicate,"cc","",DBID_to_name($reporter),
                             $whoid,$timestamp);
            SendSQL("INSERT INTO cc (who, bug_id) VALUES ($reporter, " . SqlQuote($duplicate) . ")");
        }
        # Bug 171639 - Duplicate notifications do not need to be private. 
        AppendComment($duplicate, Bugzilla->user->login,
                      "*** Bug $::FORM{'id'} has been marked as a duplicate of this bug. ***",
                      0, $timestamp);

        CheckFormFieldDefined(\%::FORM,'comment');
        SendSQL("INSERT INTO duplicates VALUES ($duplicate, $::FORM{'id'})");
        
        $vars->{'mailrecipients'} = { 'changer' => Bugzilla->user->login }; 

        $vars->{'id'} = $duplicate;
        $vars->{'type'} = "dupe";
        
        # Let the user know a duplication notation was added to the original bug.
        $template->process("bug/process/results.html.tmpl", $vars)
          || ThrowTemplateError($template->error());
        $vars->{'header_done'} = 1;
    }

    if ($check_dep_bugs) {
        foreach my $k (keys(%dependencychanged)) {
            $vars->{'mailrecipients'} = { 'changer' => Bugzilla->user->login }; 
            $vars->{'id'} = $k;
            $vars->{'type'} = "dep";

            # Let the user know we checked to see if we should email notice
            # of this change to users with a relationship to the dependent
            # bug and who did and didn't receive email about it.
            $template->process("bug/process/results.html.tmpl", $vars)
              || ThrowTemplateError($template->error());
            $vars->{'header_done'} = 1;
        }
    }
}

# now show the next bug
if ($next_bug) {
    if (detaint_natural($next_bug) && Bugzilla->user->can_see_bug($next_bug)) {
        my $bug = new Bugzilla::Bug($next_bug, $whoid);
        ThrowCodeError("bug_error", { bug => $bug }) if $bug->error;

        # next.html.tmpl includes edit.html.tmpl, and therefore we
        # need $bug defined in $vars.
        $vars->{'bug'} = $bug;

        # And we need to determine if Patch Viewer is installed, for
        # Diff link (NB: Duplicate code with show_bug.cgi.)
        eval {
            require PatchReader;
            $vars->{'patchviewerinstalled'} = 1;
        };

        $template->process("bug/process/next.html.tmpl", $vars)
          || ThrowTemplateError($template->error());

        exit;
    }
}

# End the response page.
$template->process("bug/navigate.html.tmpl", $vars)
  || ThrowTemplateError($template->error());
$template->process("global/footer.html.tmpl", $vars)
  || ThrowTemplateError($template->error());
