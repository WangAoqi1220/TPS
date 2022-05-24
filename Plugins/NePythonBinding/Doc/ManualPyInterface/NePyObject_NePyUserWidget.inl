#pragma once
#include "NePyBase.h"
#include "Blueprint/UserWidget.h"
#include "NePyUserWidget.h"
#include "NePyTemplate.h"

PyObject* NePyUserWidget_GetPyProxy(FNePyObjectBase* InSelf)
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

	UNePyUserWidget* Widget = static_cast<UNePyUserWidget*>(InSelf->Value);
	PyObject* PyWidget = Widget->GetPyProxy();
	if (PyWidget)
	{
		Py_INCREF(PyWidget);
		return PyWidget;
	}

	Py_RETURN_NONE;
}

PyObject* NePyUserWidget_BindWidget(FNePyObjectBase* InSelf, PyObject* InArg)
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
	
	PyObject* WidgetMap;
	if (!PyArg_ParseTuple(InArg, "O:BindWidget", &WidgetMap))
	{
		return nullptr;
	}

	UNePyUserWidget* UserWidget = static_cast<UNePyUserWidget*>(InSelf->Value);
	UserWidget->BindWidget(WidgetMap);
	Py_RETURN_NONE;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"GetPyProxy", NePyCFunctionCast(&NePyUserWidget_GetPyProxy), METH_NOARGS, "(self) -> object"}, \
{"BindWidget", NePyCFunctionCast(&NePyUserWidget_BindWidget), METH_VARARGS, "(self, WidgetMap: dict) -> None"}, \
EXPORT_UOBJECT_FUNC("GetRootWidget", &UUserWidget::GetRootWidget), \


