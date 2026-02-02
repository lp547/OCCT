// Created on: 1995-03-15
// Created by: Laurent PAINNOT
// Copyright (c) 1995-1999 Matra Datavision
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

#include <BRep_GCurve.hxx>      // 本类声明：带参数范围的“几何曲线表示”基类
#include <Standard_Type.hxx>    // OCCT RTTI 支持
#include <TopLoc_Location.hxx>  // TopLoc_Location：位置变换（给基类构造用）

IMPLEMENT_STANDARD_RTTIEXT(BRep_GCurve, BRep_CurveRepresentation)
// IMPLEMENT_STANDARD_RTTIEXT：为 OCCT RTTI 生成必要代码

//=================================================================================================

BRep_GCurve::BRep_GCurve(const TopLoc_Location& L,
                         const Standard_Real    First,
                         const Standard_Real    Last)
    : BRep_CurveRepresentation(L),
      myFirst(First),
      myLast(Last)

{
  // 构造时保存：
  // - Location：来自 BRep_CurveRepresentation
  // - 参数范围 [myFirst, myLast]
}

//=================================================================================================

void BRep_GCurve::Update()
{
  // 基类默认什么也不做。
  // 某些子类会重写 Update()，例如：
  // - 曲线在曲面上（BRep_CurveOnSurface）会在范围改变后重新计算 UV 端点缓存。
}

//=================================================================================================

void BRep_GCurve::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, BRep_CurveRepresentation)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myFirst)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myLast)
  // 输出字段：
  // - myFirst/myLast：参数范围
}
