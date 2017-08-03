/*=============================================================================

   Name     : FMEDotNetInterfaces.cpp

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
// FMEDotNetInterfaces : Implements C++ wrapper classes.
//

#pragma once

// Include files.
#include "stdafx.h"
#include "FMEDotNetWrapperFactory.h"

#ifndef FMEDOTNETINTERFACES_CPP
#define FMEDOTNETINTERFACES_CPP

// FME Developer kit includes.
#include <fmetypes.h>
#include <fmemap.h>
#include <fmefeat.h>
#include <fmebridg.h>
#include <ifmeobjserv.h>
#include <ilogfile.h>
#include <isession.h>
#include <string>

using namespace System;
using namespace System::Collections::Generic;

#include <msclr\marshal_cppstd.h>

namespace Safe { namespace DotNet {

// ==========================================================================
// Implements C++ wrapper classes.

/// Defines a helper IFMEString manager class.
class ISafeFMEString
{
public:
    /// Constructor.
    ISafeFMEString(IFMESession* fmeSession)
    {
        _p = (_fmeSession = fmeSession)->createString();
    }
    /// Destructor
   ~ISafeFMEString(void)
    {
        if (_p) { _fmeSession->destroyString(_p); _p = NULL; }
    }
    /// Returns the IFMEString value managed.
    inline operator IFMEString*() const
    {
        return _p;
    }
    inline operator IFMEString&() const
    {
        return *_p;
    }
    /// Returns the String^ value managed.
    inline operator String^() const
    {
        return gcnew String(_p->data());
    }

    /// Convert the specified IFMEStringArray to a managed list of strings.
    inline static IList<String^>^ ToStringList(IFMESession* fmeSession, IFMEStringArray* stringArray, bool destroyStringArray = true)
    {
        IList<String^>^ outputList = gcnew List<String^>();

        for (FME_UInt32 i = 0, icount = stringArray->entries(); i < icount; i++)
        {
            ISafeFMEString safeString = ISafeFMEString(fmeSession);
            stringArray->getElement(i, safeString);
            outputList->Add(safeString);
        }
        if (destroyStringArray) fmeSession->destroyStringArray(stringArray);
        return outputList;
    }
    /// Convert the specified managed list of strings to a IFMEStringArray.
    inline static IFMEStringArray* ToStringArray(IFMESession* fmeSession, IList<String^>^ stringList)
    {
        IFMEStringArray* stringArray = fmeSession->createStringArray();

        for (int i = 0, icount = stringList->Count; i < icount; i++)
        {
            std::string item = msclr::interop::marshal_as<std::string>(stringList->default::get(i));
            stringArray->append(item.c_str());
        }
        return stringArray;
    }

private:
    IFMESession* _fmeSession;
    IFMEString* _p;
};

/// Defines the DotNet IFMEOFeature wrapper.
ref class DotNetFMEFeature : public IFMEOFeature
{
public:
    /// Constructor
    DotNetFMEFeature::DotNetFMEFeature(FMEDotNetWrapperFactory* factoryPtr, IFMEFeature* feature)
    {
        _factoryPtr = factoryPtr;
        _feature = feature;
    }

    /// Gets the full list of attributes of the feature.
    virtual IList<System::String^>^ GetAllAttributeNames()
    {
        IFMEStringArray* stringArray = _factoryPtr->_fmeSession->createStringArray();
       _feature->getAllAttributeNames(*stringArray);
        return ISafeFMEString::ToStringList(_factoryPtr->_fmeSession, stringArray);
    }
    /// Gets the type of the specified attribute.
    virtual AttributeType GetAttributeType(String^ attributeName)
    {
        std::string tempString = msclr::interop::marshal_as<std::string>(attributeName);
        FME_AttributeType type = _feature->getAttributeType(tempString.c_str());
        return (AttributeType)(int)type;
    }
    /// Removes the specified attribute.
    virtual void RemoveAttribute(String^ attributeName)
    {
        std::string tempString = msclr::interop::marshal_as<std::string>(attributeName);
        _feature->removeAttribute(tempString.c_str());
    }

    /// Gets the value of the specified attribute as String.
    virtual String^ GetAttributeAsString(String^ attributeName)
    {
        if (attributeName->Length==0) return String::Empty;
        std::string tempString = msclr::interop::marshal_as<std::string>(attributeName);

        ISafeFMEString safeString = ISafeFMEString(_factoryPtr->_fmeSession);
        if (_feature->getAttribute(tempString.c_str(), safeString) == FME_TRUE) return safeString;
        return nullptr;
    }
    /// Gets the value of the specified attribute.
    generic<class T> T virtual GetAttributeValue(String^ attributeName)
    {
        System::ComponentModel::CancelEventArgs^ res = gcnew System::ComponentModel::CancelEventArgs();
        return GetAttributeValue<T>(attributeName, res);
    }
    /// Gets the value of the specified attribute (Returns if the attribute was fetched).
    generic<class T> T GetAttributeValue(String^ attributeName, System::ComponentModel::CancelEventArgs^ res)
    {
        std::string tempString = msclr::interop::marshal_as<std::string>(attributeName);

        if (T::typeid==String::typeid)
        {
            if (attributeName->Length==0) return (T)String::Empty;
            ISafeFMEString safeString = ISafeFMEString(_factoryPtr->_fmeSession);
            if (_feature->getAttribute(tempString.c_str(), safeString) == FME_TRUE) return (T)(String^)safeString;
            res->Cancel = true;
        }
        else
        if (T::typeid==UInt16::typeid)
        {
            FME_UInt16 value = 0;
            if (_feature->getAttribute(tempString.c_str(), value) == FME_TRUE) return (T)(UInt16)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==Int16::typeid)
        {
            FME_Int16 value = 0;
            if (_feature->getAttribute(tempString.c_str(), value) == FME_TRUE) return (T)(Int16)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==UInt32::typeid)
        {
            FME_UInt32 value = 0;
            if (_feature->getAttribute(tempString.c_str(), value) == FME_TRUE) return (T)(UInt32)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==Int32::typeid)
        {
            FME_Int32 value = 0;
            if (_feature->getAttribute(tempString.c_str(), value) == FME_TRUE) return (T)(Int32)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==UInt64::typeid)
        {
            FME_UInt64 value = 0;
            if (_feature->getUInt64Attribute(tempString.c_str(), value) == FME_TRUE) return (T)(UInt64)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==Int64::typeid)
        {
            FME_Int64 value = 0;
            if (_feature->getInt64Attribute(tempString.c_str(), value) == FME_TRUE) return (T)(UInt64)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==float::typeid)
        {
            FME_Real32 value = 0;
            if (_feature->getAttribute(tempString.c_str(), value) == FME_TRUE) return (T)(float)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==double::typeid)
        {
            FME_Real64 value = 0;
            if (_feature->getAttribute(tempString.c_str(), value) == FME_TRUE) return (T)(double)value;
            res->Cancel = true;
        }
        else
        if (T::typeid==Boolean::typeid)
        {
            FME_Boolean value = FME_FALSE;
            if (_feature->getBooleanAttribute(tempString.c_str(), value) == FME_TRUE) return (T)(Boolean)(value!=FME_FALSE);
            res->Cancel = true;
        }
        else
        if (T::typeid==DateTime::typeid)
        {
            ISafeFMEString safeString = ISafeFMEString(_factoryPtr->_fmeSession);

            if (_feature->getAttribute(tempString.c_str(), safeString) == FME_TRUE)
			{
				DateTime  dateTime = DateTime::MinValue;
				if (DateTime::TryParseExact((String^)safeString, "yyyyMMddHHmmss", s_englishCultureInfo, System::Globalization::DateTimeStyles::None, dateTime))
				return (T)dateTime;
			}
            res->Cancel = true;
        }
        else
        if (T::typeid==IList<String^>::typeid)
        {
            IFMEStringArray* stringArray = _factoryPtr->_fmeSession->createStringArray();
            if (_feature->getListAttribute(tempString.c_str(), *stringArray) == FME_TRUE) return (T)ISafeFMEString::ToStringList(_factoryPtr->_fmeSession, stringArray);
            _factoryPtr->_fmeSession->destroyStringArray(stringArray);
            res->Cancel = true;
        }
        if (res->Cancel) return T();
        throw gcnew InvalidCastException(gcnew String(FMEDotNetWrapperFactory::kMessageInvalidValueType));        
    }

    /// Gets the value of the specified attribute or expression as String.
    virtual String^ GetOrResolveAttributeAsString(String^ value)
    {
        // Resolve an attribute expression?
        if (value->StartsWith("@"))
        {
            return ResolveExpression(value);
        }
        else
        {
            String^ otherValue = GetAttributeAsString(value);
            return otherValue!=nullptr ? otherValue : value;
        }
    }
    /// Gets the value of the specified attribute or expression.
    generic<class T> T virtual GetOrResolveAttributeValue(String^ value)
    {
        // Resolve an attribute expression?
        if (value->StartsWith("@"))
        {
            return FixAttributeValue<T>(ResolveExpression(value));
        }
        else
        {
            System::ComponentModel::CancelEventArgs^ res = gcnew System::ComponentModel::CancelEventArgs();
            T otherValue = GetAttributeValue<T>(value, res);
            if (res->Cancel) otherValue = FixAttributeValue<T>(value);
            return otherValue;
        }
    }

    /// Sets the value of the specified attribute.
    generic<class T> virtual Boolean SetAttributeValue(String^ attributeName, T newValue)
    {
        std::string tempString = msclr::interop::marshal_as<std::string>(attributeName);

        if (T::typeid==String::typeid)
        {
            std::string value = msclr::interop::marshal_as<std::string>((String^)newValue);
            _feature->setAttribute(tempString.c_str(), value.c_str());
            return true;
        }
        else
        if (T::typeid==UInt16::typeid)
        {
            _feature->setAttribute(tempString.c_str(), (FME_UInt16)(UInt16)newValue);
            return true;
        }
        else
        if (T::typeid==Int16::typeid)
        {
            _feature->setAttribute(tempString.c_str(), (FME_Int16)(Int16)newValue);
            return true;
        }
        else
        if (T::typeid==UInt32::typeid)
        {
            _feature->setAttribute(tempString.c_str(), (FME_UInt32)(UInt32)newValue);
            return true;
        }
        else
        if (T::typeid==Int32::typeid)
        {
            _feature->setAttribute(tempString.c_str(), (FME_Int32)(Int32)newValue);
            return true;
        }
        else
        if (T::typeid==UInt64::typeid)
        {
            _feature->setUInt64Attribute(tempString.c_str(), (FME_UInt64)(UInt64)newValue);
            return true;
        }
        else
        if (T::typeid==Int64::typeid)
        {
            _feature->setInt64Attribute(tempString.c_str(), (FME_Int64)(Int64)newValue);
            return true;
        }
        else
        if (T::typeid==float::typeid)
        {
            _feature->setAttribute(tempString.c_str(), (FME_Real32)(float)newValue);
            return true;
        }
        else
        if (T::typeid==double::typeid)
        {
            _feature->setAttribute(tempString.c_str(), (FME_Real64)(double)newValue);
            return true;
        }
        else
        if (T::typeid==Boolean::typeid)
        {            
            _feature->setBooleanAttribute(tempString.c_str(), (FME_Boolean)( ((Boolean)newValue)!=false ? FME_TRUE : FME_FALSE ));
            return true;
        }
        else
        if (T::typeid==DateTime::typeid)
        {
            std::string value = msclr::interop::marshal_as<std::string>( ((DateTime^)newValue)->ToString("yyyyMMddHHmmss") );
            _feature->setAttribute(tempString.c_str(), value.c_str());
            return true;
        }
        else
        if (T::typeid==IList<String^>::typeid)
        {
            IFMEStringArray* stringArray = ISafeFMEString::ToStringArray(_factoryPtr->_fmeSession, (IList<String^>^)newValue);
            _feature->setListAttribute(tempString.c_str(), *stringArray);
            _factoryPtr->_fmeSession->destroyStringArray(stringArray);
            return true;
        }
        return false;
    }

    /// Converts the geometry of the feature to the OGC Well Known Text (ASCII) format.
    virtual String^ ExportGeometryToOGCWKT()
    {
        IFMEString* tempString = _factoryPtr->_fmeSession->createString();
        String^ wkt = nullptr;

        if (_feature->exportGeometryToOGCWKT(*tempString) == FME_SUCCESS) wkt = gcnew String(tempString->data());
        _factoryPtr->_fmeSession->destroyString(tempString);
        return wkt;
    }
    /// Converts the geometry of the feature to the OGC Well Known Binary format.
    virtual String^ ExportGeometryToOGCWKB()
    {
        IFMEString* tempString = _factoryPtr->_fmeSession->createString();
        String^ wkt = nullptr;

        if (_feature->exportGeometryToOGCWKB(*tempString) == FME_SUCCESS) wkt = gcnew String(tempString->data());
        _factoryPtr->_fmeSession->destroyString(tempString);
        return wkt;
    }
    /// Sets the geometry of the feature to be that specified in the OGC Well Known Text (ASCII) format.
    virtual bool ImportGeometryFromOGCWKT(String^ wkt)
    {
        std::string wkt_ = msclr::interop::marshal_as<std::string>(wkt);
        IFMEString* tempString = _factoryPtr->_fmeSession->createString();
        *tempString = wkt_.c_str();

        bool rs = _feature->importGeometryFromOGCWKT(*tempString) == FME_SUCCESS;
        _factoryPtr->_fmeSession->destroyString(tempString);
        return rs;
    }
    /// Sets the geometry of the feature to be that specified in the OGC Well Known Binary format.
    virtual bool ImportGeometryFromOGCWKB(String^ wkb)
    {
        std::string wkt_ = msclr::interop::marshal_as<std::string>(wkb);
        IFMEString* tempString = _factoryPtr->_fmeSession->createString();
        *tempString = wkt_.c_str();

        bool rs = _feature->importGeometryFromOGCWKB(*tempString) == FME_SUCCESS;
        _factoryPtr->_fmeSession->destroyString(tempString);
        return rs;
    }

internal:
    /// Returns the unmanaged pointer of the feature.
    inline IFMEFeature* data() { return _feature; }

private:
    FMEDotNetWrapperFactory* _factoryPtr;
    IFMEFeature* _feature;

    /// Fix the value to the specified type.
    generic<class T> T FixAttributeValue(String^ value)
    {
        if (T::typeid==IList<String^>::typeid)
        {
            List<String^>^ stringList = gcnew List<String^>();
            stringList->AddRange(value->Split(gcnew array<wchar_t>{L',',L';'}, StringSplitOptions::RemoveEmptyEntries));
            return (T)stringList;
        }
        else
        if (T::typeid==String::typeid)
        {
            return (T)value;
        }
        else
        {
            System::ComponentModel::TypeConverter^ converter = System::ComponentModel::TypeDescriptor::GetConverter(T::typeid);
            Object^ theValue = converter->ConvertFromString(value);
            return (T)theValue;
        }
    }

    /// Resolve the specified attribute expression and return its value.
    String^ ResolveExpression(String^ expression)
    {
        String^ value = expression;

        std::string tempString = msclr::interop::marshal_as<std::string>( value );
        IFMEString* v = _factoryPtr->_fmeSession->createString();
        IFMEString* r = _factoryPtr->_fmeSession->createString();

       *v = tempString.c_str();
        v->decodeFromFMEParsableText();

        if (_feature->performFunction(*v, *r)==FME_SUCCESS)
        {
            value = gcnew String(r->data());
        }
        _factoryPtr->_fmeSession->destroyString(v);
        _factoryPtr->_fmeSession->destroyString(r);

        return value;
    }

	/// <summary> English CultureInfo of the framework. </summary>
	static System::Globalization::CultureInfo^ s_englishCultureInfo = gcnew System::Globalization::CultureInfo("en-us");
};

/// Defines the DotNet IFMEOLogFile wrapper.
ref class DotNetFMELogFile : public IFMEOLogFile
{
public:
    /// Constructor
    DotNetFMELogFile::DotNetFMELogFile(IFMELogFile* logFile)
    {
        _logFile = logFile;
    };
    /// Write a LOG message.
    virtual void LogMessageString(String^ message, FMEOMessageLevel messageLevel)
    {
        std::string temp = msclr::interop::marshal_as<std::string>(message);
        _logFile->logMessageString(temp.c_str(), (FME_MsgLevel)(int)messageLevel);
    };

private:
    IFMELogFile* _logFile;
};

/// Defines the DotNet IFMEOSession wrapper.
ref class DotNetFMESession : public IFMEOSession
{
public:
    /// Constructor
    DotNetFMESession::DotNetFMESession(FMEDotNetWrapperFactory* factoryPtr, IFMESession* fmeSession)
    {
        _factoryPtr = factoryPtr;
        _fmeSession = fmeSession;
    };

    /// Creates a new empty Feature.
    virtual IFMEOFeature^ CreateFeature()
    {
        return gcnew DotNetFMEFeature(_factoryPtr, _fmeSession->createFeature());
    }

private:
    FMEDotNetWrapperFactory* _factoryPtr;
    IFMESession* _fmeSession;
};

/// Defines the DotNet IFMEOFactoryBridge wrapper.
ref class DotNetFMEFactoryBridge : public IFMEOFactoryBridge
{
public:
    /// Constructor
    DotNetFMEFactoryBridge(FMEDotNetWrapperFactory* factoryPtr)
    {
        _factoryPtr = factoryPtr;
    };

    /// List of keywords of the module.
    virtual property IDictionary<System::String^,System::String^>^ Keywords
    {
        IDictionary<System::String^,System::String^>^ get(void)
        {
            return _factoryPtr->_keywords;
        };
    };
    /// List of SourceTags of the module.
    virtual property IList<System::String^>^ SourceTags
    {
        IList<System::String^>^ get(void)
        {
            return _factoryPtr->_sourceTags;
        };
    };
    /// List of OutputTags of the module.
    virtual property IList<System::String^>^ OutputTags
    {
        IList<System::String^>^ get(void)
        {
            return _factoryPtr->_outputTags;
        };
    };

    /// Log manager of the module.
    virtual property IFMEOLogFile^ LogFile
    {
        IFMEOLogFile^ get(void)
        {
            return gcnew DotNetFMELogFile(FMEDotNetWrapperFactory::g_logFile);
        };
    };
    /// Fme session of the module.
    virtual property IFMEOSession^ Session
    {
        IFMEOSession^ get(void)
        {
            return gcnew DotNetFMESession(_factoryPtr, _factoryPtr->_fmeSession);
        };
    };

private:
    FMEDotNetWrapperFactory* _factoryPtr;
};

// ==========================================================================
} }

#endif
