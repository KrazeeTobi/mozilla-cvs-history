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
# Contributor(s): Myk Melez <myk@mozilla.org>

################################################################################
# Module Initialization
################################################################################

# Make it harder for us to do dangerous things in Perl.
use strict;

# This module implements bug and attachment flags.
package Bugzilla::Flag;

use Bugzilla::FlagType;
use Bugzilla::User;
use Bugzilla::Config;

use Attachment;

# Note that this line doesn't actually import these variables for some reason,
# so I have to use them as $::template and $::vars in the package code.
use vars qw($template $vars); 

# Note!  This module requires that its caller have said "require CGI.pl" 
# to import relevant functions from that script and its companion globals.pl.

################################################################################
# Global Variables
################################################################################

# basic sets of columns and tables for getting flags from the database

my @base_columns = 
  ("1", "id", "type_id", "bug_id", "attach_id", "requestee_id", "setter_id",
   "status");

# Note: when adding tables to @base_tables, make sure to include the separator 
# (i.e. a comma or words like "LEFT OUTER JOIN") before the table name, 
# since tables take multiple separators based on the join type, and therefore 
# it is not possible to join them later using a single known separator.

my @base_tables = ("flags");

################################################################################
# Searching/Retrieving Flags
################################################################################

# !!! Implement a cache for this function!
sub get {
    # Retrieves and returns a flag from the database.

    my ($id) = @_;

    my $select_clause = "SELECT " . join(", ", @base_columns);
    my $from_clause = "FROM " . join(" ", @base_tables);
    
    # Execute the query, retrieve the result, and write it into a record.
    &::PushGlobalSQLState();
    &::SendSQL("$select_clause $from_clause WHERE flags.id = $id");
    my $flag = perlify_record(&::FetchSQLData());
    &::PopGlobalSQLState();

    return $flag;
}

sub match {
    # Queries the database for flags matching the given criteria 
    # (specified as a hash of field names and their matching values)
    # and returns an array of matching records.

    my ($criteria) = @_;

    my $select_clause = "SELECT " . join(", ", @base_columns);
    my $from_clause = "FROM " . join(" ", @base_tables);
    
    my @criteria = sqlify_criteria($criteria);
    
    my $where_clause = "WHERE " . join(" AND ", @criteria);
    
    # Execute the query, retrieve the results, and write them into records.
    &::PushGlobalSQLState();
    &::SendSQL("$select_clause $from_clause $where_clause");
    my @flags;
    while (&::MoreSQLData()) {
        my $flag = perlify_record(&::FetchSQLData());
        push(@flags, $flag);
    }
    &::PopGlobalSQLState();

    return \@flags;
}

sub count {
    # Queries the database for flags matching the given criteria 
    # (specified as a hash of field names and their matching values)
    # and returns an array of matching records.

    my ($criteria) = @_;

    my @criteria = sqlify_criteria($criteria);
    
    my $where_clause = "WHERE " . join(" AND ", @criteria);
    
    # Execute the query, retrieve the result, and write it into a record.
    &::PushGlobalSQLState();
    &::SendSQL("SELECT COUNT(id) FROM flags $where_clause");
    my $count = &::FetchOneColumn();
    &::PopGlobalSQLState();

    return $count;
}

################################################################################
# Creating and Modifying
################################################################################

sub validate {
    # Validates fields containing flag modifications.
    
    my ($data) = @_;
  
    # Get a list of flags to validate.  Uses the "map" function
    # to extract flag IDs from form field names by matching fields
    # whose name looks like "flag-nnn", where "nnn" is the ID,
    # and returning just the ID portion of matching field names.
    my @ids = map(/^flag-(\d+)$/ ? $1 : (), keys %$data);
  
    foreach my $id (@ids)
    {
        my $status = $data->{"flag-$id"};
        
        # Make sure the flag exists.
        my $flag = get($id);
        $flag || &::ThrowCodeError("flag_nonexistent", { id => $id });

        # Don't bother validating flags the user didn't change.
        next if $status eq $flag->{'status'};

        # Make sure the user chose a valid status.
        grep($status eq $_, qw(X + - ?))
          || &::ThrowCodeError("flag_status_invalid", 
                               { id => $id , status => $status });
    }
}

sub process {
    # Processes changes to flags.

    # The target is the bug or attachment this flag is about, the timestamp
    # is the date/time the bug was last touched (so that changes to the flag
    # can be stamped with the same date/time), the data is the form data
    # with flag fields that the user submitted, the old bug is the bug record
    # before the user made changes to it, and the new bug is the bug record
    # after the user made changes to it.
    
    my ($target, $timestamp, $data, $oldbug, $newbug) = @_;

    # Use the date/time we were given if possible (allowing calling code
    # to synchronize the comment's timestamp with those of other records).
    $timestamp = ($timestamp ? &::SqlQuote($timestamp) : "NOW()");
    
    # Take a snapshot of flags before any changes.
    my $flags = match({ 'bug_id'    => $target->{'bug'}->{'id'} , 
                        'attach_id' => $target->{'attachment'}->{'id'} });
    my @old_summaries;
    foreach my $flag (@$flags) {
        my $summary = $flag->{'type'}->{'name'} . $flag->{'status'};
        push(@old_summaries, $summary);
    }
    
    # Create new flags and update existing flags.
    my $new_flags = FormToNewFlags($target, $data);
    foreach my $flag (@$new_flags) { create($flag, $timestamp) }
    modify($data, $timestamp);
    
    # In case the bug's product/component has changed, clear flags that are
    # no longer valid.
    &::SendSQL("
        SELECT flags.id 
        FROM flags, bugs LEFT OUTER JOIN flaginclusions i
        ON (flags.type_id = i.type_id 
            AND (bugs.product_id = i.product_id OR i.product_id IS NULL)
            AND (bugs.component_id = i.component_id OR i.component_id IS NULL))
        WHERE flags.type_id = $target->{'bug'}->{'id'} 
        AND flags.bug_id = bugs.bug_id
        AND i.type_id IS NULL
    ");
    clear(&::FetchOneColumn()) while &::MoreSQLData();
    &::SendSQL("
        SELECT flags.id 
        FROM flags, bugs, flagexclusions e
        WHERE flags.type_id = $target->{'bug'}->{'id'}
        AND flags.bug_id = bugs.bug_id
        AND flags.type_id = e.type_id 
        AND (bugs.product_id = e.product_id OR e.product_id IS NULL)
        AND (bugs.component_id = e.component_id OR e.component_id IS NULL)
    ");
    clear(&::FetchOneColumn()) while &::MoreSQLData();
    
    # Take a snapshot of flags after changes.
    $flags = match({ 'bug_id'    => $target->{'bug'}->{'id'} , 
                     'attach_id' => $target->{'attachment'}->{'id'} });
    my @new_summaries;
    foreach my $flag (@$flags) {
        my $summary = $flag->{'type'}->{'name'} . $flag->{'status'};
        push(@new_summaries, $summary);
    }

    my $old_summaries = join(", ", @old_summaries);
    my $new_summaries = join(", ", @new_summaries);
    my ($removed, $added) = &::DiffStrings($old_summaries, $new_summaries);
    if ($removed ne $added) {
        my $sql_removed = &::SqlQuote($removed);
        my $sql_added = &::SqlQuote($added);
        my $field_id = &::GetFieldID('flagtypes.name');
        my $attach_id = $target->{'attachment'}->{'id'} || 'NULL';
        &::SendSQL("INSERT INTO bugs_activity (bug_id, attach_id, who, " . 
                   "bug_when, fieldid, removed, added) VALUES " . 
                   "($target->{'bug'}->{'id'}, $attach_id, $::userid, " . 
                   "$timestamp, $field_id, $sql_removed, $sql_added)");
    }
}


sub create {
    # Creates a flag record in the database.

    my ($flag, $timestamp) = @_;

    # Determine the ID for the flag record by retrieving the last ID used
    # and incrementing it.
    &::SendSQL("SELECT MAX(id) FROM flags");
    $flag->{'id'} = (&::FetchOneColumn() || 0) + 1;
    
    # Insert a record for the flag into the flags table.
    my $attach_id = $flag->{'target'}->{'attachment'}->{'id'} || "NULL";
    my $requestee_id = $flag->{'requestee'} ? $flag->{'requestee'}->{'id'} : "NULL";
    &::SendSQL("INSERT INTO flags (id, type_id, 
                                      bug_id, attach_id, 
                                      requestee_id, setter_id, status, 
                                      creation_date, modification_date)
                VALUES ($flag->{'id'}, 
                        $flag->{'type'}->{'id'}, 
                        $flag->{'target'}->{'bug'}->{'id'}, 
                        $attach_id,
                        $requestee_id,
                        $flag->{'setter'}->{'id'}, 
                        '$flag->{'status'}', 
                        $timestamp,
                        $timestamp)");
    
    # Send an email notifying the relevant parties about the flag creation.
    if ($flag->{'requestee'} && $flag->{'requestee'}->email_prefs->{'FlagRequestee'} 
        || $flag->{'type'}->{'cc_list'}) {
        notify($flag, "request/created-email.txt.tmpl");
    }
}

sub migrate {
    # Moves a flag from one attachment to another.  Useful for migrating
    # a flag from an obsolete attachment to the attachment that obsoleted it.

    my ($old_attach_id, $new_attach_id) = @_;

    # Update the record in the flags table to point to the new attachment.
    &::SendSQL("UPDATE flags " . 
               "SET    attach_id = $new_attach_id , " . 
               "       modification_date = NOW() " . 
               "WHERE  attach_id = $old_attach_id");
}

sub modify {
    # Modifies flags in the database when a user changes them.

    my ($data, $timestamp) = @_;

    # Use the date/time we were given if possible (allowing calling code
    # to synchronize the comment's timestamp with those of other records).
    $timestamp = ($timestamp ? &::SqlQuote($timestamp) : "NOW()");
    
    # Extract a list of flags from the form data.
    my @ids = map(/^flag-(\d+)$/ ? $1 : (), keys %$data);
    
    # Loop over flags and update their record in the database.
    my @flags;
    foreach my $id (@ids) {
        my $flag = get($id);
        my $status = $data->{"flag-$id"};
        
        # Ignore flags the user didn't change.
        next if $status eq $flag->{'status'};
        
        # Since the status is validated, we know it's safe, but it's still
        # tainted, so we have to detaint it before using it in a query.
        &::trick_taint($status);
                
        if ($status eq '+' || $status eq '-') {
            &::SendSQL("UPDATE flags 
                        SET    setter_id = $::userid , 
                               status = '$status' , 
                               modification_date = $timestamp
                        WHERE  id = $flag->{'id'}");
            
            # Send an email notifying the relevant parties about the fulfillment.
            if ($flag->{'setter'}->email_prefs->{'FlagRequester'} 
                || $flag->{'type'}->{'cc_list'})
            {
                $flag->{'status'} = $status;
                notify($flag, "request/fulfilled-email.txt.tmpl");
            }
        }
        elsif ($status eq '?') {
            &::SendSQL("UPDATE flags 
                        SET    status = '$status' , 
                               modification_date = $timestamp
                        WHERE  id = $flag->{'id'}");
        }
        # The user unset the flag, so delete it from the database.
        elsif ($status eq 'X') {
            clear($flag->{'id'});
        }
        
        push(@flags, $flag);
    }
    
    return \@flags;
}

sub clear {
    my ($id) = @_;
    
    my $flag = get($id);
    
    &::PushGlobalSQLState();
    &::SendSQL("DELETE FROM flags WHERE id = $id");
    &::PopGlobalSQLState();
    
    # Set the flag's status to "cleared" so the email template
    # knows why email is being sent about the request.
    $flag->{'status'} = "X";
    
    notify($flag, "request/fulfilled-email.txt.tmpl") if $flag->{'requestee'};
}


################################################################################
# Utility Functions
################################################################################

sub FormToNewFlags {
    my ($target, $data) = @_;
    
    # Flag for whether or not we must get verification of the requestees
    # (if the user did not uniquely identify them).
    my $verify_requestees = 0;

    # Get information about the setter to add to each flag.
    # Uses a conditional to suppress Perl's "used only once" warnings.
    my $setter = new Bugzilla::User($::userid);

    # Extract a list of flag type IDs from field names.
    my @type_ids = map(/^flag_type-(\d+)$/ ? $1 : (), keys %$data);
    @type_ids = grep($data->{"flag_type-$_"} ne 'X', @type_ids);
    
    # Process the form data and create an array of flag objects.
    my @flags;
    foreach my $type_id (@type_ids) {
        my $status = $data->{"flag_type-$type_id"};
        &::trick_taint($status);
    
        # Create the flag record and populate it with data from the form.
        my $flag = { 
            type   => Bugzilla::FlagType::get($type_id) , 
            target => $target , 
            setter => $setter , 
            status => $status 
        };

        my $requestee_str = $data->{"requestee-$type_id"} || $data->{'requestee'};
        if ($requestee_str) {
            $flag->{'requestee_str'} = $requestee_str;
            MatchRequestees($flag);
            $verify_requestees = 1 if scalar(@{$flag->{'requestees'}}) != 1;
        }
        
        # Add the flag to the array of flags.
        push(@flags, $flag);
    }

    if ($verify_requestees) {
        $::vars->{'target'} = $target;
        $::vars->{'flags'} = \@flags;
        $::vars->{'form'} = $data;
        $::vars->{'mform'} = \%::MFORM || \%::MFORM;
        
        print "Content-Type: text/html\n\n" unless $::vars->{'header_done'};
        $::template->process("request/verify.html.tmpl", $::vars)
          || &::ThrowTemplateError($::template->error());
        exit;
    }
    
    # Return the list of flags.
    return \@flags;
}

sub MatchRequestees {
    my ($flag) = @_;
    
    my $requestee_str = $flag->{'requestee_str'};
    
    # To reduce the size of queries, require the user to enter at least 
    # three characters of each requestee's name unless this installation
    # automatically appends an email suffix to each user's login name,
    # in which case we can't guarantee their names are at least three
    # characters long.
    if (!Param('emailsuffix') && length($requestee_str) < 3) {
        &::ThrowUserError("requestee_too_short");
    }

    # Get a list of potential requestees whose email address or real name 
    # matches the substring entered by the user.  Try an exact match first,
    # then fall back to a substring search.  Limit search to 100 matches, 
    # since at that point there are too many to make the user wade through, 
    # and we need to get the user to enter a more constrictive match string.
    my $user_id = &::DBname_to_id($requestee_str);
    if ($user_id) { $flag->{'requestees'} = [ new Bugzilla::User($user_id) ] }
    else          { $flag->{'requestees'} = Bugzilla::User::match($requestee_str, 101, 1) }
    
    # If there is only one requestee match, make them the requestee.
    if (scalar(@{$flag->{'requestees'}}) == 1) {
        $flag->{'requestee'} = $flag->{'requestees'}[0];
    } 

    # If there are too many requestee matches, throw an error.
    elsif (scalar(@{$flag->{'requestees'}}) == 101) {
        &::ThrowUserError("requestee_too_many_matches", 
                          { requestee => $requestee_str });
    }
}


# Ideally, we'd use Bug.pm, but it's way too heavyweight, and it can't be
# made lighter without totally rewriting it, so we'll use this function
# until that one gets rewritten.
sub GetBug {
    # Returns a hash of information about a target bug.
    my ($id) = @_;

    # Save the currently running query (if any) so we do not overwrite it.
    &::PushGlobalSQLState();

    &::SendSQL("SELECT  1, short_desc, product_id, component_id
                  FROM  bugs
                 WHERE  bug_id = $id");

    my $bug = { 'id' => $id };
    
    ($bug->{'exists'}, $bug->{'summary'}, $bug->{'product_id'}, 
     $bug->{'component_id'}) = &::FetchSQLData();

    # Restore the previously running query (if any).
    &::PopGlobalSQLState();

    return $bug;
}

sub GetTarget {
    my ($bug_id, $attach_id) = @_;
    
    # Create an object representing the target bug/attachment.
    my $target = { 'exists' => 0 };

    if ($attach_id) {
        $target->{'attachment'} = new Attachment($attach_id);
        if ($bug_id) {
            # Make sure the bug and attachment IDs correspond to each other
            # (i.e. this is the bug to which this attachment is attached).
            $bug_id == $target->{'attachment'}->{'bug_id'}
              || return { 'exists' => 0 };
        }
        $target->{'bug'} = GetBug($target->{'attachment'}->{'bug_id'});
        $target->{'exists'} = $target->{'attachment'}->{'exists'};
        $target->{'type'} = "attachment";
    }
    elsif ($bug_id) {
        $target->{'bug'} = GetBug($bug_id);
        $target->{'exists'} = $target->{'bug'}->{'exists'};
        $target->{'type'} = "bug";
    }

    return $target;
}

sub notify {
    # Sends an email notification about a flag being created or fulfilled.
    
    my ($flag, $template_file) = @_;
    
    $::vars->{'flag'} = $flag;
    
    my $message;
    my $rv = 
      $::template->process($template_file, $::vars, \$message);
    if (!$rv) {
        print "Content-Type: text/html\n\n" unless $::vars->{'header_done'};
        &::ThrowTemplateError($::template->error());
    }
    
    my $delivery_mode = Param("sendmailnow") ? "" : "-ODeliveryMode=deferred";
    open(SENDMAIL, "|/usr/lib/sendmail $delivery_mode -t -i") 
      || die "Can't open sendmail";
    print SENDMAIL $message;
    close(SENDMAIL);
}

################################################################################
# Private Functions
################################################################################

sub sqlify_criteria {
    # Converts a hash of criteria into a list of SQL criteria.
    
    # a reference to a hash containing the criteria (field => value)
    my ($criteria) = @_;

    # the generated list of SQL criteria; "1=1" is a clever way of making sure
    # there's something in the list so calling code doesn't have to check list
    # size before building a WHERE clause out of it
    my @criteria = ("1=1");
    
    # If the caller specified only bug or attachment flags,
    # limit the query to those kinds of flags.
    if (defined($criteria->{'target_type'})) {
        if    ($criteria->{'target_type'} eq 'bug')        { push(@criteria, "attach_id IS NULL") }
        elsif ($criteria->{'target_type'} eq 'attachment') { push(@criteria, "attach_id IS NOT NULL") }
    }
    
    # Go through each criterion from the calling code and add it to the query.
    foreach my $field (keys %$criteria) {
        my $value = $criteria->{$field};
        next unless defined($value);
        if    ($field eq 'type_id')      { push(@criteria, "type_id      = $value") }
        elsif ($field eq 'bug_id')       { push(@criteria, "bug_id       = $value") }
        elsif ($field eq 'attach_id')    { push(@criteria, "attach_id    = $value") }
        elsif ($field eq 'requestee_id') { push(@criteria, "requestee_id = $value") }
        elsif ($field eq 'setter_id')    { push(@criteria, "setter_id    = $value") }
        elsif ($field eq 'status')       { push(@criteria, "status       = '$value'") }
    }
    
    return @criteria;
}

sub perlify_record {
    # Converts a row from the database into a Perl record.
    my ($exists, $id, $type_id, $bug_id, $attach_id, 
        $requestee_id, $setter_id, $status) = @_;
    
    my $flag =
      {
        exists    => $exists , 
        id        => $id ,
        type      => Bugzilla::FlagType::get($type_id) ,
        target    => GetTarget($bug_id, $attach_id) , 
        requestee => new Bugzilla::User($requestee_id) ,
        setter    => new Bugzilla::User($setter_id) ,
        status    => $status , 
      };
    
    return $flag;
}

1;
