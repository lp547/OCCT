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

#include <BRep_CurveOnSurface.hxx>         // 本类声明：曲面参数域曲线（PCurve）表示
#include <BRep_CurveRepresentation.hxx>    // Copy() 返回类型 Handle(BRep_CurveRepresentation)
#include <Geom2d_Curve.hxx>                // Geom2d_Curve：2D 参数曲线
#include <Geom_Surface.hxx>                // Geom_Surface：曲面
#include <gp_Pnt.hxx>                      // gp_Pnt：3D 点
#include <gp_Pnt2d.hxx>                    // gp_Pnt2d：2D 点（u,v）
#include <Precision.hxx>                   // Precision：无穷判定等工具
#include <Standard_Type.hxx>               // OCCT RTTI 支持
#include <TopLoc_Location.hxx>             // TopLoc_Location：位置变换

IMPLEMENT_STANDARD_RTTIEXT(BRep_CurveOnSurface, BRep_GCurve)
// IMPLEMENT_STANDARD_RTTIEXT：为 OCCT RTTI 生成必要代码

//=================================================================================================

BRep_CurveOnSurface::BRep_CurveOnSurface(const Handle(Geom2d_Curve)& PC,
                                         const Handle(Geom_Surface)& S,
                                         const TopLoc_Location&      L)
    : BRep_GCurve(L, PC->FirstParameter(), PC->LastParameter()),
      myPCurve(PC),
      mySurface(S)
{
  // 这里做了三件事：
  // 1) 调用基类 BRep_GCurve(L, First, Last)：
  //    - Location = L
  //    - 参数范围来自 PCurve 的范围 [PC->FirstParameter(), PC->LastParameter()]
  // 2) 保存 PCurve（2D 参数曲线）
  // 3) 保存 Surface（承载 PCurve 的曲面）
}

//=================================================================================================

void BRep_CurveOnSurface::D0(const Standard_Real U, gp_Pnt& P) const
{
  // should be D0 NYI
  // 步骤 1：在 2D 参数曲线（PCurve）上取点，得到参数域坐标 (u,v)
  gp_Pnt2d P2d = myPCurve->Value(U);
  // 步骤 2：用曲面把 (u,v) 映射到 3D 点
  P = mySurface->Value(P2d.X(), P2d.Y());
  // 步骤 3：把 3D 点从曲面局部坐标系变换到最终坐标系
  P.Transform(myLocation.Transformation());
}

//=================================================================================================

Standard_Boolean BRep_CurveOnSurface::IsCurveOnSurface() const
{
  return Standard_True; // 这是“曲面参数域曲线（PCurve）”表示
}

//=================================================================================================

Standard_Boolean BRep_CurveOnSurface::IsCurveOnSurface(const Handle(Geom_Surface)& S,
                                                       const TopLoc_Location&      L) const
{
  // 判断是否就是指定曲面 S + 指定位置 L 上的那条 PCurve
  return (S == mySurface) && (L == myLocation);
}

//=================================================================================================

const Handle(Geom_Surface)& BRep_CurveOnSurface::Surface() const
{
  return mySurface; // 返回承载该 PCurve 的曲面句柄
}

//=================================================================================================

const Handle(Geom2d_Curve)& BRep_CurveOnSurface::PCurve() const
{
  return myPCurve; // 返回 2D 参数曲线句柄
}

//=================================================================================================

void BRep_CurveOnSurface::PCurve(const Handle(Geom2d_Curve)& C)
{
  myPCurve = C; // 替换内部保存的 2D 参数曲线
}

//=================================================================================================

Handle(BRep_CurveRepresentation) BRep_CurveOnSurface::Copy() const
{
  Handle(BRep_CurveOnSurface) C = new BRep_CurveOnSurface(myPCurve, mySurface, Location());
  // Copy 的核心：复制 PCurve/Surface/Location（句柄通常共享底层几何对象）

  C->SetRange(First(), Last());
  C->SetUVPoints(myUV1, myUV2);
  // 同时复制当前裁剪过的范围与 UV 端点缓存

  return C;
}

//=================================================================================================

void BRep_CurveOnSurface::Update()
{
  // Update 的目的：当范围改变时，重新计算两端的 UV 缓存点 myUV1 / myUV2。
  Standard_Real    f     = First(); // 起始参数
  Standard_Real    l     = Last();  // 结束参数
  Standard_Boolean isneg = Precision::IsNegativeInfinite(f); // f 是否为 -∞
  Standard_Boolean ispos = Precision::IsPositiveInfinite(l); // l 是否为 +∞
  if (!isneg)
  {
    myPCurve->D0(f, myUV1); // 计算起点参数处的 (u,v)
  }
  if (!ispos)
  {
    myPCurve->D0(l, myUV2); // 计算终点参数处的 (u,v)
  }
}

//=================================================================================================

void BRep_CurveOnSurface::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, BRep_GCurve)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myUV1)
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myUV2)
  // 输出 UV 端点缓存

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myPCurve.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, mySurface.get())
  // 输出句柄底层指针（调试）
}
