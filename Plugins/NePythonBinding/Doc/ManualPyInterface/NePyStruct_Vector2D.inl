#pragma once
#include "NePyBase.h"
#include "Math/Vector2D.h"

PyObject* FNePyStruct_Vector2D_AsTuple(FNePyStruct_Vector2D* InSelf)
{
	PyObject* VecTuple = PyTuple_New(2);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		PyTuple_SetItem(VecTuple, Index, PyFloat_FromDouble(InSelf->Value[Index]));
	}
	return VecTuple;
}

PyObject* FNePyStruct_Vector2D_AsList(FNePyStruct_Vector2D* InSelf)
{
	PyObject* VecList = PyList_New(2);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		PyList_SetItem(VecList, Index, PyFloat_FromDouble(InSelf->Value[Index]));
	}
	return VecList;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"AsTuple", NePyCFunctionCast(&FNePyStruct_Vector2D_AsTuple), METH_NOARGS, "(self) -> typing.Tuple[float, float]"}, \
{"AsList", NePyCFunctionCast(&FNePyStruct_Vector2D_AsList), METH_NOARGS, "(self) -> typing.List[float, float]"}, \

