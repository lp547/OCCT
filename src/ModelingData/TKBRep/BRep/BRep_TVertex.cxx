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

#include <BRep_TVertex.hxx> // 本类声明
#include <gp_Pnt.hxx>       // gp_Pnt：三维点
#include <Standard_Type.hxx> // Standard_Type：OCCT RTTI 支持
#include <TopoDS_Shape.hxx>  // TopoDS_TShape / Handle(TopoDS_TShape)

IMPLEMENT_STANDARD_RTTIEXT(BRep_TVertex, TopoDS_TVertex)
// IMPLEMENT_STANDARD_RTTIEXT：为 OCCT RTTI 生成必要代码（动态类型识别）

//=================================================================================================

BRep_TVertex::BRep_TVertex()
    : TopoDS_TVertex(),
      myTolerance(RealEpsilon())
{
  // 说明：
  // - myPnt 使用 gp_Pnt 的默认构造（通常为 (0,0,0)）
  // - myTolerance 初始化为 RealEpsilon()，表示一个非常小的默认公差
}

//=================================================================================================

Handle(TopoDS_TShape) BRep_TVertex::EmptyCopy() const
{
  Handle(BRep_TVertex) TV = new BRep_TVertex(); // 创建同类型的新对象（空壳）
  TV->Pnt(myPnt);                               // 复制 3D 点坐标
  TV->Tolerance(myTolerance);                   // 复制公差
  return TV;                                    // 以基类句柄返回（多态）
}

//=================================================================================================

void BRep_TVertex::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)
  // OCCT_DUMP_TRANSIENT_CLASS_BEGIN：输出对象类型等通用信息（调试用途）

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TopoDS_TVertex)
  // OCCT_DUMP_BASE_CLASS：递归输出基类字段（如果 theDepth 允许）

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myPnt)
  // 输出 myPnt（gp_Pnt 的 DumpJson/转储）
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myTolerance)
  // 输出数值字段 myTolerance
  for (BRep_ListIteratorOfListOfPointRepresentation itr(myPoints); itr.More(); itr.Next())
  {
    const Handle(BRep_PointRepresentation)& aPointRepresentation = itr.Value();
    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, aPointRepresentation.get())
  }
  // 遍历输出每一个点表示（可能是顶点在某条边/某个面的参数信息）
}
