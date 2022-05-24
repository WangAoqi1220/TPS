#pragma once
#include "NePyBase.h"
#include "NePyHouseKeeper.h"
#include "NePyAutoExport.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

PyObject* FNePyWidgetBlueprintLibrary_Create(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	PyObject* PyArgs[3] = { nullptr, nullptr, nullptr };
	if (!PyArg_ParseTuple(InArgs, "OO|O:Create", &PyArgs[0], &PyArgs[1], &PyArgs[2]))
	{
		return nullptr;
	}

	UObject* WorldContextObject;
	if (FNePyObject_Object* PyWorldContextObject = NePyObjectCheck_Object(PyArgs[0]))
	{
		if (FNePyHouseKeeper::Get().IsValid(PyWorldContextObject))
		{
			WorldContextObject = PyWorldContextObject->GetValue();
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "arg1 'WorldContextObject' underlying UObject is invalid");
			return nullptr;
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'WorldContextObject' must have type 'Object'");
		return nullptr;
	}

	UClass* WidgetType;
	if (!NePyBase::ToCpp(PyArgs[1], WidgetType, UUserWidget::StaticClass()))
	{
		PyErr_SetString(PyExc_TypeError, "arg2 'WidgetType' must have type 'UClass<UserWidget>'");
		return nullptr;
	}

	APlayerController* OwningPlayer = nullptr;
	if (PyArgs[2])
	{
		if (FNePyObject_PlayerController * PyOwningPlayer = NePyObjectCheck_PlayerController(PyArgs[2]))
		{
			if (FNePyHouseKeeper::Get().IsValid(PyOwningPlayer))
			{
				OwningPlayer = PyOwningPlayer->GetValue();
			}
			else
			{
				PyErr_SetString(PyExc_TypeError, "arg3 'OwningPlayer' underlying UObject is invalid");
				return nullptr;
			}
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "arg3 'OwningPlayer' must have type 'PlayerController'");
			return nullptr;
		}
	}

	auto RetVal = UWidgetBlueprintLibrary::Create(WorldContextObject, WidgetType, OwningPlayer);
	PyObject* PyRetVal0 = NePyBase::ToPy(RetVal);
	return PyRetVal0;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"Create", NePyCFunctionCast(&FNePyWidgetBlueprintLibrary_Create), METH_VARARGS | METH_CLASS, "(WorldContext: Object, WidgetType: Class, OwningPlayer: PlayerController = ...) -> UserWidget"}, \

