/*=============================================================================

   Name     : FMEDotNetWrapperFactory.cpp

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

#include "stdafx.h"
#include "FMEDotNetWrapperFactory.h"

// FME Developer kit includes.
#include <fmetypes.h>
#include <fmemap.h>
#include <fmefeat.h>
#include <fmebridg.h>
#include <ifmeobjserv.h>
#include <ilogfile.h>
#include <isession.h>
#include <servmgr.h>
#include <string>
#include <iostream>
#include <igeometrytools.h>
#include <icrdsysmgr.h>
#include <ipoint.h>

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace Safe::DotNet;

#include <msclr\marshal_cppstd.h>
#include <msclr\auto_handle.h>

#include "FMEDotNetInterfaces.h"
#include "FMEDotNetInterfaces.cpp"

// ==========================================================================

// Message constants
const char* FMEDotNetWrapperFactory::kMessageKeywordSetErr = "There was an error in setting the keywords.";
const char* FMEDotNetWrapperFactory::kMessageInitialized = "Successfully initialized.";
const char* FMEDotNetWrapperFactory::kMessageInvalidSpec = "Invalid specification - DOTNET_ASSEMBLY_FILENAME keyword expected.";
const char* FMEDotNetWrapperFactory::kMessageInvalidValueType = "Invalid value type or attribute type not supported.";
const char* FMEDotNetWrapperFactory::kMessageAborted = "Transformer aborted.";
const char* FMEDotNetWrapperFactory::kMessageClosed = "Transformer closed.";
const char* FMEDotNetWrapperFactory::kMessageInvalidFeatureSpec = "Invalid specification - IFMEOFeature expected.";

// Static FME objects
IFMELogFile* FMEDotNetWrapperFactory::g_logFile = NULL;
IFMEMappingFile* FMEDotNetWrapperFactory::g_mappingFile = NULL;
IFMECoordSysManager* FMEDotNetWrapperFactory::g_coordSysManager = NULL;
IFMEServiceManager* FMEDotNetWrapperFactory::g_serviceManager = NULL;

// Constructor
FMEDotNetWrapperFactory::FMEDotNetWrapperFactory(IFMEFactoryBridge& bridge) :
    _fmeFactoryBridge(bridge),
    _fmeObjectsService(NULL),
    _fmeSession(NULL),
    _fmeGeometryTools(NULL),
    _factoryPtr(NULL)
{
    IDictionary<String^,String^>^ keywords = gcnew Dictionary<String^,String^>(StringComparer::CurrentCultureIgnoreCase);
    _keywords = keywords;

    IList<String^>^ sourceTags = gcnew List<String^>();
    _sourceTags = sourceTags;

    IList<String^>^ outputTags = gcnew List<String^>();
    _outputTags = outputTags;
}

// Destructor
FMEDotNetWrapperFactory::~FMEDotNetWrapperFactory(VOID)
{
    close();
}

// In the initialize method we have to set up the keywords we are interested in AND the output tags we want.
FME_Status FMEDotNetWrapperFactory::initialize()
{
    _fmeObjectsService = (IFMEObjectsService*)g_serviceManager->getService(FME_SERV_FMEOBJECTS);
    _fmeObjectsService->getSession(_fmeSession);
    _fmeGeometryTools = _fmeSession->getGeometryTools();

    // Define the keywords set.
    IFMEStringArray* keywords = _fmeSession->createStringArray();
    keywords->append("DOTNET_ASSEMBLY_FILENAME");
    FME_MsgNum rs = _fmeFactoryBridge.setKeywords(*keywords);
    _fmeSession->destroyStringArray(keywords);
    if (rs != FME_SUCCESS)
    {
        char message[255];
        sprintf_s(message, "%s, errorCode=%li", FMEDotNetWrapperFactory::kMessageKeywordSetErr, rs);
        g_logFile->logMessageString(message, FME_ERROR);
        return FME_FAILURE;
    }
    #if _DEBUG
    g_logFile->logMessageString(FMEDotNetWrapperFactory::kMessageInitialized);
    #endif

    return FME_SUCCESS;
}

// This is called when the keyword is found in the FACTORY_DEF
FME_Status FMEDotNetWrapperFactory::processClause(const IFMEStringArray &factoryClause)
{
    String^ assemblyFileName = nullptr;

    // Load the keywords collection.
    for (FME_UInt32 i = 0, icount = factoryClause.entries(); i < icount; i+=2)
    {
        const char* key = factoryClause(i  );
        const char* val = factoryClause(i+1);
        _keywords->default[gcnew String(key)] = gcnew String(val);
    }

    // Load the source and output tags collection from the related FMX definition file.
    if (_keywords->TryGetValue("DOTNET_ASSEMBLY_FILENAME", assemblyFileName))
    {
        assemblyFileName = calculateFullAssemblyFileName(Path::ChangeExtension(assemblyFileName, ".fmx"));

        if (File::Exists(assemblyFileName))
        {
            IFMEStringArray* sourceTags = NULL;
            IFMEStringArray* outputTags = NULL;

            msclr::auto_handle<System::IO::StreamReader> reader = gcnew System::IO::StreamReader(assemblyFileName);
            String^ fileLine = nullptr;
            int flagCount = 0;

            // Parse the tags from FMX file.
            while ((fileLine = reader->ReadLine())!=nullptr)
            {
                fileLine = fileLine->Trim(' ');
                if (flagCount == 2) break;

                if (fileLine->StartsWith("INPUT_TAGS:"))
                {
                    fileLine = fileLine->Substring(11)->Trim(' ');

                    for each (String^ tagName in fileLine->Split(gcnew array<wchar_t>{L' '}, StringSplitOptions::RemoveEmptyEntries))
                    {
                        if (!String::Equals(tagName, "<BLANK>", StringComparison::CurrentCultureIgnoreCase) && _sourceTags->IndexOf(tagName)==-1)
                        {
                            if (!sourceTags) sourceTags = _fmeSession->createStringArray();
                            std::string tempString = msclr::interop::marshal_as<std::string>(tagName);
                            sourceTags->append(tempString.c_str());
                           _sourceTags->Add(tagName);
                        }
                    }
                    flagCount++;
                }
                else
                if (fileLine->StartsWith("OUTPUT_TAGS:"))
                {
                    fileLine = fileLine->Substring(12)->Trim(' ');

                    for each (String^ tagName in fileLine->Split(gcnew array<wchar_t>{L' '}, StringSplitOptions::RemoveEmptyEntries))
                    {
                        if (!String::Equals(tagName, "<BLANK>", StringComparison::CurrentCultureIgnoreCase) && _outputTags->IndexOf(tagName)==-1)
                        {
                            if (!outputTags) outputTags = _fmeSession->createStringArray();
                            std::string tempString = msclr::interop::marshal_as<std::string>(tagName);
                            outputTags->append(tempString.c_str());
                           _outputTags->Add(tagName);
                        }
                    }
                    flagCount++;
                }
            }
            reader->Close();

            // Save tags definitions.
            if (sourceTags)
            {
                _fmeFactoryBridge.setInputTags(*sourceTags);
                _fmeSession->destroyStringArray(sourceTags);
                sourceTags = NULL;
            }
            if (outputTags)
            {
                _fmeFactoryBridge.setOutputTags(*outputTags);
                _fmeSession->destroyStringArray(outputTags);
                outputTags = NULL;
            }
        }
    }
    return FME_SUCCESS;
}

// Abort current process.
FME_Status FMEDotNetWrapperFactory::abort()
{
    if (_factoryPtr)
    {
        _factoryPtr->Abort();

        #if _DEBUG
        g_logFile->logMessageString(FMEDotNetWrapperFactory::kMessageAborted, FME_WARN);
        #endif
    }
    return FME_SUCCESS;
}

// Close current process.
FME_Status FMEDotNetWrapperFactory::close()
{
    if (_factoryPtr)
    {        
        _factoryPtr->Close();
        _factoryPtr->DisposeObject();
        _factoryPtr = NULL;

        #if _DEBUG
        g_logFile->logMessageString(FMEDotNetWrapperFactory::kMessageClosed, FME_INFORM);
        #endif
    }
    if (_fmeSession)
    {
        _fmeObjectsService->destroySession(_fmeSession);
        _fmeSession = NULL;
    }
    return FME_SUCCESS;
}

// Returns the full assembly name of the specified FileName.
String^ FMEDotNetWrapperFactory::calculateFullAssemblyFileName(String^ assemblyFileName)
{
    String^ transformersDir, ^fullName;
    if (File::Exists( assemblyFileName )) return Path::GetFullPath(assemblyFileName);

    // Using some folders to searh... from 'MYDOCUMENTS_PATH' folder.
    try
    {
        transformersDir = Environment::GetFolderPath( Environment::SpecialFolder::MyDocuments );
        transformersDir = Path::Combine(transformersDir->Trim('\"'), Path::Combine("FME", "Transformers"));
        if (File::Exists( fullName = Path::Combine(transformersDir, assemblyFileName) )) return fullName;
    }
    catch (System::Exception^ error)
    {
        std::string message = msclr::interop::marshal_as<std::string>(error->Message);
        g_logFile->logMessageString(message.c_str(), FME_WARN);
    }

    // ... from 'FMEEngineConfig.txt' settings file of FME Server.
    try
    {
        String^ keyName = "HKEY_LOCAL_MACHINE\\Software\\Safe Software Inc.\\Feature Manipulation Engine";
        String^ binPath = (String^)Microsoft::Win32::Registry::GetValue(keyName, "FME_SERVER_HOME", "NONE");
        
        if (!String::IsNullOrEmpty(binPath) && File::Exists(keyName = Path::Combine(binPath, "fmeEngineConfig.txt")))
        {
            msclr::auto_handle<System::IO::StreamReader> reader = gcnew System::IO::StreamReader(keyName);
            String^ fileLine = nullptr;

            // Parse the tags from settings file.
            while ((fileLine = reader->ReadLine())!=nullptr)
            {
                fileLine = fileLine->Trim(' ');

                if (fileLine->StartsWith("FME_SHARED_RESOURCE_DIR", StringComparison::CurrentCultureIgnoreCase))
                {
                    fileLine = fileLine->Replace("FME_SHARED_RESOURCE_DIR", "");
                    fileLine = fileLine->Trim('\\')->Trim(' ');

                    for each (String^ itemPath in fileLine->Split(gcnew array<wchar_t>{L';'}, StringSplitOptions::RemoveEmptyEntries))
                    {
                        String^ testPath = itemPath->Trim('\"');
                        if (File::Exists( fullName = Path::Combine(testPath, assemblyFileName) )) return fullName;
                        testPath = Path::Combine(testPath, "Transformers");
                        if (File::Exists( fullName = Path::Combine(testPath, assemblyFileName) )) return fullName;
                    }
                    break;
                }
            }
            reader->Close();
        }
    }
    catch (System::Exception^ error)
    {
        std::string message = msclr::interop::marshal_as<std::string>(error->Message);
        g_logFile->logMessageString(message.c_str(), FME_WARN);
    }

    // ... from 'SHARED_RESOURCE_PATH' folder.
    try
    {
        String^ keyName = "HKEY_CURRENT_USER\\Software\\Safe Software Inc.\\Feature Manipulation Engine";
        transformersDir = (String^)Microsoft::Win32::Registry::GetValue(keyName, "Shared Resource Directory", "NONE");

        for each (String^ itemPath in transformersDir->Split(gcnew array<wchar_t>{L';'}, StringSplitOptions::RemoveEmptyEntries))
        {
            String^ testPath = itemPath->Trim('\"');
            if (File::Exists( fullName = Path::Combine(testPath, assemblyFileName) )) return fullName;
            testPath = Path::Combine(testPath, "Transformers");
            if (File::Exists( fullName = Path::Combine(testPath, assemblyFileName) )) return fullName;
        }
    }
    catch (System::Exception^ error)
    {
        std::string message = msclr::interop::marshal_as<std::string>(error->Message);
        g_logFile->logMessageString(message.c_str(), FME_WARN);
    }

    // ... from 'FME_HOME_PATH' folder.
    IFMEString* homeValue = NULL;
    try
    {
        for each (String^ itemPath in gcnew array<String^>{"FME_HOME","FME_SERVER_HOME"})
        {
            std::string itemText = msclr::interop::marshal_as<std::string>(itemPath);

            homeValue = _fmeSession->createString();
            g_mappingFile->getMacroValueExpanded(itemText.c_str(), *homeValue);
            transformersDir = gcnew System::String(homeValue->data());
            _fmeSession->destroyString(homeValue);
            homeValue = NULL;

            if (!String::IsNullOrEmpty(transformersDir))
            {
                transformersDir = Path::Combine(transformersDir->Trim('\"'), "Transformers");
                if (File::Exists( fullName = Path::Combine(transformersDir, assemblyFileName) )) return fullName;
            }
        }
    }
    catch (System::Exception^ error)
    {
        if (homeValue) _fmeSession->destroyString(homeValue);
        std::string message = msclr::interop::marshal_as<std::string>(error->Message);
        g_logFile->logMessageString(message.c_str(), FME_ERROR);
    }
    return assemblyFileName;
}

// Occurs when the resolution of an assembly fails.
static System::Reflection::Assembly^ MyResolveEventHandler(Object^ sender, System::ResolveEventArgs^ args)
{
    for each (System::Reflection::Assembly^ assembly in AppDomain::CurrentDomain->GetAssemblies())
    {
        if (String::Equals(assembly->FullName, args->Name, StringComparison::CurrentCultureIgnoreCase))
        {
            return assembly;
        }
    }
    return nullptr;
}

// Depending on how we have chosen to process features in this factory we could:
//   a) Process and output each feature immediately or
//   b) Process and store features into a list, then when all features
//      have been collected, process and output them.
//   Under a) the bulk of the work occurs in this function since features are output immediately as soon as
//      they are done processing
//   Under b) the bulk of the works occurs in the close() function. Within this function assuming 
//      we have created a feature vector(requires work in initialize() to create an FMEObjects
//      session and the feature vector) that can store features in a list, then all we do in this function
//      is append the feature to the feature vector like this:
//   featureList_->append(feature);
//      And then only in the close() function after we have collected all the features does
//      processing occur. (Processing is exactly the same as what would occur here except it is
//      looped for each feature in the list until no more features remain.)
FME_Status FMEDotNetWrapperFactory::process(IFMEFeature*& feature, const IFMEString& inputTag)
{
    IFMEOFeature^ featureOb = nullptr;

    // --------------------------------------------------------------------------------------------
    // Initialize the DotNet Factory reference.

    if (!_factoryPtr)
    {
        String^ assemblyFileName = nullptr;
        String^ factoryName = nullptr;

        // Search the full assembly path to load (HOME_PATH, SHARED_PATH, MYDOCUMENTS_PATH...).
        if (_keywords->TryGetValue("DOTNET_ASSEMBLY_FILENAME", assemblyFileName))
        {            
            assemblyFileName = calculateFullAssemblyFileName(assemblyFileName);

            // Search and load the DotNet factory to manage.
            if (File::Exists(assemblyFileName)) try
            {
                AppDomain^ appDomain = AppDomain::CurrentDomain;
                
                // For security, append folder as private path.
                #pragma warning (disable : 4947)
			    appDomain->AppendPrivatePath(Path::GetDirectoryName(assemblyFileName));
                #pragma warning (default : 4947)

                appDomain->AssemblyResolve += gcnew ResolveEventHandler( MyResolveEventHandler );
                System::Reflection::Assembly^ assembly = System::Reflection::Assembly::LoadFile(assemblyFileName);

                factoryName = nullptr;
               _keywords->TryGetValue("DOTNET_ASSEMBLY_FACTORY_NAME", factoryName);

                for each (Type^ exportedType in assembly->GetExportedTypes())
                {
                    if (exportedType->IsInterface || exportedType->IsAbstract || exportedType->ContainsGenericParameters)
                        continue;

                    if (String::IsNullOrEmpty(factoryName) || String::Equals(exportedType->Name, factoryName, StringComparison::CurrentCultureIgnoreCase))
                    {
                        FMEDotNetFactory^ factory = (FMEDotNetFactory^)System::Activator::CreateInstance(exportedType);
                        factory->Initialize(gcnew DotNetFMEFactoryBridge(this));
                       _factoryPtr = factory;
                        break;
                    }
                }
            }
            catch (System::Exception^ error)
            {
                std::string message = msclr::interop::marshal_as<std::string>(error->Message);
                g_logFile->logMessageString(message.c_str(), FME_ERROR);
                return FME_FAILURE;
            }
        }
        if (!_factoryPtr) //-> Something wrong!
        {
            g_logFile->logMessageString(FMEDotNetWrapperFactory::kMessageInvalidSpec, FME_ERROR);
            return FME_FAILURE;
        }
    }

    // --------------------------------------------------------------------------------------------
    // Process current feature.

    String^ sourceTag = gcnew String(inputTag.data());

    for each (KeyValuePair<String^,IFMEOFeature^> outputOb in _factoryPtr->ProcessFeature(sourceTag, gcnew DotNetFMEFeature(this, feature)))
    {
        if ((featureOb = outputOb.Value) != nullptr)
        {
            IFMEString* outputTag = NULL;

            // Parameters ok?
            if (dynamic_cast<DotNetFMEFeature^>(featureOb) == nullptr)
            {
                g_logFile->logMessageString(FMEDotNetWrapperFactory::kMessageInvalidFeatureSpec, FME_ERROR);
                return FME_FAILURE;
            }

            IFMEFeature* outputFeature = outputFeature = ((DotNetFMEFeature^)featureOb)->data();
            outputTag = _fmeSession->createString();
            std::string tempString = msclr::interop::marshal_as<std::string>(outputOb.Key);
           *outputTag = tempString.c_str();
            
            // Write feature.
            if (_fmeFactoryBridge.outputFeature(*outputTag, outputFeature) != FME_SUCCESS)
            {
                _fmeSession->destroyString(outputTag);
                return FME_FAILURE;
            }
            _fmeSession->destroyString(outputTag);
        }
    }
    return FME_SUCCESS;
}
