# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This Source Code Form is "Incompatible With Secondary Licenses", as
# defined by the Mozilla Public License, v. 2.0.

package Bugzilla::BugUrl::SourceForge;
use strict;
use base qw(Bugzilla::BugUrl);

use Bugzilla::Error;
use Bugzilla::Util;

###############################
####        Methods        ####
###############################

sub should_handle {
    my ($class, $uri) = @_;
    return ($uri->authority =~ /^sourceforge.net$/i
            and $uri->path =~ m|/tracker/|) ? 1 : 0;
}

sub _check_value {
    my $class = shift;

    my $uri = $class->SUPER::_check_value(@_);

    # SourceForge tracker URLs have only one form:
    #  http://sourceforge.net/tracker/?func=detail&aid=111&group_id=111&atid=111
    if ($uri->query_param('func') eq 'detail' and $uri->query_param('aid')
        and $uri->query_param('group_id') and $uri->query_param('atid'))
    {
        # Remove any # part if there is one.
        $uri->fragment(undef);
        return $uri;
    }
    else {
        my $value = $uri->as_string;
        ThrowUserError('bug_url_invalid', { url => $value });
    }
}

1;
