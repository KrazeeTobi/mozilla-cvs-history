
#ifndef nsSoftwareUpdate_h___
#define nsSoftwareUpdate_h___

#include "nsSoftwareUpdateIIDs.h"
#include "nsISoftwareUpdate.h"

#include "nscore.h"
#include "nsIFactory.h"
#include "nsISupports.h"

#include "nsIScriptExternalNameSet.h"

class nsSoftwareUpdate: public nsISoftwareUpdate
{
    public:
        static const nsIID& IID() { static nsIID iid = NS_SoftwareUpdateInstall_CID; return iid; }

        nsSoftwareUpdate();
        ~nsSoftwareUpdate();
        
        NS_DECL_ISUPPORTS

            NS_IMETHOD Startup();
            NS_IMETHOD Shutdown();
    
    private:
        nsresult DeleteScheduledNodes();
};


class nsSoftwareUpdateNameSet : public nsIScriptExternalNameSet 
{
    public:
        nsSoftwareUpdateNameSet();
        ~nsSoftwareUpdateNameSet();

        NS_DECL_ISUPPORTS
            NS_IMETHOD InitializeClasses(nsIScriptContext* aScriptContext);
            NS_IMETHOD AddNameSet(nsIScriptContext* aScriptContext);
};


#define AUTOUPDATE_ENABLE_PREF     "autoupdate.enabled"
#define AUTOUPDATE_CONFIRM_PREF    "autoupdate.confirm_install"
#define CHARSET_HEADER             "Charset"
#define CONTENT_ENCODING_HEADER    "Content-encoding"
#define INSTALLER_HEADER           "Install-Script"
#define MOCHA_CONTEXT_PREFIX       "autoinstall:"
#define REG_SOFTUPDT_DIR           "Netscape/Communicator/SoftwareUpdate/"
#define LAST_REGPACK_TIME          "LastRegPackTime"

/* error codes */
#define ErrInvalidArgs          -1
#define ErrUnknownInstaller     -2
#define ErrInternalError        -3
#define ErrBadScript            -4
#define JarError                -5
#define DiskSpaceError          -6

#endif