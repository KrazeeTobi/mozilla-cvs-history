# 
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
# The Original Code is the Netscape Portable Runtime (NSPR).
# 
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1998-2000
# the Initial Developer. All Rights Reserved.
# 
# Contributor(s):
# 
# Alternatively, the contents of this file may be used under the terms of
# either of the GNU General Public License Version 2 or later (the "GPL"),
# or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

######################################################################
# Config stuff for Sony NEWS-OS
######################################################################
 
######################################################################
# Version-independent
######################################################################

include $(MOD_DEPTH)/config/UNIX.mk

ARCH			:= sony
CPU_ARCH		:= mips
 
CC			= cc
CCC			= CC
RANLIB			= /bin/true

OS_INCLUDES		= -I/usr/include
G++INCLUDES		=
#OS_LIBS			= -lsocket -lnsl -lgen -lresolv

PLATFORM_FLAGS		= -Xa -fullwarn -DSONY
PORT_FLAGS		= -DSYSV -DSVR4 -D__svr4 -D__svr4__ -D_PR_LOCAL_THREADS_ONLY -DHAVE_SVID_GETTOD

OS_CFLAGS		= $(PLATFORM_FLAGS) $(PORT_FLAGS)

######################################################################
# Version-specific stuff
######################################################################

######################################################################
# Overrides for defaults in config.mk (or wherever)
######################################################################

######################################################################
# Other
######################################################################

MKSHLIB			= $(LD) $(DSO_LDOPTS)
 
DSO_LDOPTS		= -G
