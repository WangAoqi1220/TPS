#pragma once
#include "NePyBase.h"
#include "NePyCharacter.h"

PyObject* NePyCharacter_GetPyProxy(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	if (!InSelf->Value->IsA(ANePyCharacter::StaticClass()))
	{
		PyErr_Format(PyExc_RuntimeError, "self(%p) is not a ANePyCharacter", InSelf);
		return nullptr;
	}

	ANePyCharacter* Character = (ANePyCharacter*)InSelf->Value;
	PyObject* PyInstance = Character->GetPyProxy();
	if (PyInstance)
	{
		Py_INCREF(PyInstance);
		return PyInstance;
	}

	Py_RETURN_NONE;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"GetPyProxy", NePyCFunctionCast(&NePyCharacter_GetPyProxy), METH_NOARGS, "(self) -> object"}, \
