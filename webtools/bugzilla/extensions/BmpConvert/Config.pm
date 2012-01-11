# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This Source Code Form is "Incompatible With Secondary Licenses", as
# defined by the Mozilla Public License, v. 2.0.

package Bugzilla::Extension::BmpConvert;
use strict;
use constant NAME => 'BmpConvert';
use constant REQUIRED_MODULES => [
  {
      package => 'PerlMagick',
      module  => 'Image::Magick',
      version => 0,
  },
];

__PACKAGE__->NAME;
