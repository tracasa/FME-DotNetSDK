# FME-DotNetSDK HelloWorld sample
A very simple HelloWorld Custom Transformer for the [FME-DotNet SDK](../README.md).

This sample provides two components, the .NET assembly, and its FME transformer definition file (.FMX)
to describe it. See [Custom Transformer documentation](https://docs.safe.com/fme/html/FME_Desktop_Documentation/FME_Workbench/Workbench/custom_transformer_creating.htm) to learn about it.

  ![Workbench sample](../Docs/HelloWorld.png)

It is very simple, it only reads all attributes from input features, and then returns them to output.

## How to build

* Build and/or deploy the [SDK](../README.md).
* Open the project file, or solution file in root folder, and click build.

## How to deploy

If you didn't before, copy the **FMEDotNetWrapperFactory.dll** file to your FME plugins folder (e.g. *%PROGRAMFILES%\FME\FME2017\plugins*).

Copy the .NET Assembly and the FMX file to a valid shared *Transformers* folder.

You can define your own *Transformers* folder setting *MENU -> Tools -> FME Options...*

  ![FME options](../Docs/FMEOptions.png)

Then, by example:
* copy .\FMEDotNetHelloWorld.fmx "C:\My Shared Resources Directory\Transformers\"
* copy .\FMEDotNetHelloWorld.dll "C:\My Shared Resources Directory\Transformers\"

or
* copy .\FMEDotNetHelloWorld.fmx "C:\Program Files\FME\FME2017\Transformers\"
* copy .\FMEDotNetHelloWorld.dll "C:\Program Files\FME\FME2017\Transformers\"

If you want to deploy the Custom Transformer to FME Server, you have to do similar steps.

## How to debug

Create a FME Workbench workflow (.FMW file) with the Custom Transformer, and define the DEBUG configuration in Visual Studio:

  ![Debug](../Docs/HelloWorld-howtodebug.png)

Set breakpoints and run it.

