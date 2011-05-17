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
# The Initial Developer of the Original Code is Google, Inc.
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Max Kanat-Alexander <mkanat@bugzilla.org>

# This module represents a test with custom URL parameters.
# Tests like this are specified in CUSTOM_SEARCH_TESTS in
# Bugzilla::Test::Search::Constants.
package Bugzilla::Test::Search::CustomTest;
use base qw(Bugzilla::Test::Search::FieldTest);
use strict;
use warnings;

use Bugzilla::Test::Search::FieldTest;
use Bugzilla::Test::Search::OperatorTest;

use Storable qw(dclone);

###############
# Constructor #
###############

sub new {
  my ($class, $test, $search_test) = @_;
  bless { raw_test => dclone($test), search_test => $search_test }, $class;
}

#############
# Accessors #
#############

sub search_test { return $_[0]->{search_test} }
sub name { return 'Custom: ' . $_[0]->test->{name} }
sub test { return $_[0]->{raw_test} }

sub operator_test { die "unimplemented" }
sub field_object { die "unimplemented" }
sub main_value { die "unimplenmented" }
sub test_value { die "unimplemented" }
# Custom tests don't use transforms.
sub transformed_value_was_equal { 0 }
sub debug_value {
    my ($self) = @_;
    my $string = '';
    my $params = $self->search_params;
    foreach my $param (keys %$params) {
        $string .= $param . "=" . $params->{$param} . '&';
    }
    chop($string);
    return $string;
}

# The tests we know are broken for this operator/field combination.
sub _known_broken { return {} }
sub contains_known_broken { return undef }
sub search_known_broken { return undef }
sub field_not_yet_implemented { return undef }
sub invalid_field_operator_combination { return undef }

#########################################
# Accessors: Bugzilla::Search Arguments #
#########################################

# Converts the f, o, v rows into f0, o0, v0, etc. and translates
# the values appropriately.
sub search_params {
    my ($self) = @_;

    my %params = %{ $self->test->{top_params} || {} };
    my $counter = 0;
    foreach my $row (@{ $self->test->{params} }) {
        $row->{v} = $self->translate_value($row) if exists $row->{v};
        foreach my $key (keys %$row) {
            $params{"${key}$counter"} = $row->{$key};
        }
        $counter++;
    }

    return \%params;
}

sub translate_value {
    my ($self, $row) = @_;
    my $as_test = { field => $row->{f}, operator => $row->{o},
                    value => $row->{v} };
    my $operator_test = new Bugzilla::Test::Search::OperatorTest($row->{o},
        $self->search_test);
    my $field = Bugzilla::Field->check($row->{f});
    my $field_test = new Bugzilla::Test::Search::FieldTest($operator_test,
      $field, $as_test);
    return $field_test->translated_value;
}

sub search_columns {
    my ($self) = @_;
    return ['bug_id', @{ $self->test->{columns} || [] }];
}

1;
