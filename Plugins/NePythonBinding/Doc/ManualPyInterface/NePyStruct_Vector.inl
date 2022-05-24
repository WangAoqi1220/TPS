#pragma once
#include "NePyBase.h"
#include "Math/Vector.h"

PyObject* FNePyStruct_Vector_AsTuple(FNePyStruct_Vector* InSelf)
{
	PyObject* VecTuple = PyTuple_New(3);
	for (int32 Index = 0; Index < 3; ++Index)
	{
		PyTuple_SetItem(VecTuple, Index, PyFloat_FromDouble(InSelf->Value[Index]));
	}
	return VecTuple;
}

PyObject* FNePyStruct_Vector_AsList(FNePyStruct_Vector* InSelf)
{
	PyObject* VecList = PyList_New(3);
	for (int32 Index = 0; Index < 3; ++Index)
	{
		PyList_SetItem(VecList, Index, PyFloat_FromDouble(InSelf->Value[Index]));
	}
	return VecList;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"AsTuple", NePyCFunctionCast(&FNePyStruct_Vector_AsTuple), METH_NOARGS, "(self) -> typing.Tuple[float, float, float]"}, \
{"AsList", NePyCFunctionCast(&FNePyStruct_Vector_AsList), METH_NOARGS, "(self) -> typing.List[float, float, float]"}, \

