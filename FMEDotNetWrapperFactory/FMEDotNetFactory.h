/*=============================================================================

   Name     : FMEDotNetFactory.h

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

#ifndef FMEDOTNETFACTORY_H
#define FMEDOTNETFACTORY_H

#include "FMEDotNetInterfaces.h"

using namespace System;
using namespace System::Collections::Generic;

namespace Safe { namespace DotNet {

// ==========================================================================

/// Defines a generic FME DotNet custom factory.
public ref class FMEDotNetFactory abstract
{
public:
    FMEDotNetFactory(void) { };
   ~FMEDotNetFactory(void) { };
    
    /// Initialize the object in the current task.
    virtual void Initialize(IFMEOFactoryBridge^ bridge) abstract;

    /// Process the specified Feature.
    virtual IEnumerable<KeyValuePair<String^,IFMEOFeature^>>^ ProcessFeature(String^ inputTag, IFMEOFeature^ feature) abstract;

    /// Abort the current task.
    virtual void Abort() abstract;

    /// Close the current task.
    virtual void Close() abstract;

    /// Releases the resources of the object.
    virtual void DisposeObject() abstract;
};

// ==========================================================================
} }

#endif
