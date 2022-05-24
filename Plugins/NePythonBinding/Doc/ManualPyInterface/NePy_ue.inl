#include "NePyBase.h"
#include "NePyGIL.h"
#include "NePyObjectBase.h"
#include "NePyAutoExport.h"
#include "RHI.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "openssl/aes.h"
#if WITH_EDITOR
#include "PackageTools.h"
#include "Editor.h"
#include "ReflectionExport/NePyExport.h"
#include "ObjectTools.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Runtime/AssetRegistry/Public/AssetData.h"
#include "Runtime/Core/Public/Containers/BinaryHeap.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#endif
#include "ShaderCodeLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"

#include "UnrealEngine.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"

#include "HAL/PlatformMisc.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/ExceptionHandling.h"

#include "Runtime/Launch/Resources/Version.h"
#include "Runtime/Launch/Public/LaunchEngineLoop.h"

#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectIterator.h"

#include "Misc/FileHelper.h"

#include "Logging/LogMacros.h"

#include "IPlatformFilePak.h"
#include "IPlatformFileSandboxWrapper.h"

#include "Runtime/CoreUObject/Public/UObject/SoftObjectPath.h"
#include "Engine/Classes/Engine/StreamableManager.h"
#include "Engine/Classes/Engine/AssetManager.h"
#include "Engine/Selection.h"

#include "NePyHouseKeeper.h"

PyObject* NePyMethod_Log(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyMessage;
	if (!PyArg_ParseTuple(InArgs, "O:Log", &PyMessage))
	{
		return nullptr;
	}

	const char* Message;
	if (!NePyBase::ToCpp(PyMessage, Message))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'Message' must have type 'str'");
		return nullptr;
	}

	UE_LOG(LogNePython, Log, TEXT("%s"), UTF8_TO_TCHAR(Message));
	Py_RETURN_NONE;
}

PyObject* NePyMethod_LogWarning(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyMessage;
	if (!PyArg_ParseTuple(InArgs, "O:LogWarning", &PyMessage))
	{
		return nullptr;
	}

	const char* Message;
	if (!NePyBase::ToCpp(PyMessage, Message))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'Message' must have type 'str'");
		return nullptr;
	}

	UE_LOG(LogNePython, Warning, TEXT("%s"), UTF8_TO_TCHAR(Message));
	Py_RETURN_NONE;
}

PyObject* NePyMethod_LogError(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyMessage;
	if (!PyArg_ParseTuple(InArgs, "O:LogError", &PyMessage))
	{
		return nullptr;
	}

	const char* Message;
	if (!NePyBase::ToCpp(PyMessage, Message))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'Message' must have type 'str'");
		return nullptr;
	}

	UE_LOG(LogNePython, Error, TEXT("%s"), UTF8_TO_TCHAR(Message));
	Py_RETURN_NONE;
}

PyObject* NePyMethod_AddOnScreenDebugMessage(PyObject* InSelf, PyObject* InArgs)
{
	if (!GEngine)
	{
		Py_RETURN_NONE;
	}

	int Key;
	float TimeToDisplay;
	PyObject* PyMessage;
	if (!PyArg_ParseTuple(InArgs, "ifO:AddOnScreenDebugMessage", &Key, &TimeToDisplay, &PyMessage))
	{
		return nullptr;
	}

	const char* Message;
	if (!NePyBase::ToCpp(PyMessage, Message))
	{
		PyErr_SetString(PyExc_TypeError, "arg3 'Message' must have type 'str'");
		return nullptr;
	}

	GEngine->AddOnScreenDebugMessage(Key, TimeToDisplay, FColor::Green, FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(Message)));
	Py_RETURN_NONE;
}

PyObject* NePyMethod_PrintString(PyObject* InSelf, PyObject* InArgs)
{
#if WITH_NEPY_AUTO_EXPORT
	if (!GEngine)
	{
		Py_RETURN_NONE;
	}

	PyObject* PyMessage;
	float TimeToDisplay = 2.0;
	PyObject* PyColor = nullptr;
	if (!PyArg_ParseTuple(InArgs, "O|fO:PrintString", &PyMessage, &TimeToDisplay, &PyColor))
	{
		return nullptr;
	}

	const char* Message;
	if (!NePyBase::ToCpp(PyMessage, Message))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'Message' must have type 'str'");
		return nullptr;
	}

	FColor Color = FColor::Cyan;
	if (PyColor)
	{
		if (!NePyBase::ToCpp(PyColor, Color))
		{
			return PyErr_Format(PyExc_Exception, "argument is not a FColor");
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, Color, FString(UTF8_TO_TCHAR(Message)));
#endif

	Py_RETURN_NONE;
}

PyObject* NePyMethod_RequestExit(PyObject* InSelf, PyObject* InArgs)
{
	bool bForce;
	if (!PyArg_ParseTuple(InArgs, "b|:RequestExit", &bForce))
	{
		return nullptr;
	}

#if PLATFORM_IOS || PLATFORM_ANDROID
	if (bForce)
	{
		_exit(0);
	}
#endif

	FPlatformMisc::RequestExit(bForce);
	Py_RETURN_NONE;
}

PyObject* NePyMethod_GetEngineDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::EngineDir()));
}

PyObject* NePyMethod_GetEngineContentDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::EngineContentDir()));
}

PyObject* NePyMethod_GetEngineConfigDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::EngineConfigDir()));
}

PyObject* NePyMethod_GetProjectDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::ProjectDir()));
}

PyObject* NePyMethod_GetContentDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::ProjectContentDir()));
}

/*
PyObject* NePyMethod_GetDocumentDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::ProjectDocumentsDir()));
}
*/

PyObject* NePyMethod_GetConfigDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::ProjectConfigDir()));
}

PyObject* NePyMethod_GetLogDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::ProjectLogDir()));
}

/*
PyObject* NePyMethod_GetLogFilename(PyObject* InSelf)
{
	FString LogFile = GetLogFullPath();
	return PyUnicode_FromString(TCHAR_TO_UTF8(*LogFile));
}
*/

PyObject* NePyMethod_GetGameSavedDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::ProjectSavedDir()));
}

PyObject* NePyMethod_GetGameUserDeveloperDir(PyObject* InSelf)
{
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::GameUserDeveloperDir()));
}

PyObject* NePyMethod_ConvertRelativePathToFull(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	if (!PyArg_ParseTuple(InArgs, "s:ConvertRelativePathToFull", &Path))
	{
		return nullptr;
	}
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(UTF8_TO_TCHAR(Path))));
}

PyObject* NePyMethod_ConvertAbsolutePathApp(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	if (!PyArg_ParseTuple(InArgs, "s:ConvertAbsolutePathApp", &Path))
	{
		return nullptr;
	}

#if PLATFORM_ANDROID && (ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 26)
	FString RelPath = UTF8_TO_TCHAR(Path);
	if (IFileManager::Get().IsSandboxEnabled())
	{
		FSandboxPlatformFile* SandboxFileMgr = (FSandboxPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("SandboxFile")));
		check(SandboxFileMgr);
		RelPath = SandboxFileMgr->ConvertToSandboxPath(*RelPath);
	}
	extern FString AndroidRelativeToAbsolutePath(bool bUseInternalBasePath, FString RelPath);
	FString AbsPath = AndroidRelativeToAbsolutePath(false, RelPath);
#else
	FString AbsPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(UTF8_TO_TCHAR(Path));
#endif
	return PyUnicode_FromString(TCHAR_TO_UTF8(*AbsPath));
}

PyObject* NePyMethod_ObjectPathToPackageName(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	if (!PyArg_ParseTuple(InArgs, "s:ObjectPathToPackageName", &Path))
	{
		return nullptr;
	}

	const FString PackageName = FPackageName::ObjectPathToPackageName((const FString)(UTF8_TO_TCHAR(Path)));
	return PyUnicode_FromString(TCHAR_TO_UTF8(*PackageName));
}

PyObject* NePyMethod_GetPath(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	if (!PyArg_ParseTuple(InArgs, "s:GetPath", &Path))
	{
		return nullptr;
	}
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::GetPath(UTF8_TO_TCHAR(Path))));
}

PyObject* NePyMethod_GetBaseFilename(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	if (!PyArg_ParseTuple(InArgs, "s:GetBaseFilename", &Path))
	{
		return nullptr;
	}
	return PyUnicode_FromString(TCHAR_TO_UTF8(*FPaths::GetBaseFilename(UTF8_TO_TCHAR(Path))));
}

PyObject* NePyMethod_FindFile(PyObject* InSelf, PyObject* InArgs)
{
	const char* FullName;
	const char* PathStr;
	if (!PyArg_ParseTuple(InArgs, "ss", &FullName, &PathStr))
	{
		return nullptr;
	}
	FString Path = UTF8_TO_TCHAR(PathStr);
	Path = Path / UTF8_TO_TCHAR(FullName);
	bool exist = FPaths::FileExists(Path);
	return Py_BuildValue("b", exist);
}

PyObject* NePyMethod_GetFile(PyObject* InSelf, PyObject* InArgs)
{
	const char* FullName;
	const char* PathStr;
	if (!PyArg_ParseTuple(InArgs, "ss", &FullName, &PathStr))
	{
		return nullptr;
	}

	FString Path = UTF8_TO_TCHAR(PathStr);
	Path = Path / UTF8_TO_TCHAR(FullName);
	TArray<uint8> ByteBuffer;
	if (!FFileHelper::LoadFileToArray(ByteBuffer, *Path))
	{
		return PyErr_Format(PyExc_Exception, "LoadFileToArray failed!");
	}
	return PyBytes_FromStringAndSize((const char*)ByteBuffer.GetData(), ByteBuffer.Num());
}

// 真名为DecryptBuffer
// 改名为RenderImgs主要是为了混淆视听
PyObject* NePyMethod_RenderImgs(PyObject* InSelf, PyObject* InArgs)
{
	typedef unsigned char BYTE;
#ifdef UE_BUILD_SHIPPING
	//VM_TIGER_RED_START // Themida shell
#endif
	PyObject* BytesObj;
	if (!PyArg_ParseTuple(InArgs, "O", &BytesObj))
	{
		return nullptr;
	}

	Py_ssize_t BufLen = PyBytes_Size(BytesObj);
	if (BufLen % 16 != 4)
	{
		PyErr_SetString(PyExc_NameError, "buffer length is error");
		return nullptr;
	}

	// WARNING! 各项目组应该将此处修改为独一无二的密钥
	char KeyStr[16];
	KeyStr[0] = 'w';
	KeyStr[1] = '5';
	KeyStr[2] = 'q';
	KeyStr[3] = '6';
	KeyStr[4] = '^';
	KeyStr[5] = 'C';
	KeyStr[6] = '0';
	KeyStr[7] = '4';
	KeyStr[8] = 'S';
	KeyStr[9] = 'W';
	KeyStr[10] = '!';
	KeyStr[11] = '@';
	KeyStr[12] = 'e';
	KeyStr[13] = '}';
	KeyStr[14] = 'a';
	KeyStr[15] = 'd';

	BYTE* Buf = (BYTE*)PyBytes_AS_STRING(BytesObj);
	size_t OriginLen = *(size_t*)Buf;
	Buf += 4;
	BufLen -= 4;

	size_t BlockSize = BufLen / 16;
	BYTE* OutBuf = new BYTE[BufLen];
	AES_KEY Key;
	AES_set_decrypt_key((BYTE*)KeyStr, 128, &Key);
	memset(KeyStr, 0, sizeof(KeyStr));
	for (size_t b = 0; b < BlockSize; ++b)
	{
		AES_decrypt(Buf + b * 16, OutBuf + b * 16, &Key);
	}
#if PY_MAJOR_VERSION >= 3
	PyObject* PyBuf = Py_BuildValue("y#", OutBuf, OriginLen);
#else
	PyObject* PyBuf = Py_BuildValue("s#", OutBuf, OriginLen);
#endif
	delete[] OutBuf;

#ifdef UE_BUILD_SHIPPING
	//VM_TIGER_RED_END
#endif
	return PyBuf;
}

#if WITH_EDITOR
PyObject* NePyMethod_EncryptBuffer(PyObject* InSelf, PyObject* InArgs)
{
	typedef unsigned char BYTE;
#ifdef UE_BUILD_SHIPPING
	//VM_TIGER_RED_START // Themida shell
#endif
	PyObject* BytesObj;
	if (!PyArg_ParseTuple(InArgs, "O", &BytesObj))
	{
		return nullptr;
	}

	Py_ssize_t BufLen = PyBytes_Size(BytesObj);
	BYTE* Buf = (BYTE*)PyBytes_AS_STRING(BytesObj);

	// WARNING! 各项目组应该将此处修改为独一无二的密钥
	char KeyStr[16];
	KeyStr[0] = 'w';
	KeyStr[1] = '5';
	KeyStr[2] = 'q';
	KeyStr[3] = '6';
	KeyStr[4] = '^';
	KeyStr[5] = 'C';
	KeyStr[6] = '0';
	KeyStr[7] = '4';
	KeyStr[8] = 'S';
	KeyStr[9] = 'W';
	KeyStr[10] = '!';
	KeyStr[11] = '@';
	KeyStr[12] = 'e';
	KeyStr[13] = '}';
	KeyStr[14] = 'a';
	KeyStr[15] = 'd';

	size_t BlockSize = (BufLen - 1) / 16 + 1;
	BYTE* OutBuf = new BYTE[BlockSize * 16 + 4];
	*(size_t*)OutBuf = BufLen;
	BYTE* p = OutBuf + 4;
	AES_KEY Key;
	AES_set_encrypt_key((BYTE*)KeyStr, 128, &Key);
	memset(KeyStr, 0, sizeof(KeyStr));
	for (size_t b = 0; b < BlockSize; ++b)
	{
		bool last_block = (b == BlockSize - 1);
		BYTE bb[16];
		if (last_block)
		{
			memset(bb, 0, 16);
			memcpy(bb, Buf + b * 16, BufLen - b * 16);
		}
		AES_encrypt((last_block ? bb : Buf + b * 16), p + b * 16, &Key);
	}

#if PY_MAJOR_VERSION >= 3
	PyObject* PyBuf = Py_BuildValue("y#", OutBuf, BlockSize * 16 + 4);
#else
	PyObject* PyBuf = Py_BuildValue("s#", OutBuf, BlockSize * 16 + 4);
#endif
	delete[] OutBuf;

#ifdef UE_BUILD_SHIPPING
	//VM_TIGER_RED_END
#endif
	return PyBuf;
}
#endif

PyObject* NePyMethod_NewModule(PyObject* InSelf, PyObject* InArgs)
{
	char* ModuleName;
	PyObject* PyCode;
	PyObject* PyPath = nullptr;
	if (!PyArg_ParseTuple(InArgs, "sO|O", &ModuleName, &PyCode, &PyPath))
	{
		return 0;
	}

	PyObject* PyModule = PyImport_AddModule(ModuleName);
	if (PyModule == nullptr)
	{
		return PyErr_Format(PyExc_Exception, "PyImport_AddModule failed: %s", ModuleName);
	}

	PyObject* PyObj;
	PyObject* PyModules;

	/* If the module is being reloaded, we get the old module back
	   and re-use its dict to exec the new code. */
	PyObject* PyDict = PyModule_GetDict(PyModule);
	if (PyDict_GetItemString(PyDict, "__builtins__") == nullptr)
	{
		if (PyDict_SetItemString(PyDict, "__builtins__", PyEval_GetBuiltins()) != 0)
		{
			goto LABEL_ERROR;
		}
	}

	//多这段代码是要在载入package的时候先加上__path__属性
	if (nullptr != PyPath && Py_None != PyPath)
	{
		if (PyDict_SetItemString(PyDict, "__path__", PyPath) != 0)
		{
			goto LABEL_ERROR;
		}
	}

	/* Remember the filename as the __file__ attribute */
	if (PyDict_SetItemString(PyDict, "__file__", ((PyCodeObject*)PyCode)->co_filename) != 0)
	{
		/* Not important enough to report */
		PyErr_Clear();
	}

#if PY_MAJOR_VERSION == 2
	PyObj = PyEval_EvalCode((PyCodeObject*)PyCode, PyDict, PyDict);
#else
	PyObj = PyEval_EvalCode(PyCode, PyDict, PyDict);
#endif
	if (PyObj == nullptr)
	{
		goto LABEL_ERROR;
	}
	Py_DECREF(PyObj);

	PyModules = PyImport_GetModuleDict();
	if ((PyModule = PyDict_GetItemString(PyModules, ModuleName)) == nullptr)
	{
		PyErr_Format(PyExc_ImportError, "Loaded module %.200s not found in sys.modules", ModuleName);
		return nullptr;
	}

	Py_INCREF(PyModule);
	return PyModule;

LABEL_ERROR:
	{
		PyObject* PyModulesError = PyImport_GetModuleDict();
		if (PyDict_GetItemString(PyModulesError, ModuleName))
		{
			if (PyDict_DelItemString(PyModulesError, ModuleName) < 0)
			{
				Py_FatalError("import:  deleting existing key in sys.modules failed");
			}
		}
	}
	return nullptr;
}

PyObject* NePyMethod_MountPak(PyObject* InSelf, PyObject* InArgs)
{
	const char* PakFile;
	int Order;
	const char* MountPoint = nullptr;
	if (!PyArg_ParseTuple(InArgs, "si|s", &PakFile, &Order, &MountPoint))
	{
		return nullptr;
	}

	bool bResult = false;
	FPakPlatformFile* PakFileMgr = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));
	if (PakFileMgr == nullptr)
	{
		UE_LOG(LogNePython, Log, TEXT("MountPak failed: no PakFile found"));
		Py_RETURN_FALSE;
	}

	FString pakfile_path = UTF8_TO_TCHAR(PakFile);
	if (!FPaths::FileExists(pakfile_path))
	{
		bResult = false;
		UE_LOG(LogNePython, Error, TEXT("MountPak failed, %s not exists!"), *pakfile_path);
	}
	else if (MountPoint)
	{
		bResult = PakFileMgr->Mount(*pakfile_path, Order, UTF8_TO_TCHAR(MountPoint));
	}
	else
	{
		bResult = PakFileMgr->Mount(*pakfile_path, Order);
	}
	return Py_BuildValue("b", bResult);
}

PyObject* NePyMethod_CreateWorld(PyObject* InSelf, PyObject* InArgs)
{
	int WorldType = EWorldType::None;
	if (!PyArg_ParseTuple(InArgs, "|i:CreateWorld", &WorldType))
	{
		return nullptr;
	}

	UWorld* World = UWorld::CreateWorld((EWorldType::Type)WorldType, false);
	return NePyBase::ToPy(World);
}

PyObject* NePyMethod_ParsePropertyFlags(PyObject* InSelf, PyObject* InArgs)
{
	uint64 Flags;
	if (!PyArg_ParseTuple(InArgs, "l:ParsePropertyFlags", &Flags))
	{
		return nullptr;
	}

	auto RetVal = ParsePropertyFlags((EPropertyFlags)Flags);

	PyObject* PyRetVal0 = NePyBase::ToPy(RetVal);
	return PyRetVal0;
}

PyObject* NePyMethod_GetTransientPackage(PyObject* InSelf)
{
	auto RetVal = GetTransientPackage();

	PyObject* PyRetVal0 = NePyBase::ToPy(RetVal);
	return PyRetVal0;
}

PyObject* NePyMethod_GetIniFilenameFromObjectsReference(PyObject* InSelf, PyObject* InArg)
{
	PyObject* PyArgs[1] = { InArg };

	FString ObjectsReferenceString;
	if (!NePyBase::ToCpp(PyArgs[0], ObjectsReferenceString))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'ObjectsReferenceString' must have type 'str'");
		return nullptr;
	}

	auto RetVal = GetIniFilenameFromObjectsReference(ObjectsReferenceString);

	PyObject* PyRetVal0;
	if (!NePyBase::ToPy(RetVal, PyRetVal0))
	{
		PyErr_SetString(PyExc_TypeError, "PyRetVal0 'RetVal' with type 'str' convert to PyObject failed!");
		return nullptr;
	}
	return PyRetVal0;
}

PyObject* NePyMethod_ResolveIniObjectsReference(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyArgs[3] = { nullptr, nullptr, nullptr };
	if (!PyArg_ParseTuple(InArgs, "O|OO:ResolveIniObjectsReference", &PyArgs[0], &PyArgs[1], &PyArgs[2]))
	{
		return nullptr;
	}

	FString ObjectReference;
	if (!NePyBase::ToCpp(PyArgs[0], ObjectReference))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'ObjectReference' must have type 'str'");
		return nullptr;
	}

	FString* IniFilename = nullptr;
	FString TempIniFilename;
	if (PyArgs[1] && PyArgs[1] != Py_None)
	{
		if (!NePyBase::ToCpp(PyArgs[1], TempIniFilename))
		{
			PyErr_SetString(PyExc_TypeError, "arg2 'IniFilename' must have type 'str'");
			return nullptr;
		}
		IniFilename = &TempIniFilename;
	}

	bool bThrow = false;
	if (PyArgs[2])
	{
		if (!NePyBase::ToCpp(PyArgs[2], bThrow))
		{
			PyErr_SetString(PyExc_TypeError, "arg3 'bThrow' must have type 'bool'");
			return nullptr;
		}
	}

	auto RetVal = ResolveIniObjectsReference(ObjectReference, IniFilename, bThrow);

	PyObject* PyRetVal0;
	if (!NePyBase::ToPy(RetVal, PyRetVal0))
	{
		PyErr_SetString(PyExc_TypeError, "PyRetVal0 'RetVal' with type 'str' convert to PyObject failed!");
		return nullptr;
	}
	return PyRetVal0;
}

PyObject* NePyMethod_LoadPackage(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:LoadPackage", &Name))
	{
		return nullptr;
	}

	UPackage* Package = LoadPackage(nullptr, UTF8_TO_TCHAR(Name), LOAD_None);
	if (!Package)
	{
		return PyErr_Format(PyExc_Exception, "unable to load package %s", Name);
	}

	return NePyBase::ToPy(Package);
}

PyObject* NePyMethod_LoadPackageAsync(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	PyObject* PyCallable;
	int32 PackagePriority = 0;
	uint32 PackageFlags = PKG_None;
	if (!PyArg_ParseTuple(InArgs, "sO|iI:LoadPackageAsync", &Name, &PyCallable, &PackagePriority, &PackageFlags))
	{
		return nullptr;
	}

	if (!PyCallable_Check(PyCallable))
	{
		return PyErr_Format(PyExc_TypeError, "argument is not callable");
	}

	FNePyObjectPtrWithGIL PyCallablePtr = NePyNewReferenceWithGIL(PyCallable);
	FLoadPackageAsyncDelegate CompletionDelegate = FLoadPackageAsyncDelegate::CreateLambda([PyCallablePtr](const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
		{
			FNePyScopedGIL GIL;

			PyObject* PyArgs = PyTuple_New(3);
			PyTuple_SetItem(PyArgs, 0, NePyString_FromString(TCHAR_TO_ANSI(*(PackageName.ToString()))));
			PyTuple_SetItem(PyArgs, 1, NePyBase::ToPy(LoadedPackage));
			PyTuple_SetItem(PyArgs, 2, PyLong_FromLong(Result));
			PyObject_Call(PyCallablePtr, PyArgs, nullptr);
			Py_DECREF(PyArgs);
		});
	LoadPackageAsync(Name, CompletionDelegate, PackagePriority, (EPackageFlags)PackageFlags);
	Py_RETURN_NONE;
}

#if WITH_EDITOR
PyObject* NePyMethod_UnloadPackage(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyObj;
	if (!PyArg_ParseTuple(InArgs, "O:UnloadPackage", &PyObj))
	{
		return nullptr;
	}

	UPackage* PackageToUnload = NePyBase::ToCppObject<UPackage>(PyObj);
	if (!PackageToUnload)
	{
		return PyErr_Format(PyExc_Exception, "argument is not a UPackage");
	}

	FText OutErrorMsg;
	if (!PackageTools::UnloadPackages({ PackageToUnload }, OutErrorMsg))
	{
		return PyErr_Format(PyExc_Exception, "%s", TCHAR_TO_UTF8(*OutErrorMsg.ToString()));
	}

	Py_RETURN_NONE;
}

PyObject* NePyMethod_GetPackageFileName(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:GetPackageFileName", &Name))
	{
		return nullptr;
	}

	FString FileName;
	if (!FPackageName::DoesPackageExist(FString(UTF8_TO_TCHAR(Name)), nullptr, &FileName))
	{
		return PyErr_Format(PyExc_Exception, "package does not exist");
	}

	return PyUnicode_FromString(TCHAR_TO_UTF8(*FileName));
}
#endif

PyObject* NePyMethod_FindClass(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:FindClass", &Name))
	{
		return nullptr;
	}

	UClass* Class = FindObject<UClass>(ANY_PACKAGE, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Class);
}

PyObject* NePyMethod_LoadClass(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:LoadClass", &Name))
	{
		return nullptr;
	}

	UObject* Object = StaticLoadObject(UClass::StaticClass(), nullptr, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Object);
}

PyObject* NePyMethod_AsyncLoadClass(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	PyObject* PyCallable;
	int32 Priority = FStreamableManager::DefaultAsyncLoadPriority;
	if (!PyArg_ParseTuple(InArgs, "sO|i:AsyncLoadClass", &Name, &PyCallable, &Priority))
	{
		return nullptr;
	}

	if (!PyCallable_Check(PyCallable))
	{
		return PyErr_Format(PyExc_TypeError, "argument is not callable");
	}

	FSoftClassPath SoftPath(Name);

	FNePyObjectPtrWithGIL PyCallablePtr = NePyNewReferenceWithGIL(PyCallable);
	FStreamableDelegate StreamableDelegate = FStreamableDelegate::CreateLambda([SoftPath, PyCallablePtr]()
		{
			FNePyScopedGIL GIL;

			PyObject* PyName = NePyString_FromString(TCHAR_TO_ANSI(*(SoftPath.GetAssetPathString())));
			PyObject* PyClass = NePyBase::ToPy(SoftPath.ResolveClass());
			PyObject* PyArgs = PyTuple_New(2);
			PyTuple_SetItem(PyArgs, 0, PyName);
			PyTuple_SetItem(PyArgs, 1, PyClass);
			PyObject_Call(PyCallablePtr, PyArgs, nullptr);
			Py_DECREF(PyArgs);
		});
	UAssetManager::GetStreamableManager().RequestAsyncLoad(MoveTemp(SoftPath), MoveTemp(StreamableDelegate), Priority);
	Py_RETURN_NONE;
}

// 注意：这个函数会把已经Destroy（但还没被GC）的UObject也返回回来
// 如果只是想找场景上的Actor话，建议使用UWorld->FindActor(Name)
PyObject* NePyMethod_FindObject(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:FindObject", &Name))
	{
		return nullptr;
	}

	UObject* Object = FindObject<UObject>(ANY_PACKAGE, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Object);
}

PyObject* NePyMethod_LoadObject(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyClass;
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "Os:LoadObject", &PyClass, &Name))
	{
		return nullptr;
	}

	UClass* Class = NePyBase::ToCppClass(PyClass);
	if (!Class)
	{
		return PyErr_Format(PyExc_Exception, "argument is not a UClass");
	}

	UObject* Object = StaticLoadObject(Class, nullptr, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Object);
}

PyObject* NePyMethod_AsyncLoadObject(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	PyObject* PyCallable;
	int32 Priority = FStreamableManager::DefaultAsyncLoadPriority;
	if (!PyArg_ParseTuple(InArgs, "sO|i:AsyncLoadObject", &Name, &PyCallable, &Priority))
	{
		return nullptr;
	}

	if (!PyCallable_Check(PyCallable))
	{
		return PyErr_Format(PyExc_TypeError, "argument is not callable");
	}

	FSoftObjectPath SoftPath(Name);

	FNePyObjectPtrWithGIL PyCallablePtr = NePyNewReferenceWithGIL(PyCallable);
	FStreamableDelegate StreamableDelegate = FStreamableDelegate::CreateLambda([SoftPath, PyCallablePtr]()
		{
			FNePyScopedGIL GIL;

			PyObject* PyName = NePyString_FromString(TCHAR_TO_ANSI(*(SoftPath.GetAssetPathString())));
			PyObject* PyObj = NePyBase::ToPy(SoftPath.ResolveObject());
			PyObject* PyArgs = PyTuple_New(2);
			PyTuple_SetItem(PyArgs, 0, PyName);
			PyTuple_SetItem(PyArgs, 1, PyObj);
			PyObject_Call(PyCallablePtr, PyArgs, nullptr);
			Py_DECREF(PyArgs);
		});
	UAssetManager::GetStreamableManager().RequestAsyncLoad(MoveTemp(SoftPath), MoveTemp(StreamableDelegate), Priority);
	Py_RETURN_NONE;
}

PyObject* NePyMethod_FindStruct(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:FindStruct", &Name))
	{
		return nullptr;
	}

	UScriptStruct* Struct = FindObject<UScriptStruct>(ANY_PACKAGE, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Struct);
}

PyObject* NePyMethod_LoadStruct(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:LoadStruct", &Name))
	{
		return nullptr;
	}

	UObject* Object = StaticLoadObject(UScriptStruct::StaticClass(), nullptr, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Object);
}

PyObject* NePyMethod_FindEnum(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:FindEnum", &Name))
	{
		return nullptr;
	}

	UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Enum);
}

PyObject* NePyMethod_LoadEnum(PyObject* InSelf, PyObject* InArgs)
{
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s:LoadEnum", &Name))
	{
		return nullptr;
	}

	UObject* Object = StaticLoadObject(UEnum::StaticClass(), nullptr, UTF8_TO_TCHAR(Name));

	return NePyBase::ToPy(Object);
}

PyObject* NePyMethod_CancelAsyncLoading(PyObject* InSelf)
{
	CancelAsyncLoading();
	Py_RETURN_NONE;
}

PyObject* NePyMethod_GetAsyncLoadPercentage(PyObject* InSelf, PyObject* InArg)
{
	PyObject* PyArgs[1] = { InArg };

	FName PackageName;
	if (!NePyBase::ToCpp(PyArgs[0], PackageName))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'PackageName' must have type 'str'");
		return nullptr;
	}

	auto RetVal = GetAsyncLoadPercentage(PackageName);

	PyObject* PyRetVal0 = PyFloat_FromDouble(RetVal);
	return PyRetVal0;
}

PyObject* NePyMethod_FlushAsyncLoading(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyArgs[1] = { nullptr };
	if (!PyArg_ParseTuple(InArgs, "|O:FlushAsyncLoading", &PyArgs[0]))
	{
		return nullptr;
	}

	int32 PackageID = INDEX_NONE;
	if (PyArgs[0])
	{
		if (!NePyBase::ToCpp(PyArgs[0], PackageID))
		{
			PyErr_SetString(PyExc_TypeError, "arg1 'PackageID' must have type 'int'");
			return nullptr;
		}
	}

	FlushAsyncLoading(PackageID);
	Py_RETURN_NONE;
}

PyObject* NePyMethod_GetNumAsyncPackages(PyObject* InSelf)
{
	auto RetVal = GetNumAsyncPackages();

	PyObject* PyRetVal0 = PyLong_FromLong(RetVal);
	return PyRetVal0;
}

PyObject* NePyMethod_IsLoading(PyObject* InSelf)
{
	auto RetVal = IsLoading();

	PyObject* PyRetVal0 = PyBool_FromLong(RetVal);
	return PyRetVal0;
}

PyObject* NePyMethod_FindPackage(PyObject* InSelf, PyObject* InArgs)
{
	char* PackageName;
	if (!PyArg_ParseTuple(InArgs, "s:FindPackage", &PackageName))
	{
		return nullptr;
	}

	auto RetVal = FindPackage(ANY_PACKAGE, UTF8_TO_TCHAR(PackageName));

	PyObject* PyRetVal0 = NePyBase::ToPy(RetVal);
	return PyRetVal0;
}

PyObject* NePyMethod_CreatePackage(PyObject* InSelf, PyObject* InArgs)
{
	char* PackageName;
	if (!PyArg_ParseTuple(InArgs, "s:CreatePackage", &PackageName))
	{
		return nullptr;
	}

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION < 26
	auto RetVal = CreatePackage(nullptr, UTF8_TO_TCHAR(PackageName));
#else
	auto RetVal = CreatePackage(UTF8_TO_TCHAR(PackageName));
#endif

	PyObject* PyRetVal0 = NePyBase::ToPy(RetVal);
	return PyRetVal0;
}

PyObject* NePyMethod_NewObject(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyClass;
	PyObject* PyOuter = nullptr;
	char* NameStr = nullptr;
	uint64 Flags = (uint64)(RF_Public);
	if (!PyArg_ParseTuple(InArgs, "O|OsK:NewObject", &PyClass, &PyOuter, &NameStr, &Flags))
	{
		return nullptr;
	}

	UClass* Class = NePyBase::ToCppClass(PyClass);
	if (!Class)
	{
		return PyErr_Format(PyExc_Exception, "Class is not a UClass");
	}

	FName Name = NAME_None;
	if (NameStr && strlen(NameStr) > 0)
	{
		Name = FName(UTF8_TO_TCHAR(NameStr));
	}

	UObject* Outer = GetTransientPackage();
	if (PyOuter && PyOuter != Py_None)
	{
		Outer = NePyBase::ToCppObject(PyOuter);
		if (!Outer)
		{
			return PyErr_Format(PyExc_Exception, "Outer is not a UObject");
		}
	}

	UObject* RetObject = nullptr;
	RetObject = NewObject<UObject>(Outer, Class, Name, (EObjectFlags)Flags);
	if (RetObject)
	{
		RetObject->PostLoad();
	}

	if (!RetObject)
	{
		return PyErr_Format(PyExc_Exception, "unable to create object");
	}

	return NePyBase::ToPy(RetObject);
}

PyObject* NePyMethod_StringToGuid(PyObject* InSelf, PyObject* InArgs)
{
#if WITH_NEPY_AUTO_EXPORT
	char* Str;
	if (!PyArg_ParseTuple(InArgs, "s:StringToGuid", &Str))
	{
		return nullptr;
	}

	FGuid Guid;
	if (FGuid::Parse(FString(Str), Guid))
	{
		return NePyStructNew_Guid(Guid);
	}

	return PyErr_Format(PyExc_Exception, "unable to build FGuid");
#else
	Py_RETURN_NONE;
#endif
}

PyObject* NePyMethod_GuidToString(PyObject* InSelf, PyObject* InArgs)
{
#if WITH_NEPY_AUTO_EXPORT
	PyObject* PyArg;
	if (!PyArg_ParseTuple(InArgs, "O:GuidToString", &PyArg))
	{
		return nullptr;
	}

	FGuid* Guid;
	if (FNePyStruct_Guid* PyGuid = NePyStructCheck_Guid(PyArg))
	{
		Guid = &PyGuid->Value;
	}
	else
	{
		return PyErr_Format(PyExc_Exception, "object is not a FGuid");
	}

	return PyUnicode_FromString(TCHAR_TO_UTF8(*Guid->ToString()));
#else
	Py_RETURN_NONE;
#endif
}

PyObject* NePyMethod_TickSlate(PyObject* InSelf)
{
	FSlateApplication::Get().PumpMessages();
	FSlateApplication::Get().Tick();
	Py_RETURN_NONE;
}

PyObject* NePyMethod_TickEngine(PyObject* InSelf, PyObject* InArgs)
{
	float DeltaSeconds = FApp::GetDeltaTime();
	PyObject* PyIdle = nullptr;
	if (!PyArg_ParseTuple(InArgs, "|fO:EngineTick", &DeltaSeconds, &PyIdle))
	{
		return nullptr;
	}

	bool bIdle = false;
	if (PyIdle && PyObject_IsTrue(PyIdle))
	{
		bIdle = true;
	}

	GEngine->Tick(DeltaSeconds, bIdle);

	Py_RETURN_NONE;
}

#if WITH_EDITOR
PyObject* NePyMethod_TickRenderingTickables(PyObject* InSelf)
{
	TickRenderingTickables();

	Py_RETURN_NONE;
}
#endif

PyObject* NePyMethod_GetDeltaTime(PyObject* InSelf)
{
	return PyFloat_FromDouble(FApp::GetDeltaTime());
}

PyObject* NePyMethod_GetAllWorlds(PyObject* InSelf)
{
	FNePyObjectPtr Ret = NePyNewReference(PyList_New(0));
	for (TObjectIterator<UWorld> Itr; Itr; ++Itr)
	{
		PyObject* PyWorld = NePyBase::ToPy(*Itr);
		PyList_Append(Ret, PyWorld);
		Py_DECREF(PyWorld);
	}
	return Ret;
}

PyObject* NePyMethod_GetGameViewportSize(PyObject* InSelf)
{
	if (!GEngine->GameViewport)
	{
		return PyErr_Format(PyExc_Exception, "unable to get GameViewport");
	}

	FVector2D size;
	GEngine->GameViewport->GetViewportSize(size);

	return Py_BuildValue("(ff)", size.X, size.Y);
}

PyObject* NePyMethod_GetResolution(PyObject* InSelf)
{
	return Py_BuildValue("(ff)", GSystemResolution.ResX, GSystemResolution.ResY);
}

PyObject* NePyMethod_GetViewportScreenshot(PyObject* InSelf, PyObject* InArgs)
{
#if WITH_NEPY_AUTO_EXPORT
	if (!GEngine->GameViewport)
	{
		Py_RETURN_NONE;
	}

	bool bAsIntList = false;
	if (!PyArg_ParseTuple(InArgs, "|b:GetViewportScreenshot", &bAsIntList))
	{
		return nullptr;
	}

	FViewport* Viewport = GEngine->GameViewport->Viewport;
	TArray<FColor> Bitmap;
	bool bSuccess = GetViewportScreenShot(Viewport, Bitmap);
	if (!bSuccess)
	{
		Py_RETURN_NONE;
	}

	if (bAsIntList)
	{
		FNePyObjectPtr BitmapTuple = NePyNewReference(PyTuple_New(Bitmap.Num() * 4));
		for (int i = 0; i < Bitmap.Num(); i++)
		{
			PyTuple_SetItem(BitmapTuple, i * 4, PyLong_FromLong(Bitmap[i].R));
			PyTuple_SetItem(BitmapTuple, i * 4 + 1, PyLong_FromLong(Bitmap[i].G));
			PyTuple_SetItem(BitmapTuple, i * 4 + 2, PyLong_FromLong(Bitmap[i].B));
			PyTuple_SetItem(BitmapTuple, i * 4 + 3, PyLong_FromLong(Bitmap[i].A));
		}
		return BitmapTuple;
	}

	FNePyObjectPtr BitmapTuple = NePyNewReference(PyTuple_New(Bitmap.Num()));
	for (int i = 0; i < Bitmap.Num(); i++)
	{
		PyTuple_SetItem(BitmapTuple, i, NePyStructNew_Color(Bitmap[i]));
	}
	return BitmapTuple;
#else
	Py_RETURN_NONE;
#endif
}

PyObject* NePyMethod_GetStatUnit(PyObject* InSelf)
{
	if (!GEngine->GameViewport)
	{
		Py_RETURN_NONE;
	}

	// GEngine->GameViewport->IsStatEnabled(TEXT("Unit"));
	if (const TArray<FString>* CurrentStats = GEngine->GameViewport->GetEnabledStats())
	{
		TArray<FString> NewStats = *CurrentStats;
		NewStats.Add(TEXT("Unit"));
		GEngine->GameViewport->SetEnabledStats(NewStats);
		// GEngine->GameViewport->SetShowStats(false);
	}

	FStatUnitData* StatUnit = GEngine->GameViewport->GetStatUnitData();
	PyObject* PyDict = PyDict_New();

	/** Unit frame times filtered with a simple running average */
	PyDict_SetItem(PyDict, PyUnicode_FromString("RenderThreadTime"), PyFloat_FromDouble(StatUnit->RenderThreadTime));
	PyDict_SetItem(PyDict, PyUnicode_FromString("GameThreadTime"), PyFloat_FromDouble(StatUnit->GameThreadTime));
#if ENGINE_MINOR_VERSION >= 26
#define PyDict_SetItem_GPUFrameTimeArray(Num) \
		PyDict_SetItem(PyDict, PyUnicode_FromString("GPUFrameTime"#Num), PyFloat_FromDouble(StatUnit->GPUFrameTime[Num]))
	PyDict_SetItem_GPUFrameTimeArray(0);
#if PLATFORM_DESKTOP
	PyDict_SetItem_GPUFrameTimeArray(1);
	PyDict_SetItem_GPUFrameTimeArray(2);
	PyDict_SetItem_GPUFrameTimeArray(3);
#endif

#else
	PyDict_SetItem(PyDict, PyUnicode_FromString("GPUFrameTime"), PyFloat_FromDouble(StatUnit->GPUFrameTime));
#endif
	PyDict_SetItem(PyDict, PyUnicode_FromString("FrameTime"), PyFloat_FromDouble(StatUnit->FrameTime));
	PyDict_SetItem(PyDict, PyUnicode_FromString("RHITTime"), PyFloat_FromDouble(StatUnit->RHITTime));

	/** Raw equivalents of the above variables */
	PyDict_SetItem(PyDict, PyUnicode_FromString("RawRenderThreadTime"), PyFloat_FromDouble(StatUnit->RawRenderThreadTime));
	PyDict_SetItem(PyDict, PyUnicode_FromString("RawGameThreadTime"), PyFloat_FromDouble(StatUnit->RawGameThreadTime));
#if ENGINE_MINOR_VERSION >= 26
#define PyDict_SetItem_RawGPUFrameTimeArray(Num) \
		PyDict_SetItem(PyDict, PyUnicode_FromString("RawGPUFrameTime"#Num), PyFloat_FromDouble(StatUnit->RawGPUFrameTime[Num]))
	PyDict_SetItem_RawGPUFrameTimeArray(0);
#if PLATFORM_DESKTOP
	PyDict_SetItem_RawGPUFrameTimeArray(1);
	PyDict_SetItem_RawGPUFrameTimeArray(2);
	PyDict_SetItem_RawGPUFrameTimeArray(3);
#endif
#else
	PyDict_SetItem(PyDict, PyUnicode_FromString("RawGPUFrameTime"), PyFloat_FromDouble(StatUnit->RawGPUFrameTime));
#endif
	PyDict_SetItem(PyDict, PyUnicode_FromString("RawFrameTime"), PyFloat_FromDouble(StatUnit->RawFrameTime));
	PyDict_SetItem(PyDict, PyUnicode_FromString("RawRHITTime"), PyFloat_FromDouble(StatUnit->RawRHITTime));

	/** Time that has transpired since the last draw call */
	PyDict_SetItem(PyDict, PyUnicode_FromString("LastTime"), PyFloat_FromDouble(StatUnit->LastTime));

	return PyDict;
}

PyObject* NePyMethod_GetStatFps(PyObject* InSelf)
{
	extern ENGINE_API float GAverageFPS;
	return PyFloat_FromDouble(GAverageFPS);
}

PyObject* NePyMethod_GetStatRhi(PyObject* InSelf)
{
	PyObject* PyDict = PyDict_New();

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 26
	PyDict_SetItem(PyDict, PyUnicode_FromString("GNumPrimitivesDrawnRHI"), PyFloat_FromDouble(GNumPrimitivesDrawnRHI));
	PyDict_SetItem(PyDict, PyUnicode_FromString("GNumDrawCallsRHI"), PyFloat_FromDouble(GNumDrawCallsRHI));
#else
	PyObject* PyArgs1 = PyTuple_New(MAX_NUM_GPUS);
	PyObject* PyArgs2 = PyTuple_New(MAX_NUM_GPUS);
	for (int i = 0; i < MAX_NUM_GPUS; ++i)
	{
		PyTuple_SetItem(PyArgs1, i, PyFloat_FromDouble(GNumPrimitivesDrawnRHI[i]));
		PyTuple_SetItem(PyArgs2, i, PyFloat_FromDouble(GNumDrawCallsRHI[i]));
	}
	PyDict_SetItem(PyDict, PyUnicode_FromString("GNumPrimitivesDrawnRHI"), PyArgs1);
	PyDict_SetItem(PyDict, PyUnicode_FromString("GNumDrawCallsRHI"), PyArgs2);
#endif

	return PyDict;
}

//PyObject* NePyMethod_GetAppPerformanceInfo(PyObject* InSelf, PyObject* InArgs)
//{
//	float battery = AccessBatteryInfo();
//	float ram = AccessCurrentRAMMemoryUsage();
//	TArray<double> vmem = AccessCurrentVirtualMemory();
//	float memFree = vmem[0];
//	float memUsed = vmem[1];
//	float cpuUsage = AccessCurrentCpuUsage();
//	TArray<int> iOBytes = AccessCurrentIOBytes();
//	int rByte = iOBytes[0];
//	int wByte = iOBytes[1];
//	TArray<int> netBytes = AccessCurrentInterfaceBytes();
//	int iByte = netBytes[0];
//	int oByte = netBytes[1];
//
//	PyObject* PyDict = PyDict_New();
//
//	PyDict_SetItem(PyDict, PyUnicode_FromString("batteryLevel"), PyFloat_FromDouble(battery));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("virtualMemoryFree"), PyFloat_FromDouble(memFree));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("memoryUsage"), PyFloat_FromDouble(ram));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("virtualMemoryUsed"), PyFloat_FromDouble(memUsed));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("readBytes"), PyFloat_FromDouble(rByte));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("writeBytes"), PyFloat_FromDouble(wByte));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("uploadBytes"), PyFloat_FromDouble(iByte));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("dowloadBytes"), PyFloat_FromDouble(oByte));
//	PyDict_SetItem(PyDict, PyUnicode_FromString("cpuUsage"), PyFloat_FromDouble(cpuUsage));
//
//	return PyDict;
//}
//
//#if WITH_EDITOR
//PyObject* NePyMethod_DumpReflectionInfos(PyObject*, PyObject*)
//{
//	TSharedRef<FJsonObject> ReflectionInfos = NePy::DumpReflectionInfos();
//
//	FString OutputString;
//	auto Writer = TJsonWriterFactory<>::Create(&OutputString);
//	FJsonSerializer::Serialize(ReflectionInfos, Writer);
//
//	FString DumpFilePath = FPaths::Combine(*FPaths::ProjectContentDir(), UTF8_TO_TCHAR("reflection_infos.json"));
//	FFileHelper::SaveStringToFile(OutputString, *DumpFilePath);
//
//	Py_RETURN_NONE;
//}
//#endif
//
//PyObject* NePyMethod_CreatePackage(PyObject* InSelf, PyObject* InArgs)
//{
//
//	char* Name;
//
//	if (!PyArg_ParseTuple(InArgs, "s:CreatePackage", &Name))
//	{
//		return nullptr;
//	}
//
//	UPackage* u_package = (UPackage*)StaticFindObject(nullptr, ANY_PACKAGE, UTF8_TO_TCHAR(Name), true);
//	// create a new package if it does not exist
//	if (u_package)
//	{
//		return PyErr_Format(PyExc_Exception, "package %s already exists", TCHAR_TO_UTF8(*u_package->GetPathName()));
//	}
//	u_package = CreatePackage(nullptr, UTF8_TO_TCHAR(Name));
//	if (!u_package)
//		return PyErr_Format(PyExc_Exception, "unable to create package");
//	u_package->FileName = *FPackageName::LongPackageNameToFilename(UTF8_TO_TCHAR(Name), FPackageName::GetAssetPackageExtension());
//
//	u_package->FullyLoad();
//	u_package->MarkPackageDirty();
//
//	return NePyBase::ToPy(u_package);
//}
//
//PyObject* NePyMethod_get_or_CreatePackage(PyObject* InSelf, PyObject* InArgs)
//{
//
//	char* Name;
//
//	if (!PyArg_ParseTuple(InArgs, "s:get_or_CreatePackage", &Name))
//	{
//		return nullptr;
//	}
//
//	UPackage* u_package = (UPackage*)StaticFindObject(nullptr, ANY_PACKAGE, UTF8_TO_TCHAR(Name), true);
//	// create a new package if it does not exist
//	if (!u_package)
//	{
//		u_package = CreatePackage(nullptr, UTF8_TO_TCHAR(Name));
//		if (!u_package)
//			return PyErr_Format(PyExc_Exception, "unable to create package");
//		u_package->FileName = *FPackageName::LongPackageNameToFilename(UTF8_TO_TCHAR(Name), FPackageName::GetAssetPackageExtension());
//
//		u_package->FullyLoad();
//		u_package->MarkPackageDirty();
//	}
//
//	return NePyBase::ToPy(u_package);
//}
//
//PyObject* NePyMethod_GetTransientPackage(PyObject* InSelf, PyObject* InArgs)
//{
//	return NePyBase::ToPy(GetTransientPackage());
//}
//
//PyObject* NePyMethod_open_file_diaLog(PyObject* InSelf, PyObject* InArgs)
//{
//	char* title;
//	char* default_path = (char*)"";
//	char* default_file = (char*)"";
//	char* file_types = (char*)"";
//	PyObject* py_multiple = nullptr;
//
//	if (!PyArg_ParseTuple(InArgs, "s|sssO:open_file_diaLog", &title, &default_path, &default_file, &file_types, &py_multiple))
//		return nullptr;
//
//	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//	if (!DesktopPlatform)
//		return PyErr_Format(PyExc_Exception, "unable to get reference to DesktopPlatform module");
//
//	TSharedPtr<SWindow> ParentWindow = FGlobalTabmanager::Get()->GetRootWindow();
//	if (!ParentWindow.IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get the Root Window");
//
//	if (!ParentWindow->GetNativeWindow().IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get Native Root Window");
//
//	TArray<FString> files;
//
//	if (!DesktopPlatform->OpenFileDiaLog(ParentWindow->GetNativeWindow()->GetOSWindowHandle(), FString(UTF8_TO_TCHAR(title)),
//		FString(UTF8_TO_TCHAR(default_path)),
//		FString(UTF8_TO_TCHAR(default_file)),
//		FString(UTF8_TO_TCHAR(file_types)),
//		(py_multiple && PyObject_IsTrue(py_multiple)) ? EFileDiaLogFlags::Multiple : EFileDiaLogFlags::None,
//		files))
//	{
//		Py_RETURN_NONE;
//	}
//
//	PyObject* py_list = PyList_New(0);
//	for (FString file : files)
//	{
//		UEP_PyList_Append_and_DecRef(py_list, PyUnicode_FromString(TCHAR_TO_UTF8(*file)));
//	}
//	return py_list;
//}
//
//PyObject* NePyMethod_open_directory_diaLog(PyObject* InSelf, PyObject* InArgs)
//{
//	char* title;
//	char* default_path = (char*)"";
//
//	if (!PyArg_ParseTuple(InArgs, "s|s:open_directory_diaLog", &title, &default_path))
//		return nullptr;
//
//	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//	if (!DesktopPlatform)
//		return PyErr_Format(PyExc_Exception, "unable to get reference to DesktopPlatform module");
//
//	TSharedPtr<SWindow> ParentWindow = FGlobalTabmanager::Get()->GetRootWindow();
//	if (!ParentWindow.IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get the Root Window");
//
//	if (!ParentWindow->GetNativeWindow().IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get Native Root Window");
//
//	FString choosen_dir;
//
//	if (!DesktopPlatform->OpenDirectoryDiaLog(ParentWindow->GetNativeWindow()->GetOSWindowHandle(), FString(UTF8_TO_TCHAR(title)),
//		FString(UTF8_TO_TCHAR(default_path)),
//		choosen_dir))
//	{
//		Py_RETURN_NONE;
//	}
//
//	return PyUnicode_FromString(TCHAR_TO_UTF8(*choosen_dir));
//}
//
//PyObject* NePyMethod_open_font_diaLog(PyObject* InSelf, PyObject* InArgs)
//{
//
//	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//	if (!DesktopPlatform)
//		return PyErr_Format(PyExc_Exception, "unable to get reference to DesktopPlatform module");
//
//	TSharedPtr<SWindow> ParentWindow = FGlobalTabmanager::Get()->GetRootWindow();
//	if (!ParentWindow.IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get the Root Window");
//
//	if (!ParentWindow->GetNativeWindow().IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get Native Root Window");
//
//	FString font_name;
//	float height;
//	EFontImportFlags flags;
//
//	if (!DesktopPlatform->OpenFontDiaLog(ParentWindow->GetNativeWindow()->GetOSWindowHandle(), font_name, height, flags))
//	{
//		Py_RETURN_NONE;
//	}
//
//	return Py_BuildValue((char*)"(sfi)", TCHAR_TO_UTF8(*font_name), height, flags);
//}
//
//PyObject* NePyMethod_save_file_diaLog(PyObject* InSelf, PyObject* InArgs)
//{
//	char* title;
//	char* default_path = (char*)"";
//	char* default_file = (char*)"";
//	char* file_types = (char*)"";
//	PyObject* py_multiple = nullptr;
//
//	if (!PyArg_ParseTuple(InArgs, "s|sssO:save_file_diaLog", &title, &default_path, &default_file, &file_types, &py_multiple))
//		return nullptr;
//
//	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//	if (!DesktopPlatform)
//		return PyErr_Format(PyExc_Exception, "unable to get reference to DesktopPlatform module");
//
//	TSharedPtr<SWindow> ParentWindow = FGlobalTabmanager::Get()->GetRootWindow();
//	if (!ParentWindow.IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get the Root Window");
//
//	if (!ParentWindow->GetNativeWindow().IsValid())
//		return PyErr_Format(PyExc_Exception, "unable to get Native Root Window");
//
//	TArray<FString> files;
//
//	if (!DesktopPlatform->SaveFileDiaLog(ParentWindow->GetNativeWindow()->GetOSWindowHandle(), FString(UTF8_TO_TCHAR(title)),
//		FString(UTF8_TO_TCHAR(default_path)),
//		FString(UTF8_TO_TCHAR(default_file)),
//		FString(UTF8_TO_TCHAR(file_types)),
//		(py_multiple && PyObject_IsTrue(py_multiple)) ? EFileDiaLogFlags::Multiple : EFileDiaLogFlags::None,
//		files))
//	{
//		Py_RETURN_NONE;
//	}
//
//	PyObject* py_list = PyList_New(0);
//	for (FString file : files)
//	{
//		UEP_PyList_Append_and_DecRef(py_list, PyUnicode_FromString(TCHAR_TO_UTF8(*file)));
//	}
//	return py_list;
//}

PyObject* NePyMethod_CopyPropertiesForUnrelatedObjects(PyObject* InSelf, PyObject* InArgs, PyObject* kwInArgs)
{
	PyObject* PyOldObject;
	PyObject* PyNewObject;

	PyObject* PyAggressiveDefaultSubobjectReplacement = nullptr;
	PyObject* PyCopyDeprecatedProperties = nullptr;
	PyObject* PyDoDelta = nullptr;
	PyObject* PyNotifyObjectReplacement = nullptr;
	PyObject* PyPreserveRootComponent = nullptr;
	PyObject* PyReplaceObjectClassReferences = nullptr;
	PyObject* PySkipCompilerGeneratedDefaults = nullptr;

	static char* kw_names[] = {
		(char*)"OldObject",
		(char*)"NewObject",
		(char*)"AggressiveDefaultSubobjectReplacement",
		(char*)"CopyDeprecatedProperties",
		(char*)"DoDelta",
		(char*)"NotifyObjectReplacement",
		(char*)"PreserveRootComponent",
		(char*)"ReplaceObjectClassReferences",
		(char*)"SkipCompilerGeneratedDefaults",
		nullptr
	};

	if (!PyArg_ParseTupleAndKeywords(InArgs, kwInArgs, "OO|OOOOOOO:CopyPropertiesForUnrelatedObjects", kw_names,
		&PyOldObject,
		&PyNewObject,
		&PyAggressiveDefaultSubobjectReplacement,
		&PyCopyDeprecatedProperties,
		&PyDoDelta,
		&PyNotifyObjectReplacement,
		&PyPreserveRootComponent,
		&PyReplaceObjectClassReferences,
		&PySkipCompilerGeneratedDefaults))
	{
		return nullptr;
	}

	UObject* OldObject = NePyBase::ToCppObject(PyOldObject);
	if (!OldObject)
	{
		return PyErr_Format(PyExc_Exception, "OldObject is not a UObject");
	}

	UObject* NewObject = NePyBase::ToCppObject(PyNewObject);
	if (!NewObject)
	{
		return PyErr_Format(PyExc_Exception, "NewObject is not a UObject");
	}

	UEngine::FCopyPropertiesForUnrelatedObjectsParams Params;
	Params.bAggressiveDefaultSubobjectReplacement = (PyAggressiveDefaultSubobjectReplacement && PyObject_IsTrue(PyAggressiveDefaultSubobjectReplacement));
	Params.bCopyDeprecatedProperties = (PyCopyDeprecatedProperties && PyObject_IsTrue(PyCopyDeprecatedProperties));
	Params.bDoDelta = (PyDoDelta && PyObject_IsTrue(PyDoDelta));
	Params.bNotifyObjectReplacement = (PyNotifyObjectReplacement && PyObject_IsTrue(PyNotifyObjectReplacement));
	Params.bPreserveRootComponent = (PyPreserveRootComponent && PyObject_IsTrue(PyPreserveRootComponent));
	Params.bReplaceObjectClassReferences = (PyReplaceObjectClassReferences && PyObject_IsTrue(PyReplaceObjectClassReferences));
	Params.bSkipCompilerGeneratedDefaults = (PySkipCompilerGeneratedDefaults && PyObject_IsTrue(PySkipCompilerGeneratedDefaults));

	GEngine->CopyPropertiesForUnrelatedObjects(
		OldObject,
		NewObject,
		Params);

	Py_RETURN_NONE;
}

PyObject* NePyMethod_SetRandomSeed(PyObject* InSelf, PyObject* InArgs)
{
	int Seed;
	if (!PyArg_ParseTuple(InArgs, "i:SetRandomSeed", &Seed))
	{
		return nullptr;
	}

	// Thanks to Sven Mika (Ducandu GmbH) for spotting this
	FMath::RandInit(Seed);
	FGenericPlatformMath::SRandInit(Seed);
	FGenericPlatformMath::RandInit(Seed);

	Py_RETURN_NONE;
}

PyObject* NePyMethod_ClipboardCopy(PyObject* InSelf, PyObject* InArgs)
{
	char* Text;
	if (!PyArg_ParseTuple(InArgs, "s:ClipboardCopy", &Text))
	{
		return nullptr;
	}

	FPlatformApplicationMisc::ClipboardCopy(UTF8_TO_TCHAR(Text));
	Py_RETURN_NONE;
}

PyObject* NePyMethod_ClipboardPaste(PyObject* InSelf)
{
	FString Clipboard;
	FPlatformApplicationMisc::ClipboardPaste(Clipboard);
	return PyUnicode_FromString(TCHAR_TO_UTF8(*Clipboard));
}

#if WITH_EDITOR
PyObject* NePyMethodDumpReflectionInfos(PyObject* InSelf, PyObject* InArgs)
{
	char* PyOutputDir = nullptr;
	if (!PyArg_ParseTuple(InArgs, "|s", &PyOutputDir))
	{
		return nullptr;
	}

	FString OutputDir;
	if (PyOutputDir)
	{
		OutputDir = PyOutputDir;
	}
	else
	{
		OutputDir = NePyGetDefaultDumpReflectionInfosDirectory();
	}

	NePyDumpReflectionInfosToFile(OutputDir);
	Py_RETURN_NONE;
}

PyObject* NePyMethodGetEditorWorld(PyObject* InSelf)
{
	if (!GEditor)
		return PyErr_Format(PyExc_Exception, "no GEditor found");

	UWorld* World = GEditor->GetEditorWorldContext().World();
	return NePyBase::ToPy(World);
}


PyObject* NePyMethod_DeleteAsset(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	PyObject* PyShowConfirmation = nullptr;
	if (!PyArg_ParseTuple(InArgs, "s|O:DeleteAsset", &Path, &PyShowConfirmation))
	{
		return nullptr;
	}
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	bool bShowConfirmation = false;
	if (PyShowConfirmation && PyObject_IsTrue(PyShowConfirmation))
	{
		bShowConfirmation = true;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData Asset = AssetRegistryModule.Get().GetAssetByObjectPath(UTF8_TO_TCHAR(Path));
	if (!Asset.IsValid())
	{
		return PyErr_Format(PyExc_Exception, "unable to find Asset %s", Path);
	}

	UObject* Object = Asset.GetAsset();
	TArray<UObject*> Objects;
	Objects.Add(Object);

	if (ObjectTools::DeleteObjects(Objects, bShowConfirmation) < 1)
	{
		if (ObjectTools::ForceDeleteObjects(Objects, bShowConfirmation) < 1)
		{
			return PyErr_Format(PyExc_Exception, "unable to delete Asset %s", Path);
		}
	}

	Py_RETURN_NONE;
}

/*以依赖顺序删除资源*/
PyObject* NePyMethod_DeleteAssets(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* PyToDeleteAssets = nullptr;
	if (!PyArg_ParseTuple(InArgs, "O:DeleteAssets", &PyToDeleteAssets))
	{
		return nullptr;
	}
	if (!PyList_Check(PyToDeleteAssets))
	{
		return PyErr_Format(PyExc_Exception, "PyToDeleteAssets is not a valid List");
	}
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	int Num = PyList_Size(PyToDeleteAssets);
	if (Num <= 0)
	{
		return PyErr_Format(PyExc_Exception, "PyToDeleteAssets is not a valid list");
	}

	FBinaryHeap<int> ReferencersHeap;
	TMultiMap<FName, uint32> PathToIndex;
	TMap<uint32, FName> IndexToPath;
	TArray<FName> RawObjectList;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	char* FileName = nullptr;
	FString SepStr(".");
	for (int i = 0; i < Num; ++i)
	{
		PyObject* StrObj = PyList_GetItem(PyToDeleteAssets, i);
#if PY_MAJOR_VERSION >= 3
		FileName = PyBytes_AS_STRING(PyUnicode_AsEncodedString(StrObj, "utf-8", "Error"));
#else
		FileName = PyString_AsString(FNePyObjectPtr::StealReference(PyObject_Str(StrObj)));
#endif

		TArray<FName> Referencers;
		FName ObjectPath(UTF8_TO_TCHAR(FileName));
		RawObjectList.Add(ObjectPath);
		FString PackagePathStr;
		FString AssetNameStr;
		ObjectPath.ToString().Split(SepStr, &PackagePathStr, &AssetNameStr);
		FName PackagePath(*PackagePathStr);
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION < 26
		AssetRegistryModule.Get().GetReferencers(PackagePath, Referencers, EAssetRegistryDependencyType::All);
#else
		AssetRegistryModule.Get().GetReferencers(PackagePath, Referencers);
#endif
		PathToIndex.Add(PackagePath, i);
		IndexToPath.Add(i, PackagePath);
		ReferencersHeap.Add(Referencers.Num(), i);
	}

	while (ReferencersHeap.Num())
	{
		int TopIndex = ReferencersHeap.Top();
		ReferencersHeap.Pop();

		FName PackagePath = IndexToPath[TopIndex];
		FAssetData Asset = AssetRegistryModule.Get().GetAssetByObjectPath(RawObjectList[TopIndex]);
		if (!Asset.IsValid())
		{
			continue;
		}

		UObject* Object = Asset.GetAsset();

		TArray<FName> Dependencies;
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION < 26
		AssetRegistryModule.Get().GetDependencies(PackagePath, Dependencies, EAssetRegistryDependencyType::All);
#else
		AssetRegistryModule.Get().GetDependencies(PackagePath, Dependencies);
#endif

		TArray<UObject*> Objects;
		Objects.Add(Object);

		if (ObjectTools::DeleteObjects(Objects, false) < 1)
		{
			if (ObjectTools::ForceDeleteObjects(Objects, false) < 1)
			{
				UE_LOG(LogNePython, Error, TEXT("NePyMethod_DeleteAssets: delete object fail %s"), *PackagePath.ToString());
			}
		}
		int RefCnts;
		int Index;
		for (FName Outer : Dependencies)
		{
			if (!PathToIndex.Contains(Outer))
			{
				continue;
			}

			Index = PathToIndex.FindRef(Outer);
			// 考虑循环引用的情况这里需要先检查是否存在
			if (ReferencersHeap.IsPresent(Index))
			{
				RefCnts = ReferencersHeap.GetKey(Index);
				ReferencersHeap.Update(--RefCnts, Index);
			}
			else
			{
				UE_LOG(LogNePython, Warning, TEXT("NePyMethod_DeleteAssets: there may have loop ref for %s"), *PackagePath.ToString());
			}
		}
	}
	Py_RETURN_NONE;
}

PyObject* NePyMethod_GetAssets(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	PyObject* PyRecursive = nullptr;
	if (!PyArg_ParseTuple(InArgs, "s|O:GetAssets", &Path, &PyRecursive))
	{
		return nullptr;
	}
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	bool bRecursive = false;
	if (PyRecursive && PyObject_IsTrue(PyRecursive))
	{
		bRecursive = true;
	}

	TArray<FAssetData> Assets;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().GetAssetsByPath(UTF8_TO_TCHAR(Path), Assets, bRecursive);

	PyObject* AssetsList = PyList_New(0);
	for (FAssetData Asset : Assets)
	{
		if (!Asset.IsValid())
		{
			continue;
		}
		PyObject* PyAsset = NePyBase::ToPy(Asset.GetAsset());
		if (PyAsset)
		{
			PyList_Append(AssetsList, PyAsset);
			Py_DECREF(PyAsset);
		}
	}

	return AssetsList;
}

PyObject* NePyMethod_GetAllAssetsPath(PyObject* InSelf, PyObject* InArgs)
{
	char* Path;
	PyObject* PyRecursive = nullptr;
	if (!PyArg_ParseTuple(InArgs, "s|O:GetAllAssetsPath", &Path, &PyRecursive))
	{
		return nullptr;
	}
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	bool bRecursive = false;
	if (PyRecursive && PyObject_IsTrue(PyRecursive))
	{
		bRecursive = true;
	}

	TArray<FAssetData> Assets;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().GetAssetsByPath(UTF8_TO_TCHAR(Path), Assets, bRecursive);

	PyObject* AssetsList = PyList_New(0);
	for (FAssetData Asset : Assets)
	{
		if (!Asset.IsValid())
		{
			continue;
		}
		PyObject* PyAssetClass = PyUnicode_FromString(TCHAR_TO_UTF8(*Asset.AssetClass.ToString()));
		PyObject* PyObjectPath = PyUnicode_FromString(TCHAR_TO_UTF8(*Asset.ObjectPath.ToString()));
		PyList_Append(AssetsList, PyAssetClass);
		PyList_Append(AssetsList, PyObjectPath);
		Py_DECREF(PyAssetClass);
		Py_DECREF(PyObjectPath);
	}

	return AssetsList;
}

PyObject* NePyMethod_GetAssetUnused(PyObject* InSelf, PyObject* InArgs)
{
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}

	FString ContentRelativeDir(TEXT("/Game"));
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UObject::StaticClass(), false, true);
	ObjectLibrary->LoadAssetDataFromPath(*ContentRelativeDir);
	TArray<FAssetData> AssetData;
	ObjectLibrary->GetAssetDataList(AssetData);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	PyObject* UnusedList = PyList_New(0);
	for (auto& Asset : AssetData)
	{
		TArray<FName> Referencers;
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION < 26
		AssetRegistry.GetReferencers(Asset.PackageName, Referencers, EAssetRegistryDependencyType::All);
#else
		AssetRegistry.GetReferencers(Asset.PackageName, Referencers);
#endif
		TArray<FName> Dependencies;
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION < 26
		AssetRegistryModule.Get().GetDependencies(Asset.PackageName, Dependencies, EAssetRegistryDependencyType::All);
#else
		AssetRegistryModule.Get().GetDependencies(Asset.PackageName, Dependencies);
#endif
		if (Referencers.Num() + Dependencies.Num() == 0)
		{
			//UE_LOG(LogNePython, Warning, TEXT("%s"), *Asset.PackageName.ToString());
			PyObject* PyPackageName = PyUnicode_FromString(TCHAR_TO_UTF8(*Asset.PackageName.ToString()));
			PyList_Append(UnusedList, PyPackageName);
			Py_DECREF(PyPackageName);
			continue;
		}
		if ((Referencers.Num() == 1) && (Dependencies.Num() == 0) && (Referencers[0] == Asset.PackageName))
		{
			PyObject* PyPackageName = PyUnicode_FromString(TCHAR_TO_UTF8(*Asset.PackageName.ToString()));
			PyList_Append(UnusedList, PyPackageName);
			Py_DECREF(PyPackageName);
		}
	}
	return UnusedList;
}

PyObject* NePyMethod_GetAssetUnreferenced(PyObject* InSelf, PyObject* InArgs)
{
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}

	FString ContentRelativeDir(TEXT("/Game"));
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UObject::StaticClass(), false, true);
	ObjectLibrary->LoadAssetDataFromPath(*ContentRelativeDir);
	TArray<FAssetData> AssetData;
	ObjectLibrary->GetAssetDataList(AssetData);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	PyObject* UnreferencedList = PyList_New(0);
	for (auto& Asset : AssetData) {
		TArray<FName> Referencers;
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION < 26
		AssetRegistry.GetReferencers(Asset.PackageName, Referencers, EAssetRegistryDependencyType::All);
#else
		AssetRegistry.GetReferencers(Asset.PackageName, Referencers);
#endif
		// UE_LOG(LogNePython, Warning, TEXT("%s"), *Asset.PackageName.ToString());
		if (Referencers.Num() == 0)
		{
			//UE_LOG(LogNePython, Warning, TEXT("%s"), *Asset.PackageName.ToString());
			PyObject* PyPackageName = PyUnicode_FromString(TCHAR_TO_UTF8(*Asset.PackageName.ToString()));
			PyList_Append(UnreferencedList, PyPackageName);
			Py_DECREF(PyPackageName);
			continue;
		}
		if ((Referencers.Num() == 1) && (Referencers[0] == Asset.PackageName))
		{
			PyObject* PyPackageName = PyUnicode_FromString(TCHAR_TO_UTF8(*Asset.PackageName.ToString()));
			PyList_Append(UnreferencedList, PyPackageName);
			Py_DECREF(PyPackageName);
		}
	}
	return UnreferencedList;
}

PyObject* NePyMethod_GetReachableAssets(PyObject* InSelf, PyObject* InArgs)
{
	PyObject* RootList = nullptr;
	if (!PyArg_ParseTuple(InArgs, "O:GetReachableAssets", &RootList))
	{
		return nullptr;
	}
	if (!PyList_Check(RootList))
	{
		return PyErr_Format(PyExc_Exception, "Root List is not a valid List");
	}
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	int NumLines = PyList_Size(RootList);
	if (NumLines <= 0)
	{
		return PyErr_Format(PyExc_Exception, "Root paths is not a valid list");
	}

	TSet<FName> ReachableAssets;
	TQueue<FName> OpenList;
	for (int i = 0; i < NumLines; ++i)
	{
		PyObject* StrObj = PyList_GetItem(RootList, i);
#if PY_MAJOR_VERSION >= 3
		char* FileName = PyBytes_AS_STRING(PyUnicode_AsEncodedString(StrObj, "utf-8", "Error"));
#else
		char* FileName = PyString_AsString(FNePyObjectPtr::StealReference(PyObject_Str(StrObj)));
#endif
		OpenList.Enqueue(UTF8_TO_TCHAR(FileName));
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");

	while (!OpenList.IsEmpty())
	{
		FName FileName;
		OpenList.Dequeue(FileName);
		// 提前添加，避免循环引用导致循环往队列添加同一资源
		ReachableAssets.Add(FileName);
		TArray<FName> Dependencies;
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION < 26
		AssetRegistryModule.Get().GetDependencies(FileName, Dependencies, EAssetRegistryDependencyType::All);
#else
		AssetRegistryModule.Get().GetDependencies(FileName, Dependencies);
#endif
		for (FName Name : Dependencies)
		{
			if (ReachableAssets.Find(Name))
			{
				continue;
			}
			OpenList.Enqueue(Name);
		}
	}

	PyObject* AssetsList = PyList_New(0);
	for (FName Name : ReachableAssets)
	{
		PyObject* PyName = PyUnicode_FromString(TCHAR_TO_UTF8(*Name.ToString()));
		PyList_Append(AssetsList, PyName);
		Py_DECREF(PyName);
	}
	return AssetsList;
}

PyObject* NePyMethod_GetSelectedActors(PyObject* InSelf)
{
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	FNePyObjectPtr Ret = NePyNewReference(PyList_New(0));
	USelection* Selection = GEditor->GetSelectedActors();
	for (int32 I = 0; I < Selection->Num(); ++I)
	{
		UObject* Obj = Selection->GetSelectedObject(I);
		if (!Obj->IsA<AActor>())
		{
			continue;
		}
		FNePyObjectPtr PyActor = NePyStealReference(NePyBase::ToPy((AActor*)Obj));
		PyList_Append(Ret, PyActor);
	}
	return Ret;
}

PyObject* NePyMethod_SelectNone(PyObject* InSelf)
{
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	GEditor->SelectNone(true, true);
	Py_RETURN_TRUE;
}

PyObject* NePyMethod_SelectActor(PyObject* InSelf, PyObject* InArgs)
{
	if (!GEditor)
	{
		return PyErr_Format(PyExc_Exception, "no GEditor found");
	}
	PyObject* PyActor;
	AActor* Actor;
	bool bInSelected;
	bool bNotify;
	if (!PyArg_ParseTuple(InArgs, "Obb:SelectActor", &PyActor, &bInSelected, &bNotify))
	{
		return nullptr;
	}
	if (!NePyBase::ToCpp(PyActor, Actor))
	{
		PyErr_SetString(PyExc_TypeError, "'Actor' must have type 'AActor'");
		return nullptr;
	}
	GEditor->SelectActor(Actor, bInSelected, bNotify, true, true);
	Py_RETURN_TRUE;
}

#endif // WITH_EDITOR

// 是否正在以命令行形式启动UE
PyObject* NePyMethod_IsRunningCommandlet(PyObject* InSelf)
{
	if (IsRunningCommandlet())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

PyObject* NePyMethod_ReloadShaderByteCode(PyObject* InSelf)
{
	FShaderCodeLibrary::OpenLibrary("Global", FPaths::ProjectContentDir());
	FShaderCodeLibrary::OpenLibrary(FApp::GetProjectName(), FPaths::ProjectContentDir());
	UE_LOG(LogNePython, Log, TEXT("reload_shader_byte_code..."));
	Py_RETURN_TRUE;
}

PyObject* NePyMethod_ConvertWorldToViewPort(PyObject* InSelf, PyObject* InArgs)
{
#if !WITH_NEPY_AUTO_EXPORT
	return nullptr;
#else
	PyObject* PyArgs[3] = { nullptr, nullptr, nullptr };
	if (!PyArg_ParseTuple(InArgs, "OO|O:ConvertWorldToViewPort", &PyArgs[0], &PyArgs[1], &PyArgs[2]))
	{
		return nullptr;
	}

	UWorld* World = NePyBase::TryGetWorld(PyArgs[0]);
	if (!World)
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'WorldContextObject' must have type 'UObject'");
		return nullptr;
	}

	FVector WorldPosition;
	if (!NePyBase::ToCpp(PyArgs[1], WorldPosition))
	{
		PyErr_SetString(PyExc_TypeError, "arg2 'WorldPosition' must have type 'FVector'");
		return nullptr;
	}

	bool bPlayerViewportRelative = false;
	if (PyArgs[2] && !NePyBase::ToCpp(PyArgs[2], bPlayerViewportRelative))
	{
		PyErr_SetString(PyExc_TypeError, "arg3 'bPlayerViewportRelative' must have type 'bool'");
		return nullptr;
	}

	APlayerController* PlayerControl = World->GetFirstPlayerController();
	if (!PlayerControl)
	{
		UE_LOG(LogNePython, Warning, TEXT("ConvertWorldToViewPort Faied: get first player controller is None"));
		Py_RETURN_NONE;
	}

	FVector2D ScreenLocation;
	bool bSuccess = UGameplayStatics::ProjectWorldToScreen(PlayerControl, WorldPosition, ScreenLocation, bPlayerViewportRelative);
	if (!bSuccess)
	{
		// 正常情况也会投射失败，所以这里不要打印日志
		Py_RETURN_NONE;
	}

	FVector2D ViewportPosition2D;
	const FVector2D RoundedPosition2D(FMath::RoundToInt(ScreenLocation.X), FMath::RoundToInt(ScreenLocation.Y));
	USlateBlueprintLibrary::ScreenToViewport(PlayerControl, RoundedPosition2D, ViewportPosition2D);

	PyObject* PyPosition;
	NePyBase::ToPy(ViewportPosition2D, PyPosition);
	return PyPosition;
#endif
}

#if NEPY_USE_POST_GARBAGE_COLLECT
#if !UE_BUILD_SHIPPING
PyObject* NePyMethod_GetInvalidObjectInfos(PyObject* InSelf)
{
	TArray<TPair<FName, FName>> InvalidObjectInfos = FNePyHouseKeeper::Get().GetInvalidObjectInfos();
	PyObject* PyList = PyList_New(InvalidObjectInfos.Num());
	for (int32 Index = 0; Index < InvalidObjectInfos.Num(); ++Index)
	{
		auto& ClassNameAndObjectName = InvalidObjectInfos[Index];
		PyObject* PyItem = PyTuple_New(2);
		PyObject* PyClassName;
		NePyBase::ToPy(ClassNameAndObjectName.Key, PyClassName);
		PyTuple_SetItem(PyItem, 0, PyClassName);
		PyObject* PyObjectName;
		NePyBase::ToPy(ClassNameAndObjectName.Value, PyObjectName);
		PyTuple_SetItem(PyItem, 0, PyObjectName);
		PyList_SetItem(PyList, Index, PyItem);
	}
	return PyList;
}
#endif // !UE_BUILD_SHIPPING
#endif // NEPY_USE_POST_GARBAGE_COLLECT

// 顶层模块静态方法
PyMethodDef NePyTopModuleMethods[] = {
	{ "Log", (PyCFunction)NePyMethod_Log, METH_VARARGS, "(Message: str) -> None" },
	{ "LogWarning", (PyCFunction)NePyMethod_LogWarning, METH_VARARGS, "(Message: str) -> None" },
	{ "LogError", (PyCFunction)NePyMethod_LogError, METH_VARARGS, "(Message: str) -> None" },
	{ "AddOnScreenDebugMessage", (PyCFunction)NePyMethod_AddOnScreenDebugMessage, METH_VARARGS, "(Key: int, TimeToDisplay: float, Message: str) -> None" },
	{ "PrintString", (PyCFunction)NePyMethod_PrintString, METH_VARARGS, "(Message: str, TimeToDisplay: float = ..., Color: Color = ...) -> None" },
	{ "RequestExit", (PyCFunction)NePyMethod_RequestExit, METH_VARARGS, "(bForce: bool) -> None" },
	{ "GetEngineDir", (PyCFunction)NePyMethod_GetEngineDir, METH_NOARGS, "() -> str" },
	{ "GetEngineContentDir", (PyCFunction)NePyMethod_GetEngineContentDir, METH_NOARGS, "() -> str" },
	{ "GetEngineConfigDir", (PyCFunction)NePyMethod_GetEngineConfigDir, METH_NOARGS, "() -> str" },
	{ "GetProjectDir", (PyCFunction)NePyMethod_GetProjectDir, METH_NOARGS, "() -> str" },
	{ "GetContentDir", (PyCFunction)NePyMethod_GetContentDir, METH_NOARGS, "() -> str" },
	// { "GetDocumentDir", (PyCFunction)NePyMethod_GetDocumentDir, METH_NOARGS, "() -> str" },
	{ "GetConfigDir", (PyCFunction)NePyMethod_GetConfigDir, METH_NOARGS, "() -> str" },
	{ "GetLogDir", (PyCFunction)NePyMethod_GetLogDir, METH_NOARGS, "() -> str" },
	// { "GetLogFilename", (PyCFunction)NePyMethod_GetLogFilename, METH_NOARGS, "() -> str" },
	{ "GetGameSavedDir", (PyCFunction)NePyMethod_GetGameSavedDir, METH_NOARGS, "() -> str" },
	{ "GetGameUserDeveloperDir", (PyCFunction)NePyMethod_GetGameUserDeveloperDir, METH_NOARGS, "() -> str" },
	{ "ConvertRelativePathToFull", (PyCFunction)NePyMethod_ConvertRelativePathToFull, METH_VARARGS, "(Path: str) -> str" },
	{ "ConvertAbsolutePathApp", (PyCFunction)NePyMethod_ConvertAbsolutePathApp, METH_VARARGS, "(Path: str) -> str" },
	{ "ObjectPathToPackageName", (PyCFunction)NePyMethod_ObjectPathToPackageName, METH_VARARGS, "(Path: str) -> str" },
	{ "GetPath", (PyCFunction)NePyMethod_GetPath, METH_VARARGS, "(Path: str) -> str" },
	{ "GetBaseFilename", (PyCFunction)NePyMethod_GetBaseFilename, METH_VARARGS, "(Path: str) -> str" },
	{ "FindFile", (PyCFunction)NePyMethod_FindFile, METH_VARARGS, "(FullName: str, Path: str) -> bool" },
	{ "GetFile", (PyCFunction)NePyMethod_GetFile, METH_VARARGS, "(FullName: str, Path: str) -> bytes" },
	{ "RenderImgs", (PyCFunction)NePyMethod_RenderImgs, METH_VARARGS, "" },
#if WITH_EDITOR
	{ "EncryptBuffer", (PyCFunction)NePyMethod_EncryptBuffer, METH_VARARGS, "" },
#endif
	{ "NewModule", (PyCFunction)NePyMethod_NewModule, METH_VARARGS, "" },
	{ "MountPak", (PyCFunction)NePyMethod_MountPak, METH_VARARGS, "" },
	{ "CreateWorld", (PyCFunction)NePyMethod_CreateWorld, METH_VARARGS, "(WorldType: EWorldType = ...) -> World" },
	{ "ParsePropertyFlags", (PyCFunction)NePyMethod_ParsePropertyFlags, METH_VARARGS, "(Flags: int) -> typing.List[str]" },
	{ "GetTransientPackage", (PyCFunction)NePyMethod_GetTransientPackage, METH_NOARGS, "() -> Package" },
	{ "GetIniFilenameFromObjectsReference", (PyCFunction)NePyMethod_GetIniFilenameFromObjectsReference, METH_O, "(ObjectsReferenceString: str) -> str" },
	{ "ResolveIniObjectsReference", (PyCFunction)NePyMethod_ResolveIniObjectsReference, METH_O, "(ObjectReference: str, IniFilename: str = ..., bThrow: bool = ...) -> str" },
	{ "LoadPackage", (PyCFunction)NePyMethod_LoadPackage, METH_VARARGS, "(Name: str) -> Package" },
	{ "LoadPackageAsync", (PyCFunction)NePyMethod_LoadPackageAsync, METH_VARARGS, "(Name: str, Callback: typing.Callable[[str, Package, EAsyncLoadingResult], typing.Any], Priority: int = ..., PackageFlags: EPackageFlags = ...) -> None" },
#if WITH_EDITOR
	{ "UnloadPackage", (PyCFunction)NePyMethod_UnloadPackage, METH_VARARGS, "(PackageToUnload: Package) -> None" },
	{ "GetPackageFileName", (PyCFunction)NePyMethod_GetPackageFileName, METH_VARARGS, "(Name: str) -> str" },
#endif
	{ "FindClass", (PyCFunction)NePyMethod_FindClass, METH_VARARGS, "(Name: str) -> Class" },
	{ "LoadClass", (PyCFunction)NePyMethod_LoadClass, METH_VARARGS, "(Name: str) -> Class" },
	{ "AsyncLoadClass", (PyCFunction)NePyMethod_AsyncLoadClass, METH_VARARGS, "(Name: str, Callback: typing.Callable[[str, Class], typing.Any], Priority: int = ...) -> None" },
	{ "FindObject", (PyCFunction)NePyMethod_FindObject, METH_VARARGS, "(Name: str) -> Object" },
	{ "LoadObject", (PyCFunction)NePyMethod_LoadObject, METH_VARARGS, "(Class: Class, Name: str) -> Object" },
	{ "AsyncLoadObject", (PyCFunction)NePyMethod_AsyncLoadObject, METH_VARARGS, "(Name: str, Callback: typing.Callable[[str, Object], typing.Any], Priority: int = ...) -> None" },
	{ "FindStruct", (PyCFunction)NePyMethod_FindStruct, METH_VARARGS, "(Name: str) -> Object" },
	{ "LoadStruct", (PyCFunction)NePyMethod_LoadStruct, METH_VARARGS, "(Name: str) -> Object" },
	{ "FindEnum", (PyCFunction)NePyMethod_FindEnum, METH_VARARGS, "(Name: str) -> Object" },
	{ "LoadEnum", (PyCFunction)NePyMethod_LoadEnum, METH_VARARGS, "(Name: str) -> Object" },
	{ "CancelAsyncLoading", (PyCFunction)NePyMethod_CancelAsyncLoading, METH_NOARGS, "() -> None" },
	{ "GetAsyncLoadPercentage", (PyCFunction)NePyMethod_GetAsyncLoadPercentage, METH_O, "(PackageName: str) -> float" },
	{ "FlushAsyncLoading", (PyCFunction)NePyMethod_FlushAsyncLoading, METH_VARARGS, "(PackageID: int = ...) -> None" },
	{ "GetNumAsyncPackages", (PyCFunction)NePyMethod_GetNumAsyncPackages, METH_NOARGS, "() -> int" },
	{ "IsLoading", (PyCFunction)NePyMethod_IsLoading, METH_NOARGS, "() -> bool" },
	{ "FindPackage", (PyCFunction)NePyMethod_FindPackage, METH_VARARGS, "(PackageName: str) -> Package" },
	{ "CreatePackage", (PyCFunction)NePyMethod_CreatePackage, METH_VARARGS, "(PackageName: str) -> Package" },
	{ "NewObject", (PyCFunction)NePyMethod_NewObject, METH_VARARGS, "(Class: Class, Outer: Object = ..., Name: str = ..., Flags: int = ...) -> Object" },
	{ "StringToGuid", (PyCFunction)NePyMethod_StringToGuid, METH_VARARGS, "(Str: str) -> Guid" },
	{ "GuidToString", (PyCFunction)NePyMethod_GuidToString, METH_VARARGS, "(Guid: Guid) -> str" },
	{ "TickSlate", (PyCFunction)NePyMethod_TickSlate, METH_NOARGS, "() -> None" },
	{ "TickEngine", (PyCFunction)NePyMethod_TickEngine, METH_VARARGS, "(DeltaSeconds: float, bIdle: bool) -> None" },
#if WITH_EDITOR
	{ "TickRenderingTickables", (PyCFunction)NePyMethod_TickRenderingTickables, METH_NOARGS, "() -> None" },
#endif
	{ "GetDeltaTime", (PyCFunction)NePyMethod_GetDeltaTime, METH_NOARGS, "() -> float" },
	{ "GetAllWorlds", (PyCFunction)NePyMethod_GetAllWorlds, METH_NOARGS, "() -> typing.List[World]" },
	{ "GetGameViewportSize", (PyCFunction)NePyMethod_GetGameViewportSize, METH_NOARGS, "() -> typing.Tuple[float, float]" },
	{ "GetResolution", (PyCFunction)NePyMethod_GetResolution, METH_NOARGS, "() -> typing.Tuple[float, float]" },
	{ "GetViewportScreenshot", (PyCFunction)NePyMethod_GetViewportScreenshot, METH_VARARGS, "(bAsIntList: bool) -> typing.List[Color] | typing.List[typing.Tuple[float, float, float, float]]" },
	{ "GetStatUnit", (PyCFunction)NePyMethod_GetStatUnit, METH_NOARGS, "() -> typing.Dict" },
	{ "GetStatFps", (PyCFunction)NePyMethod_GetStatFps, METH_NOARGS, "()-> typing.float" },
	{ "GetStatRhi", (PyCFunction)NePyMethod_GetStatRhi, METH_NOARGS, "() -> typing.Dict" },
	{ "CopyPropertiesForUnrelatedObjects", NePyCFunctionCast(NePyMethod_CopyPropertiesForUnrelatedObjects), METH_VARARGS | METH_KEYWORDS, "" },
	{ "SetRandomSeed", (PyCFunction)NePyMethod_SetRandomSeed, METH_VARARGS, "(Seed: int) -> None" },
	{ "ClipboardCopy", (PyCFunction)NePyMethod_ClipboardCopy, METH_VARARGS, "(Text: str) -> None" },
	{ "ClipboardPaste", (PyCFunction)NePyMethod_ClipboardPaste, METH_NOARGS, "() -> str" },
#if WITH_EDITOR
	{ "DumpReflectionInfos", NePyMethodDumpReflectionInfos, METH_VARARGS, "" },
	{ "GetEditorWorld", (PyCFunction)NePyMethodGetEditorWorld, METH_NOARGS, "() -> World" },
	{ "DeleteAsset", NePyMethod_DeleteAsset, METH_VARARGS, "(Path: str, bShowConfirmation: bool = ...) -> None" },
	{ "DeleteAssets", NePyMethod_DeleteAssets, METH_VARARGS, "(Paths: typing.List[str]) -> None " },
	{ "GetAssets", NePyMethod_GetAssets, METH_VARARGS, "" },
	{ "GetAllAssetsPath", NePyMethod_GetAllAssetsPath, METH_VARARGS, "" },
	{ "GetAssetUnused", NePyMethod_GetAssetUnused, METH_VARARGS, "" },
	{ "GetAssetUnreferenced", NePyMethod_GetAssetUnreferenced, METH_VARARGS, "" },
	{ "GetReachableAssets", NePyMethod_GetReachableAssets, METH_VARARGS, "" },
	{ "GetSelectedActors", (PyCFunction)NePyMethod_GetSelectedActors, METH_VARARGS, "" },
	{ "SelectNone", (PyCFunction)NePyMethod_SelectNone, METH_VARARGS, "" },
	{ "SelectActor", (PyCFunction)NePyMethod_SelectActor, METH_VARARGS, "(Actor: AActor, bInSelected:bool, bNotify:bool) -> bool" },

#endif
	{ "IsRunningCommandlet", (PyCFunction)NePyMethod_IsRunningCommandlet, METH_NOARGS, "() -> bool" },
	{ "ReloadShaderByteCode", (PyCFunction)NePyMethod_ReloadShaderByteCode, METH_NOARGS, "() -> None" },
	{ "ConvertWorldToViewport", NePyMethod_ConvertWorldToViewPort, METH_VARARGS, "(World: World, WorldPosition: Vector, bPlayerViewportRelative: bool = ...) -> Vector" },
#if NEPY_USE_POST_GARBAGE_COLLECT
#if !UE_BUILD_SHIPPING
	{ "GetInvalidObjectInfos", (PyCFunction)NePyMethod_GetInvalidObjectInfos, METH_NOARGS, "() -> typing.List[]" },
#endif // !UE_BUILD_SHIPPING
#endif // NEPY_USE_POST_GARBAGE_COLLECT

	{NULL, NULL}  /* Sentinel */
};
