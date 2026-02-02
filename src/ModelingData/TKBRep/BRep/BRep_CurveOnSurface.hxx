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

#ifndef _BRep_CurveOnSurface_HeaderFile
#define _BRep_CurveOnSurface_HeaderFile
// 你先把它当成一句话就够了：
// BRep_CurveOnSurface = “BRep_GCurve（带范围的曲线表示）” + “曲面上的 2D 参数曲线 PCurve”
//                       + “对应曲面 Surface” + “Location（把曲面点变换到最终坐标系）”。
// 它描述的是：边在某个面上的那条“参数曲线”（2D），通过曲面可以映射回 3D。

#include <Standard.hxx>

#include <gp_Pnt2d.hxx>      // gp_Pnt2d：二维点（u,v）
#include <BRep_GCurve.hxx>   // 带范围的几何曲线表示基类
#include <Standard_Real.hxx> // Standard_Real：参数类型
class Geom2d_Curve;
class Geom_Surface;
class TopLoc_Location;
class gp_Pnt;
class BRep_CurveRepresentation;
// 前置声明：减少头文件依赖

class BRep_CurveOnSurface;
DEFINE_STANDARD_HANDLE(BRep_CurveOnSurface, BRep_GCurve)
// 定义 Handle(BRep_CurveOnSurface)

//! Representation  of a  curve   by a   curve  in the
//! parametric space of a surface.
class BRep_CurveOnSurface : public BRep_GCurve
{

public:
  Standard_EXPORT BRep_CurveOnSurface(const Handle(Geom2d_Curve)& PC,
                                      const Handle(Geom_Surface)& S,
                                      const TopLoc_Location&      L);
  // 构造函数：
  // - PC：面参数域中的 2D 曲线（PCurve），参数范围来自 PC->First/LastParameter()
  // - S：承载这条 PCurve 的曲面
  // - L：曲面的位置变换（把曲面点变换到最终坐标系）

  void SetUVPoints(const gp_Pnt2d& P1, const gp_Pnt2d& P2);
  // 设置缓存的 UV 端点（对应参数范围 First/Last 处的 (u,v)）。
  // 注意：这两个 UV 通常用于加速或边界处理，不一定总是有效（范围可能是无穷）。

  void UVPoints(gp_Pnt2d& P1, gp_Pnt2d& P2) const;
  // 读取缓存的 UV 端点。

  //! Computes the point at parameter U.
  Standard_EXPORT void D0(const Standard_Real U, gp_Pnt& P) const Standard_OVERRIDE;
  // D0：把参数 U 映射为 3D 点：
  // 1) 在 PCurve 上取 2D 点 (u,v)
  // 2) 用 Surface->Value(u,v) 得到曲面上的 3D 点
  // 3) 再应用 myLocation 变换到最终坐标系

  //! Returns True.
  Standard_EXPORT virtual Standard_Boolean IsCurveOnSurface() const Standard_OVERRIDE;
  // 表示类型判断：这是“曲面上的曲线（PCurve）”表示，所以返回 True。

  //! A curve in the parametric space of a surface.
  Standard_EXPORT virtual Standard_Boolean IsCurveOnSurface(const Handle(Geom_Surface)& S,
                                                            const TopLoc_Location&      L) const
    Standard_OVERRIDE;
  // 更精确判断：是否就是某个具体曲面 S + 具体 Location L 上的 PCurve。

  Standard_EXPORT virtual const Handle(Geom_Surface)& Surface() const Standard_OVERRIDE;
  // 返回内部保存的曲面句柄。

  Standard_EXPORT virtual const Handle(Geom2d_Curve)& PCurve() const Standard_OVERRIDE;
  // 返回内部保存的 2D 参数曲线句柄。

  Standard_EXPORT virtual void PCurve(const Handle(Geom2d_Curve)& C) Standard_OVERRIDE;
  // 设置内部保存的 2D 参数曲线句柄。

  //! Return a copy of this representation.
  Standard_EXPORT virtual Handle(BRep_CurveRepresentation) Copy() const Standard_OVERRIDE;
  // Copy：复制该表示（复制 PCurve/Surface/Location/范围/UV 端点缓存）。

  //! Recomputes any derived data after a modification.
  //! This is called when the range is modified.
  Standard_EXPORT virtual void Update() Standard_OVERRIDE;
  // Update：当范围改变时，重新计算 First/Last 处的 UV 端点缓存（见 .cxx）。

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;
  // DumpJson：调试输出。

  DEFINE_STANDARD_RTTIEXT(BRep_CurveOnSurface, BRep_GCurve)

protected:
  gp_Pnt2d myUV1; // 缓存：参数 First() 对应的 (u,v)
  gp_Pnt2d myUV2; // 缓存：参数 Last()  对应的 (u,v)

private:
  Handle(Geom2d_Curve) myPCurve;  // 2D 参数曲线（PCurve）
  Handle(Geom_Surface) mySurface; // 承载 PCurve 的曲面
};

#include <BRep_CurveOnSurface.lxx>

#endif // _BRep_CurveOnSurface_HeaderFile
