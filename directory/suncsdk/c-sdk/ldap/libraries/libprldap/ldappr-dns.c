/*
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 */

/*
 * DNS callback functions for libldap that use the NSPR (Netscape
 * Portable Runtime) thread API.
 *
 */

#include "nspr.h"
#include "ldappr-int.h"

static LDAPHostEnt *prldap_gethostbyname( const char *name,
	LDAPHostEnt *result, char *buffer, int buflen, int *statusp,
	void *extradata );
static LDAPHostEnt *prldap_gethostbyaddr( const char *addr, int length,
	int type, LDAPHostEnt *result, char *buffer, int buflen,
	int *statusp, void *extradata );
static int prldap_getpeername( LDAP *ld, struct sockaddr *addr,
	char *buffer, int buflen );
static LDAPHostEnt *prldap_convert_hostent( LDAPHostEnt *ldhp,
	PRHostEnt *prhp );


/*
 * Install NSPR DNS functions into ld (if ld is NULL, they are installed
 * as the default functions for new LDAP * handles).
 *
 * Returns 0 if all goes well and -1 if not.
 */
int
prldap_install_dns_functions( LDAP *ld )
{
    struct ldap_dns_fns			dnsfns;

    memset( &dnsfns, '\0', sizeof(struct ldap_dns_fns) );
    dnsfns.lddnsfn_bufsize = PR_NETDB_BUF_SIZE;
    dnsfns.lddnsfn_gethostbyname = prldap_gethostbyname;
    dnsfns.lddnsfn_gethostbyaddr = prldap_gethostbyaddr;
	    dnsfns.lddnsfn_getpeername = prldap_getpeername;
	    if ( ldap_set_option( ld, LDAP_OPT_DNS_FN_PTRS, (void *)&dnsfns ) != 0 ) {
		return( -1 );
	    }

	    return( 0 );
	}


	static LDAPHostEnt *
	prldap_gethostbyname( const char *name, LDAPHostEnt *result,
		char *buffer, int buflen, int *statusp, void *extradata )
	{
	    PRHostEnt	prhent;

	    if( !statusp || ( *statusp = (int)PR_GetIPNodeByName( name,
			PRLDAP_DEFAULT_ADDRESS_FAMILY, PR_AI_DEFAULT,
			buffer, buflen, &prhent )) == PR_FAILURE ) {
		return( NULL );
	    }

	    return( prldap_convert_hostent( result, &prhent ));
	}


	static LDAPHostEnt *
	prldap_gethostbyaddr( const char *addr, int length, int type,
		LDAPHostEnt *result, char *buffer, int buflen, int *statusp,
		void *extradata )
	{
	    PRHostEnt	prhent;
	    PRNetAddr	iaddr;

	    if ( PR_SetNetAddr(PR_IpAddrNull, PRLDAP_DEFAULT_ADDRESS_FAMILY,
			0, &iaddr) == PR_FAILURE
			|| PR_StringToNetAddr( addr, &iaddr ) == PR_FAILURE ) {
		return( NULL );
	    }

	    if( !statusp || (*statusp = PR_GetHostByAddr(&iaddr, buffer,
		     buflen, &prhent )) == PR_FAILURE ) {
		return( NULL );
	    }

	    return( prldap_convert_hostent( result, &prhent ));
	}


	static int
	prldap_getpeername( LDAP *ld, struct sockaddr *addr, char *buffer, int buflen)
{
    PRLDAPIOSocketArg *sa;
    PRFileDesc	*fd;
    PRNetAddr	iaddr;
    int		ret;

    if (NULL != ld) {
	    ret = prldap_socket_arg_from_ld( ld, &sa );
	    if (ret != LDAP_SUCCESS) {
		return (-1);
	    }
	    ret = PR_GetPeerName(sa->prsock_prfd, &iaddr);
	    if( ret == PR_FAILURE ) {
		return( -1 );
	    }
	    *addr = *((struct sockaddr *)&iaddr.raw);
	    ret = PR_NetAddrToString(&iaddr, buffer, buflen);
	    if( ret == PR_FAILURE ) {
		return( -1 );
	    }
	    return (0);
    }
    return (-1);
}


/*
 * Function: prldap_convert_hostent()
 * Description: copy the fields of a PRHostEnt struct to an LDAPHostEnt
 * Returns: the LDAPHostEnt pointer passed in.
 */
static LDAPHostEnt *
prldap_convert_hostent( LDAPHostEnt *ldhp, PRHostEnt *prhp )
{
	ldhp->ldaphe_name = prhp->h_name;
	ldhp->ldaphe_aliases = prhp->h_aliases;
	ldhp->ldaphe_addrtype = prhp->h_addrtype;
	ldhp->ldaphe_length =  prhp->h_length;
	ldhp->ldaphe_addr_list =  prhp->h_addr_list;
	return( ldhp );
}
