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
# The Initial Developer of the Original Code is Everything Solved, Inc.
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Max Kanat-Alexander <mkanat@bugzilla.org>

use strict;
package Bugzilla::DB::Schema::Sqlite;
use base qw(Bugzilla::DB::Schema);

use Bugzilla::Error;
use Bugzilla::Util qw(generate_random_password);

use Storable qw(dclone);

use constant FK_ON_CREATE => 1;

sub _initialize {

    my $self = shift;

    $self = $self->SUPER::_initialize(@_);

    $self->{db_specific} = {
        BOOLEAN =>      'integer',
        FALSE =>        '0', 
        TRUE =>         '1',

        INT1 =>         'integer',
        INT2 =>         'integer',
        INT3 =>         'integer',
        INT4 =>         'integer',

        SMALLSERIAL =>  'SERIAL',
        MEDIUMSERIAL => 'SERIAL',
        INTSERIAL =>    'SERIAL',

        TINYTEXT =>     'text',
        MEDIUMTEXT =>   'text',
        LONGTEXT =>     'text',

        LONGBLOB =>     'blob',

        DATETIME =>     'DATETIME',
    };

    $self->_adjust_schema;

    return $self;

}

#################################
# General SQLite Schema Helpers #
#################################

sub _sqlite_create_table {
    my ($self, $table) = @_;
    return scalar Bugzilla->dbh->selectrow_array(
        "SELECT sql FROM sqlite_master WHERE name = ? AND type = 'table'",
        undef, $table);
}

# This does most of the "heavy lifting" of the schema-altering functions.
sub _sqlite_alter_schema {
    my ($self, $table, $create_table, $options) = @_;
    
    my $dbh = Bugzilla->dbh;
    
    my $random = generate_random_password(5);
    my $rename_to = "${table}_$random";

    my @columns = $dbh->bz_table_columns_real($table);
    push(@columns, $options->{extra_column}) if $options->{extra_column};
    if (my $exclude = $options->{exclude_column}) {
        @columns = grep { $_ ne $exclude } @columns;
    }
    my @insert_cols = @columns;
    my @select_cols = @columns;
    if (my $rename = $options->{rename}) {
        foreach my $from (keys %$rename) {
            my $to = $rename->{$from};
            @insert_cols = map { $_ eq $from ? $to : $_ } @insert_cols;
        }
    }
    
    my $insert_str = join(',', @insert_cols);
    my $select_str = join(',', @select_cols);
    my $copy_sql = "INSERT INTO $table ($insert_str)"
                   . " SELECT $select_str FROM $rename_to";
                   
    # We have to turn FKs off before doing this. Otherwise, when we rename
    # the table, all of the FKs in the other tables will be automatically
    # updated to point to the renamed table. Note that PRAGMA foreign_keys
    # can only be set outside of a transaction--otherwise it is a no-op.
    if ($dbh->bz_in_transaction) {
        die "can't alter the schema inside of a transaction";
    }
    my @sql = (
        'PRAGMA foreign_keys = OFF',
        'BEGIN EXCLUSIVE TRANSACTION',
        @{ $options->{pre_sql} || [] },
        "ALTER TABLE $table RENAME TO $rename_to",
        $create_table,
        $copy_sql,
        "DROP TABLE $rename_to",
        'COMMIT TRANSACTION',
        'PRAGMA foreign_keys = ON',
    );    
}

# For finding a particular column's definition in a CREATE TABLE statement.
sub _sqlite_column_regex {
    my ($column) = @_;
    # 1 = Comma at start
    # 2 = Column name + Space
    # 3 = Definition
    # 4 = Ending comma
    return qr/(^|,)(\s\Q$column\E\s+)(.*?)(,|$)/m;
}

#############################
# Schema Setup & Alteration #
#############################

sub get_create_database_sql {
    # If we get here, it means there was some error creating the
    # database file during bz_create_database in Bugzilla::DB,
    # and we just want to display that error instead of doing
    # anything else.
    Bugzilla->dbh;
    die "Reached an unreachable point";
}

sub get_type_ddl {
    my $self = shift;
    my $def = dclone($_[0]);
    
    my $ddl = $self->SUPER::get_type_ddl(@_);
    if ($def->{PRIMARYKEY} and $def->{TYPE} =~ /SERIAL/i) {
        $ddl =~ s/\bSERIAL\b/integer/;
        $ddl =~ s/\bPRIMARY KEY\b/PRIMARY KEY AUTOINCREMENT/;
    }
    if ($def->{TYPE} =~ /text/i or $def->{TYPE} =~ /char/i) {
        $ddl .= " COLLATE bugzilla";
    }
    # Don't collate DATETIME fields.
    if ($def->{TYPE} eq 'DATETIME') {
        $ddl =~ s/\bDATETIME\b/text COLLATE BINARY/;
    }
    return $ddl;
}

sub get_alter_column_ddl {
    my $self = shift;
    my ($table, $column, $new_def, $set_nulls_to) = @_;
    my $dbh = Bugzilla->dbh;
    
    my $table_sql = $self->_sqlite_create_table($table);
    my $new_ddl = $self->get_type_ddl($new_def);
    # When we do ADD COLUMN, columns can show up all on one line separated
    # by commas, so we have to account for that.
    my $column_regex = _sqlite_column_regex($column);
    $table_sql =~ s/$column_regex/$1$2$new_ddl$4/
        || die "couldn't find $column in $table:\n$table_sql";
    my @pre_sql = $self->_set_nulls_sql(@_);
    return $self->_sqlite_alter_schema($table, $table_sql,
                                       { pre_sql => \@pre_sql });
}

sub get_add_column_ddl {
    my $self = shift;
    my ($table, $column, $definition, $init_value) = @_;
    # SQLite can use the normal ADD COLUMN when:
    # * The column isn't a PK
    if ($definition->{PRIMARYKEY}) {
        if ($definition->{NOTNULL} and $definition->{TYPE} !~ /SERIAL/i) {
            die "You can only add new SERIAL type PKs with SQLite";
        }
        my $table_sql = $self->_sqlite_new_column_sql(@_);
        # This works because _sqlite_alter_schema will exclude the new column
        # in its INSERT ... SELECT statement, meaning that when the "new"
        # table is populated, it will have AUTOINCREMENT values generated
        # for it.
        return $self->_sqlite_alter_schema($table, $table_sql);
    }
    # * The column has a default one way or another. Either it
    #   defaults to NULL (it lacks NOT NULL) or it has a DEFAULT
    #   clause. Since we also require this when doing bz_add_column (in
    #   the way of forcing an init_value for NOT NULL columns with no
    #   default), we first set the init_value as the default and then
    #   alter the column.
    if ($definition->{NOTNULL} and !defined $definition->{DEFAULT}) {
        my %with_default = %$definition;
        $with_default{DEFAULT} = $init_value;
        my @pre_sql =
            $self->SUPER::get_add_column_ddl($table, $column, \%with_default);
        my $table_sql = $self->_sqlite_new_column_sql(@_);
        return $self->_sqlite_alter_schema($table, $table_sql,
            { pre_sql => \@pre_sql, extra_column => $column });
    }
    
    return $self->SUPER::get_add_column_ddl(@_);
}

sub _sqlite_new_column_sql {
    my ($self, $table, $column, $def) = @_;
    my $table_sql = $self->_sqlite_create_table($table);
    my $new_ddl = $self->get_type_ddl($def);
    my $new_line = "\t$column\t$new_ddl";
    $table_sql =~ s/^(CREATE TABLE \w+ \()/$1\n$new_line,/s
        || die "Can't find start of CREATE TABLE:\n$table_sql";
    return $table_sql;
}

sub get_drop_column_ddl {
    my ($self, $table, $column) = @_;
    my $table_sql = $self->_sqlite_create_table($table);
    my $column_regex = _sqlite_column_regex($column);
    $table_sql =~ s/$column_regex/$1/
        || die "Can't find column $column: $table_sql";
    # Make sure we don't end up with a comma at the end of the definition.
    $table_sql =~ s/,\s+\)$/\n)/s;
    return $self->_sqlite_alter_schema($table, $table_sql,
                                       { exclude_column => $column });
}

sub get_rename_column_ddl {
    my ($self, $table, $old_name, $new_name) = @_;
    my $table_sql = $self->_sqlite_create_table($table);
    my $column_regex = _sqlite_column_regex($old_name);
    $table_sql =~ s/$column_regex/$1\t$new_name\t$3$4/
        || die "Can't find $old_name: $table_sql";
    my %rename = ($old_name => $new_name);
    return $self->_sqlite_alter_schema($table, $table_sql,
                                       { rename => \%rename });
}

################
# Foreign Keys #
################

sub get_add_fks_sql {
    my ($self, $table, $column_fks) = @_;
    my @add = $self->_column_fks_to_ddl($table, $column_fks);
    my $table_sql = $self->_sqlite_create_table($table);
    my $add_lines = join("\n,", @add) . "\n";
    $table_sql =~ s/\)$/$add_lines)/s
        || die "Can't find end of CREATE TABLE: $table_sql";
    return $self->_sqlite_alter_schema($table, $table_sql);
}

sub get_drop_fk_sql {
    my ($self, $table, $column, $references) = @_;
    my $table_sql = $self->_sqlite_create_table($table);
    my $fk_name = $self->_get_fk_name($table, $column, $references);
    $table_sql =~ s/^\s+CONSTRAINT $fk_name.*?ON DELETE \S+,?$//ms
        || die "Can't find $fk_name: $table_sql";
    return $self->_sqlite_alter_schema($table, $table_sql);
}


1;
