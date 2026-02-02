// Created on: 1993-07-05
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

#ifndef _BRep_CurveRepresentation_HeaderFile
#define _BRep_CurveRepresentation_HeaderFile
// 说明：本文件定义“边的曲线表示”基类。
// 在 OCCT 里，一条拓扑 Edge（边）可能有多种几何/离散表示：
// - 3D 曲线（Geom_Curve）
// - 面上的 2D 参数曲线（Geom2d_Curve，也叫 PCurve）
// - 多边形/三角网格上的折线（用于离散化/显示/加速）
// BRep_CurveRepresentation 就是这些“表示”的共同基类。

#include <Standard.hxx>

#include <TopLoc_Location.hxx>    // TopLoc_Location：几何位置变换（平移/旋转等）
#include <Standard_Transient.hxx> // Standard_Transient：OCCT 可被 Handle 管理的基类
#include <GeomAbs_Shape.hxx>      // GeomAbs_Shape：连续性枚举（C0/C1/C2/G1...）
class Geom_Surface;
class Poly_Triangulation;
class Geom_Curve;
class Geom2d_Curve;
class Poly_Polygon3D;
class Poly_Polygon2D;
class Poly_PolygonOnTriangulation;
// 以上都是前置声明：这里只需要“指针/句柄类型”，不需要包含完整定义，从而加快编译。

class BRep_CurveRepresentation;
DEFINE_STANDARD_HANDLE(BRep_CurveRepresentation, Standard_Transient)
// DEFINE_STANDARD_HANDLE：定义 Handle(BRep_CurveRepresentation) 智能句柄类型。

//! Root class for the curve representations. Contains
//! a location.
class BRep_CurveRepresentation : public Standard_Transient
{

public:
  //! A 3D curve representation.
  Standard_EXPORT virtual Standard_Boolean IsCurve3D() const;
  // 返回值说明：
  // - 默认实现返回 False（见 .cxx）
  // - 具体子类如果表示 3D 曲线，就会重写返回 True

  //! A curve in the parametric space of a surface.
  Standard_EXPORT virtual Standard_Boolean IsCurveOnSurface() const;
  // 是否“面上的 2D 参数曲线（PCurve）”表示。

  //! A continuity between two surfaces.
  Standard_EXPORT virtual Standard_Boolean IsRegularity() const;
  // 是否“两个曲面之间的连续性（相邻面光顺信息）”表示。

  //! A curve with two parametric   curves  on the  same
  //! surface.
  Standard_EXPORT virtual Standard_Boolean IsCurveOnClosedSurface() const;
  // 是否“同一个曲面上有两条 PCurve”的表示（闭合曲面常见：一条边可能跨越参数边界）。

  //! Is it a curve in the parametric space  of <S> with
  //! location <L>.
  Standard_EXPORT virtual Standard_Boolean IsCurveOnSurface(const Handle(Geom_Surface)& S,
                                                            const TopLoc_Location&      L) const;
  // 用于更精确判断：是否是“指定曲面 S + 指定位置 L”上的曲线表示。

  //! Is it  a  regularity between  <S1> and   <S2> with
  //! location <L1> and <L2>.
  Standard_EXPORT virtual Standard_Boolean IsRegularity(const Handle(Geom_Surface)& S1,
                                                        const Handle(Geom_Surface)& S2,
                                                        const TopLoc_Location&      L1,
                                                        const TopLoc_Location&      L2) const;
  // 用于更精确判断：是否是“曲面 S1/L1 与 曲面 S2/L2 之间”的连续性表示。

  //! A 3D polygon representation.
  Standard_EXPORT virtual Standard_Boolean IsPolygon3D() const;
  // 是否为“3D 折线（Poly_Polygon3D）”表示。

  //! A representation by an array of nodes on a
  //! triangulation.
  Standard_EXPORT virtual Standard_Boolean IsPolygonOnTriangulation() const;
  // 是否为“三角网格上的折线（Poly_PolygonOnTriangulation）”表示。

  //! Is it a polygon in the definition of <T> with
  //! location <L>.
  Standard_EXPORT virtual Standard_Boolean IsPolygonOnTriangulation(
    const Handle(Poly_Triangulation)& T,
    const TopLoc_Location&            L) const;
  // 用于更精确判断：是否是“指定网格 T + 指定位置 L”上的折线表示。

  //! A representation by two arrays of nodes on a
  //! triangulation.
  Standard_EXPORT virtual Standard_Boolean IsPolygonOnClosedTriangulation() const;
  // 是否为“闭合网格上的两条折线”表示（与闭合曲面类似：可能需要两条折线描述一条边）。

  //! A polygon in the parametric space of a surface.
  Standard_EXPORT virtual Standard_Boolean IsPolygonOnSurface() const;
  // 是否为“曲面参数空间上的 2D 折线（Poly_Polygon2D）”表示。

  //! Is it a polygon in the parametric space  of <S> with
  //! location <L>.
  Standard_EXPORT virtual Standard_Boolean IsPolygonOnSurface(const Handle(Geom_Surface)& S,
                                                              const TopLoc_Location&      L) const;
  // 用于更精确判断：是否为“指定曲面 S/L 上”的 2D 折线表示。

  //! Two   2D polygon  representations  in the  parametric
  //! space of a surface.
  Standard_EXPORT virtual Standard_Boolean IsPolygonOnClosedSurface() const;
  // 是否为“同曲面参数空间上两条 2D 折线”的表示（闭合曲面参数域断裂处常见）。

  const TopLoc_Location& Location() const;
  // 返回本表示自带的位置变换（注意：这是“曲线表示”的位置，不是边/面的 Location）。

  void Location(const TopLoc_Location& L);
  // 设置本表示的位置变换。

  Standard_EXPORT virtual const Handle(Geom_Curve)& Curve3D() const;
  // 获取 3D 曲线句柄。
  // 默认实现会抛异常（见 .cxx），只有 IsCurve3D() 为 True 的子类才应该实现它。

  Standard_EXPORT virtual void Curve3D(const Handle(Geom_Curve)& C);
  // 设置 3D 曲线句柄（同上：默认抛异常，具体子类才实现）。

  Standard_EXPORT virtual const Handle(Geom_Surface)& Surface() const;
  // 获取关联曲面（用于 PCurve / polygon-on-surface / regularity 等表示）。

  Standard_EXPORT virtual const Handle(Geom2d_Curve)& PCurve() const;
  // 获取第一条 2D 参数曲线（PCurve）。

  Standard_EXPORT virtual void PCurve(const Handle(Geom2d_Curve)& C);
  // 设置第一条 PCurve。

  Standard_EXPORT virtual const Handle(Geom2d_Curve)& PCurve2() const;
  // 获取第二条 PCurve（闭合曲面/闭合参数域时可能需要）。

  Standard_EXPORT virtual void PCurve2(const Handle(Geom2d_Curve)& C);
  // 设置第二条 PCurve。

  Standard_EXPORT virtual const Handle(Poly_Polygon3D)& Polygon3D() const;
  // 获取 3D 折线表示（离散）。

  Standard_EXPORT virtual void Polygon3D(const Handle(Poly_Polygon3D)& P);
  // 设置 3D 折线表示。

  Standard_EXPORT virtual const Handle(Poly_Polygon2D)& Polygon() const;
  // 获取 2D 折线（曲面参数域中的折线）。

  Standard_EXPORT virtual void Polygon(const Handle(Poly_Polygon2D)& P);
  // 设置 2D 折线（曲面参数域）。

  Standard_EXPORT virtual const Handle(Poly_Polygon2D)& Polygon2() const;
  // 获取第二条 2D 折线（闭合曲面/闭合参数域场景）。

  Standard_EXPORT virtual void Polygon2(const Handle(Poly_Polygon2D)& P);
  // 设置第二条 2D 折线。

  Standard_EXPORT virtual const Handle(Poly_Triangulation)& Triangulation() const;
  // 获取关联三角网格句柄（用于“网格上的折线”表示）。

  Standard_EXPORT virtual const Handle(Poly_PolygonOnTriangulation)& PolygonOnTriangulation() const;
  // 获取三角网格上的折线（第一条）。

  Standard_EXPORT virtual void PolygonOnTriangulation(const Handle(Poly_PolygonOnTriangulation)& P);
  // 设置三角网格上的折线（第一条）。

  Standard_EXPORT virtual const Handle(Poly_PolygonOnTriangulation)& PolygonOnTriangulation2()
    const;
  // 获取三角网格上的第二条折线（闭合网格/复杂场景）。

  Standard_EXPORT virtual void PolygonOnTriangulation2(
    const Handle(Poly_PolygonOnTriangulation)& P2);
  // 设置三角网格上的第二条折线。

  Standard_EXPORT virtual const Handle(Geom_Surface)& Surface2() const;
  // Regularity 表示可能需要第二个曲面：Surface2() 返回第二曲面。

  Standard_EXPORT virtual const TopLoc_Location& Location2() const;
  // Regularity 表示可能需要第二个位置：Location2() 返回第二位置。

  Standard_EXPORT virtual const GeomAbs_Shape& Continuity() const;
  // Regularity 表示的连续性类型（C0/C1/C2/G1...）。

  Standard_EXPORT virtual void Continuity(const GeomAbs_Shape C);
  // 设置连续性类型。

  //! Return a copy of this representation.
  Standard_EXPORT virtual Handle(BRep_CurveRepresentation) Copy() const = 0;
  // 纯虚函数：每个具体表示必须能复制自己（用于拷贝 Edge 时复制曲线表示）。

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const;
  // DumpJson：调试用，把对象内容以 JSON 格式写到输出流。

  DEFINE_STANDARD_RTTIEXT(BRep_CurveRepresentation, Standard_Transient)
  // RTTI：OCCT 运行时类型识别（DynamicType/IsKind）。

protected:
  Standard_EXPORT BRep_CurveRepresentation(const TopLoc_Location& L);
  // 受保护构造：只允许子类构造，基类存储 location。

  TopLoc_Location myLocation; // 本表示对应的几何变换（比如曲线所在坐标系）

private:
};

#include <BRep_CurveRepresentation.lxx> // 内联方法实现（Location 的 getter/setter）

#endif // _BRep_CurveRepresentation_HeaderFile
