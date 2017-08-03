/*=============================================================================

   Name     : FMEDotNetInterfaces.h

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

#ifndef FMEDOTNETINTERFACES_H
#define FMEDOTNETINTERFACES_H

using namespace System;
using namespace System::Collections::Generic;

namespace Safe { namespace DotNet {

// ==========================================================================
// Defines the public DotNet interfaces.

/// Defines the available MessageLevel types.
public enum class FMEOMessageLevel
{
    Inform = 0,
    Warn = 1,
    Error = 2,
    Fatal = 3,
    Statistic = 4,
    StatusReport = 5,
};

/// Defines the available attribute types.
public enum class AttributeType
{
    Undefined  = 0,
    Boolean    = 1,
    Int8       = 2,
    UInt8      = 3,
    Int16      = 4,
    UInt16     = 5,
    Int32      = 6,
    UInt32     = 7,
    Real32     = 8,
    Real64     = 9,
    Real80     = 10,
    String     = 11,
    EncodedString = 12,
    Int64      = 13,
    UInt64     = 14
};

/// Defines the DotNet IFMEOFeature interface.
public interface class IFMEOFeature
{
    /// Gets the full list of attributes of the feature.
    IList<String^>^ GetAllAttributeNames();

    /// Gets the type of the specified attribute.
    AttributeType GetAttributeType(String^ attributeName);

    /// Gets the value of the specified attribute.
    generic<class T> T GetAttributeValue(String^ attributeName);
    /// Gets the value of the specified attribute as String.
    String^ GetAttributeAsString(String^ attributeName);

    /// Gets the value of the specified attribute or expression.
    generic<class T> T GetOrResolveAttributeValue(String^ value);
    /// Gets the value of the specified attribute or expression as String.
    String^ GetOrResolveAttributeAsString(String^ value);

    /// Sets the value of the specified attribute.
    generic<class T> Boolean SetAttributeValue(String^ attributeName, T value);
    /// Removes the specified attribute.
    void RemoveAttribute(String^ attributeName);

    /// Converts the geometry of the feature to the OGC Well Known Text (ASCII) format.
    String^ ExportGeometryToOGCWKT();
    /// Converts the geometry of the feature to the OGC Well Known Binary format.
    String^ ExportGeometryToOGCWKB();

    /// Sets the geometry of the feature to be that specified in the OGC Well Known Text (ASCII) format.
    bool ImportGeometryFromOGCWKT(String^ wkt);
    /// Sets the geometry of the feature to be that specified in the OGC Well Known Binary format.
    bool ImportGeometryFromOGCWKB(String^ wkb);
};

/// Defines the DotNet IFMEOLogFile interface.
public interface class IFMEOLogFile
{
    /// Write a LOG message.
    void LogMessageString(String^ message, FMEOMessageLevel messageLevel);
};

/// Defines the DotNet IFMEOSession interface.
public interface class IFMEOSession
{
    /// Creates a new empty Feature.
    IFMEOFeature^ CreateFeature(void);
};

/// Defines the DotNet IFMEOFactoryBridge interface.
public interface class IFMEOFactoryBridge
{
    /// List of keywords of the module.
    property IDictionary<String^,String^>^ Keywords
    {
        IDictionary<String^,String^>^ get(void);
    };
    /// List of SourceTags of the module.
    property IList<String^>^ SourceTags
    {
        IList<String^>^ get(void);
    };
    /// List of OutputTags of the module.
    property IList<String^>^ OutputTags
    {
        IList<String^>^ get(void);
    };

    /// Log manager of the module.
    property IFMEOLogFile^ LogFile
    {
        IFMEOLogFile^ get(void);
    };
    /// Fme Session of the module.
    property IFMEOSession^ Session
    {
        IFMEOSession^ get(void);
    };
};

// ==========================================================================
} }

#endif
