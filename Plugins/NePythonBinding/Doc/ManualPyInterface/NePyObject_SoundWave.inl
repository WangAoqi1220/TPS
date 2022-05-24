#pragma once
#include "NePyBase.h"
#include "Sound/SoundWave.h"

PyObject* NePySoundWave_GetRawData(FNePyObjectBase* InSelf)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	USoundWave* SoundWave = (USoundWave*)InSelf->Value;
	FByteBulkData RawData = SoundWave->RawData;

	char* Data = (char*)RawData.Lock(LOCK_READ_ONLY);
	int32 DataSize = RawData.GetBulkDataSize();
	PyObject* PyRawData = PyBytes_FromStringAndSize(Data, DataSize);
	RawData.Unlock();

	return PyRawData;
}

PyObject* NePySoundWave_SetRawData(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	Py_buffer PySoundWaveBuffer;
	if (!PyArg_ParseTuple(InArgs, "y*:SoundSetData", &PySoundWaveBuffer))
	{
		return nullptr;
	}

	if (PySoundWaveBuffer.buf == nullptr)
	{
		PyBuffer_Release(&PySoundWaveBuffer);
		return PyErr_Format(PyExc_Exception, "Invalid SoundWave buffer.");
	}

	USoundWave* SoundWave = (USoundWave*)InSelf->Value;
	SoundWave->FreeResources();
	SoundWave->InvalidateCompressedData();

	SoundWave->RawData.Lock(LOCK_READ_WRITE);
	void* data = SoundWave->RawData.Realloc(PySoundWaveBuffer.len);
	FMemory::Memcpy(data, PySoundWaveBuffer.buf, PySoundWaveBuffer.len);
	SoundWave->RawData.Unlock();

	PyBuffer_Release(&PySoundWaveBuffer);

	Py_RETURN_NONE;
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"GetRawData", NePyCFunctionCast(&NePySoundWave_GetRawData), METH_NOARGS, "(self) -> bytes"}, \
{"SetRawData", NePyCFunctionCast(&NePySoundWave_SetRawData), METH_VARARGS, "(self Buffer: buffer) -> None"}, \
