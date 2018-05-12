// Fill out your copyright notice in the Description page of Project Settings.

#include "ShaderToyCustomization.h"
#include <PropertyHandle.h>
#include "ShaderToy.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <SMultiLineEditableTextBox.h>

#define LOCTEXT_NAMESPACE "UnrealShadertoy"

TSharedPtr<IPropertyHandle> GetPropertyHandleByName(TSharedRef<class IPropertyHandle> StructPropertyHandle, FString ChildPropertyName)
{
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		if (ChildHandle->GetProperty()->GetName() == ChildPropertyName)
		{
			return ChildHandle;
		}
	}
	ensure(false);
	return nullptr;
}

TSharedRef<IDetailCustomization> ShaderToyCustomization::MakeInstance()
{
	return MakeShareable(new ShaderToyCustomization());
}

void ShaderToyCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

}

TSharedRef<IPropertyTypeCustomization> ShaderToyHLSLFunctionCustomization::MakeInstance()
{
	return MakeShareable(new ShaderToyHLSLFunctionCustomization());
}

void ShaderToyHLSLFunctionCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> FunctionName_PropertyHandle = GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FShaderToyHLSLFunction, FunctionName));

	HeaderRow.NameContent()
	[
		StructPropertyHandle->GetPropertyDisplayName().EqualTo(FText::FromString(TEXT("MainFunction"))) ? StructPropertyHandle->CreatePropertyNameWidget() : FunctionName_PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
		.HAlign(HAlign_Fill)
	[
		FunctionName_PropertyHandle->CreatePropertyValueWidget()
	];
}

void ShaderToyHLSLFunctionCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> FunctionCodes_PropertyHandle = GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FShaderToyHLSLFunction, FunctionCodes));

	FString Value;
	FunctionCodes_PropertyHandle->GetValue(Value);

	TSharedRef<SMultiLineEditableTextBox> MultiLineEditableTextBox = SNew(SMultiLineEditableTextBox)
		.SelectAllTextWhenFocused(false)
		.ClearKeyboardFocusOnCommit(false)
		.SelectAllTextOnCommit(false)
		.AutoWrapText(false)
		.ModiferKeyForNewLine(EModifierKey::None)
		.Text(FText::FromString(Value))
		.OnTextCommitted_Lambda([=](const FText& NewText, ETextCommit::Type CommitInfo)
		{
			FunctionCodes_PropertyHandle->SetValue(NewText.ToString());
		});

	//关闭Tab导航
	MultiLineEditableTextBox->SetOnKeyDownHandler(FOnKeyDown::CreateLambda([=](const FGeometry& Geometry, const FKeyEvent& KeyEvent)
	{
		if (KeyEvent.GetKey() == EKeys::Tab)
		{
			MultiLineEditableTextBox->InsertTextAtCursor(TEXT("\t"));
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}));

	//简单括号补全
	MultiLineEditableTextBox->SetOnKeyCharHandler(FOnKeyChar::CreateLambda([=](const FGeometry& Geometry, const FCharacterEvent& CharacterEvent)
	{
		if (CharacterEvent.GetCharacter() == TEXT('('))
		{
			MultiLineEditableTextBox->InsertTextAtCursor(TEXT("()"));
			return FReply::Handled();
		}
		if (CharacterEvent.GetCharacter() == TEXT('{'))
		{
			MultiLineEditableTextBox->InsertTextAtCursor(TEXT("{}"));
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}));

	StructBuilder.AddCustomRow(LOCTEXT("FunctionCodes", "FunctionCodes")).NameContent()
	[
		FunctionCodes_PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
		.HAlign(HAlign_Fill)
	[
		MultiLineEditableTextBox
	];
}

#undef LOCTEXT_NAMESPACE
