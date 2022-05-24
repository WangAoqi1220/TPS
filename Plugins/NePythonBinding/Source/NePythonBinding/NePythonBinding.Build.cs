// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class NePythonBinding : ModuleRules
	{
		private string ThirdPartyPath
		{
			get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
		}
		public NePythonBinding(ReadOnlyTargetRules Target) : base(Target)
		{
			bUsePrecompiled = true;
			if (Target.Platform != UnrealTargetPlatform.Win64
				&& Target.Platform != UnrealTargetPlatform.Win32)
			{
				// Clang编译器在开启了UnityBuild以后，无法正确处理NePyBase::ToCpp和NePyBase::ToPy函数重载，导致编译报错
				bUseUnity = false;
			}

			// 是否监听NotifyUObjectDeleted来切断与Python联系，而不是使用OnPostGarbageCollect
			PublicDefinitions.Add("NEPY_USE_NOTIFY_OBJECT_DELETED=1");

			// 即使开启NEPY_USE_NOTIFY_OBJECT_DELETED=1，也保留旧逻辑继续执行，来做一些校验
			// 后续确定NEPY_USE_NOTIFY_OBJECT_DELETED可行，则把这个宏设为0（或把包裹的代码直接删除）
			PublicDefinitions.Add("NEPY_USE_POST_GARBAGE_COLLECT=0");

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"SlateCore",
					"Slate",
					"UMG",
					"LevelSequence",
					"CinematicCamera",
					"MovieScene",
					"MovieSceneTracks",
					"PhysicsCore",
					"PakFile",
					"SandboxFile",
					"RenderCore",
					"ApplicationCore",
					"AppFramework",
					"RHI",
					"Json",
				}
			);

			if (Target.bBuildEditor) {
				PrivateDependencyModuleNames.AddRange(
					new string[] {
					"UnrealEd",
					"BlueprintGraph",
					}
				);
			}

			PrivateIncludePaths.Add("NePythonBinding/Public/NePy/Auto");
			
			PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "Python27"));
			PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "openssl", "include"));

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "Python27", "Lib", "Win64", "python27.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "openssl", "Lib", "Win64", "libssl.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "openssl", "Lib", "Win64", "libcrypto.lib"));
			}
		}
	}
}

