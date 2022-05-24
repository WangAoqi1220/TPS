#pragma once
#include "UObject/ObjectMacros.h"
#include "Components/ActorComponent.h"
#include "Engine/UserDefinedEnum.h"
#include "Containers/StringConv.h"
#include "NePyBase.h"
#include "NePyObjectBase.h"
#include "NePyHouseKeeper.h"

PyObject* NePyObject_GetUniqueID(FNePyObjectBase* InSelf)
{
	uint32 UniqueID = 0xffffffff;
	if (FNePyHouseKeeper::Get().IsValid(InSelf))
	{
		UniqueID = InSelf->Value->GetUniqueID();
	}

	return PyLong_FromUnsignedLongLong(UniqueID);
}

PyObject* NePyObject_GetClass(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	return NePyBase::ToPy(InSelf->Value->GetClass());
}

PyObject* NePyObject_GetFlags(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	return PyLong_FromUnsignedLongLong((uint64)InSelf->Value->GetFlags());
}

PyObject* NePyObject_SetFlags(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	uint64 Flags;
	PyObject* PyReset = nullptr;
	if (!PyArg_ParseTuple(InArgs, "K|O", &Flags, &PyReset))
	{
		return nullptr;
	}

	if (PyReset && PyObject_IsTrue(PyReset))
	{
		InSelf->Value->ClearFlags(InSelf->Value->GetFlags());
	}

	InSelf->Value->SetFlags((EObjectFlags)Flags);
	Py_RETURN_NONE;
}

PyObject* NePyObject_CreateDefaultSubobject(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	PyObject* PyClass;
	char* Name;
	if (!PyArg_ParseTuple(InArgs, "Os", &PyClass, &Name))
	{
		return nullptr;
	}

	UClass* Class = NePyBase::ToCppClass(PyClass, UObject::StaticClass());
	if (!Class)
	{
		return PyErr_Format(PyExc_Exception, "argument is not a UClass derived from UObject");
	}

	UObject* Ret = InSelf->Value->CreateDefaultSubobject(FName(UTF8_TO_TCHAR(Name)), Class, Class, true, false);
	return NePyBase::ToPy(Ret);
}

PyObject* NePyObject_ClearFlags(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	uint64 Flags;
	if (!PyArg_ParseTuple(InArgs, "K", &Flags))
	{
		return nullptr;
	}

	InSelf->Value->ClearFlags((EObjectFlags)Flags);
	Py_RETURN_NONE;
}

PyObject* NePyObject_ResetFlags(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	InSelf->Value->ClearFlags(InSelf->Value->GetFlags());
	Py_RETURN_NONE;
}

PyObject* NePyObject_GetOuter(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	UObject* Outer = InSelf->Value->GetOuter();
	if (!Outer)
	{
		Py_RETURN_NONE;
	}

	return NePyBase::ToPy(Outer);
}

PyObject* NePyObject_SetOuter(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	PyObject* PyOuter;
	if (!PyArg_ParseTuple(InArgs, "O", &PyOuter))
	{
		return nullptr;
	}

	UPackage* Package = NePyBase::ToCppObject<UPackage>(PyOuter);
	if (!Package)
	{
		return PyErr_Format(PyExc_Exception, "argument is not a UPackage");
	}

	if (!InSelf->Value->Rename(nullptr, Package, REN_Test))
	{
		return PyErr_Format(PyExc_Exception, "cannot move to package %s", TCHAR_TO_UTF8(*Package->GetPathName()));
	}

	if (InSelf->Value->Rename(nullptr, Package))
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

PyObject* NePyObject_GetOutermost(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	UObject* Outermost = InSelf->Value->GetOutermost();
	if (!Outermost)
	{
		Py_RETURN_NONE;
	}

	return NePyBase::ToPy(Outermost);
}

PyObject* NePyObject_IsValid(FNePyObjectBase* InSelf)
{
	if (FNePyHouseKeeper::Get().IsValid(InSelf))
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

PyObject* NePyObject_IsA(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	PyObject* PyObj;
	if (!PyArg_ParseTuple(InArgs, "O", &PyObj))
	{
		return nullptr;
	}

	UClass* Class = nullptr;
	if (!NePyBase::ToCpp(PyObj, Class, nullptr))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'InType' must have type 'Class'");
		return nullptr;
	}

	if (InSelf->Value->IsA(Class))
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

PyObject* NePyObject_GetName(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	return PyUnicode_FromString(TCHAR_TO_UTF8(*(InSelf->Value->GetName())));
}

PyObject* NePyObject_SetName(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	char* Name;
	if (!PyArg_ParseTuple(InArgs, "s", &Name))
	{
		return nullptr;
	}

	if (!InSelf->Value->Rename(UTF8_TO_TCHAR(Name), InSelf->Value->GetOutermost(), REN_Test))
	{
		return PyErr_Format(PyExc_Exception, "cannot set name %s", Name);
	}

	if (InSelf->Value->Rename(UTF8_TO_TCHAR(Name)))
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

PyObject* NePyObject_GetFullName(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	return PyUnicode_FromString(TCHAR_TO_UTF8(*(InSelf->Value->GetFullName())));
}

PyObject* NePyObject_GetPathName(FNePyObjectBase* InSelf)
{

	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	return PyUnicode_FromString(TCHAR_TO_UTF8(*(InSelf->Value->GetPathName())));
}

PyObject* NePyObject_BindEvent(FNePyObjectBase* InSelf, PyObject* InArgs)
{

	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	char* EventName;
	PyObject* PyCallable;
	if (!PyArg_ParseTuple(InArgs, "sO:BindEvent", &EventName, &PyCallable))
	{
		return nullptr;
	}

	if (!PyCallable_Check(PyCallable))
	{
		return PyErr_Format(PyExc_Exception, "object is not a callable");
	}

	return NePyObjectBaseBindEvent(InSelf, FString(EventName), PyCallable, true);
}

PyObject* NePyObject_UnbindEvent(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	char* EventName;
	PyObject* PyCallable;
	if (!PyArg_ParseTuple(InArgs, "sO:UnbindEvent", &EventName, &PyCallable))
	{
		return nullptr;
	}

	if (!PyCallable_Check(PyCallable))
	{
		return PyErr_Format(PyExc_Exception, "object is not a callable");
	}

	return NePyObjectBaseUnbindEvent(InSelf, FString(EventName), PyCallable, true);
}

PyObject* NePyObject_UnbindAllEvent(FNePyObjectBase* InSelf, PyObject* InArg)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	FString EventName;
	if (!NePyBase::ToCpp(InArg, EventName))
	{
		return PyErr_Format(PyExc_Exception, "argument is not a str");
	}

	return NePyObjectBaseUnbindAllEvent(InSelf, EventName, true);
}

PyObject* NePyObject_SetProperty(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	char* PropName;
	PyObject* PropValue;
	int Index = 0;
	if (!PyArg_ParseTuple(InArgs, "sO|i:SetProperty", &PropName, &PropValue, &Index))
	{
		return nullptr;
	}

	UStruct* Struct = nullptr;
	if (InSelf->Value->IsA<UStruct>())
	{
		Struct = (UStruct*)InSelf->Value;
	}
	else
	{
		Struct = (UStruct*)InSelf->Value->GetClass();
	}

	auto* Prop = Struct->FindPropertyByName(FName(UTF8_TO_TCHAR(PropName)));
	if (!Prop)
	{
		return PyErr_Format(PyExc_Exception, "unable to find property %s", PropName);
	}

	if (!NePyBase::TryConvertPyObjectToFPropertyInContainer(PropValue, Prop, InSelf->Value, Index))
	{
		return PyErr_Format(PyExc_Exception, "unable to set property %s", PropName);
	}

	Py_RETURN_NONE;
}

// 判断UE4对象当前是否由Python来管理生命周期
// 如果对象由Python管理生命周期，则只要Python持有着该对象，它就不会被UE4垃圾回收掉
PyObject* NePyObject_IsOwnedByPython(FNePyObjectBase* InSelf)
{
	if (FNePyHouseKeeper::Get().IsOwnedByPython(InSelf))
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

// 将所有权转移给Python，让Python来管理对象的生命周期
PyObject* NePyObject_OwnByPython(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	FNePyHouseKeeper::Get().ChangeOwnershipToPython(InSelf);
	Py_RETURN_NONE;
}

// 将所有权转移给UE4，让UE4来管理对象的生命周期
PyObject* NePyObject_DisownByPython(FNePyObjectBase* InSelf)
{
	FNePyHouseKeeper::Get().ChangeOwnershipToCpp(InSelf);
	Py_RETURN_NONE;
}

PyObject* NePyObject_AsDict(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	UStruct* Struct = nullptr;
	UObject* ObjectPtr = InSelf->Value;

	if (ObjectPtr->IsA<UStruct>())
	{
		Struct = (UStruct*)ObjectPtr;
		if (ObjectPtr->IsA<UClass>())
		{
			UClass* ClassPtr = (UClass*)ObjectPtr;
			ObjectPtr = ClassPtr->GetDefaultObject();
		}
	}
	else
	{
		Struct = (UStruct*)ObjectPtr->GetClass();
	}

	PyObject* PyStructDict = PyDict_New();
	TFieldIterator<FProperty> SArgs(Struct);
	for (; SArgs; ++SArgs)
	{
		PyObject* PyStruct = NePyBase::TryConvertFPropertyToPyObjectInContainer(*SArgs, ObjectPtr, 0);
		if (!PyStruct)
		{
			Py_DECREF(PyStructDict);
			return nullptr;
		}
		PyDict_SetItemString(PyStructDict, TCHAR_TO_UTF8(*SArgs->GetName()), PyStruct);
		Py_DECREF(PyStruct);
	}
	return PyStructDict;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"GetUniqueID", NePyCFunctionCast(&NePyObject_GetUniqueID), METH_NOARGS, "(self) -> int"}, \
{"GetClass", NePyCFunctionCast(&NePyObject_GetClass), METH_NOARGS, "(self) -> Class"}, \
{"GetFlags", NePyCFunctionCast(&NePyObject_GetFlags), METH_NOARGS, "(self) -> int"}, \
{"SetFlags", NePyCFunctionCast(&NePyObject_SetFlags), METH_VARARGS, "(self, Flags: int, Reset: bool) -> None"}, \
{"CreateDefaultSubobject", NePyCFunctionCast(&NePyObject_CreateDefaultSubobject), METH_VARARGS, "(self, Class: Class, Name: str) -> Object"}, \
{"ClearFlags", NePyCFunctionCast(&NePyObject_ClearFlags), METH_VARARGS, "(Flags: int) -> None"}, \
{"ResetFlags", NePyCFunctionCast(&NePyObject_ResetFlags), METH_NOARGS, "(self) -> None"}, \
{"GetOuter", NePyCFunctionCast(&NePyObject_GetOuter), METH_NOARGS, "(self) -> Object"}, \
{"SetOuter", NePyCFunctionCast(&NePyObject_SetOuter), METH_VARARGS, "(self, InPackage: Package) -> bool"}, \
{"GetOutermost", NePyCFunctionCast(&NePyObject_GetOutermost), METH_NOARGS, "(self) -> Package"}, \
{"IsValid", NePyCFunctionCast(&NePyObject_IsValid), METH_NOARGS, "(self) -> bool"}, \
{"IsA", NePyCFunctionCast(&NePyObject_IsA), METH_VARARGS, "(self, InClass: Class) -> bool"}, \
{"GetName", NePyCFunctionCast(&NePyObject_GetName), METH_NOARGS, "(self) -> str"}, \
{"SetName", NePyCFunctionCast(&NePyObject_SetName), METH_VARARGS, "(self, InName: str) -> bool"}, \
{"GetFullName", NePyCFunctionCast(&NePyObject_GetFullName), METH_NOARGS, "(self) -> str"}, \
{"GetPathName", NePyCFunctionCast(&NePyObject_GetPathName), METH_NOARGS, "(self) -> str"}, \
{"BindEvent", NePyCFunctionCast(&NePyObject_BindEvent), METH_VARARGS, "(self, EventName: str, Callback: typing.Callable) -> None"}, \
{"UnbindEvent", NePyCFunctionCast(&NePyObject_UnbindEvent), METH_VARARGS, "(self, EventName: str, Callback: typing.Callable) -> None"}, \
{"UnbindAllEvent", NePyCFunctionCast(&NePyObject_UnbindAllEvent), METH_O, "(self, EventName: str) -> None"}, \
{"SetProperty", NePyCFunctionCast(&NePyObject_SetProperty), METH_VARARGS, "(self, PropName: str, PropValue: typing.Any) -> None"}, \
{"IsOwnedByPython", NePyCFunctionCast(&NePyObject_IsOwnedByPython), METH_NOARGS, "(self) -> bool"}, \
{"OwnByPython", NePyCFunctionCast(&NePyObject_OwnByPython), METH_NOARGS, "(self) -> None"}, \
{"DisownByPython", NePyCFunctionCast(&NePyObject_DisownByPython), METH_NOARGS, "(self) -> None"}, \
{"AsDict", NePyCFunctionCast(&NePyObject_AsDict), METH_NOARGS, "(self) -> typing.Dict[str, typing.Any]"}, \
