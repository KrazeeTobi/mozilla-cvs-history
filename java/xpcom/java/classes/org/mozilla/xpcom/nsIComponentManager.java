/*
 * ************* DO NOT EDIT THIS FILE ***********
 *
 * This file was automatically generated from nsIComponentManager.idl.
 */


package org.mozilla.xpcom;


/**
 * Interface nsIComponentManager
 *
 * IID: 0x8458a740-d5dc-11d2-92fb-00e09805570f
 */

public interface nsIComponentManager extends nsISupports
{
    public static final String IID =
        "8458a740-d5dc-11d2-92fb-00e09805570f";


    /* nsIFactory findFactory (in nsCIDRef aClass); */
    public void findFactory(IID aClass, nsIFactory[] _retval);

    /* string CLSIDToContractID (in nsCIDRef aClass, out string aClassName); */
    public void cLSIDToContractID(IID aClass, String[] aClassName, String[] _retval);

    /* string registryLocationForSpec (in nsIFile aSpec); */
    public void registryLocationForSpec(nsISupports aSpec, String[] _retval);

    /* nsIFile specForRegistryLocation (in string aLocation); */
    public void specForRegistryLocation(String aLocation, nsISupports[] _retval);

    /* void registerFactory (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFactory aFactory, in boolean aReplace); */
    public void registerFactory(IID aClass, String aClassName, String aContractID, nsIFactory aFactory, boolean aReplace);

    /* void registerComponent (in nsCIDRef aClass, in string aClassName, in string aContractID, in string aLocation, in boolean aReplace, in boolean aPersist); */
    public void registerComponent(IID aClass, String aClassName, String aContractID, String aLocation, boolean aReplace, boolean aPersist);

    /* void registerComponentWithType (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFile aSpec, in string aLocation, in boolean aReplace, in boolean aPersist, in string aType); */
    public void registerComponentWithType(IID aClass, String aClassName, String aContractID, nsISupports aSpec, String aLocation, boolean aReplace, boolean aPersist, String aType);

    /* void registerComponentSpec (in nsCIDRef aClass, in string aClassName, in string aContractID, in nsIFile aLibrary, in boolean aReplace, in boolean aPersist); */
    public void registerComponentSpec(IID aClass, String aClassName, String aContractID, nsISupports aLibrary, boolean aReplace, boolean aPersist);

    /* void registerComponentLib (in nsCIDRef aClass, in string aClassName, in string aContractID, in string aDllName, in boolean aReplace, in boolean aPersist); */
    public void registerComponentLib(IID aClass, String aClassName, String aContractID, String aDllName, boolean aReplace, boolean aPersist);

    /* void unregisterFactory (in nsCIDRef aClass, in nsIFactory aFactory); */
    public void unregisterFactory(IID aClass, nsIFactory aFactory);

    /* void unregisterComponent (in nsCIDRef aClass, in string aLocation); */
    public void unregisterComponent(IID aClass, String aLocation);

    /* void unregisterComponentSpec (in nsCIDRef aClass, in nsIFile aLibrarySpec); */
    public void unregisterComponentSpec(IID aClass, nsISupports aLibrarySpec);

    /* void freeLibraries (); */
    public void freeLibraries();

    /* const long NS_Startup = 0; */
    public static final int NS_Startup = 0;

    /* const long NS_Script = 1; */
    public static final int NS_Script = 1;

    /* const long NS_Timer = 2; */
    public static final int NS_Timer = 2;

    /* const long NS_Shutdown = 3; */
    public static final int NS_Shutdown = 3;

    /* void autoRegister (in long when, in nsIFile directory); */
    public void autoRegister(int when, nsISupports directory);

    /* void autoRegisterComponent (in long when, in nsIFile aFileLocation); */
    public void autoRegisterComponent(int when, nsISupports aFileLocation);

    /* void autoUnregisterComponent (in long when, in nsIFile aFileLocation); */
    public void autoUnregisterComponent(int when, nsISupports aFileLocation);

    /* boolean isRegistered (in nsCIDRef aClass); */
    public void isRegistered(IID aClass, boolean[] _retval);

    /* nsIEnumerator enumerateCLSIDs (); */
    public void enumerateCLSIDs(nsIEnumerator[] _retval);

    /* nsIEnumerator enumerateContractIDs (); */
    public void enumerateContractIDs(nsIEnumerator[] _retval);

}

/*
 * end
 */
