#############################################################################
# $Id: Conn.pm,v 1.14 1998/08/09 01:16:52 leif Exp $
#
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
# The Original Code is PerlDAP. The Initial Developer of the Original
# Code is Netscape Communications Corp. and Clayton Donley. Portions
# created by Netscape are Copyright (C) Netscape Communications
# Corp., portions created by Clayton Donley are Copyright (C) Clayton
# Donley. All Rights Reserved.
#
# Contributor(s):
#
# DESCRIPTION
#    This is the main object class for connecting to an LDAP server,
#    and perform searches and updates. It depends on the ::Entry
#    object class, which is the data type returned from a search
#    operation for instance.
#
#############################################################################

package Mozilla::LDAP::Conn;

use Mozilla::LDAP::Utils;
use Mozilla::LDAP::API qw(/.+/);
use Mozilla::LDAP::Entry;


#############################################################################
# Creator, create and initialize a new LDAP object ("connection"). We support
# either providing all parameters as a hash array, or as individual
# arguments.
#
sub new
{
  my $class = shift;
  my $self = {};
  my $ref;

  $ref = ref($_[$[]);
  if (ref $_[$[] eq "HASH")
    {
      my $hash;

      $hash = $_[$[];
      $self->{host} = $hash->{host};
      $self->{port} = $hash->{port};
      $self->{binddn} = $hash->{bind};
      $self->{bindpasswd} = $hash->{pswd};
      $self->{certdb} = $hash->{cert};
    }
  else
    {
      my ($host, $port, $binddn, $bindpasswd, $certdb, $authmeth) = @_;

      $self->{host} = $host;
      $self->{port} = $port;
      $self->{binddn} = $binddn;
      $self->{bindpasswd} = $bindpasswd;
      $self->{certdb} = $certdb;
    }

  if (!defined($self->{port}) || ($self->{port} eq ""))
    {
      $self->{port} = (($self->{certdb} ne "") ? LDAPS_PORT : LDAP_PORT);
    }

  bless $self, $class;

  return unless $self->init();
  return $self;
}


#############################################################################
# Destructor, makes sure we close any open LDAP connections.
#
sub DESTROY
{
  my $self = shift;

  return unless defined($self->{ld});

  ldap_unbind_s($self->{ld});
  ldap_msgfree($self->{ldres}) if defined($self->{ldres});

  undef $self->{ld};
}


#############################################################################
# Initialize a normal connection. This seems silly, why not just merge
# this back into the creator method (new)...
#
sub init
{
  my $self = shift;
  my $ret;
  my $ld;

  if ($self->{certdb} ne "")
    {
      $ret = ldapssl_client_init($self->{certdb}, "");
      return 0 if ($ret < 0);

      $ld = ldapssl_init($self->{host}, $self->{port}, 1);
    }
  else
    {
      $ld = ldap_init($self->{host}, $self->{port});
    }
  if (!$ld)
    {
      perror("ldap_init");

      return 0;
    }

  $self->{ld} = $ld;
  $ret = ldap_simple_bind_s($ld, $self->{binddn}, $self->{bindpasswd});

  if ($ret)
    {
      ldap_perror($ld, "Authentication failed");

      return 0;
    }

  return 1;
}


#############################################################################
# Checks if a string is a properly formed LDAP URL.
#
sub isURL
{
  my ($self, $url) = @_;

  return ldap_is_ldap_url($url);
}


#############################################################################
# Return the actual low level LD connection structure, which is needed if
# you want to call any of the API functions yourself...
#
sub getLD
{
  my ($self) = @_;

  return $self->{ld} if $self->{ld};
}


#############################################################################
# Return the Error code from the last LDAP api function call. The last two
# optional arguments are pointers to strings, and will be set to the
# match string and extra error string if appropriate.
#
sub getErrorCode
{
  my ($self, $match, $msg) = @_;
  my $ret;

  return ldap_get_lderrno($self->{ld}, $match, $msg);
}
*getError = \*getErrorCode;


#############################################################################
# Return the Error string from the last LDAP api function call.  
#
sub getErrorString 
{
  my ($self) = @_;
  my ($err);
  
  $err = ldap_get_lderrno($self->{ld}, undef, undef);

  return ldap_err2string($err);
}


#############################################################################
# Print the last error code...
#
sub printError
{
  my ($self, $str) = @_;

  $str = "LDAP error: " if ($str eq "");
  ldap_perror($self->{ld}, $str);
}


#############################################################################
# Normal LDAP search. Note that this will actually perform LDAP URL searches
# if the filter string looks like a proper URL.
#
sub search
{
  my ($self, $basedn, $scope, $filter, $attrsonly, @attrs) = @_;
  my $resv;
  my $entry;
  my $res = \$resv;

  $scope = Mozilla::LDAP::Utils::str2Scope($scope);
  $filter = "(objectclass=*)" if ($filter =~ /^ALL$/i);

  ldap_msgfree($self->{ldres}) if defined($self->{ldres});
  if (ldap_is_ldap_url($filter))
    {
      if (! ldap_url_search_s($self->{ld}, $filter, $attrsonly, $res))
	{
	  $self->{ldres} = $res;
	  $self->{ldfe} = 1;
	  $entry = $self->nextEntry();
	}
    }
  else
    {
      if (! ldap_search_s($self->{ld}, $basedn, $scope, $filter, \@attrs,
			  $attrsonly, $res))
	{
	  $self->{ldres} = $res;
	  $self->{ldfe} = 1;
	  $entry = $self->nextEntry();
	}
    }

  return $entry;
}


#############################################################################
# URL search, optimized for LDAP URL searches.
#
sub searchURL
{
  my ($self, $url, $attrsonly) = @_;
  my $resv;
  my $entry;
  my $res = \$resv;

  ldap_msgfree($self->{ldres}) if defined($self->{ldres});
  if (! ldap_url_search_s($self->{ld}, $url, $attrsonly, $res))
    {
      $self->{ldres} = $res;
      $self->{ldfe} = 1;
      $entry = $self->nextEntry();
    }

  return $entry;
}


#############################################################################
# Get an entry from the search, either the first entry, or the next entry,
# depending on the call order.
#
sub nextEntry
{
  my $self = shift;
  my %entry;
  my @ocorder;
  my ($attr, $vals, $obj, $ldentry, $berv, $dn);
  my $ber = \$berv;

  # I use the object directly, to avoid setting the "change" flags
  $obj = tie %entry, Mozilla::LDAP::Entry;

  $self->{dn} = "";
  if ($self->{ldfe} == 1)
    {
      $self->{ldfe} = 0;
      $ldentry = ldap_first_entry($self->{ld}, $self->{ldres}); 
      $self->{ldentry} = $ldentry;
    }
  else
    {
      return "" unless $self->{ldentry};
      $ldentry = ldap_next_entry($self->{ld}, $self->{ldentry}); 
      $self->{ldentry} = $ldentry;
    }
  return "" unless $ldentry;

  $dn = ldap_get_dn($self->{ld}, $self->{ldentry});
  $obj->{dn} = $dn;
  $self->{dn} = $dn;
  $attr = ldap_first_attribute($self->{ld}, $self->{ldentry}, $ber);
  return (bless \%entry, Mozilla::LDAP::Entry) unless $attr;

  @vals = ldap_get_values_len($self->{ld}, $self->{ldentry}, $attr);
  $obj->{$attr} = [@vals];
  push(@ocorder, $attr);

  while ($attr = ldap_next_attribute($self->{ld},
				     $self->{ldentry}, $ber))
    {
      @vals = ldap_get_values_len($self->{ld}, $self->{ldentry}, $attr);
      $obj->{$attr} = [@vals];
      push(@ocorder, $attr);
    }
  $obj->{_oc_order_} = \@ocorder;
  $obj->{_self_obj_} = $obj;

  ldap_ber_free($ber, 0) if $ber;

  return bless \%entry, Mozilla::LDAP::Entry;
}

# This is deprecated...
*entry = \*nextEntry;


#############################################################################
# Close the connection to the LDAP server.
#
sub close
{
  my $self = shift;
  my $ret = 1;

  $ret = ldap_unbind_s($self->{ld}) if defined($self->{ld});
  undef $self->{ld};

  return ($ret == LDAP_SUCCESS);
}


#############################################################################
# Delete an object.
#
sub delete
{
  my ($self, $dn) = @_;
  my $ret = 1;

  if ($dn ne "")
    {
      $dn = Mozilla::LDAP::Utils::normalizeDN($dn);
    }
  else
    {
      $dn = Mozilla::LDAP::Utils::normalizeDN($self->{dn});
    }
  $ret = ldap_delete_s($self->{ld}, $dn) if ($dn ne "");

  return ($ret == LDAP_SUCCESS)
}


#############################################################################
# Add an object.
#
sub add
{
  my ($self, $entry) = @_;
  my ($ref, $key, $val, %ent);
  my $ret = 1;
  my $gotcha = 0;

  $ref = ref($entry);
  if (($ref eq "Mozilla::LDAP::Entry") || ($ref eq "HASH"))
    {
      foreach $key (keys %{$entry})
	{
	  next if (($key eq "dn") || ($key =~ /^_.+_$/));
	  $ent{$key} = $entry->{$key};
	  $gotcha++;
	}

      $ret = ldap_add_s($self->{ld}, $entry->{dn}, \%ent) if $gotcha;
    }

  return ($ret == LDAP_SUCCESS);
}


#############################################################################
# Modify the RDN, and update the entry accordingly. Note that the last
# two arguments (DN and "delete") are optional.
#
sub modifyRDN
{
  my ($self, $rdn, $dn, $del) = ($_[$[], lc $_[$[ + 1], $_[$[ + 2], $_[$[ + 3]);
  my (@vals);
  my $ret = 1;

  $del = 1 if ($del eq "");
  $dn = $self->{dn} if ($dn eq "");

  @vals = ldap_explode_dn(lc $dn, 0);
  if ($vals[$[] ne $rdn)
    {
      $ret = ldap_modrdn2_s($self->{ld}, $dn, $rdn, $del);
      if ($ret == LDAP_SUCCESS)
	{
	  shift(@vals);
	  unshift(@vals, ($rdn));
	  $ld->{dn} = join(@vals);
	}
    }

  return ($ret == LDAP_SUCCESS);
}


#############################################################################
# Update an object. NOTE: I'd like to clean up my change tracking tags here,
# so that we can call update() again with the same entry.
#
sub update
{
  my ($self, $entry) = @_;
  my (@vals, %mod, %new, @arr);
  my ($key, $val);
  my $ret = 1;
  local $_;

  foreach $key (keys (%$entry))
    {
      next if (($key eq "dn") || ($key =~ /^_.+_/));

      if ($entry->{"_${key}_modified_"})
	{
	  @vals = @{$entry->{$key}};
	  if ($#vals == $[)
	    {
	      $mod{$key} = { "rb", [$vals[$[]] };
	    }
	  else
	    {
	      @arr = ();
	      grep(($new{$_} = 1), @vals);
	      foreach (@{$entry->{"_${key}_save_"}})
		{
		  if (! $new{$_})
		    {
		      push(@arr, $_);
		    }
		  $new{$_} = 0;
		}
	      $mod{$key}{"db"} = [@arr] if ($#arr >= $[);

	      @arr = ();
	      foreach (keys(%new))
		{
		  push(@arr, $_) if ($new{$_} == 1);
		}
	      $mod{$key}{"ab"} = [@arr] if ($#arr >= $[);
	    }

	  delete $entry->{_self_obj_}->{"_${key}_modified_"};
	  undef @{$entry->{"_${key}_save_"}};
	}
      elsif ($entry->{"_${key}_deleted_"})
	{
	  $mod{$key} = { "db", [] };
	  undef @{$entry->{"_${key}_save_"}};
	  delete $entry->{_self_obj_}->{"_${key}_deleted_"};
	}
    }

  @arr = keys %mod;
  # This is here for debug purposes only...
  if ($main::LDAP_DEBUG)
    {
      foreach $key (@arr)
	{
	  print "Working on $key\n";
	  foreach $op (keys %{$mod{$key}})
	    {
	      print "\tDoing operation: $op\n";
	      foreach $val (@{$mod{$key}{$op}})
		{
		  print "\t\t$val\n";
		}
	    }
	}
    }

  $ret = ldap_modify($self->{ld}, $entry->{dn}, \%mod)
    if ($#arr >= $[);

  return ($ret == LDAP_SUCCESS);
}


#############################################################################
# Set the rebind procedure. This is old and obsolete...
#
sub setRebindProc
{
  my ($self, $proc) = @_;

  # Should we try to reinitialize the connection?
  die "No LDAP connection" unless defined($self->{ld});

  ldap_set_rebind_proc($self->{"ld"}, $proc);
}


#############################################################################
# Mandatory TRUE return value.
#
1;


#############################################################################
# POD documentation...
#
__END__

=head1 NAME

  Mozilla::LDAP::Conn - Object Oriented API for the LDAP SDK.

=head1 SYNOPSIS

  use Mozilla::LDAP::Conn;
  use Mozilla::LDAP::Utils;

=head1 ABSTRACT

This package is the main API for using our Perl Object Oriented LDAP
module. Even though it's certainly possible, and sometimes even necessary,
to call the native LDAP C SDK functions, we strongly recommend you use
these object classes.

It's not required to use our Mozilla::LDAP::Utils.pm package, but it's
convenient and good for portability if you use as much as you can from
that package as well. This implies using the LdapConf package as well,
even though you usually don't need to use it directly.

You should read this document in combination with the Mozilla::LDAP::Entry
document. Both modules depend on each other heavily.

=head1 DESCRIPTION

First, this is not ment to be a crash course in how LDAP works, if you
have no experience with LDAP, I suggest you read some of the literature
that's available out there. The LDAP Deployment Book from Netscape, or the
LDAP C SDK documentation are good starting points.

This object class basically tracks and manages the LDAP connection, it's
current status, and the current search operation (if any). Every time you
call the B<search> method of an object instance, you'll reset it's
internal state. It depends heavily on the ::Entry class, which are used to
retrieve, modify and update a single entry.

The B<search> and B<nextEntry> methods returns Mozilla::LDAP::Entry
objects, naturally. You also have to instantiate (and modify) a new
::Entry object when you want to add new entries to an LDAP
server. Alternatively, the add() method will also take a hash array as
argument, to make it easy to create new LDAP entries.

To assure that changes to an entry are updated properly, we strongly
recommend you use the native methods of the ::Entry object class. Even
though you can modify certain elements directly, it could cause changes
not to be committed to the LDAP server. If there's something missing from
the API, please let us know, or even fix it yourself.

=head1 SOME PERLDAP/OO BASICS

An entry consist of a DN, and a hash array of pointers to attribute
values. Each attribute value (except the DN) is an array, but you have to
remember the hash array in the entry stores pointers to the array, not the
array. So, to access the first CN value of an entry, you'd do

    $cn = $entry->{cn}[0];

To set the CN attribute to a completely new array of values, you'd do

    $entry->{cn} = [ "Leif Hedstrom", "The Swede" ];

As long as you remember this, and try to use native Mozilla::LDAP::Entry
methods, this package will take care of most the work. Once you master
this, working with LDAP in Perl is surprisingly easy.

We already mentioned DN, which stands for Distinguished Name. Every entry
on an LDAP server must have a DN, and it's always guaranteed to be unique
within your database. Some typical DNs are

    uid=leif,ou=people,o=netscape.com
    cn=gene-staff,ou=mailGroup,o=netscape.com
    dc=data,dc=netscape,dc=com

There's also a term called RDN, which stands for Relative Distinguished
Name. In the above examples, C<uid=leif>, C<cn=gene-staff> and C<dc=data>
are all RDNs. One particular property for a RDN is that they must be
unique within it's sub-tree. Hence, there can only be one user with
C<uid=leif> within the ou=people tree, there can never be a name conflict.

=head1 CREATING A NEW OBJECT INSTANCE

Before you can do anything with PerLDAP, you'll need to instantiate at
least one Mozilla::LDAP::Conn object, and connect it to an LDAP server. As
you probably guessed already, this is done with the B<new> method:

    $conn = new Mozilla::LDAP::Conn("ldap", "389", $bind, $pswd, $cert);
    die "Couldn't connect to LDAP server ldap" unless  $conn;

The arguments are: Host name, port number, and optionally a bind-DN, it's
password, and a certificate. If there is no bind-DN, the connection will
be bound as the anonymous user. If the certificate file is specified, the
connection will be over SSL, and you should then probably connect to port
636. You have to check that the object was created properly, and take
proper actions if you couldn't get a connection.

There's one convenient alternative call method to this function. Instead of
providing each individual argument, you can provide one hash array
(actually, a pointer to a hash). For example:

    %ld = Mozilla::LDAP::Utils::ldapArgs();
    $conn = new Mozilla::LDAP::Conn(\%ld);

The components of the hash are:

    $ld->{host}
    $ld->{port}
    $ld->{root}
    $ld->{bind}
    $ld->{pswd}
    $ld->{cert}

and (not used in the B<new> method)

    $ld->{scope}

Once a connection is established, the package will take care of the
rest. If for some reason the connection is lost, the object should
reconnect on it's own, automatically. [Note: This doesn't work
now... ]. You can use the Mozilla::LDAP:Conn object for any number of
operations, but since everything is currently done synchronously, you can
only have one operation active at any single time. You can of course have
multiple Mozilla::LDAP::Conn instanced active at the same time.

=head1 PERFORMING LDAP SEARCHES

We assume that you are familiar with the LDAP filter syntax already, all
searches performed by this object class uses these filters. You should
also be familiar with LDAP URLs, and LDAP object classes. There are some
of the few things you actually must know about LDAP. Perhaps the simples
filter is

    (uid=leif)

This matches all entries with the UID set to "leif". Normally that
would only match one entry, but there is no guarantee for that. To find
everyone with the name "leif", you'd instead do

    (cn=*leif*)

A more complicated search involves logic operators. To find all mail
groups owned by "leif" (or actually his DN), you could do

    (&(objectclass=mailGroup)(owner=uid=leif,ou=people,o=netscape))

The I<owner> attribute is what's called a DN attribute, so to match on it
we have to specify the entire DN in the filter above. We could of course
also do a sub string "wild card" match, but it's less efficient, and
requires indexes to perform reasonably well.

Ok, now we are prepared to actually do a real search on the LDAP server:

    $base = "o=netscape.com";
    $conn = new Mozilla::LDAP::Conn("ldap", "389", "", ""); die "No LDAP
    connection" unless $conn;

    $entry = $conn->search($base, "subtree", "(uid=leif)");
    if (! $entry)
      { # handle this event, no entries found, dude!
      }
    else
      {
        while ($entry)
          {
            $entry->printLDIF();
            $entry = $conn->nextEntry();
          }
      }

This is in fact a poor mans implementation of the I<ldapsearch> command
line utility. The B<search> method returns an Mozilla::LDAP::Entry object,
which holds the first entry from the search, if any. To get the second and
subsequent entries you call the B<entry> method, until there are no more
entries. The B<printLDIF> method is a convenient function, requesting the
entry to print itself on STDOUT, in LDIF format.

The arguments to the B<search> methods are the I<LDAP Base-DN>, the
I<scope> of the search ("base", "one" or "sub"), and the actual LDAP
I<filter>. The entry return contains the DN, and all attribute values. To
access a specific attribute value, you just have to use the hash array:

    $cn = $entry->{cn}[0];

Since many LDAP attributes can have more than one value, value of the hash
array is another array (or actually a pointer to an array). In many cases
you can just assume the value is in the first slot (indexed by [0]), but
for some attributes you have to support multiple values. To find out how
many values a specific attribute has, you'd call the B<size> method:

    $numVals = $entry->size("objectclass");

One caveat: Many LDAP attributes are case insensitive, but the methods in
the Mozilla::LDAP::Entry package are not aware of this. Hence, if you
compare values with case sensitivity, you can experience weird
behavior. If you know an attribute is CIS (Case Insensitive), make sure
you do case insensitive string comparisons.

Unfortunately some methods in this package can't do this, and by default
will do case sensitive comparisons. We are working on this, and in a
future release some of the methods will handle this more gracefully. As an
extension (for LDAP v3.0) we could also use schema discovery for handling
this even better.

There is an alternative search method, to use LDAP URLs instead of a
filter string. This can be used to easily parse and process URLs, which is
a compact way of storing a "link" to some specific LDAP information. To
process such a search, you use the B<searchURL> method:

    $entry->searchURL("ldap:///o=netscape.com??sub?(uid=leif");

As it turns out, the B<search> method also supports LDAP URL searches. If
the search filter looks like a proper URL, we will actually do an URL
search instead. This is for backward compatibility, and for ease of use.

To achieve better performance and use less memory, you can limit your
search to only retrieve certain attributes. With the LDAP URLs you specify
this as an optional parameter, and with the B<search> method you add two
more options, like

    $entry = $conn->search($base, "sub", $filter, 0, ("mail", "cn");

The last argument specifies an array of attributes to retrieve, the fewer
the attributes, the faster the search will be. The second to last argument
is a boolean value indicating if we should retrieve only the attribute
names (and no values). In most cases you want this to be FALSE, to
retrieve both the attribute names, and all their values. To do this with
the B<searchURL> method, add a second argument, which should be 0 or 1.

=head1 MODIFYING AND CREATING NEW LDAP ENTRIES

Once you have an LDAP entry, either from a search, or created directly to
get a new empty object, you are ready to modify it. If you are creating a
new entry, the first thing to set it it's DN:

    $entry->setDN("uid=leif,ou=people,o=netscape.com");

You should not do this for an existing LDAP entry, changing the RDN (or
DN) for such an entry must be done with B<modifyRDN>. To populate (or
modify) some other attributes, we can do

    $entry->{objectclass} = [ "top", "person", "inetOrgPerson" ];
    $entry->{cn} = [ "Leif Hedstrom" ];
    $entry->{mail} = [ "leif@netscape.com" ];

Once you are done modifying your LDAP entry, call the B<update> method
from the Mozilla::LDAP::Conn object instance:

    $conn->update($entry);

Or, if you are creating an entirely new LDAP entry, you must call the
B<add> method:

    $conn->add($entry);

If all comes to worse, and you have to remove an entry again from the LDAP
server, just call the B<delete> method, like

    $conn->delete($entry);

You can't use native Perl functions like push() and splice() on attribute
values, since they won't update the ::Entry instance state properly.
Instead use one of the methods provided by the object class, for instance

    $conn->addValue("cn", "The Swede");
    $conn->removeValue("mailAlternateAddress", "leif@mcom.com");
    $conn->remove("seeAlso");

These methods return a TRUE or FALSE value, depending on the outcome
of the operation. If there was no value to remove, or a value already
exists, we return FALSE, otherwise TRUE. To check if an attribute has a
certain value, use the B<hasValue> method, like

    if ($conn->hasValue("mail", "leif@netscape.com")) {
        # Do something
    }

There is a similar method, B<matchValue>, which takes a regular
expression to match against, instead of the entire string. For more
information this and other methods in the Entry class, see below.
    
=head1 OBJECT CLASS METHODS

We have already described the fundamentals of this class earlier. This is
a summary of all available methods which you can use. Be careful not to
use any undocumented features or heaviour, since the internals in this
module is likely to change.

=head2 Searching and updating entries

=over 13

=item B<new>

This creates and initialized a new LDAP connection and object. The
required arguments are host name, port number, bind DN and the bind
password. An optional argument is a certificate (public key), which causes
the LDAP connection to be established over an SSL channel. Currently we do
not support Client Authentication, so you still have to use the simple
authentication method (i.e. with a password).

Also, remember that if you use SSL, the port is (usually) 636.

=item B<search>

The B<search> method is the main entry point into this module. It requires
at least three arguments: The Base DN, the scope, and the search
strings. Two more optional arguments can be given, the first specifies if
only attribute names should be returned (TRUE or FALSE). The second
argument is a list (array) of attributes to return.

The last option is very important for performance. If you are only
interested in say the "mail" and "mailHost" attributes, specifying this in
the search will signficantly reduce the search time.

=item B<searchURL>

This is almost identical to B<search>, except this function takes only two
arguments, an LDAP URL and an optional flag to specify if we only want the
attribute names to be returned (and no values). This function isn't very
useful, since the B<search> method will actually honor properly formed
LDAP URL's, and use it if appropriate.

=item B<nextEntry>

This method will return the next entry from the search result, and can
therefore only be called after a succesful search has been initiated. If
there are no more entries to retrieve, it returns nothing (empty string).

=item B<update>

After modifying an Ldap::Entry entry (see below), use the B<update>
method to commit changes to the LDAP server. Only attributes that has been
changed will be updated, assuming you have used the appropriate methods in
the Entry object. For instance, do not use B<push> or B<splice> to
modify an entry, the B<update> will not recognize such changes.

=item B<delete>

This will delete the current entry, or possibly an entry as specified with
the optional argument. You can use this function to delete any entry you
like, by passing it an explicit DN. If you don't pass it this argument,
B<delete> defaults to delete the current entry, from the last call to
B<search> or B<entry> .

=item B<add>

Add a new entry to the LDAP server. Make sure you use the B<new> method
for the Mozilla::LDAP::Entry object, to create a proper entry.

=item B<close>

Close the LDAP connection, and clean up the object. If you don't call this
directly, the destructor for the object instance will do the job for you.

=item B<modifyRDN>

This will rename the specified LDAP entry, by modifying it's RDN.

=back

=head2 Other methods

=over 13

=item B<isURL>

Returns TRUE or FALSE if the given argument is a properly formed URL.

=item B<getLD>

Return the (internal) LDAP* connection handle, which you can use
(carefully) to call the native LDAP API functions. You shouldn't have to
use this in most cases, unless of course our OO layer is seriously flawed.

=item B<getErrorCode>

Return the error code (numeric) from the last LDAP API function
call. Remember that this can only be called I<after> the successful
creation of a new object instance.

=item B<getErrorString>

Very much like B<getError>, but return a string with a human readable
error message.

=item B<printError>

Print the last error message on standard output.

=item B<setRebindProc>

Tell the LDAP SDK to call the provided Perl function when it has to follow
referrals. The Perl function should return an array of three elements, the
new Bind DN, password and authentication method. A typical usage is

    sub rebindProc {
        return ("uid=ldapadmin", "secret", LDAP_AUTH_SIMPLE);
    }

    $ld->setRebindProc(\&rebindProc);

=item B<setDefaultRebindProc>

This is very much like the previous function, except instead of specifying
the function to use, you give it the DN, password and Auth method. Then
we'll use a default rebind procedure (internal in C) to handle the rebind
credentials. This was a solution for the Windows/NT problem/bugs we have
with rebind procedures written in Perl.

=back

=head1 EXAMPLES

There are plenty of examples to look at, in the examples directory. We are
adding more examples every day (almost).

=head1 INSTALLATION

Installing this package is part of the Makefile supplied in the
package. See the installation procedures which are part of this package.

=head1 AVAILABILITY

This package can be retrieved from a number of places, including:

    http://www.mozilla.org/
    Your local CPAN server

=head1 AUTHOR INFORMATION

Address bug reports and comments to:
xxx@netscape.com

=head1 CREDITS

Most of this code was developed by Leif Hedstrom, Netscape Communications
Corporation. 

=head1 BUGS

None. :)

=head1 SEE ALSO

L<Mozilla::LDAP::Entry>, L<LDAP::Mozilla:Utils> L<LDAP::Mozilla:API> and
of course L<Perl>.

=cut
