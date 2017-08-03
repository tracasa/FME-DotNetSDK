/*=============================================================================

   Name     : FMEDotNetWrapperFactory.h

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

#pragma once

#ifndef FMEDOTNETWRAPPERFACTORY_H
#define FMEDOTNETWRAPPERFACTORY_H

#include <fmetypes.h>
#include <fmefact.h>
#include <fmefeat.h>
#include <memory>

// Forward declarations
class IFMESession;
class IFMEObjectsService;
class IFMEGeometryTools;
class IFMECoordSysManager;

namespace Safe { namespace DotNet {
ref class DotNetFMEFactoryBridge;
ref class DotNetFMEFeature;
} }

// This constant holds the factory ID assigned by Safe Software for this factory.
// Change to your own ID provided by Safe Software.
const FME_UInt32 kStartEndFactoryId = 14758974;

// Reference to the DotNet factory managed.
#include <gcroot.h>
#include "FMEDotNetFactory.h"

// ==========================================================================

// Defines the DotNet factory wrapper.
class FMEDotNetWrapperFactory : public IFMEDevFactory
{
    // Friend classes to manage private members of this class.
    friend ref class Safe::DotNet::DotNetFMEFactoryBridge;
    friend ref class Safe::DotNet::DotNetFMEFeature;

public:
    FMEDotNetWrapperFactory(IFMEFactoryBridge& bridge);
   ~FMEDotNetWrapperFactory(void);
    
    // Initialize the factory -- set the keywords we will use.
    FME_Status initialize();

    // Extract the values for our keywords.
    FME_Status processClause(const IFMEStringArray &factoryClause);

    // Abort current process.
    FME_Status abort();

    // Close current process.
    FME_Status close();

    // Process a feature.
    FME_Status process(IFMEFeature*& feature, const IFMEString& inputTag);
  
    // Return the unique ID assigned to this factory.
    inline FME_UInt32 id() { return kStartEndFactoryId; }

    // -----------------------------------------------------------------------------
    // Declare static public class members
    // (by convention these are identified by a leading "g_").

    // The LogFile member stores a pointer to an IFMELogFile object
    // that allows the plug-in to log messages to the FME log file.
    // It is initialized externally after the plug-in object is created.
    static IFMELogFile* g_logFile;

    // The MappingFile member stores a pointer to an IFMEMappingFile
    // object that allows the plug-in to access information from the
    // mapping file. It is initialized externally after the plug-in
    // object is created.
    static IFMEMappingFile* g_mappingFile;

    // The CoorSysManager member stores a pointer to an IFMECoorSysManager
    // object that allows the plug-in to retrieve and define
    // coordinate systems. It is initialized externally after the
    // plug-in object is created.
    static IFMECoordSysManager* g_coordSysManager;

    // The ServiceManager member stores a pointer to an
    // IFMEServiceManager object that allows the plug-in to
    // access various services available to plug-ins:
    // - Topology Service
    // - Columnar ASCII Service
    // - Coordinate System Service
    // - Feature Processing Service
    // - Schema Scan Service
    // - TCL Service
    // It is initialized externally after the plug-in
    // object is created.
    static IFMEServiceManager* g_serviceManager;

private:
    // copy constructor
    FMEDotNetWrapperFactory(const FMEDotNetWrapperFactory&);

    // assignment operator.
    FMEDotNetWrapperFactory &operator=(const FMEDotNetWrapperFactory&);

    // -----------------------------------------------------------------------------
    // Declare private data members

    // The bridge object we need to access the factory infrastructure.
    IFMEFactoryBridge& _fmeFactoryBridge;

    // The FMEObjectsService member stores a pointer to an IFMEObjectsService object 
    // that allows the plug-in to access all the services of FME Objects.
    IFMEObjectsService* _fmeObjectsService;

    // The gSession member stores a pointer to an/ IFMESession object
    // which performs the services on the FME Objects.
    IFMESession* _fmeSession;

    // The gGeometryTools member stores a pointer to an IFMEGeometryTools object 
    // which is used to create and manipulate geometries.
    IFMEGeometryTools* _fmeGeometryTools;

    // Informational and error messages.
    static const char* kMessageKeywordSetErr;
    static const char* kMessageInitialized;
    static const char* kMessageInvalidSpec;
    static const char* kMessageInvalidFeatureSpec;
    static const char* kMessageInvalidValueType;
    static const char* kMessageAborted;
    static const char* kMessageClosed;

    // -----------------------------------------------------------------------------
    // Declare DotNet data members

    // Returns the full assembly name of the specified FileName.
    String^ calculateFullAssemblyFileName(String^ fileName);

    // Keywords collection.
    gcroot<IDictionary<String^,String^>^> _keywords;

    // SourceTags collection.
    gcroot<IList<String^>^> _sourceTags;

    // OutputTags collection.
    gcroot<IList<String^>^> _outputTags;

    // DotNet Factory class managed!!!
    gcroot<Safe::DotNet::FMEDotNetFactory^> _factoryPtr;
};

#endif
