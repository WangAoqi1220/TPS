#pragma once
#include "NePyBase.h"
#include "Animation/AnimSequence.h"
#include "NePyAutoExport.h"

PyObject* NePyAnimSequence_GetBoneTransform(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	int TrackIndex;
	float FrameTime;
	PyObject* PyUseRawData = nullptr;
	if (!PyArg_ParseTuple(InArgs, "if|O:GetBoneTransform", &TrackIndex, &FrameTime, &PyUseRawData))
	{
		return nullptr;
	}

	bool bUseRawData = false;
	if (PyUseRawData && PyObject_IsTrue(PyUseRawData))
	{
		bUseRawData = true;
	}

	FTransform OutAtom;
	UAnimSequence* AnimSequence = (UAnimSequence*)InSelf->Value;
	AnimSequence->GetBoneTransform(OutAtom, TrackIndex, FrameTime, bUseRawData);

	return NePyStructNew_Transform(OutAtom);
}

// unsupported FRawAnimSequenceTrack
//PyObject* NePyAnimSequence_ExtractBoneTransform(FNePyObjectBase* InSelf, PyObject* InArgs)
//{
//	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
//	{
//		return nullptr;
//	}
//
//	PyObject* PySources;
//	float FrameTime;
//
//	if (!PyArg_ParseTuple(InArgs, "Of:ExtractBoneTransform", &PySources, &FrameTime))
//	{
//		return nullptr;
//	}
//
//	ue_PyFRawAnimSequenceTrack* rast = NePyAnimSequence_is_fraw_AnimSequenceuence_track(PySources);
//	if (!rast)
//	{
//		return PyErr_Format(PyExc_Exception, "argument is not an FRawAnimSequenceTrack");
//	}
//
//	FTransform OutAtom;
//	UAnimSequence* AnimSequence = (UAnimSequence*)InSelf->Value;
//	anim->ExtractBoneTransform(rast->raw_AnimSequenceuence_track, OutAtom, FrameTime);
//
//	return NePyStructNew_Transform(OutAtom);
//}

PyObject* NePyAnimSequence_ExtractRootMotion(FNePyObjectBase* InSelf, PyObject* InArgs)
{
	if (!NePyBase::CheckValidAndSetPyErr(InSelf, "Self"))
	{
		return nullptr;
	}

	float StartTime;
	float DeltaTime;
	PyObject* PyAllowLooping = nullptr;
	if (!PyArg_ParseTuple(InArgs, "ff|O:ExtractRootMotion", &StartTime, &DeltaTime, &PyAllowLooping))
	{
		return nullptr;
	}

	bool bAllowLooping = false;
	if (PyAllowLooping && PyObject_IsTrue(PyAllowLooping))
	{
		bAllowLooping = true;
	}

	UAnimSequence* AnimSequence = (UAnimSequence*)InSelf->Value;
	FTransform Transform = AnimSequence->ExtractRootMotion(StartTime, DeltaTime, bAllowLooping);

	return NePyStructNew_Transform(Transform);
}

#undef NePyManualExportFuncs
#define NePyManualExportFuncs \
{"GetBoneTransform", NePyCFunctionCast(&NePyAnimSequence_GetBoneTransform), METH_VARARGS, "(self, TrackIndex: int, FrameTime: float, bUseRawData: bool) -> Transform"}, \
{"ExtractRootMotion", NePyCFunctionCast(&NePyAnimSequence_ExtractRootMotion), METH_VARARGS, "(self, StartTime: float, DeltaTime: float, bAllowLooping: float) -> Transform"}, \
