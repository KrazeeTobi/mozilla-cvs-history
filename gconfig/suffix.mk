# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is mozilla.org code.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1998
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

#######################################################################
# Master "Core Components" suffixes                                   #
#######################################################################

#
# Object suffixes
#

ifndef OBJ_SUFFIX
	ifeq ($(OS_ARCH), WINNT)
		OBJ_SUFFIX = .obj
	else
		OBJ_SUFFIX = .o
	endif
endif

#
# Resource suffixes
#

ifndef RC_SUFFIX
	ifeq ($(OS_ARCH), WINNT)
		RC_SUFFIX = .rc
		RES_SUFFIX = .res
	else
		RC_SUFFIX =
		RES_SUFFIX =
	endif
endif

#
# Library suffixes
#

STATIC_LIB_EXTENSION =

ifndef DYNAMIC_LIB_EXTENSION
	ifeq ($(OS_ARCH)$(OS_RELEASE), AIX4.1)
		DYNAMIC_LIB_EXTENSION = _shr
	else
		DYNAMIC_LIB_EXTENSION =
	endif
endif


ifndef STATIC_LIB_SUFFIX
	STATIC_LIB_SUFFIX = .$(LIB_SUFFIX)
endif


ifndef DYNAMIC_LIB_SUFFIX
	DYNAMIC_LIB_SUFFIX = .$(DLL_SUFFIX)
endif


ifndef IMPORT_LIB_SUFFIX
	ifeq ($(OS_ARCH), WINNT)
		IMPORT_LIB_SUFFIX = .$(LIB_SUFFIX)
	else
		IMPORT_LIB_SUFFIX = 
	endif
endif


ifndef PURE_LIB_SUFFIX
	ifeq ($(OS_ARCH), WINNT)
		PURE_LIB_SUFFIX =
	else
		ifdef DSO_BACKEND
			PURE_LIB_SUFFIX = .$(DLL_SUFFIX)
		else
			PURE_LIB_SUFFIX = .$(LIB_SUFFIX)
		endif
	endif
endif


ifndef STATIC_LIB_SUFFIX_FOR_LINKING
	STATIC_LIB_SUFFIX_FOR_LINKING = $(STATIC_LIB_SUFFIX)
endif


ifndef DYNAMIC_LIB_SUFFIX_FOR_LINKING
	ifeq ($(OS_ARCH), WINNT)
		DYNAMIC_LIB_SUFFIX_FOR_LINKING = $(IMPORT_LIB_SUFFIX)
	else
		DYNAMIC_LIB_SUFFIX_FOR_LINKING = $(DYNAMIC_LIB_SUFFIX)
	endif
endif

#
# Program suffixes
#

ifndef PROG_SUFFIX
	ifeq ($(OS_ARCH), WINNT)
		PROG_SUFFIX = .exe
	else
		PROG_SUFFIX =
	endif
endif
