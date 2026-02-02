// Created on: 1993-07-06
// Created by: Remi LEQUETTE
// Copyright (c) 1993-1999 Matra Datavision
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

#include <BRep_Curve3D.hxx>              // 本类声明：3D 曲线表示
#include <BRep_CurveRepresentation.hxx>  // 返回类型 Handle(BRep_CurveRepresentation)
#include <Geom_Curve.hxx>                // Geom_Curve：3D 几何曲线
#include <gp_Pnt.hxx>                    // gp_Pnt：3D 点
#include <Standard_Type.hxx>             // OCCT RTTI 支持
#include <TopLoc_Location.hxx>           // TopLoc_Location：位置变换

IMPLEMENT_STANDARD_RTTIEXT(BRep_Curve3D, BRep_GCurve)
// IMPLEMENT_STANDARD_RTTIEXT：为 OCCT RTTI 生成必要代码

//=================================================================================================

BRep_Curve3D::BRep_Curve3D(const Handle(Geom_Curve)& C, const TopLoc_Location& L)
    : BRep_GCurve(L,
                  C.IsNull() ? RealFirst() : C->FirstParameter(),
                  C.IsNull() ? RealLast() : C->LastParameter()),
      myCurve(C)
{
  // 这里做了两件事：
  // 1) 调用基类 BRep_GCurve(L, First, Last) 保存 Location 与参数范围
  //    - 如果曲线句柄 C 为空，就用 RealFirst()/RealLast() 作为“无限范围”占位
  //    - 如果 C 非空，就用曲线自身的参数范围 [FirstParameter, LastParameter]
  // 2) 保存 3D 曲线句柄到 myCurve
}

//=================================================================================================

void BRep_Curve3D::D0(const Standard_Real U, gp_Pnt& P) const
{
  // should be D0 NYI
  // 这里直接取曲线在参数 U 的点：
  // - Value(U) 返回 3D 点
  // - D0 通常只算点，不算导数
  P = myCurve->Value(U);
}

//=================================================================================================

Standard_Boolean BRep_Curve3D::IsCurve3D() const
{
  return Standard_True; // 明确告诉外界：我就是 3D 曲线表示
}

//=================================================================================================

const Handle(Geom_Curve)& BRep_Curve3D::Curve3D() const
{
  return myCurve; // 返回内部保存的 3D 曲线句柄
}

//=================================================================================================

void BRep_Curve3D::Curve3D(const Handle(Geom_Curve)& C)
{
  myCurve = C; // 替换内部保存的 3D 曲线句柄
}

//=================================================================================================

Handle(BRep_CurveRepresentation) BRep_Curve3D::Copy() const
{
  Handle(BRep_Curve3D) C = new BRep_Curve3D(myCurve, Location());
  // Copy 的核心：复制几何曲线句柄 + 复制 Location
  // 注意：这里的 myCurve 是句柄，复制后两个表示共享同一个几何曲线对象（通常是允许的）。

  C->SetRange(First(), Last());
  // 把当前对象的范围也复制过去（可能不同于曲线默认范围，例如被裁剪过的边）
  return C;
}

//=================================================================================================

void BRep_Curve3D::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, BRep_GCurve)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myCurve.get())
  // 输出字段：myCurve（底层指针，用于调试）
}
