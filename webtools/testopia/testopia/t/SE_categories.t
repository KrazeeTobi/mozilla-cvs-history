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
# The Original Code is the Bugzilla Testopia System.
#
# The Initial Developer of the Original Code is Greg Hendricks.
# Portions created by Greg Hendricks are Copyright (C) 2006
# Novell. All Rights Reserved.
#
# Contributor(s): Greg Hendricks <ghendricks@novell.com>
#                 Jeff Dayley    <jedayley@novell.com>
#                 Ben Warby      <bwarby@novell.com>

use strict;

use lib "..";
use lib "../..";

use Test::More tests => 4;
use Test::Deep;

use Bugzilla;
use Bugzilla::Util;
use Bugzilla::Constants;

use Bugzilla::Testopia::Product;
use Bugzilla::Testopia::Category;

use Testopia::Test::Util;
use Testopia::Test::Constants;
use Testopia::Test::Selenium::Util;
use Testopia::Test::Selenium::Constants;

my ($user_type, $login, $passwd) = @ARGV;

use constant DEBUG => 1;

Bugzilla->error_mode(ERROR_MODE_DIE);

my $isanon = 0;
# $se object from Test::Selenium::Util
$se->start();

if ($login && $passwd){
    unless ( Testopia::Test::Selenium::Util->login( $login, $passwd) ) {
        $se->stop();
        fail('login');
        exit;
    }
}
else {
    $se->open( 'tr_categories.cgi' );
    $se->wait_for_page_to_load(TIMEOUT);
    $se->pause(3000) if DEBUG;
    
    ok( $se->is_text_present("I need a legitimate login and password to continue"), "anonymous user check" );
    $isanon = 1;

}
SKIP: {
    skip "Anonymous Login", 2 if $isanon;
    test_add();
    test_edit();
    test_list();
    test_delete();
}

sub test_list {
    my $self = shift;
    my $rep = get_rep('test_case_categories');
    
    my $test = {
        url    => "tr_categories.cgi",
        action => "list",
    };

    # Make sure it gives an error for no product_id
    $se->open( format_url($test) );
    $se->wait_for_page_to_load(TIMEOUT);
    ok(
        $se->is_text_present("testopia-missing-parameter"),
        "Failed missing product test"
    );

    $test->{params} = { product_id => $rep->{'product_id'} };

    # Make sure the page opens properly
    $se->open( format_url($test) );
    $se->wait_for_page_to_load(TIMEOUT);
    ok( $se->is_text_present("{categories:["), "Unable to open 'tr_categories.cgi'" );
#    $se->pause(3000);

}

sub test_add {
    my $self = shift;

    #  Create a unique name for the build using the number of seconds since epoch
    my $build_name = "SELENIUM CREATE ". time();
    my $rep = get_rep('products');

    #Add new build - suppose to error because missing name
    my $product = new Bugzilla::Testopia::Product($rep->{'id'});
    my $test    = {
        url    => "tr_categories.cgi",
        action => "add",
        params => {
            milestone   => $product->default_milestone,
            description => 'Generated by selenium',
            product_id  => $rep->{'id'},
            isactive    => 1,
        }
    };

    $se->open( format_url($test) );
    $se->wait_for_page_to_load(TIMEOUT);
    ok( $se->is_text_present("testopia-missing-required-field"), "No error for missing parameters" );

    #Add valid build
    $test->{params}->{name} = $build_name;

    $se->open( format_url($test) );
    $se->wait_for_page_to_load(TIMEOUT);
    ok( $se->is_text_present('success: true'), "'action=add' failed for tr_categories.cgi" );

    #check to make sure it was added properly
    my $dbh = Bugzilla->dbh;
    my $ref = $dbh->selectrow_hashref(
        'SELECT * 
           FROM test_case_categories 
          WHERE name = ? 
            AND product_id = ?',
        undef, ( $build_name, $rep->{'id'} )
    );

    $test->{params}->{build_id} = $ref->{build_id};
    cmp_deeply( $test->{params}, $ref, "Build Hashes match" );

    #check for adding duplicate names
    $se->open( format_url($test) );
    $se->wait_for_page_to_load(TIMEOUT);
    ok( $se->is_text_present("testopia-name-not-unique"), "category - duplicate check" );
    
    $test->{action} = 'delete';
    $se->open( format_url($test) );
    $se->wait_for_page_to_load(TIMEOUT);
    ok( $se->is_text_present("success: true"), "category - delete" );    

}

sub test_edit {
    my $self = shift;

    my $dbh = Bugzilla->dbh;

    #Get current build inforamtion
    my $rep = get_rep('test_categories');

    my $test = {
        url    => "tr_categories.cgi",
        action => "edit",
        params => {

            #product_id is not suppose to update anything
            #but it is required to update
            product_id => $rep->{'product_id'},
            build_id   => $rep->{'build_id'},
        }
    };

    $test->{params}->{name}        = "SELENIUM UPDATE ". time();
    $test->{params}->{milestone}   = get_rep_by_field('milestones', 'product_id', $rep->{'product_id'})->{'value'};
    $test->{params}->{description} = "Revised Selenium Test Description";
    $test->{params}->{isactive} = 0;

    #Try to update things
    $se->open( format_url($test) );
    $se->wait_for_page_to_load(TIMEOUT);
    ok( $se->is_text_present("success: true"), "'action=edit' failed for tr_categories.cgi" );

    #check edit succeded and values updated
    my $build = new Bugzilla::Testopia::Build($rep->{'build_id'});
    
    cmp_deeply( $test->{params}, noclass($build), "Build Hashes match" );
}
