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

#ifndef _BRep_Curve3D_HeaderFile
#define _BRep_Curve3D_HeaderFile
// 你先把它当成一句话就够了：
// BRep_Curve3D = “BRep_GCurve（带范围的曲线表示）” + “一个真正的 3D 几何曲线 Geom_Curve”。
// 这就是最常见的“边的 3D 曲线表示”。

#include <Standard.hxx>

#include <BRep_GCurve.hxx>   // 带参数范围的几何曲线表示基类（包含 Location + [First,Last]）
#include <Standard_Real.hxx> // Standard_Real：参数类型
class Geom_Curve;
class TopLoc_Location;
class gp_Pnt;
class BRep_CurveRepresentation;
// 前置声明：避免包含过多头文件

class BRep_Curve3D;
DEFINE_STANDARD_HANDLE(BRep_Curve3D, BRep_GCurve)
// 定义 Handle(BRep_Curve3D) 智能句柄

//! Representation of a curve by a 3D curve.
class BRep_Curve3D : public BRep_GCurve
{

public:
  Standard_EXPORT BRep_Curve3D(const Handle(Geom_Curve)& C, const TopLoc_Location& L);
  // 构造函数：
  // - C：3D 几何曲线（可以为空句柄，但多数情况下不会为空）
  // - L：该曲线表示的 Location（坐标系变换）
  // 同时会把 [First,Last] 设置为曲线的参数范围（见 .cxx 实现）。

  //! Computes the point at parameter U.
  Standard_EXPORT void D0(const Standard_Real U, gp_Pnt& P) const Standard_OVERRIDE;
  // D0：给定参数 U，计算 3D 点 P（直接调用 myCurve->Value(U)）。

  //! Returns True.
  Standard_EXPORT virtual Standard_Boolean IsCurve3D() const Standard_OVERRIDE;
  // 这是“3D 曲线表示”，所以返回 True。

  Standard_EXPORT virtual const Handle(Geom_Curve)& Curve3D() const Standard_OVERRIDE;
  // 返回内部保存的 3D 曲线句柄。

  Standard_EXPORT virtual void Curve3D(const Handle(Geom_Curve)& C) Standard_OVERRIDE;
  // 设置内部保存的 3D 曲线句柄。

  //! Return a copy of this representation.
  Standard_EXPORT Handle(BRep_CurveRepresentation) Copy() const Standard_OVERRIDE;
  // Copy：复制一个等价的曲线表示（用于复制 Edge 的几何表示列表）。

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;
  // DumpJson：调试用输出。

  DEFINE_STANDARD_RTTIEXT(BRep_Curve3D, BRep_GCurve)
  // RTTI：OCCT 运行时类型识别

protected:
private:
  Handle(Geom_Curve) myCurve; // 保存的 3D 几何曲线（边的真实几何形状）
};

#endif // _BRep_Curve3D_HeaderFile
