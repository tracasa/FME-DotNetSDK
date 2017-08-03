/*=============================================================================

   Name     : FMEDotNetEntryPoints.cpp

   System   : FME Plug-in SDK

   Language : C++

   Purpose  : Implementation of Plug-in Library Entry Points

         Copyright (c) 2017, Tracasa - ahuarte@tracasa.es. All rights reserved.

   Redistribution and use of this code in source and binary forms, with 
   or without modification, are permitted provided that the following 
   conditions are met:

   * Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.

   THIS SAMPLE CODE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SAMPLE CODE, EVEN IF 
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/
//
// FMEDotNetEntryPoints : Defines the exported functions for the DLL application.
//

// Include files
#include "stdafx.h"
#include "FMEDotNetWrapperFactory.h"

#include <fmedllexport.h>

// ==========================================================================
// This method returns the version of the FME API that this plug-in
// was compiled with. The value returned here is a constant defined in
// fmetypes.h and should not be changed.
//
// If this plug-in is compiled with a beta build of FME, compatibility
// issues may arise with subsequent builds of FME. To prevent crashes,
// FME will not load a plug-in compiled against a beta if there is a
// mismatch in the FME API version. To resolve this issue, re-compile
// this plug-in with the version of FME that will run this plug-in.
//
// It is recommended that this plug-in is compiled with a release
// version of FME before being distributed for use. As long as this
// plug-in is compiled with a release version of FME, there should
// be no compatibility issues when running this plug-in on any
// subsequent build of FME.
//
// A plug-in compiled with a release version of FME may generate
// a warning when used with subsequent builds of FME, but the plug-in
// will still operate correctly.
//
FME_DLLEXPORT_C const char* FME_apiVersion()
{
    return kFMEDevKitVersion;
}

// --------------------------------------------------------------------------
// After creation, the factory is initialized,
// giving it a pointer to an object which manages access to certain FME services.
//
FME_DLLEXPORT_C FME_Status FME_initialize(IFMEServiceManager &serviceManager)
{
    // Pass the IFMEServiceManager interface onto the factory object.
    FMEDotNetWrapperFactory::g_serviceManager = &serviceManager;

    return FME_SUCCESS;
}

// --------------------------------------------------------------------------
// This creates our factory. 
// Normally, we also keep a pointer to the logfile, coordinate system manager, 
// and mapping file in case we need them later.
//
// These static variables below will be available to the rest of the
// routines for access to the FME infrastructure.
//
FME_DLLEXPORT_C FME_Status FME_createFactory(IFMELogFile& logFile, IFMEMappingFile& mappingFile, IFMECoordSysManager& coordSysManager, IFMEFactoryBridge& bridge, IFMEDevFactory*& factory, const char* factoryName)
{
    // First, keep the references to the FME-owned objects so we can use them later in this module
    FMEDotNetWrapperFactory::g_logFile = &logFile;
    FMEDotNetWrapperFactory::g_mappingFile = &mappingFile;
    FMEDotNetWrapperFactory::g_coordSysManager = &coordSysManager;

    // Now, create an instance of our particular factory object
    factory = new FMEDotNetWrapperFactory(bridge);

    // And that is it.  We could call some additional initialize routine
    // or something like that on the factory here, but usually
    // this kind of initialization is done in the "initialize" routine

    return FME_SUCCESS;
}

// -----------------------------------------------------------------------
// This destroys our factory. 
//
FME_DLLEXPORT_C FME_Status FME_destroyFactory(IFMEDevFactory*& factory)
{
    // There is nothing much to do -- we just delete the factory and it will clean up after itself.
    delete factory;
    factory = NULL;

    return FME_SUCCESS;
}

/* #######################################################################

#include <fmefunc.h>

// Static reference of the current FME session.
static IFMESession* g_fmeSession = NULL; 

// This method gets the reference of the current FME session.
//
FME_DLLEXPORT_C void FME_acceptSession(IFMESession* fmeSession)
{
    g_fmeSession = fmeSession;
}

// -----------------------------------------------------------------------
// This creates our function. 
// Normally, we also keep a pointer to the logfile, coordinate system manager, 
// and mapping file in case we need them later.
//
// These static variables below will be available to the rest of the
// routines for access to the FME infrastructure.
//
FME_DLLEXPORT_C FME_Status FME_createFunction(IFMELogFile& logFile, IFMEMappingFile& mappingFile, IFMECoordSysManager& coordSysManager, IFMEFunction*& function, const char* functionName)
{
    // First, keep the references to the FME-owned objects so we can use them later in this module
    FMEDotNetWrapperFactory::g_logFile = &logFile;
    FMEDotNetWrapperFactory::g_mappingFile = &mappingFile;
    FMEDotNetWrapperFactory::g_coordSysManager = &coordSysManager;

    // Now, create an instance of our particular function object
    if (_strcmpi(functionName, "my_user_function")==0)
    {
    //->function = new CenterOfMass;
        return FME_SUCCESS;
    }
    else
    {
        function = NULL;
        return FME_FAILURE;
    }
}

// -----------------------------------------------------------------------
// This destroys our function. 
//
FME_DLLEXPORT_C FME_Status FME_destroyFunction(IFMEFunction*& function)
{
    // There is nothing much to do -- we just delete the function and it will clean up after itself.
    delete function;
    function = NULL;

    return FME_SUCCESS;
}*/

// -----------------------------------------------------------------------
