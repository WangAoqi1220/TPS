#pragma once
#include "NePyBase.h"
#include "GameFramework/Actor.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "NePyUserWidget.h"
#include "NePyTemplate.h"

PyObject* NePyUserWidget_SetPythonInstance(FNePyObjectBase* InSelf, PyObject* InArg)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	PyObject* PyInstance;
	if (!PyArg_ParseTuple(InArg, "O:SetPythonInstance", &PyInstance))
	{
		return nullptr;
	}
	if (!InSelf->Value->IsA(UNePyUserWidget::StaticClass()))
	{
		PyErr_Format(PyExc_RuntimeError, "self(%p) is not a UNePyUserWidget", InSelf);
		return nullptr;
	}

	UNePyUserWidget* UserWidget = (UNePyUserWidget*)InSelf->Value;
	UserWidget->SetPythonInstance(PyInstance);

	Py_RETURN_NONE;
}

PyObject* NePyUserWidget_NativeConstruct(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	if (!InSelf->Value->IsA(UNePyUserWidget::StaticClass()))
	{
		PyErr_Format(PyExc_RuntimeError, "self(%p) is not a UNePyUserWidget", InSelf);
		return nullptr;
	}

	UNePyUserWidget* UserWidget = (UNePyUserWidget*)InSelf->Value;
	UserWidget->NativeConstruct();

	Py_RETURN_NONE;
}

PyObject* NePyUserWidget_GetWidgetFromName(FNePyObjectBase* InSelf, PyObject* InArg)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	PyObject* PyName;
	if (!PyArg_ParseTuple(InArg, "O:GetWidgetFromName", &PyName))
	{
		return nullptr;
	}

	FName WidgetName;
	if (!NePyBase::ToCpp(PyName, WidgetName))
	{
		PyErr_SetString(PyExc_TypeError, "arg1 'WidgetName' must have type 'FName'");
		return nullptr;
	}

	UUserWidget* UserWidget = (UUserWidget*)InSelf->Value;
	UWidget*  Widget = UserWidget->GetWidgetFromName(WidgetName);

	PyObject* PyRetVal = NePyBase::ToPy(Widget);
	return PyRetVal;
}


PyObject* NePyUserWidget_GetAllWidget(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	UUserWidget* UserWidget = (UUserWidget*)InSelf->Value;
	UWidgetTree* WidgetTree = UserWidget->WidgetTree;
	TArray<UWidget*> OutWidgets;
	if (WidgetTree)
	{
		WidgetTree->GetAllWidgets(OutWidgets);
	}

	PyObject* PyWidgetsList = PyList_New(0);
	PyObject* PyWidget = nullptr;
	for (UWidget* WidgetPtr : OutWidgets)
	{
		NePyBase::ToPy(WidgetPtr, PyWidget);
		if (PyWidget)
		{
			PyList_Append(PyWidgetsList, PyWidget);
			Py_DECREF(PyWidget);
		}
	}
	return PyWidgetsList;
}

PyObject* NepyUserWidget_GetAnimations(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	UNePyUserWidget* UserWidget = (UNePyUserWidget*)InSelf->Value;
	TMap<FName, UWidgetAnimation*> animationMap;
	UserWidget->GetAnimations(animationMap);

	PyObject* animationDict = PyDict_New();
	for (auto& elem : animationMap)
	{
		PyDict_SetItemString(animationDict, TCHAR_TO_ANSI(*elem.Key.ToString()), NePyBase::ToPy(elem.Value));
	}
	return animationDict;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"SetPythonInstance", NePyCFunctionCast(&NePyUserWidget_SetPythonInstance), METH_VARARGS, "(self, PythonInstance: object) -> None"}, \
{"NativeConstruct", NePyCFunctionCast(&NePyUserWidget_NativeConstruct), METH_NOARGS, "(self) -> None"}, \
{"GetWidgetFromName", NePyCFunctionCast(&NePyUserWidget_GetWidgetFromName), METH_VARARGS, "(self, WidgetName: str) -> Widget"}, \
{"GetAllWidget", NePyCFunctionCast(&NePyUserWidget_GetAllWidget), METH_NOARGS, "(self) -> WidgetsList: Tuple[Widget]"}, \
{"GetAnimations", NePyCFunctionCast(&NepyUserWidget_GetAnimations), METH_VARARGS, "(self) -> Dict"},\
EXPORT_UOBJECT_FUNC("GetRootWidget", &UUserWidget::GetRootWidget), \