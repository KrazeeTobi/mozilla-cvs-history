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
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Terry Weissman <terry@mozilla.org>
#                 Andreas Franke <afranke@mathweb.org>
#                 Christian Reis <kiko@async.com.br>
#                 Myk Melez <myk@mozilla.org>

use strict;

use lib qw(.);
require "CGI.pl";

# Use global template variables.
use vars qw($template $vars);

use vars %::FORM;

ConnectToDatabase();

quietly_check_login();

# Connect to the shadow database if this installation is using one to improve
# performance.
Bugzilla->instance->switch_to_shadow_db();

# More warning suppression silliness.
$::userid = $::userid;

################################################################################
# Data/Security Validation                                                     #
################################################################################

# Make sure the bug ID is a positive integer representing an existing
# bug that the user is authorized to access.
ValidateBugID($::FORM{'id'});
my $id = $::FORM{'id'};

my $hide_resolved = $::FORM{'hide_resolved'} ? 1 : 0;

my $maxdepth = $::FORM{'maxdepth'} || 0;
if ($maxdepth !~ /^\d+$/) { $maxdepth = 0 };

################################################################################
# Main Section                                                                 #
################################################################################

# The column/value to select as the target milestone for bugs,
# either the target_milestone column or the empty string value
# (for installations that don't use target milestones).  Makes
# it easier to query the database for bugs because we don't
# have to embed a conditional statement into each query.
my $milestone_column = Param('usetargetmilestone') ? "target_milestone" : "''";

# The greatest depth to which either tree goes.
my $realdepth = 0;

# Generate the tree of bugs that this bug depends on and a list of IDs
# appearing in the tree.
my $dependson_tree = { $id => GetBug($id) };
my $dependson_ids = {};
GenerateTree($id, "dependson", 1, $dependson_tree, $dependson_ids);
$vars->{'dependson_tree'} = $dependson_tree;
$vars->{'dependson_ids'} = [keys(%$dependson_ids)];

# Generate the tree of bugs that this bug blocks and a list of IDs
# appearing in the tree.
my $blocked_tree = { $id => GetBug($id) };
my $blocked_ids = {};
GenerateTree($id, "blocked", 1, $blocked_tree, $blocked_ids);
$vars->{'blocked_tree'} = $blocked_tree;
$vars->{'blocked_ids'} = [keys(%$blocked_ids)];

$vars->{'realdepth'}      = $realdepth;

$vars->{'bugid'}          = $id;
$vars->{'maxdepth'}       = $maxdepth;
$vars->{'hide_resolved'}  = $hide_resolved;
$vars->{'canedit'}        = UserInGroup("editbugs");

print "Content-Type: text/html\n\n";
$template->process("bug/dependency-tree.html.tmpl", $vars)
  || ThrowTemplateError($template->error());

################################################################################
# Recursive Tree Generation Function                                           #
################################################################################

sub GenerateTree {
    # Generates a dependency tree for a given bug.  Calls itself recursively
    # to generate sub-trees for the bug's dependencies.
    
    my ($bug_id, $relationship, $depth, $bugs, $ids) = @_;
    
    # Query the database for bugs with the given dependency relationship.
    my @dependencies = GetDependencies($bug_id, $relationship);
    
    # Don't do anything if this bug doesn't have any dependencies.
    return unless scalar(@dependencies);
    
    # Record this depth in the global $realdepth variable if it's farther 
    # than we've gone before.
    $realdepth = max($realdepth, $depth);

    foreach my $dep_id (@dependencies) {
        # Get this dependency's record from the database and generate
        # its sub-tree if we haven't already done so (which happens
        # when bugs appear in dependency trees multiple times).
        if (!$bugs->{$dep_id}) {
            $bugs->{$dep_id} = GetBug($dep_id);
            GenerateTree($dep_id, $relationship, $depth+1, $bugs, $ids);
        }

        # Add this dependency to the list of this bug's dependencies 
        # if it exists, if we haven't exceeded the maximum depth the user 
        # wants the tree to go, and if the dependency isn't resolved 
        # (if we're ignoring resolved dependencies).
        if ($bugs->{$dep_id}->{'exists'}
            && (!$maxdepth || $depth <= $maxdepth) 
            && ($bugs->{$dep_id}->{'open'} || !$hide_resolved))
        {
            push (@{$bugs->{$bug_id}->{'dependencies'}}, $dep_id);
            $ids->{$dep_id} = 1;
        }
    }
}

sub GetBug {
    # Retrieves the necessary information about a bug, stores it in the bug cache,
    # and returns it to the calling code.
    my ($id) = @_;
    
    my $bug = {};
    if (CanSeeBug($id, $::userid)) {
        SendSQL("SELECT 1, 
                                  bug_status, 
                                  short_desc, 
                                  $milestone_column, 
                                  assignee.userid, 
                                  assignee.login_name
                             FROM bugs, profiles AS assignee
                            WHERE bugs.bug_id = $id
                              AND bugs.assigned_to = assignee.userid");
    
    
        ($bug->{'exists'}, 
         $bug->{'status'}, 
         $bug->{'summary'}, 
         $bug->{'milestone'}, 
         $bug->{'assignee_id'}, 
         $bug->{'assignee_email'}) = FetchSQLData();
     }
    
    $bug->{'open'} = $bug->{'exists'} && IsOpenedState($bug->{'status'});
    $bug->{'dependencies'} = [];
    
    return $bug;
}

sub GetDependencies {
    # Returns a list of dependencies for a given bug.
    
    my ($id, $relationship) = @_;
    
    my $bug_type = ($relationship eq "blocked") ? "dependson" : "blocked";
    
    SendSQL("  SELECT $relationship 
                 FROM dependencies 
                WHERE $bug_type = $id 
             ORDER BY $relationship");
    
    my @dependencies = ();
    push(@dependencies, FetchOneColumn()) while MoreSQLData();
    
    return @dependencies;
}

