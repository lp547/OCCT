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

#include <BRep_CurveRepresentation.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_DomainError.hxx>
#include <Standard_Type.hxx>
#include <TopLoc_Location.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRep_CurveRepresentation, Standard_Transient)
// IMPLEMENT_STANDARD_RTTIEXT：为 OCCT RTTI 生成必要代码（支持 DynamicType/IsKind）

//=================================================================================================

BRep_CurveRepresentation::BRep_CurveRepresentation(const TopLoc_Location& L)
    : myLocation(L)
{
  // 构造函数：仅保存 location（位置变换）
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurve3D() const
{
  return Standard_False; // 基类默认：不是 3D 曲线（具体子类重写返回 True）
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurveOnSurface() const
{
  return Standard_False; // 基类默认：不是曲面参数域上的曲线（PCurve）
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurveOnClosedSurface() const
{
  return Standard_False; // 基类默认：不是闭合曲面上的“双 PCurve”表示
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsRegularity() const
{
  return Standard_False; // 基类默认：不是“两个曲面间连续性”表示
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurveOnSurface(const Handle(Geom_Surface)&,
                                                            const TopLoc_Location&) const
{
  return Standard_False; // 基类默认：不匹配任何特定曲面/位置
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsRegularity(const Handle(Geom_Surface)&,
                                                        const Handle(Geom_Surface)&,
                                                        const TopLoc_Location&,
                                                        const TopLoc_Location&) const
{
  return Standard_False; // 基类默认：不匹配任何特定的两曲面连续性
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygon3D() const
{
  return Standard_False; // 基类默认：不是 3D 折线表示
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnTriangulation() const
{
  return Standard_False; // 基类默认：不是网格上的折线表示
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnTriangulation(
  const Handle(Poly_Triangulation)&,
  const TopLoc_Location&) const
{
  return Standard_False; // 基类默认：不匹配任何特定网格/位置
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnClosedTriangulation() const
{
  return Standard_False; // 基类默认：不是闭合网格上的“双折线”表示
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnClosedSurface() const
{
  return Standard_False; // 基类默认：不是闭合曲面上的“双 2D 折线”表示
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnSurface() const
{
  return Standard_False; // 基类默认：不是曲面参数域上的 2D 折线表示
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnSurface(const Handle(Geom_Surface)&,
                                                              const TopLoc_Location&) const
{
  return Standard_False; // 基类默认：不匹配任何特定曲面/位置
}

//=================================================================================================

const Handle(Geom_Curve)& BRep_CurveRepresentation::Curve3D() const
{
  // 重要：基类不知道自己是不是 3D 曲线表示，因此默认“访问无效”并抛异常。
  // 只有 IsCurve3D() == True 的子类才会重写并返回真正的 3D 曲线句柄。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Curve3D(const Handle(Geom_Curve)&)
{
  // 同 Curve3D()：基类默认不支持设置 3D 曲线。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Geom_Surface)& BRep_CurveRepresentation::Surface() const
{
  // 获取曲面：只有“曲面相关”的子类才会重写此函数（比如 CurveOnSurface / Regularity 等）。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Geom2d_Curve)& BRep_CurveRepresentation::PCurve() const
{
  // 获取第一条 PCurve：只有“曲面上的曲线”子类才会重写此函数。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Geom2d_Curve)& BRep_CurveRepresentation::PCurve2() const
{
  // 获取第二条 PCurve：用于闭合曲面/闭合参数域等情况，具体子类实现。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PCurve(const Handle(Geom2d_Curve)&)
{
  // 设置第一条 PCurve：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PCurve2(const Handle(Geom2d_Curve)&)
{
  // 设置第二条 PCurve：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const GeomAbs_Shape& BRep_CurveRepresentation::Continuity() const
{
  // 连续性：只有 Regularity 子类才会重写此函数并返回 C0/C1/C2/G1... 等信息。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Continuity(const GeomAbs_Shape)
{
  // 设置连续性：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Geom_Surface)& BRep_CurveRepresentation::Surface2() const
{
  // 第二曲面：Regularity 子类会重写它（描述两曲面之间的连续性）。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const TopLoc_Location& BRep_CurveRepresentation::Location2() const
{
  // 第二位置：Regularity 子类会重写它（第二曲面对应的位置变换）。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_Polygon3D)& BRep_CurveRepresentation::Polygon3D() const
{
  // 3D 折线：只有 Polygon3D 子类才会重写。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Polygon3D(const Handle(Poly_Polygon3D)&)
{
  // 设置 3D 折线：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_Polygon2D)& BRep_CurveRepresentation::Polygon() const
{
  // 2D 折线（曲面参数域）：只有 PolygonOnSurface 子类才会重写。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Polygon(const Handle(Poly_Polygon2D)&)
{
  // 设置 2D 折线：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_PolygonOnTriangulation)& BRep_CurveRepresentation::PolygonOnTriangulation2() const
{
  // 网格上的第二条折线：只在“闭合网格/双折线”子类中有效。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PolygonOnTriangulation2(const Handle(Poly_PolygonOnTriangulation)&)
{
  // 设置网格上的第二条折线：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PolygonOnTriangulation(const Handle(Poly_PolygonOnTriangulation)&)
{
  // 设置网格上的第一条折线：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_PolygonOnTriangulation)& BRep_CurveRepresentation::PolygonOnTriangulation() const
{
  // 网格上的第一条折线：只有相关子类才会重写。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_Triangulation)& BRep_CurveRepresentation::Triangulation() const
{
  // 获取关联三角网格：只有“PolygonOnTriangulation”相关子类才会重写。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_Polygon2D)& BRep_CurveRepresentation::Polygon2() const
{
  // 第二条 2D 折线：只在闭合曲面/双折线子类中有效。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Polygon2(const Handle(Poly_Polygon2D)&)
{
  // 设置第二条 2D 折线：基类默认不支持。
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myLocation)
  // 输出字段：myLocation（本曲线表示的 Location）
}
