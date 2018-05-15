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

TSharedRef<SMultiLineEditableTextBox> CreateCodeEditorTextBox(const TSharedPtr<IPropertyHandle>& PropertyHandle)
{
	FString Value;
	PropertyHandle->GetValue(Value);

	TSharedRef<SMultiLineEditableTextBox> MultiLineEditableTextBox = SNew(SMultiLineEditableTextBox)
		.SelectAllTextWhenFocused(false)
		.ClearKeyboardFocusOnCommit(false)
		.SelectAllTextOnCommit(false)
		.AutoWrapText(false)
		.ModiferKeyForNewLine(EModifierKey::None)
		.Text(FText::FromString(Value))
		.OnTextCommitted_Lambda([=](const FText& NewText, ETextCommit::Type CommitInfo)
	{
		PropertyHandle->SetValue(NewText.ToString());
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

	//简单括号补全->感觉不好用，先去掉
// 	MultiLineEditableTextBox->SetOnKeyCharHandler(FOnKeyChar::CreateLambda([=](const FGeometry& Geometry, const FCharacterEvent& CharacterEvent)
// 	{
// 		if (CharacterEvent.GetCharacter() == TEXT('('))
// 		{
// 			MultiLineEditableTextBox->InsertTextAtCursor(TEXT("()"));
// 			return FReply::Handled();
// 		}
// 		if (CharacterEvent.GetCharacter() == TEXT('{'))
// 		{
// 			MultiLineEditableTextBox->InsertTextAtCursor(TEXT("{}"));
// 			return FReply::Handled();
// 		}
// 		return FReply::Unhandled();
// 	}));

	return MultiLineEditableTextBox;
}

TSharedRef<IPropertyTypeCustomization> FCodeableStringCustomization::MakeInstance()
{
	return MakeShareable(new FCodeableStringCustomization());
}

void FCodeableStringCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> FunctionCodes_PropertyHandle = GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FCodeableString, Code));

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		.HAlign(HAlign_Fill)
		[
			CreateCodeEditorTextBox(FunctionCodes_PropertyHandle)
		];
}

TSharedRef<IDetailCustomization> FShaderToyCustomization::MakeInstance()
{
	return MakeShareable(new FShaderToyCustomization());
}

void FShaderToyCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

}

TSharedRef<IPropertyTypeCustomization> FShaderToyHLSLFunctionCustomization::MakeInstance()
{
	return MakeShareable(new FShaderToyHLSLFunctionCustomization());
}

void FShaderToyHLSLFunctionCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
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

void FShaderToyHLSLFunctionCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		if (ChildHandle->GetProperty()->GetName() != GET_MEMBER_NAME_STRING_CHECKED(FShaderToyHLSLFunction, FunctionName))
		{
			StructBuilder.AddProperty(ChildHandle);
		}
	}
}

#undef LOCTEXT_NAMESPACE
