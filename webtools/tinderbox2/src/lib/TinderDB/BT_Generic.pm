# -*- Mode: perl; indent-tabs-mode: nil -*-
#

# TinderDB::BT_GENERIC - a generic methods to display the changes to
# the bug tracking system.  I am hoping that we will not need to have
# one specialized method per bug ticket system but can instead use
# this method for all commonly used systems.  This module will display
# bugs which have been 'closed' (developers close bugs when their
# latest version control chceck in fixes them) or 'reopened' (since
# that means that developers closed the bugs incorrectly.)


# The contents of this file are subject to the Mozilla Public
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

# complete rewrite by Ken Estes, Mail.com (kestes@staff.mail.com).
# Contributor(s): 

package TinderDB::BT;

# the raw CVS implemenation of the Version Control DB for Tinderbox.
# This column of the status table will report who has changed files in
# the CVS repository and what files they have changed.


#   We store the hash of all names who modified the tree at a
#   particular time as follows:

#   $DATABASE{$tree}{$timenow}{$status}{$bug_id} = $record;

# Where $rec is an anoymous hash of name vaule pairs from the bug
# tracking system.

# we also store information in the metadata structure

#   $METADATA{$tree}{'updates_since_trim'} += 1;
#


# Load standard perl libraries
use File::Basename;
use Time::Local;

# Load Tinderbox libraries
use TinderDB::BasicTxtDB;
use Utils;
use HTMLPopUp;
use TreeData;
use VCDisplay;


$VERSION = ( qw $Revision: 1.1 $ )[1];

@ISA = qw(TinderDB::BasicTxtDB);

# Add an empty object, of this DB subclass, to end of the set of all
# HTML columns.  This registers the subclass with TinderDB and defines
# the order of the HTML columns.

push @TinderDB::HTML_COLUMNS, TinderDB::BT->new();



# remove all records from the database which are older then last_time.

sub trim_db_history {
  my ($self, $tree,) = (@_);

  my ($last_time) =  $main::TIME - $TinderDB::TRIM_SECONDS;

  # sort numerically ascending
  my (@times) = sort {$a <=> $b} keys %{ $DATABASE{$tree} };
  foreach $time (@times) {
    ($time >= $last_time) && last;

    delete $DATABASE{$tree}{$time};
  }

  return ;
}


# get the recent data from CVS and the treestate file.


sub apply_db_updates {
  my ($self, $tree, ) = @_;
  
  # If the cell immediately after us is defined, then we can have
  # a previousbuildtime.  New builds always start right after old
  # builds finish.  The only time there is a pause, is when the
  # old build broke right away.  Hence we can use the difference
  # in start time as the time for the build.  If this is wrong the
  # build broke early and we do not care about the runtime.  Throw
  # away updates from newbuilds which start before mintime is up.

  my ($filename) = $self->update_file($tree);
  my ($dirname)= File::Basename::dirname($filename);
  my ($prefixname) = File::Basename::basename($filename);
  my (@sorted_files) = $self->readdir_file_prefix( $dirname, 
                                                   $prefixname);

  scalar(@sorted_files) || return 0;

  foreach $update_file (@sorted_files) {
    my $full_file = "$dirname/$update_file";
    my ($record) = Persistence::load_structure($full_file);

    ($record) ||
      die("Error reading Bug Tracking update file '$full_file'.\n");

    my($timenow) =  $record->{'tinderbox_timenow'};
    
    my($status) = $record->{'tinderbox_status'};
    my($bug_id) = $record->{'tinderbox_bug_id'};
    
    # sanity check the record, taint checks are done in processmail.
    {
      ($tree eq $record->{'tinderbox_tree'}) ||
        die("Error in updatefile: $full_file, ".
            "Tree: $tree, not equal to Tree: $record->{'tree'}.");

      (main::is_time_valid($timenow)) ||
        die("Error in updatefile: $full_file, ".
            "timenow: $timenow, is not a valid time.");
   }  


    # add the record to the datastructure

    # Notice: We store the raw $status and do not use
    # BTData::STATUS_PROGRESS to convert it to $progress.  This is so
    # that the progress mapping can change later and not invalidate
    # old data.

    $DATABASE{$tree}{$timenow}{$status}{$bug_id} = $record;

  } # $update_file 

  $METADATA{$tree}{'updates_since_trim'}+=   
    scalar(@sorted_files);

  if ( ($METADATA{$tree}{'updates_since_trim'} >
        $TinderDB::MAX_UPDATES_SINCE_TRIM)
     ) {
    $METADATA{$tree}{'updates_since_trim'}=0;
    trim_db_history(@_);
  }

  # be sure to save the updates to the database before we delete their
  # files.

  $self->savetree_db($tree);

  $self->unlink_files($dirname, @sorted_files);
  
  return scalar(@sorted_files);
}


sub status_table_legend {
  my ($out)='';

  # There are two bug tracking columns, tickets which are moving
  # forward and tickets which are moving backwards.

#  $out .= "\t<td align=right valign=top>Bug Tracking Progress</td>\n";
#  $out .= "\t<td align=right valign=top>Bug Tracking Slipage</td>\n";

  return ($out);  
}


sub status_table_header {
  my $out = '';

  my (@progress_states) = BTData::get_all_progress_states();

  foreach $progress (@progress_states) {  
    $out .= "\t<th>BT $progress</th>\n";
  }

  return ($out);
}



# clear data structures in preparation for printing a new table

sub status_table_start {
  my ($self, $row_times, $tree, ) = @_;
  
  # create an ordered list of all times which any data is stored
  
  # sort numerically descending
  @DB_TIMES = sort {$b <=> $a} keys %{ $DATABASE{$tree} };

  # adjust the $NEXT_DB to skip data which came after the first cell
  # at the top of the page.  We make the first cell bigger then the
  # rest to allow for some overlap between pages.

  my ($first_cell_seconds) = 2*($row_times->[0] - $row_times->[1]);
  my ($earliest_data) = $row_times->[0] + $first_cell_seconds;

  $NEXT_DB = 0;
  while ( ($DB_TIMES[$NEXT_DB] > $earliest_data) &&    
          ($NEXT_DB < $#DB_TIMES) ) {
    $NEXT_DB++
  }

  return ;  
}





sub status_table_row {
  my ($self, $row_times, $row_index, $tree, ) = @_;

  my @outrow = ();

  # find all the bug_ids which changed at any point in this cell.

  my %bug_ids = ();
  
  while (1) {
   my ($time) = $DB_TIMES[$NEXT_DB];

    # find the DB entries which are needed for this cell
    ($time < $row_times->[$row_index]) && last;

    $NEXT_DB++;

    foreach $status (keys %{ $DATABASE{$tree}{$time} }) {
      my $query_links = '';
      foreach $bug_id (sort keys %{ $DATABASE{$tree}{$time}{$status} }) {
	
	my ($table) = '';
	my ($num_rows) = 0;
	my ($max_length) = 0;
        my ($rec) = $DATABASE{$tree}{$time}{$status}{$bug_id};

	# display all the interesting fields
	
        $table .= "Ticket updated at: ".localtime($time);
	foreach $field (@BTData::DISPLAY_FIELDS) {

	  # we display all fields even the empty ones, so that users
	  # can see which fields are empty.

	  my ($value) = $rec->{$field};

          # $max_length = main::max($max_length , length($value));
	  $num_rows++;
	  $table .= (
		     "\t".
		     "<font size=-1><tt>$field</tt></font>".
		     ": ".
		     $value.
		     "<br>\n".
		     "");
	}

        # fix the size so that long summaries do not cause our window
        # to get too large.

	$max_length = 40;

	# a link to the cgibin page which displays the bug
	
	my ($href) = $rec->{'tinderbox_bug_url'};
	my ($window_title) = "BT Info bug_id: $bug_id";

	# we display the list of names in 'teletype font' so that the
	# names do not bunch together. It seems to make a difference if
	# there is a <cr> between each link or not, but it does make a
	# difference if we close the <tt> for each author or only for
	# the group of links.
	my ($query_link) = 
	  HTMLPopUp::Link(
			  "linktxt" => "<tt>$bug_id</tt>",
			  "href" => $href,
			  
			  "windowtxt" => $table,
			  "windowtitle" => $window_title,
			  "windowheight" => ($num_rows * 20) + 100,
			  "windowwidth" => ($max_length * 10) + 100,
			 );

	# put each link on its own line and add good comments so we
	# can debug the HTML.

	$query_link =  "\t\t".$query_link."\n";
	
	$query_links .= (
                         "\t\t<!-- BT: ".("bug_id: $bug_id, ".
                                          "Time: '".localtime($time)."', ".
                                          "Progress: $progress, ".
                                          "Status: $status, ".
                                          "Tree: $tree, ".
                                          "").
                         "  -->\n".
                         "");
	
	$query_links .= $query_link;
      } # foreach $bug_id

      my ($progress) = $BTData::STATUS_PROGRESS{$status};
      $bug_ids{$progress} .= $query_links;
    } # foreach $status
  } # while (1)

 
  my (@progress_states) = BTData::get_all_progress_states();

  foreach $progress (@progress_states) {

    if ($bug_ids{$progress}) {
      push @outrow, (
                     "\t<td align=center>\n".
                     $bug_ids{$progress}.
                     "\t</td>\n".
                     "");
    } else {
      
      push @outrow, ("\t<!-- skipping: BT: Progress: $progress tree: $tree -->".
                     "<td align=center>$HTMLPopUp::EMPTY_TABLE_CELL</td>\n");
    }
    
  }

  return @outrow; 
}

1;
