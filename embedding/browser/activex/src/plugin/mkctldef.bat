@echo off

REM This script generates the DEF file for the control DLL depending on
REM what has been set to go into it

echo ; npmozax.def : Declares the module parameters.   > %1
echo ; This file was autogenerated by mkctldef.bat!     >> %1
echo.                                                   >> %1
echo LIBRARY      "npmozax.DLL"                        >> %1
echo EXPORTS                                            >> %1
echo     ; Plugin exports                               >> %1
echo     NP_GetEntryPoints   @1                         >> %1
echo     NP_Initialize       @2                         >> %1
echo     NP_Shutdown         @3                         >> %1
echo     ; NSGetFactory		@10                         >> %1
