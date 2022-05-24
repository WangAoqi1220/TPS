#pragma once
#include "NePyBase.h"
#include "Math/Vector4.h"

PyObject* FNePyStruct_Vector4_AsTuple(FNePyStruct_Vector4* InSelf)
{
	PyObject* VecTuple = PyTuple_New(4);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		PyTuple_SetItem(VecTuple, Index, PyFloat_FromDouble(InSelf->Value[Index]));
	}
	return VecTuple;
}

PyObject* FNePyStruct_Vector4_AsList(FNePyStruct_Vector4* InSelf)
{
	PyObject* VecList = PyList_New(4);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		PyList_SetItem(VecList, Index, PyFloat_FromDouble(InSelf->Value[Index]));
	}
	return VecList;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"AsTuple", NePyCFunctionCast(&FNePyStruct_Vector4_AsTuple), METH_NOARGS, "(self) -> typing.Tuple[float, float, float, float]"}, \
{"AsList", NePyCFunctionCast(&FNePyStruct_Vector4_AsList), METH_NOARGS, "(self) -> typing.List[float, float, float, float]"}, \

