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
# Contributor(s): Marc Schumann <wurblzap@gmail.com>
#                 Max Kanat-Alexander <mkanat@bugzilla.org>
#                 Mads Bondo Dydensborg <mbd@dbc.dk>

package Bugzilla::WebService::Bugzilla;

use strict;
use base qw(Bugzilla::WebService);
use Bugzilla::Constants;
use Bugzilla::Hook;

use DateTime;

# Basic info that is needed before logins
use constant LOGIN_EXEMPT => {
    timezone => 1,
    version => 1,
};

sub version {
    my $self = shift;
    return { version => $self->type('string', BUGZILLA_VERSION) };
}

sub extensions {
    my $self = shift;
    my $extensions = Bugzilla::Hook::enabled_plugins();
    foreach my $name (keys %$extensions) {
        my $info = $extensions->{$name};
        foreach my $data (keys %$info) {
            $extensions->{$name}->{$data} = 
                $self->type('string', $info->{$data});
        }
    }
    return { extensions => $extensions };
}

sub timezone {
    my $self = shift;
    my $offset = Bugzilla->local_timezone->offset_for_datetime(DateTime->now());
    $offset = (($offset / 60) / 60) * 100;
    $offset = sprintf('%+05d', $offset);
    return { timezone => $self->type('string', $offset) };
}

1;

__END__

=head1 NAME

Bugzilla::WebService::Bugzilla - Global functions for the webservice interface.

=head1 DESCRIPTION

This provides functions that tell you about Bugzilla in general.

=head1 METHODS

See L<Bugzilla::WebService> for a description of what B<STABLE>, B<UNSTABLE>,
and B<EXPERIMENTAL> mean.

=over

=item C<version> B<EXPERIMENTAL>

=over

=item B<Description>

Returns the current version of Bugzilla.

=item B<Params> (none)

=item B<Returns>

A hash with a single item, C<version>, that is the version as a
string.

=item B<Errors> (none)

=back

=item C<extensions> B<EXPERIMENTAL>

=over

=item B<Description>

Gets information about the extensions that are currently installed and enabled
in this Bugzilla.

=item B<Params> (none)

=item B<Returns>

A hash with a single item, C<extesions>. This points to a hash. I<That> hash
contains the names of extensions as keys, and information about the extension
as values. One of the values that must be returned is the 'version' of the
extension

=back

=item C<timezone> B<EXPERIMENTAL>

=over

=item B<Description>

Returns the timezone of the server Bugzilla is running on. This is
important because all dates/times that the webservice interface
returns will be in this timezone.

=item B<Params> (none)

=item B<Returns>

A hash with a single item, C<timezone>, that is the timezone as a
string in (+/-)XXXX (RFC 2822) format.

=back

=back
