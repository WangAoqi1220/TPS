#pragma once
#include "NePyBase.h"
#include "NePyGameInstance.h"
#include "Engine/GameInstance.h"
#include "NePyAutoExport.h"

PyObject* NePyGameInstance_GetPyProxy(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	if (!InSelf->Value->IsA(UNePyGameInstance::StaticClass()))
	{
		PyErr_Format(PyExc_RuntimeError, "self(%p) is not a UNePyGameInstance", InSelf);
		return nullptr;
	}

	UNePyGameInstance* GameInstance = (UNePyGameInstance*)InSelf->Value;
	PyObject* PyInstance = GameInstance->GetPyProxy();
	if (PyInstance)
	{
		Py_INCREF(PyInstance);
		return PyInstance;
	}

	Py_RETURN_NONE;
}

PyObject* NePyGameInstance_GetWorld(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	UGameInstance* GameInstance = (UGameInstance*)InSelf->Value;
	UWorld* World = GameInstance->GetWorld();
	PyObject* PyWorld = NePyBase::ToPy(World);
	return PyWorld;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"GetPyProxy", NePyCFunctionCast(&NePyGameInstance_GetPyProxy), METH_NOARGS, "(self) -> object"}, \
{"GetWorld", NePyCFunctionCast(&NePyGameInstance_GetWorld), METH_NOARGS, "(self) -> World"}, \

