/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 * 
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 * 
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
 * the Initial Developer. All Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 * 
 * ***** END LICENSE BLOCK ***** */

/* ldap.h - general header file for libldap */

#ifndef _LDAP_H
#define _LDAP_H

/* Standard LDAP API functions and declarations */ 
#include "ldap-standard.h"

/* Extensions to the LDAP standard */
#include "ldap-extension.h"

/* A deprecated API is an API that we recommend you no longer use,
 * due to improvements in the LDAP C SDK. While deprecated APIs are
 * currently still implemented, they may be removed in future
 * implementations, and we recommend using other APIs.
 */

/* Soon-to-be deprecated functions and declarations */
#include "ldap-to-be-deprecated.h"

/* Deprecated functions and declarations */
#include "ldap-deprecated.h"

#endif /* _LDAP_H */

