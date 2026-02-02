// Created on: 1992-08-25
// Created by: Modelistation
// Copyright (c) 1992-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <BRep_CurveOn2Surfaces.hxx>       // 边在两曲面上的曲线表示（特殊情况）
#include <BRep_CurveRepresentation.hxx>    // 曲线表示基类
#include <BRep_GCurve.hxx>                 // 基于几何曲线的曲线表示（通常包含 3D 曲线/PCurve）
#include <BRep_TEdge.hxx>                  // 本类声明
#include <Standard_Type.hxx>               // OCCT RTTI 支持
#include <TopoDS_Shape.hxx>                // TopoDS_TShape / Handle(TopoDS_TShape)

IMPLEMENT_STANDARD_RTTIEXT(BRep_TEdge, TopoDS_TEdge)
// IMPLEMENT_STANDARD_RTTIEXT：为 OCCT RTTI 生成必要代码（动态类型识别）

static const Standard_Integer ParameterMask   = 1; // 位标志：SameParameter 对应的 bit
static const Standard_Integer RangeMask       = 2; // 位标志：SameRange 对应的 bit
static const Standard_Integer DegeneratedMask = 4; // 位标志：Degenerated 对应的 bit

//=================================================================================================

BRep_TEdge::BRep_TEdge()
    : TopoDS_TEdge(),
      myTolerance(RealEpsilon()),
      myFlags(0)
{
  SameParameter(Standard_True); // 默认认为“同参”（多数边在构建时会保证/期望同参）
  SameRange(Standard_True);     // 默认认为“同范围”（多数表示共享相同参数区间）
}

//=================================================================================================

Standard_Boolean BRep_TEdge::SameParameter() const
{
  return (myFlags & ParameterMask) != 0; // 检查 ParameterMask 位是否为 1
}

//=================================================================================================

void BRep_TEdge::SameParameter(const Standard_Boolean S)
{
  if (S)
    myFlags |= ParameterMask; // 置位：设置 SameParameter = true
  else
    myFlags &= ~ParameterMask; // 清位：设置 SameParameter = false
}

//=================================================================================================

Standard_Boolean BRep_TEdge::SameRange() const
{
  return (myFlags & RangeMask) != 0; // 检查 RangeMask 位是否为 1
}

//=================================================================================================

void BRep_TEdge::SameRange(const Standard_Boolean S)
{
  if (S)
    myFlags |= RangeMask; // 置位：设置 SameRange = true
  else
    myFlags &= ~RangeMask; // 清位：设置 SameRange = false
}

//=================================================================================================

Standard_Boolean BRep_TEdge::Degenerated() const
{
  return (myFlags & DegeneratedMask) != 0; // 检查 DegeneratedMask 位是否为 1
}

//=================================================================================================

void BRep_TEdge::Degenerated(const Standard_Boolean S)
{
  if (S)
    myFlags |= DegeneratedMask; // 置位：设置 Degenerated = true
  else
    myFlags &= ~DegeneratedMask; // 清位：设置 Degenerated = false
}

//=================================================================================================

Handle(TopoDS_TShape) BRep_TEdge::EmptyCopy() const
{
  Handle(BRep_TEdge) TE = new BRep_TEdge(); // 创建同类型的新对象（空壳）
  TE->Tolerance(myTolerance);               // 复制边公差
  // copy the curves representations
  BRep_ListOfCurveRepresentation&              l = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itr(myCurves);

  while (itr.More())
  {
    // on ne recopie PAS les polygones
    if (itr.Value()->IsKind(STANDARD_TYPE(BRep_GCurve))
        || itr.Value()->IsKind(STANDARD_TYPE(BRep_CurveOn2Surfaces)))
    {
      l.Append(itr.Value()->Copy()); // 复制曲线表示（仅复制几何曲线类表示）
    }
    itr.Next();
  }

  TE->Degenerated(Degenerated());     // 复制退化标志
  TE->SameParameter(SameParameter()); // 复制同参标志
  TE->SameRange(SameRange());         // 复制同范围标志

  return TE;
}

//=================================================================================================

void BRep_TEdge::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)
  // OCCT_DUMP_TRANSIENT_CLASS_BEGIN：输出对象类型等通用信息（调试用途）

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TopoDS_TEdge)
  // OCCT_DUMP_BASE_CLASS：递归输出基类字段（如果 theDepth 允许）

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myTolerance)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myFlags)
  // myFlags：以数值方式输出当前位标志（便于诊断 SameParameter/SameRange/Degenerated 状态）

  for (BRep_ListIteratorOfListOfCurveRepresentation itr(myCurves); itr.More(); itr.Next())
  {
    const Handle(BRep_CurveRepresentation)& aCurveRepresentation = itr.Value();
    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, aCurveRepresentation.get())
    // 逐个输出曲线表示对象（3D 曲线、PCurve、多边形等，具体类型由 RTTI 决定）
  }
}
