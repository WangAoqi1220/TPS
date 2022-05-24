#pragma once
#include "NePyBase.h"
//#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"
#include "Components/InputComponent.h"
#include "NePyDelegate.h"
#include "NePyAutoExport.h"

PyObject* NePyInputComponent_GetAxisValue(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	char* AxisName;
	if (!PyArg_ParseTuple(InArgs, "s:GetAxisValue", &AxisName))
	{
		return nullptr;
	}

	UInputComponent* InputComponent = (UInputComponent*)InSelf->Value;
	return Py_BuildValue("f", InputComponent->GetAxisValue(FName(UTF8_TO_TCHAR(AxisName))));
}

PyObject* NePyInputComponent_GetAxisKeyValue(FNePyObjectBase* InSelf, PyObject* InArg)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	FKey AxisKey;
	if (FNePyStruct_Key* PyAxisKey = NePyStructCheck_Key(InArg))
	{
		AxisKey = PyAxisKey->Value;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "argument type must be 'FKey'");
		return nullptr;
	}

	UInputComponent* InputComponent = (UInputComponent*)InSelf->Value;
	return Py_BuildValue("f", InputComponent->GetAxisKeyValue(AxisKey));
}


PyObject* NePyInputComponent_GetVectorAxisValue(FNePyObjectBase* InSelf, PyObject* InArg)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	FKey AxisKey;
	if (FNePyStruct_Key* PyAxisKey = NePyStructCheck_Key(InArg))
	{
		AxisKey = PyAxisKey->Value;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "argument type must be 'FKey'");
		return nullptr;
	}

	UInputComponent* InputComponent = (UInputComponent*)InSelf->Value;
	FVector AxisValue = InputComponent->GetVectorAxisValue(AxisKey);
	return NePyStructNew_Vector(AxisValue);
}

PyObject* NePyInputComponent_HasBindings(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	UInputComponent* InputComponent = (UInputComponent*)InSelf->Value;
	if (InputComponent->HasBindings())
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

PyObject* NePyInputComponent_BindAction(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	char* ActionNameStr;
	int Key;
	PyObject* PyCallable;
	if (!PyArg_ParseTuple(InArgs, "siO:BindAction", &ActionNameStr, &Key, &PyCallable))
	{
		return nullptr;
	}

	if (!PyCallable_Check(PyCallable))
	{
		return PyErr_Format(PyExc_Exception, "object is not a callable");
	}

	UInputComponent* InputComponent = (UInputComponent *)InSelf->Value;
	if (!InputComponent)
	{
		return PyErr_Format(PyExc_Exception, "InputComponent is nullptr");
	}

	FName ActionName(UTF8_TO_TCHAR(ActionNameStr));
	UNePyDelegate* PyDelegate = FNePyHouseKeeper::Get().NewDelegate(InputComponent, PyCallable, nullptr, ActionName);
	FInputActionBinding ActionBinding(ActionName, (const EInputEvent)Key);
	ActionBinding.ActionDelegate.BindDelegate(PyDelegate, &UNePyDelegate::PyInputHandler);
	InputComponent->AddActionBinding(ActionBinding);

	Py_RETURN_NONE;
}

PyObject* NePyInputComponent_BindAxis(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	char* AxisNameStr;
	PyObject* PyCallable;
	if (!PyArg_ParseTuple(InArgs, "sO:BindAxis", &AxisNameStr, &PyCallable))
	{
		return nullptr;
	}

	if (!PyCallable_Check(PyCallable))
	{
		return PyErr_Format(PyExc_Exception, "object is not a callable");
	}

	UInputComponent* InputComponent = (UInputComponent*)InSelf->Value;
	if (!InputComponent)
	{
		return PyErr_Format(PyExc_Exception, "InputComponent is nullptr");
	}

	FName AxisName(UTF8_TO_TCHAR(AxisNameStr));
	UNePyDelegate* PyDelegate = FNePyHouseKeeper::Get().NewDelegate(InputComponent, PyCallable, nullptr, AxisName);
	FInputAxisBinding AxisBinding(AxisName);
	AxisBinding.AxisDelegate.BindDelegate(PyDelegate, &UNePyDelegate::PyInputAxisHandler);
	InputComponent->AxisBindings.Add(AxisBinding);

	Py_RETURN_NONE;
}

//
//PyObject* NePyInputComponent_BindKey(FNePyObjectBase* InSelf, PyObject* InArgs)
//{
//
//	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
//	{
//		return nullptr;
//	}
//
//	char* key_name;
//	int key;
//	PyObject* PyCallable;
//	if (!PyArg_ParseTuple(InArgs, "siO:BindKey", &key_name, &key, &PyCallable))
//	{
//		return nullptr;
//	}
//
//	if (!PyCallable_Check(PyCallable))
//	{
//		return PyErr_Format(PyExc_Exception, "object is not a callable");
//	}
//
//	UInputComponent* InputComponent = nullptr;
//
//	if (InSelf->Value->IsA<AActor>())
//	{
//		InputComponent = ((AActor*)InSelf->Value)->InputComponent;
//	}
//	else if (InSelf->Value->IsA<UActorComponent>())
//	{
//		UActorComponent* component = (UActorComponent*)InSelf->Value;
//		if (!component->GetOwner())
//			return PyErr_Format(PyExc_Exception, "component is still not mapped to an Actor");
//		InputComponent = component->GetOwner()->InputComponent;
//	}
//	else
//	{
//		return PyErr_Format(PyExc_Exception, "uobject is not an actor or a component");
//	}
//
//	if (!InputComponent)
//	{
//		return PyErr_Format(PyExc_Exception, "no InputComponent manager for this uobject");
//	}
//
//	UPythonDelegate* PyDelegate = FUnrealEnginePythonHouseKeeper::Get()->NewDelegate(InputComponent, PyCallable, nullptr);
//
//	FInputKeyBinding InputKey_binding(FKey(UTF8_TO_TCHAR(key_name)), (const EInputEvent)key);
//	InputKey_binding.KeyDelegate.BindDelegate(PyDelegate, &UPythonDelegate::PyInputHandler);
//	InputComponent->KeyBindings.Add(InputKey_binding);
//
//	Py_RETURN_NONE;
//
//}
//
//PyObject* NePyInputComponent_BindPressedKey(FNePyObjectBase* InSelf, PyObject* InArgs)
//{
//	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
//	{
//		return nullptr;
//	}
//	char* key_name;
//	PyObject* PyCallable;
//	if (!PyArg_ParseTuple(InArgs, "sO:BindPressedKey", &key_name, &PyCallable))
//	{
//		return nullptr;
//	}
//	FStealRefPyObj pykey = Py_BuildValue("siO", key_name, EInputEvent::IE_Pressed, PyCallable);
//	return NePyInputComponent_BindKey(InSelf, pykey);
//}
//
//PyObject* NePyInputComponent_BindReleasedKey(FNePyObjectBase* InSelf, PyObject* InArgs)
//{
//	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
//	{
//		return nullptr;
//	}
//	char* key_name;
//	PyObject* PyCallable;
//	if (!PyArg_ParseTuple(InArgs, "sO:BindReleasedKey", &key_name, &PyCallable))
//	{
//		return nullptr;
//	}
//	FStealRefPyObj pykey = Py_BuildValue("siO", key_name, EInputEvent::IE_Released, PyCallable);
//	return NePyInputComponent_BindKey(InSelf, pykey);
//}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"GetAxisValue", NePyCFunctionCast(&NePyInputComponent_GetAxisValue), METH_VARARGS, "(self, AxisName: str) -> float"}, \
{"GetAxisKeyValue", NePyCFunctionCast(&NePyInputComponent_GetAxisKeyValue), METH_O, "(self, AxisKey: Key) -> float"}, \
{"GetVectorAxisValue", NePyCFunctionCast(&NePyInputComponent_GetVectorAxisValue), METH_O, "(self, AxisKey: Key) -> Vector"}, \
{"HasBindings", NePyCFunctionCast(&NePyInputComponent_HasBindings), METH_NOARGS, "(self) -> bool"}, \
{"BindAction", NePyCFunctionCast(&NePyInputComponent_BindAction), METH_VARARGS, "(self, ActionName: str, Key: int, Callback: typing.Callable) -> None"}, \
{"BindAxis", NePyCFunctionCast(&NePyInputComponent_BindAxis), METH_VARARGS, "(self, AxisName: str, Callback: typing.Callable) -> None"}, \
