// Fill out your copyright notice in the Description page of Project Settings.
#include "ShaderToy.h"

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "Misc/Guid.h"
#include "UObject/RenderingObjectVersion.h"
#include "Misc/App.h"
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectAnnotation.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineGlobals.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "MaterialShared.h"
#include "MaterialExpressionIO.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionMaterialAttributeLayers.h"
#include "Materials/MaterialFunctionInterface.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialFunctionMaterialLayer.h"
#include "Materials/MaterialFunctionMaterialLayerBlend.h"
#include "Materials/MaterialFunctionInstance.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Styling/CoreStyle.h"

#include "EditorSupportDelegates.h"
#include "MaterialCompiler.h"
#if WITH_EDITOR
#include "MaterialGraph/MaterialGraphNode_Comment.h"
#include "MaterialGraph/MaterialGraphNode.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif //WITH_EDITOR
#include "Materials/MaterialInstanceConstant.h"
#include <RHIDefinitions.h>

#define LOCTEXT_NAMESPACE "UnrealShaderToy"

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionCustom
///////////////////////////////////////////////////////////////////////////////
UShaderToy::UShaderToy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		FText NAME_Custom;
		FConstructorStatics()
			: NAME_Custom(LOCTEXT("UnrealShaderToy", "UnrealShaderToy"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	MainFunction.FunctionName = TEXT("main");
	MainFunction.FunctionCodes = TEXT("return float3(1,1,1);");

	NodeTitle = TEXT("UnrealShaderToy");

#if WITH_EDITORONLY_DATA
	MenuCategories.Add(ConstructorStatics.NAME_Custom);
#endif

	OutputType = CMOT_Float3;

	Inputs.Add(FCustomInput());
	Inputs[0].InputName = TEXT("");

	bCollapsed = false;
}

#if WITH_EDITOR

//在这里查看私有变量的位移
//#include <Runtime/Engine/Private/Materials/HLSLMaterialTranslator.h>
//constexpr int offset = offsetof(FHLSLMaterialTranslator, MaterialCompilationOutput);
//constexpr int offset = offsetof(FUniformExpressionSet, Uniform2DTextureExpressions);

struct FHLSLMaterialTranslatorReader
{
	template<typename T>
	static T& GetCompilerMemberByOffset(const class FMaterialCompiler* Compiler, int32 Offset)
	{
		return reinterpret_cast<T&>(*((char*)Compiler + Offset));
	}

	static EShaderFrequency& ShaderFrequency(const class FMaterialCompiler* Compiler)
	{
		return GetCompilerMemberByOffset<EShaderFrequency>(Compiler, 8);
	}

	static TArray<FString>& CustomExpressionImplementations(const class FMaterialCompiler* Compiler)
	{
		return GetCompilerMemberByOffset<TArray<FString>>(Compiler, 1448);
	}

 	static FMaterialCompilationOutput& MaterialCompilationOutput(const class FMaterialCompiler* Compiler)
 	{
		//获取这个地址偏移的办法只能Debug看Compiler的地址和这边的地址的相对位置，然后做运算
 		return GetCompilerMemberByOffset<FMaterialCompilationOutput>(Compiler, 192 - 520);
 	}

	//FUniformExpressionSet
	template<typename T>
	static T& GetExpressionSetMemberByOffset(FUniformExpressionSet& UniformExpressionSet, int32 Offset)
	{
		return reinterpret_cast<T&>(*((char*)&UniformExpressionSet + Offset));
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& PerFramePrevUniformScalarExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 128);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& PerFrameUniformScalarExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 96);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& UniformScalarExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 32);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& PerFramePrevUniformVectorExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 144);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& PerFrameUniformVectorExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 112);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& UniformVectorExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 16);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& Uniform2DTextureExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 48);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& UniformCubeTextureExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 64);
	}

	static TArray<TRefCountPtr<FMaterialUniformExpression>>& UniformExternalTextureExpressions(FUniformExpressionSet& UniformExpressionSet)
	{
		return GetExpressionSetMemberByOffset<TArray<TRefCountPtr<FMaterialUniformExpression>>>(UniformExpressionSet, 80);
	}
	//

	struct FShaderCodeChunk
	{
		/**
		* Definition string of the code chunk.
		* If !bInline && !UniformExpression || UniformExpression->IsConstant(), this is the definition of a local variable named by SymbolName.
		* Otherwise if bInline || (UniformExpression && UniformExpression->IsConstant()), this is a code expression that needs to be inlined.
		*/
		FString Definition;
		/**
		* Name of the local variable used to reference this code chunk.
		* If bInline || UniformExpression, there will be no symbol name and Definition should be used directly instead.
		*/
		FString SymbolName;
		/** Reference to a uniform expression, if this code chunk has one. */
		TRefCountPtr<FMaterialUniformExpression> UniformExpression;
		EMaterialValueType Type;
		/** Whether the code chunk should be inlined or not.  If true, SymbolName is empty and Definition contains the code to inline. */
		bool bInline;

		/** Ctor for creating a new code chunk with no associated uniform expression. */
		FShaderCodeChunk(const TCHAR* InDefinition, const FString& InSymbolName, EMaterialValueType InType, bool bInInline) :
			Definition(InDefinition),
			SymbolName(InSymbolName),
			UniformExpression(NULL),
			Type(InType),
			bInline(bInInline)
		{}

		/** Ctor for creating a new code chunk with a uniform expression. */
		FShaderCodeChunk(FMaterialUniformExpression* InUniformExpression, const TCHAR* InDefinition, EMaterialValueType InType) :
			Definition(InDefinition),
			UniformExpression(InUniformExpression),
			Type(InType),
			bInline(false)
		{}
	};

	static TArray<FShaderCodeChunk>*& CurrentScopeChunks(const class FMaterialCompiler* Compiler)
	{
		return GetCompilerMemberByOffset<TArray<FShaderCodeChunk>*&>(Compiler, 48);
	}

	static int32& NextSymbolIndex(const class FMaterialCompiler* Compiler)
	{
		return GetCompilerMemberByOffset<int32>(Compiler, 1440);
	}

	static int32 AccessUniformExpression(class FMaterialCompiler* Compiler, int32 Index)
	{
		//没法拿到位域的偏移
		bool bCompilingPreviousFrame = true;

		check(Index >= 0 && Index < CurrentScopeChunks(Compiler)->Num());
		const FShaderCodeChunk&	CodeChunk = (*CurrentScopeChunks(Compiler))[Index];
		check(CodeChunk.UniformExpression && !CodeChunk.UniformExpression->IsConstant());

		FMaterialUniformExpressionTexture* TextureUniformExpression = CodeChunk.UniformExpression->GetTextureUniformExpression();
		FMaterialUniformExpressionExternalTexture* ExternalTextureUniformExpression = CodeChunk.UniformExpression->GetExternalTextureUniformExpression();

		// Any code chunk can have a texture uniform expression (eg FMaterialUniformExpressionFlipBookTextureParameter),
		// But a texture code chunk must have a texture uniform expression
		check(!(CodeChunk.Type & MCT_Texture) || TextureUniformExpression || ExternalTextureUniformExpression);
		// External texture samples must have a corresponding uniform expression
		check(!(CodeChunk.Type & MCT_TextureExternal) || ExternalTextureUniformExpression);

		TCHAR FormattedCode[MAX_SPRINTF] = TEXT("");
		if (CodeChunk.Type == MCT_Float)
		{
			if (CodeChunk.UniformExpression->IsChangingPerFrame())
			{
				if (bCompilingPreviousFrame)
				{
					const int32 ScalarInputIndex = PerFramePrevUniformScalarExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(CodeChunk.UniformExpression);
					FCString::Sprintf(FormattedCode, TEXT("UE_Material_PerFramePrevScalarExpression%u"), ScalarInputIndex);
				}
				else
				{
					const int32 ScalarInputIndex = PerFrameUniformScalarExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(CodeChunk.UniformExpression);
					FCString::Sprintf(FormattedCode, TEXT("UE_Material_PerFrameScalarExpression%u"), ScalarInputIndex);
				}
			}
			else
			{
				const static TCHAR IndexToMask[] = { 'x', 'y', 'z', 'w' };
				const int32 ScalarInputIndex = UniformScalarExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(CodeChunk.UniformExpression);
				// Update the above FMemory::Malloc if this FCString::Sprintf grows in size, e.g. %s, ...
				FCString::Sprintf(FormattedCode, TEXT("Material.ScalarExpressions[%u].%c"), ScalarInputIndex / 4, IndexToMask[ScalarInputIndex % 4]);
			}
		}
		else if (CodeChunk.Type & MCT_Float)
		{
			const TCHAR* Mask;
			switch (CodeChunk.Type)
			{
			case MCT_Float:
			case MCT_Float1: Mask = TEXT(".r"); break;
			case MCT_Float2: Mask = TEXT(".rg"); break;
			case MCT_Float3: Mask = TEXT(".rgb"); break;
			default: Mask = TEXT(""); break;
			};

			if (CodeChunk.UniformExpression->IsChangingPerFrame())
			{
				if (bCompilingPreviousFrame)
				{
					const int32 VectorInputIndex = PerFramePrevUniformVectorExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(CodeChunk.UniformExpression);
					FCString::Sprintf(FormattedCode, TEXT("UE_Material_PerFramePrevVectorExpression%u%s"), VectorInputIndex, Mask);
				}
				else
				{
					const int32 VectorInputIndex = PerFrameUniformVectorExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(CodeChunk.UniformExpression);
					FCString::Sprintf(FormattedCode, TEXT("UE_Material_PerFrameVectorExpression%u%s"), VectorInputIndex, Mask);
				}
			}
			else
			{
				const int32 VectorInputIndex = UniformVectorExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(CodeChunk.UniformExpression);
				FCString::Sprintf(FormattedCode, TEXT("Material.VectorExpressions[%u]%s"), VectorInputIndex, Mask);
			}
		}
		else if (CodeChunk.Type & MCT_Texture)
		{
			check(!CodeChunk.UniformExpression->IsChangingPerFrame());
			int32 TextureInputIndex = INDEX_NONE;
			const TCHAR* BaseName = TEXT("");
			switch (CodeChunk.Type)
			{
			case MCT_Texture2D:
			{
				TextureInputIndex = Uniform2DTextureExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(TextureUniformExpression);
				BaseName = TEXT("Texture2D");
				break;
			}
			case MCT_TextureCube:
				TextureInputIndex = UniformCubeTextureExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(TextureUniformExpression);
				BaseName = TEXT("TextureCube");
				break;
			case MCT_TextureExternal:
				TextureInputIndex = UniformExternalTextureExpressions(MaterialCompilationOutput(Compiler).UniformExpressionSet).AddUnique(ExternalTextureUniformExpression);
				BaseName = TEXT("ExternalTexture");
				break;
			default: UE_LOG(LogTemp, Fatal, TEXT("Unrecognized texture material value type: %u"), (int32)CodeChunk.Type);
			};
			FCString::Sprintf(FormattedCode, TEXT("Material.%s_%u"), BaseName, TextureInputIndex);
		}
		else
		{
			//UE_LOG(LogTemp, Fatal, TEXT("User input of unknown type: %s"), DescribeType(CodeChunk.Type));
		}

		return AddInlinedCodeChunk(Compiler, (*CurrentScopeChunks(Compiler))[Index].Type, FormattedCode);
	}

	static int32 AddInlinedCodeChunk(class FMaterialCompiler* Compiler, EMaterialValueType Type, const TCHAR* Format, ...)
	{
		int32	BufferSize = 256;
		TCHAR*	FormattedCode = NULL;
		int32	Result = -1;

		while (Result == -1)
		{
			FormattedCode = (TCHAR*)FMemory::Realloc(FormattedCode, BufferSize * sizeof(TCHAR));
			GET_VARARGS_RESULT(FormattedCode, BufferSize, BufferSize - 1, Format, Format, Result);
			BufferSize *= 2;
		};
		FormattedCode[Result] = 0;
		const int32 CodeIndex = AddCodeChunkInner(Compiler, FormattedCode, Type, true);
		FMemory::Free(FormattedCode);

		return CodeIndex;
	}

	static FString GetParameterCode(class FMaterialCompiler* Compiler, int32 Index, const TCHAR* Default = 0)
	{
		if (Index == INDEX_NONE && Default)
		{
			return Default;
		}

		//checkf(Index >= 0 && Index < CurrentScopeChunks(Compiler)->Num(), TEXT("Index %d/%d, Platform=%d"), Index, CurrentScopeChunks(Compiler)->Num(), (int)Platform);
		const FShaderCodeChunk& CodeChunk = (*CurrentScopeChunks(Compiler))[Index];
		if ((CodeChunk.UniformExpression && CodeChunk.UniformExpression->IsConstant()) || CodeChunk.bInline)
		{
			// Constant uniform expressions and code chunks which are marked to be inlined are accessed via Definition
			return CodeChunk.Definition;
		}
		else
		{
			if (CodeChunk.UniformExpression)
			{
				// If the code chunk has a uniform expression, create a new code chunk to access it
				const int32 AccessedIndex = AccessUniformExpression(Compiler, Index);
				const FShaderCodeChunk& AccessedCodeChunk = (*CurrentScopeChunks(Compiler))[AccessedIndex];
				if (AccessedCodeChunk.bInline)
				{
					// Handle the accessed code chunk being inlined
					return AccessedCodeChunk.Definition;
				}
				// Return the symbol used to reference this code chunk
				check(AccessedCodeChunk.SymbolName.Len() > 0);
				return AccessedCodeChunk.SymbolName;
			}

			// Return the symbol used to reference this code chunk
			check(CodeChunk.SymbolName.Len() > 0);
			return CodeChunk.SymbolName;
		}
	}

	static int32 AddCodeChunk(class FMaterialCompiler* Compiler, EMaterialValueType Type, const TCHAR* Format, ...)
	{
		int32	BufferSize = 256;
		TCHAR*	FormattedCode = NULL;
		int32	Result = -1;

		while (Result == -1)
		{
			FormattedCode = (TCHAR*)FMemory::Realloc(FormattedCode, BufferSize * sizeof(TCHAR));
			GET_VARARGS_RESULT(FormattedCode, BufferSize, BufferSize - 1, Format, Format, Result);
			BufferSize *= 2;
		};
		FormattedCode[Result] = 0;

		const int32 CodeIndex = AddCodeChunkInner(Compiler, FormattedCode, Type, false);
		FMemory::Free(FormattedCode);

		return CodeIndex;
	}

	static FString CreateSymbolName(class FMaterialCompiler* Compiler, const TCHAR* SymbolNameHint)
	{
		NextSymbolIndex(Compiler)++;
		return FString(SymbolNameHint) + FString::FromInt(NextSymbolIndex(Compiler));
	}

	static const TCHAR* HLSLTypeString(EMaterialValueType Type)
	{
		switch (Type)
		{
		case MCT_Float1:		return TEXT("MaterialFloat");
		case MCT_Float2:		return TEXT("MaterialFloat2");
		case MCT_Float3:		return TEXT("MaterialFloat3");
		case MCT_Float4:		return TEXT("MaterialFloat4");
		case MCT_Float:			return TEXT("MaterialFloat");
		case MCT_Texture2D:		return TEXT("texture2D");
		case MCT_TextureCube:	return TEXT("textureCube");
		case MCT_StaticBool:	return TEXT("static bool");
		case MCT_MaterialAttributes:	return TEXT("MaterialAttributes");
		default:				return TEXT("unknown");
		};
	}

	static int32 AddCodeChunkInner(class FMaterialCompiler* Compiler, const TCHAR* FormattedCode, EMaterialValueType Type, bool bInlined)
	{
		if (Type == MCT_Unknown)
		{
			return INDEX_NONE;
		}

		if (bInlined)
		{
			const int32 CodeIndex = CurrentScopeChunks(Compiler)->Num();
			// Adding an inline code chunk, the definition will be the code to inline
			new(*CurrentScopeChunks(Compiler)) FShaderCodeChunk(FormattedCode, TEXT(""), Type, true);
			return CodeIndex;
		}
		// Can only create temporaries for float and material attribute types.
		else if (Type & (MCT_Float))
		{
			const int32 CodeIndex = CurrentScopeChunks(Compiler)->Num();
			// Allocate a local variable name
			const FString SymbolName = CreateSymbolName(Compiler, TEXT("Local"));
			// Construct the definition string which stores the result in a temporary and adds a newline for readability
			const FString LocalVariableDefinition = FString("	") + HLSLTypeString(Type) + TEXT(" ") + SymbolName + TEXT(" = ") + FormattedCode + TEXT(";") + LINE_TERMINATOR;
			// Adding a code chunk that creates a local variable
			new(*CurrentScopeChunks(Compiler)) FShaderCodeChunk(*LocalVariableDefinition, SymbolName, Type, false);
			return CodeIndex;
		}
		else
		{
			if (Type == MCT_MaterialAttributes)
			{
				return Compiler->Errorf(TEXT("Operation not supported on Material Attributes"));
			}

			if (Type & MCT_Texture)
			{
				return Compiler->Errorf(TEXT("Operation not supported on a Texture"));
			}

			if (Type == MCT_StaticBool)
			{
				return Compiler->Errorf(TEXT("Operation not supported on a Static Bool"));
			}

			return INDEX_NONE;
		}
	}
};

int32 ShaderToy(class FMaterialCompiler* Compiler, class UShaderToy* Custom, TArray<int32>& CompiledInputs, FString DefineBody)
{
	int32 ResultIdx = INDEX_NONE;

	FString OutputTypeString;
	EMaterialValueType OutputType;
	switch (Custom->OutputType)
	{
		case CMOT_Float2:
			OutputType = MCT_Float2;
			OutputTypeString = TEXT("MaterialFloat2");
			break;
		case CMOT_Float3:
			OutputType = MCT_Float3;
			OutputTypeString = TEXT("MaterialFloat3");
			break;
		case CMOT_Float4:
			OutputType = MCT_Float4;
			OutputTypeString = TEXT("MaterialFloat4");
			break;
		default:
			OutputType = MCT_Float;
			OutputTypeString = TEXT("MaterialFloat");
		break;
	}
 
 	// Declare implementation function
 	FString InputParamDecl;
 	check(Custom->Inputs.Num() == CompiledInputs.Num());
 	for (int32 i = 0; i < Custom->Inputs.Num(); i++)
 	{
 		// skip over unnamed inputs
 		if (Custom->Inputs[i].InputName.IsNone())
 		{
 			continue;
 		}
 		InputParamDecl += TEXT(",");
 		const FString InputNameStr = Custom->Inputs[i].InputName.ToString();
 		switch (Compiler->GetParameterType(CompiledInputs[i]))
 		{
 		case MCT_Float:
 		case MCT_Float1:
 			InputParamDecl += TEXT("MaterialFloat ");
 			InputParamDecl += InputNameStr;
 			break;
 		case MCT_Float2:
 			InputParamDecl += TEXT("MaterialFloat2 ");
 			InputParamDecl += InputNameStr;
 			break;
 		case MCT_Float3:
 			InputParamDecl += TEXT("MaterialFloat3 ");
 			InputParamDecl += InputNameStr;
 			break;
 		case MCT_Float4:
 			InputParamDecl += TEXT("MaterialFloat4 ");
 			InputParamDecl += InputNameStr;
 			break;
 		case MCT_Texture2D:
 			InputParamDecl += TEXT("Texture2D ");
 			InputParamDecl += InputNameStr;
 			InputParamDecl += TEXT(", SamplerState ");
 			InputParamDecl += InputNameStr;
 			InputParamDecl += TEXT("Sampler ");
 			break;
 		case MCT_TextureCube:
 			InputParamDecl += TEXT("TextureCube ");
 			InputParamDecl += InputNameStr;
 			InputParamDecl += TEXT(", SamplerState ");
 			InputParamDecl += InputNameStr;
 			InputParamDecl += TEXT("Sampler ");
 			break;
 		default:
 			//return Errorf(TEXT("Bad type %s for %s input %s"), DescribeType(GetParameterType(CompiledInputs[i])), *Custom->Description, *InputNameStr);
 			break;
 		}
 	}

	TArray<FString>& CustomExpressionImplementations = FHLSLMaterialTranslatorReader::CustomExpressionImplementations(Compiler);
 	int32 CustomExpressionIndex = CustomExpressionImplementations.Num();
 
	//宏声明
	FString DefineDesc = DefineBody + TEXT("\r\n");
	CustomExpressionImplementations.Add(DefineDesc);

 	//------------------------先把需要调用的函数加上去-----------------------------------------
 	for (int32 i = 0; i < Custom->HLSLFunctions.Num(); i++)
 	{
 		FString Code = Custom->HLSLFunctions[i].FunctionCodes;
 		FString FunctionName = Custom->HLSLFunctions[i].FunctionName;
 		if (!Code.Contains(TEXT("return")))
 		{
 			Code = FString(TEXT("return ")) + Code + TEXT(";");
 		}
 		Code.ReplaceInline(TEXT("\n"), TEXT("\r\n"), ESearchCase::CaseSensitive);
		
		EShaderFrequency& ShaderFrequency = FHLSLMaterialTranslatorReader::ShaderFrequency(Compiler);
 		FString ParametersType = ShaderFrequency == SF_Vertex ? TEXT("Vertex") : (ShaderFrequency == SF_Domain ? TEXT("Tessellation") : TEXT("Pixel"));
 		FString ImplementationCode = FString::Printf(TEXT("%s\r\n{\r\n%s\r\n}\r\n"), *FunctionName, *Code);
 		CustomExpressionImplementations.Add(ImplementationCode);
 	}
 	FString Code = Custom->MainFunction.FunctionCodes;
 	if (!Code.Contains(TEXT("return")))
 	{
 		Code = FString(TEXT("return ")) + Code + TEXT(";");
 	}
 	Code.ReplaceInline(TEXT("\n"), TEXT("\r\n"), ESearchCase::CaseSensitive);

	EShaderFrequency& ShaderFrequency = FHLSLMaterialTranslatorReader::ShaderFrequency(Compiler);
 	FString ParametersType = ShaderFrequency == SF_Vertex ? TEXT("Vertex") : (ShaderFrequency == SF_Domain ? TEXT("Tessellation") : TEXT("Pixel"));
 	FString ImplementationCode = FString::Printf(TEXT("%s CustomExpression%d(FMaterial%sParameters Parameters%s)\r\n{\r\n%s\r\n}\r\n"), *OutputTypeString, CustomExpressionIndex, *ParametersType, *InputParamDecl, *Code);
 	CustomExpressionImplementations.Add(ImplementationCode);
 	//-------------------------------------------------------------------------------------------
 
 	// Add call to implementation function
  	FString CodeChunk = FString::Printf(TEXT("CustomExpression%d(Parameters"), CustomExpressionIndex);
  	for (int32 i = 0; i < CompiledInputs.Num(); i++)
  	{
  		// skip over unnamed inputs
  		if (Custom->Inputs[i].InputName.IsNone())
  		{
  			continue;
  		}
  
  		FString ParamCode = FHLSLMaterialTranslatorReader::GetParameterCode(Compiler, CompiledInputs[i]);
  		EMaterialValueType ParamType = Compiler->GetParameterType(CompiledInputs[i]);
  
  		CodeChunk += TEXT(",");
  		CodeChunk += *ParamCode;
  		if (ParamType == MCT_Texture2D || ParamType == MCT_TextureCube)
  		{
  			CodeChunk += TEXT(",");
  			CodeChunk += *ParamCode;
  			CodeChunk += TEXT("Sampler");
  		}
  	}
  	CodeChunk += TEXT(")");
  
  	ResultIdx = FHLSLMaterialTranslatorReader::AddCodeChunk(Compiler,
  		OutputType,
  		*CodeChunk
  	);
  	return ResultIdx;
}


int32 UShaderToy::Compile(class FMaterialCompiler* Compiler, int32 OutputIndex)
{
	TArray<int32> CompiledInputs;

	for (int32 i = 0; i < Inputs.Num(); i++)
	{
		// skip over unnamed inputs
		if (Inputs[i].InputName.IsNone())
		{
			CompiledInputs.Add(INDEX_NONE);
		}
		else
		{
			if (!Inputs[i].Input.GetTracedInput().Expression)
			{
				return Compiler->Errorf(TEXT("Custom material %s missing input %d (%s)"), *NodeTitle, i + 1, *Inputs[i].InputName.ToString());
			}
			int32 InputCode = Inputs[i].Input.Compile(Compiler);
			if (InputCode < 0)
			{
				return InputCode;
			}
			CompiledInputs.Add(InputCode);
		}
	}

	return ShaderToy(Compiler, this, CompiledInputs, DefineBody);
}

void UShaderToy::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(NodeTitle);
}
#endif // WITH_EDITOR

const TArray<FExpressionInput*> UShaderToy::GetInputs()
{
	TArray<FExpressionInput*> Result;
	for (int32 i = 0; i < Inputs.Num(); i++)
	{
		Result.Add(&Inputs[i].Input);
	}
	return Result;
}

FExpressionInput* UShaderToy::GetInput(int32 InputIndex)
{
	if (InputIndex < Inputs.Num())
	{
		return &Inputs[InputIndex].Input;
	}
	return NULL;
}

FName UShaderToy::GetInputName(int32 InputIndex) const
{
	if (InputIndex < Inputs.Num())
	{
		return Inputs[InputIndex].InputName;
	}
	return NAME_None;
}

#if WITH_EDITOR
void UShaderToy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// strip any spaces from input name
	UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	if (PropertyThatChanged && PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(FCustomInput, InputName))
	{
		for (FCustomInput& Input : Inputs)
		{
			FString InputName = Input.InputName.ToString();
			if (InputName.ReplaceInline(TEXT(" "), TEXT("")) > 0)
			{
				Input.InputName = *InputName;
			}
		}
	}

	if (PropertyChangedEvent.MemberProperty && GraphNode)
	{
		const FName PropertyName = PropertyChangedEvent.MemberProperty->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UShaderToy, Inputs))
		{
			GraphNode->ReconstructNode();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

uint32 UShaderToy::GetOutputType(int32 OutputIndex)
{
	switch (OutputType)
	{
	case CMOT_Float1:
		return MCT_Float;
	case CMOT_Float2:
		return MCT_Float2;
	case CMOT_Float3:
		return MCT_Float3;
	case CMOT_Float4:
		return MCT_Float4;
	default:
		return MCT_Unknown;
	}
}
#endif // WITH_EDITOR

void UShaderToy::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FRenderingObjectVersion::GUID);

	// Make a copy of the current code before we change it
	const FString PreFixUp = MainFunction.FunctionCodes;

	bool bDidUpdate = false;

	if (Ar.UE4Ver() < VER_UE4_INSTANCED_STEREO_UNIFORM_UPDATE)
	{
		// Look for WorldPosition rename
		if (MainFunction.FunctionCodes.Code.ReplaceInline(TEXT("Parameters.WorldPosition"), TEXT("Parameters.AbsoluteWorldPosition"), ESearchCase::CaseSensitive) > 0)
		{
			bDidUpdate = true;
		}
	}
	// Fix up uniform references that were moved from View to Frame as part of the instanced stereo implementation
	else if (Ar.UE4Ver() < VER_UE4_INSTANCED_STEREO_UNIFORM_REFACTOR)
	{
		// Uniform members that were moved from View to Frame
		static const FString UniformMembers[] = {
			FString(TEXT("FieldOfViewWideAngles")),
			FString(TEXT("PrevFieldOfViewWideAngles")),
			FString(TEXT("ViewRectMin")),
			FString(TEXT("ViewSizeAndInvSize")),
			FString(TEXT("BufferSizeAndInvSize")),
			FString(TEXT("ExposureScale")),
			FString(TEXT("DiffuseOverrideParameter")),
			FString(TEXT("SpecularOverrideParameter")),
			FString(TEXT("NormalOverrideParameter")),
			FString(TEXT("RoughnessOverrideParameter")),
			FString(TEXT("PrevFrameGameTime")),
			FString(TEXT("PrevFrameRealTime")),
			FString(TEXT("OutOfBoundsMask")),
			FString(TEXT("WorldCameraMovementSinceLastFrame")),
			FString(TEXT("CullingSign")),
			FString(TEXT("NearPlane")),
			FString(TEXT("AdaptiveTessellationFactor")),
			FString(TEXT("GameTime")),
			FString(TEXT("RealTime")),
			FString(TEXT("Random")),
			FString(TEXT("FrameNumber")),
			FString(TEXT("CameraCut")),
			FString(TEXT("UseLightmaps")),
			FString(TEXT("UnlitViewmodeMask")),
			FString(TEXT("DirectionalLightColor")),
			FString(TEXT("DirectionalLightDirection")),
			FString(TEXT("DirectionalLightShadowTransition")),
			FString(TEXT("DirectionalLightShadowSize")),
			FString(TEXT("DirectionalLightScreenToShadow")),
			FString(TEXT("DirectionalLightShadowDistances")),
			FString(TEXT("UpperSkyColor")),
			FString(TEXT("LowerSkyColor")),
			FString(TEXT("TranslucencyLightingVolumeMin")),
			FString(TEXT("TranslucencyLightingVolumeInvSize")),
			FString(TEXT("TemporalAAParams")),
			FString(TEXT("CircleDOFParams")),
			FString(TEXT("DepthOfFieldFocalDistance")),
			FString(TEXT("DepthOfFieldScale")),
			FString(TEXT("DepthOfFieldFocalLength")),
			FString(TEXT("DepthOfFieldFocalRegion")),
			FString(TEXT("DepthOfFieldNearTransitionRegion")),
			FString(TEXT("DepthOfFieldFarTransitionRegion")),
			FString(TEXT("MotionBlurNormalizedToPixel")),
			FString(TEXT("GeneralPurposeTweak")),
			FString(TEXT("DemosaicVposOffset")),
			FString(TEXT("IndirectLightingColorScale")),
			FString(TEXT("HDR32bppEncodingMode")),
			FString(TEXT("AtmosphericFogSunDirection")),
			FString(TEXT("AtmosphericFogSunPower")),
			FString(TEXT("AtmosphericFogPower")),
			FString(TEXT("AtmosphericFogDensityScale")),
			FString(TEXT("AtmosphericFogDensityOffset")),
			FString(TEXT("AtmosphericFogGroundOffset")),
			FString(TEXT("AtmosphericFogDistanceScale")),
			FString(TEXT("AtmosphericFogAltitudeScale")),
			FString(TEXT("AtmosphericFogHeightScaleRayleigh")),
			FString(TEXT("AtmosphericFogStartDistance")),
			FString(TEXT("AtmosphericFogDistanceOffset")),
			FString(TEXT("AtmosphericFogSunDiscScale")),
			FString(TEXT("AtmosphericFogRenderMask")),
			FString(TEXT("AtmosphericFogInscatterAltitudeSampleNum")),
			FString(TEXT("AtmosphericFogSunColor")),
			FString(TEXT("AmbientCubemapTint")),
			FString(TEXT("AmbientCubemapIntensity")),
			FString(TEXT("RenderTargetSize")),
			FString(TEXT("SkyLightParameters")),
			FString(TEXT("SceneFString(TEXTureMinMax")),
			FString(TEXT("SkyLightColor")),
			FString(TEXT("SkyIrradianceEnvironmentMap")),
			FString(TEXT("MobilePreviewMode")),
			FString(TEXT("HMDEyePaddingOffset")),
			FString(TEXT("DirectionalLightShadowFString(TEXTure")),
			FString(TEXT("SamplerState")),
		};

		const FString ViewUniformName(TEXT("View."));
		const FString FrameUniformName(TEXT("Frame."));
		for (const FString& Member : UniformMembers)
		{
			const FString SearchString = FrameUniformName + Member;
			const FString ReplaceString = ViewUniformName + Member;
			if (MainFunction.FunctionCodes.Code.ReplaceInline(*SearchString, *ReplaceString, ESearchCase::CaseSensitive) > 0)
			{
				bDidUpdate = true;
			}
		}
	}

	if (Ar.CustomVer(FRenderingObjectVersion::GUID) < FRenderingObjectVersion::RemovedRenderTargetSize)
	{
		if (MainFunction.FunctionCodes.Code.ReplaceInline(TEXT("View.RenderTargetSize"), TEXT("View.BufferSizeAndInvSize.xy"), ESearchCase::CaseSensitive) > 0)
		{
			bDidUpdate = true;
		}
	}

	// If we made changes, copy the original into the description just in case
	if (bDidUpdate)
	{
		Desc += TEXT("\n*** Original source before expression upgrade ***\n");
		Desc += PreFixUp;
		UE_LOG(LogTemp, Log, TEXT("Uniform references updated for Mycustom material expression %s."), *NodeTitle);
	}
}

#undef LOCTEXT_NAMESPACE

