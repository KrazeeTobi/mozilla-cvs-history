#!/usr/bonsaitools/bin/perl -w
# -*- Mode: perl; indent-tabs-mode: nil -*-
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
# 
# The Original Code is the Despot Account Administration System.
# 
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are Copyright (C) 1998
# Netscape Communications Corporation. All Rights Reserved.
# 
# Contributor(s): Terry Weissman <terry@mozilla.org>

# $F::debug = 1;


$cvs = "/opt/cvs-tools/bin/cvs";
    
$dontcommit = 1;                # I don't want to commit anything at all, yet.
$usertoblame = "";

for ($i=0 ; $i<@ARGV ; $i++) {
    if ($ARGV[$i] eq "-n") {
        $dontcommit = 1;
    }
    if ($ARGV[$i] eq "-user") {
        $usertoblame = $ARGV[++$i];
    }
}

$srcdir = $0;
$srcdir =~ s:/[^/]*$::;      # Remove last word, and slash before it.
if ($srcdir eq "") {
    $srcdir = ".";
}
chdir $srcdir || die "Couldn't chdir to $srcdir";

use Mysql;
require 'utils.pl';

$db = Mysql->Connect("localhost", "mozusers")
    || die "Can't connect to database server";

$db = $db;                      # Make -w shut up.

($mylogin = `/usr/ucb/whoami`) || ($mylogin = `/bin/whoami`);
chop($mylogin);

$hostname = 'unknown';
if (open(HOST, "/bin/hostname|")) {
    $hostname = <HOST>;
    chop($hostname);
    close(HOST);
}

if ($hostname ne "lounge.mozilla.org") {
    $dontcommit = 1;
}

$ENV{"HOME"} = glob("~$mylogin"); # CVS wants this.

$ENV{"CVS_PASSFILE"} = "./.cvspass";

if ($usertoblame eq "") {
    $usertoblame = $mylogin;
}

$boilerplate = "";

open(BOILERPLATE, "<commitcheck.templ") || die "Can't open template file";
while (<BOILERPLATE>) {
    if ( /^#/ ) {
        # Strip out comments from the boilerplate.  Might as well; the
        # faster our generated perl script runs, the better.
        next;
    }
    $boilerplate .= $_;
}
close BOILERPLATE;



$repquery = Query("select id,name,cvsroot from repositories order by name");
while (@reprow = $repquery->fetchrow()) {
    ($repid,$repname,$reproot) = (@reprow);

    $query = Query("select email,passwd,${repname}_group,neednewpassword from users where ${repname}_group != 'None' and passwd != '' order by email");

    $tmpdir = "/tmp/syncit.$$";
    mkdir $tmpdir, 0777;
    chdir $tmpdir;
    
    $ENV{CVSROOT} = $reproot;

    system "$cvs co CVSROOT/passwd CVSROOT/commitcheck.pl" || die "Couldn't checkout files.";
    
    $outfile = "CVSROOT/passwd";
    open(PASSWD, ">$outfile") || die "Can't open $outfile";
    
    print PASSWD "# DO NOT EDIT THIS FILE!  You must instead go to http://warp/mozilla.org, and\n";
    print PASSWD "# tweak things from there.\n";
    
    while (@row = $query->fetchrow()) {
        ($email,$password,$group,$neednew) = @row;
        if ($neednew eq "Yes") {
            next;
        }
        $login = $email;
        $login =~ s/@/%/g;
        print PASSWD "$login:$password:$group\n";
    }
    close PASSWD;
    
#    system "$cvs co CVSROOT/commitcheck.pl" || die "Couldn't checkout passwd file.";
    
    $outfile = "CVSROOT/commitcheck.pl";
    open(COMMITCHECK, ">$outfile") || die "Can't open $outfile";

    print COMMITCHECK "#!/tools/ns/bin/perl5.004 --\n";
    print COMMITCHECK "# DO NOT EDIT THIS FILE!  You must instead go to http://cvs-mirror.mozilla.org/webtools/despot, and\n";
    print COMMITCHECK "# tweak things from there.\n\n";

    $query = Query("select partitions.id,partitions.name,state,branches.name from partitions,branches where repositoryid = '$repid' and branches.id=branchid order by partitions.name");
    while (@row = $query->fetchrow()) {
        ($id,$name,$state,$branch) = (@row);
        $d = "\$";
        print COMMITCHECK $d . "mode{'$id'} = '$state';\n";
        print COMMITCHECK $d . "branch{'$id'} = '$branch';\n";
        print COMMITCHECK $d . "fullname{'$id'} = '$name';\n";
        if ($state ne "Open") {
            foreach $n ("blessed", "super") {
                print COMMITCHECK $d . "$n" . "{'$id'} = [";
                $eq = "=";
                if ($n eq "super") {
                    $eq = "!=";
                }
                $q2 = Query("select email from members,users where partitionid = $id and class $eq 'Member' and users.id = userid");
                while (@r2 = $q2->fetchrow()) {
                    my $n = $r2[0];
                    $n =~ s/@/%/;
                    print COMMITCHECK "'$n',";
                }
                print COMMITCHECK "];\n";
            }
        }
    }
    print COMMITCHECK "sub GetT {\n";
    print COMMITCHECK '($b,$_) = (@_);' . "\n";
    $query = Query("select branches.name,partitions.id from partitions,branches where repositoryid = '$repid' and state != 'Open' and branches.id = branchid order by branches.name");
    $lastbranch = "";
    while (@row = $query->fetchrow()) {
        ($branchname,$partid) = (@row);
        if ($branchname ne $lastbranch) {
            if ($lastbranch ne "") {
                print COMMITCHECK "}\n";
            }
            print COMMITCHECK "if (" . $d . "b eq '$branchname') {\n";
            $lastbranch = $branchname;
        }
        $q2 = Query("select pattern from files where partitionid=$partid order by pattern");
        while (@r2 = $q2->fetchrow()) {
            my $regexp = $r2[0];
            $regexp =~ s/\./\\./g;
            $regexp =~ s:\*$:.*:;
            $regexp =~ s:\%$:[^/]*:;
            print COMMITCHECK "if (m:$regexp:) {return '$partid';}\n";
        }
    }
    if ($lastbranch ne "") {
        print COMMITCHECK "}\n";
    }
    print COMMITCHECK "return '';\n";
    print COMMITCHECK "}\n";

    print COMMITCHECK $boilerplate;

    close COMMITCHECK;
    
    chdir "CVSROOT";
    if ($dontcommit) {
        system "$cvs diff -c passwd";
        system "$cvs diff -c commitcheck.pl";
        # system "$cvs commit -m 'Pseudo-automatic update of changes made by $usertoblame.' commitcheck.pl";
    } else {
        system "$cvs commit -m 'Pseudo-automatic update of changes made by $usertoblame.'";
    }
    
    chdir "/";
    
    system "rm -rf $tmpdir";
    
}


Query("delete from syncneeded");
    
