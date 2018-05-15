// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "UnrealShadertoy.h"
#include <PropertyEditorModule.h>
#include "ShaderToyCustomization.h"

#define LOCTEXT_NAMESPACE "FUnrealShadertoyModule"

void FUnrealShadertoyModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("CodeableString", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCodeableStringCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("ShaderToy", FOnGetDetailCustomizationInstance::CreateStatic(&FShaderToyCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("ShaderToyHLSLFunction", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FShaderToyHLSLFunctionCustomization::MakeInstance));
}

void FUnrealShadertoyModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealShadertoyModule, UnrealShadertoy)