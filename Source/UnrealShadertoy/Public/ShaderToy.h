// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MaterialExpressionIO.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionCustom.h"
#include "ShaderToy.generated.h"

USTRUCT()
struct FCodeableString
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "CodeableString")
	FString Code;

	FCodeableString() = default;
	FCodeableString(const FString& Code)
		:Code(Code)
	{}
	template <
		typename CharType,
		typename = typename TEnableIf<TIsCharType<CharType>::Value>::Type
	>
	FORCEINLINE FCodeableString(const CharType* Src)
		:Code(Src)
	{}

	operator FString() const { return Code; }
};

USTRUCT()
struct FShaderToyHLSLFunction
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "FuncDecs"))
	FString FunctionName;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "FuncBody"))
	FCodeableString FunctionCodes;
};

/**
 * 
 */
UCLASS(collapsecategories, hidecategories = Object, MinimalAPI)
class UShaderToy : public UMaterialExpression
{	
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "NodeName"))
	FString NodeTitle;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "DefineBody"))
	FCodeableString DefineBody;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "Inputs"))
	TArray<struct FCustomInput> Inputs;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "MainFunctionOutput"))
	TEnumAsByte<enum ECustomMaterialOutputType> OutputType;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "MainFunction"))
	FShaderToyHLSLFunction MainFunction;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionCustom, meta = (DisplayName = "ChildFunctions"))
	TArray<FShaderToyHLSLFunction> HLSLFunctions;

	//~ Begin UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface.

	//~ Begin UMaterialExpression Interface
#if WITH_EDITOR
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	virtual const TArray<FExpressionInput*> GetInputs() override;
	virtual FExpressionInput* GetInput(int32 InputIndex) override;
	virtual FName GetInputName(int32 InputIndex) const override;
#if WITH_EDITOR
	virtual uint32 GetInputType(int32 InputIndex) override { return MCT_Unknown; }
	virtual uint32 GetOutputType(int32 OutputIndex) override;
#endif // WITH_EDITOR
	//~ End UMaterialExpression Interface
};
