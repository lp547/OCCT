// Created on: 1991-07-02
// Created by: Remi LEQUETTE
// Copyright (c) 1991-1999 Matra Datavision
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

#include <BRep_Builder.hxx>
#include <BRep_Curve3D.hxx>
#include <BRep_CurveOn2Surfaces.hxx>
#include <BRep_CurveOnClosedSurface.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_ListIteratorOfListOfPointRepresentation.hxx>
#include <BRep_PointOnCurve.hxx>
#include <BRep_PointOnCurveOnSurface.hxx>
#include <BRep_PointOnSurface.hxx>
#include <BRep_Polygon3D.hxx>
#include <BRep_PolygonOnClosedSurface.hxx>
#include <BRep_PolygonOnClosedTriangulation.hxx>
#include <BRep_PolygonOnSurface.hxx>
#include <BRep_PolygonOnTriangulation.hxx>
#include <BRep_TEdge.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <Standard_DomainError.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_LockedShape.hxx>
#include <TopoDS_Vertex.hxx>

//=================================================================================================
//  辅助函数区域
//  这些静态函数用于管理 Edge (边) 和 Vertex (顶点) 的内部几何表示列表。
//  它们不是 BRep_Builder 的成员函数，而是文件内部使用的工具。
//=================================================================================================

//=======================================================================
// function : UpdateCurves
// purpose  : Insert a 3d curve <C> with location <L>
//            in a list of curve representations <lcr>
//            (在曲线表示列表中插入或更新 3D 曲线)
//=======================================================================
static void UpdateCurves(BRep_ListOfCurveRepresentation& lcr,
                         const Handle(Geom_Curve)&       C,
                         const TopLoc_Location&          L)
{
  // 遍历现有的几何表示列表
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_GCurve)                          GC;
  Standard_Real                                f = 0., l = 0.;

  while (itcr.More())
  {
    // 尝试将当前表示转换为几何曲线 (GCurve)
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull())
    {
      // 记录当前的参数范围，以防我们需要保留它
      GC->Range(f, l);
      // 如果找到了现有的 3D 曲线表示，就停止搜索，准备更新它
      if (GC->IsCurve3D())
        break;
    }
    itcr.Next();
  }

  if (itcr.More())
  {
    // 如果找到了，更新它的 3D 曲线和位置
    itcr.Value()->Curve3D(C);
    itcr.Value()->Location(L);
  }
  else
  {
    // 如果没找到，创建一个新的 3D 曲线表示
    Handle(BRep_Curve3D) C3d = new BRep_Curve3D(C, L);
    // 如果之前有其他几何表示提供了参数范围，应用它
    if (!GC.IsNull())
    {
      C3d->SetRange(f, l);
    }
    // 添加到列表末尾
    lcr.Append(C3d);
  }
}

//=======================================================================
// function : UpdateCurves
// purpose  : Insert a pcurve <C> on surface <S> with location <L>
//            in a list of curve representations <lcr>
//            Remove the pcurve on <S> from <lcr> if <C> is null
//            (插入、更新或移除面上的 PCurve)
//=======================================================================

static void UpdateCurves(BRep_ListOfCurveRepresentation& lcr,
                         const Handle(Geom2d_Curve)&     C,
                         const Handle(Geom_Surface)&     S,
                         const TopLoc_Location&          L)
{
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_CurveRepresentation)             cr;
  Handle(BRep_GCurve)                          GC;
  Standard_Real f = -Precision::Infinite(), l = Precision::Infinite();

  // 1. 搜索现有的 3D 曲线范围 (作为默认范围)
  // 2. 查找是否已经存在该面 S 上的 PCurve，如果存在则移除旧的
  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull())
    {
      if (GC->IsCurve3D())
      {
        // 获取 3D 曲线的参数范围
        GC->Range(f, l);
      }
      // 检查是否是当前面 S 上的曲线
      if (GC->IsCurveOnSurface(S, L))
      {
        // 移除旧的表示
        // cr 用于保持引用，防止对象被立即销毁
        cr = itcr.Value();
        lcr.Remove(itcr);
      }
      else
      {
        itcr.Next();
      }
    }
    else
    {
      itcr.Next();
    }
  }

  // 如果传入的曲线 C 不为空，创建一个新的 PCurve 表示并添加
  if (!C.IsNull())
  {
    Handle(BRep_CurveOnSurface) COS   = new BRep_CurveOnSurface(C, S, L);
    Standard_Real               aFCur = 0.0, aLCur = 0.0;
    COS->Range(aFCur, aLCur); // 获取新曲线的默认范围

    // 如果之前找到了有效的 3D 范围，使用它
    if (!Precision::IsInfinite(f))
    {
      aFCur = f;
    }
    if (!Precision::IsInfinite(l))
    {
      aLCur = l;
    }

    COS->SetRange(aFCur, aLCur);
    lcr.Append(COS);
  }
}

//=======================================================================
// function : UpdateCurves
// purpose  : Insert a pcurve <C> on surface <S> with location <L>
//            (带 UV 边界点的版本)
//=======================================================================
static void UpdateCurves(BRep_ListOfCurveRepresentation& lcr,
                         const Handle(Geom2d_Curve)&     C,
                         const Handle(Geom_Surface)&     S,
                         const TopLoc_Location&          L,
                         const gp_Pnt2d&                 Pf,
                         const gp_Pnt2d&                 Pl)
{
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_CurveRepresentation)             cr;
  Handle(BRep_GCurve)                          GC;
  Standard_Real f = -Precision::Infinite(), l = Precision::Infinite();

  // 逻辑同上：搜索范围，移除旧表示
  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull())
    {
      if (GC->IsCurve3D())
      {
        GC->Range(f, l);
      }
      if (GC->IsCurveOnSurface(S, L))
      {
        cr = itcr.Value();
        lcr.Remove(itcr);
      }
      else
      {
        itcr.Next();
      }
    }
    else
    {
      itcr.Next();
    }
  }

  // 添加新 PCurve 并设置 UV 边界点
  if (!C.IsNull())
  {
    Handle(BRep_CurveOnSurface) COS   = new BRep_CurveOnSurface(C, S, L);
    Standard_Real               aFCur = 0.0, aLCur = 0.0;
    COS->Range(aFCur, aLCur);
    if (!Precision::IsInfinite(f))
    {
      aFCur = f;
    }
    if (!Precision::IsInfinite(l))
    {
      aLCur = l;
    }

    COS->SetRange(aFCur, aLCur);
    COS->SetUVPoints(Pf, Pl); // 设置 UV 空间的首尾点
    lcr.Append(COS);
  }
}

//=======================================================================
// function : UpdateCurves
// purpose  : Insert two pcurves <C1,C2> on surface <S> with location <L>
//            (用于闭合面，同时插入两条 PCurve)
//=======================================================================
static void UpdateCurves(BRep_ListOfCurveRepresentation& lcr,
                         const Handle(Geom2d_Curve)&     C1,
                         const Handle(Geom2d_Curve)&     C2,
                         const Handle(Geom_Surface)&     S,
                         const TopLoc_Location&          L)
{
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_CurveRepresentation)             cr;
  Handle(BRep_GCurve)                          GC;
  Standard_Real f = -Precision::Infinite(), l = Precision::Infinite();

  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull())
    {
      if (GC->IsCurve3D())
      {
        GC->Range(f, l);
      }
      Standard_Boolean iscos = GC->IsCurveOnSurface(S, L);
      if (iscos)
        break; // 找到了现有的，跳出循环准备移除
    }
    itcr.Next();
  }

  if (itcr.More())
  {
    cr = itcr.Value();
    lcr.Remove(itcr); // 移除旧的
  }

  // 如果两条曲线都有效，创建 CurveOnClosedSurface
  if (!C1.IsNull() && !C2.IsNull())
  {
    Handle(BRep_CurveOnClosedSurface) COS = new BRep_CurveOnClosedSurface(C1, C2, S, L, GeomAbs_C0);
    Standard_Real                     aFCur = 0.0, aLCur = 0.0;
    COS->Range(aFCur, aLCur);
    if (!Precision::IsInfinite(f))
    {
      aFCur = f;
    }
    if (!Precision::IsInfinite(l))
    {
      aLCur = l;
    }

    COS->SetRange(aFCur, aLCur);
    lcr.Append(COS);
  }
}

//=======================================================================
// function : UpdateCurves
// purpose  : Insert two pcurves <C1,C2> on surface <S> with location <L>
//            (带 UV 边界点的闭合面版本)
//=======================================================================
static void UpdateCurves(BRep_ListOfCurveRepresentation& lcr,
                         const Handle(Geom2d_Curve)&     C1,
                         const Handle(Geom2d_Curve)&     C2,
                         const Handle(Geom_Surface)&     S,
                         const TopLoc_Location&          L,
                         const gp_Pnt2d&                 Pf,
                         const gp_Pnt2d&                 Pl)
{
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_CurveRepresentation)             cr;
  Handle(BRep_GCurve)                          GC;
  Standard_Real f = -Precision::Infinite(), l = Precision::Infinite();

  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull())
    {
      if (GC->IsCurve3D())
      {
        GC->Range(f, l);
      }
      Standard_Boolean iscos = GC->IsCurveOnSurface(S, L);
      if (iscos)
        break;
    }
    itcr.Next();
  }

  if (itcr.More())
  {
    cr = itcr.Value();
    lcr.Remove(itcr);
  }

  if (!C1.IsNull() && !C2.IsNull())
  {
    Handle(BRep_CurveOnClosedSurface) COS = new BRep_CurveOnClosedSurface(C1, C2, S, L, GeomAbs_C0);
    Standard_Real                     aFCur = 0.0, aLCur = 0.0;
    COS->Range(aFCur, aLCur);
    if (!Precision::IsInfinite(f))
    {
      aFCur = f;
    }
    if (!Precision::IsInfinite(l))
    {
      aLCur = l;
    }

    COS->SetRange(aFCur, aLCur);
    COS->SetUVPoints2(Pf, Pl); // 设置 UV 边界点
    lcr.Append(COS);
  }
}

// 更新连续性 (Regularity) 表示
static void UpdateCurves(BRep_ListOfCurveRepresentation& lcr,
                         const Handle(Geom_Surface)&     S1,
                         const Handle(Geom_Surface)&     S2,
                         const TopLoc_Location&          L1,
                         const TopLoc_Location&          L2,
                         const GeomAbs_Shape             C)
{
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr     = itcr.Value();
    Standard_Boolean                        isregu = cr->IsRegularity(S1, S2, L1, L2);
    if (isregu)
      break;
    itcr.Next();
  }

  if (itcr.More())
  {
    Handle(BRep_CurveRepresentation) cr = itcr.Value();
    cr->Continuity(C); // 更新现有连续性
  }
  else
  {
    Handle(BRep_CurveOn2Surfaces) COS = new BRep_CurveOn2Surfaces(S1, S2, L1, L2, C); // 创建新连续性记录
    lcr.Append(COS);
  }
}

// 更新顶点的点表示：在 3D 曲线上的点
static void UpdatePoints(BRep_ListOfPointRepresentation& lpr,
                         Standard_Real                   p,
                         const Handle(Geom_Curve)&       C,
                         const TopLoc_Location&          L)
{
  BRep_ListIteratorOfListOfPointRepresentation itpr(lpr);
  while (itpr.More())
  {
    const Handle(BRep_PointRepresentation)& pr     = itpr.Value();
    Standard_Boolean                        isponc = pr->IsPointOnCurve(C, L);
    if (isponc)
      break;
    itpr.Next();
  }

  if (itpr.More())
  {
    Handle(BRep_PointRepresentation) pr = itpr.Value();
    pr->Parameter(p); // 更新参数
  }
  else
  {
    Handle(BRep_PointOnCurve) POC = new BRep_PointOnCurve(p, C, L); // 新增点表示
    lpr.Append(POC);
  }
}

// 更新顶点的点表示：在 PCurve 上的点
static void UpdatePoints(BRep_ListOfPointRepresentation& lpr,
                         Standard_Real                   p,
                         const Handle(Geom2d_Curve)&     PC,
                         const Handle(Geom_Surface)&     S,
                         const TopLoc_Location&          L)
{
  BRep_ListIteratorOfListOfPointRepresentation itpr(lpr);
  while (itpr.More())
  {
    const Handle(BRep_PointRepresentation)& pr        = itpr.Value();
    Standard_Boolean                        isponcons = pr->IsPointOnCurveOnSurface(PC, S, L);
    if (isponcons)
      break;
    itpr.Next();
  }

  if (itpr.More())
  {
    Handle(BRep_PointRepresentation) pr = itpr.Value();
    pr->Parameter(p);
  }
  else
  {
    Handle(BRep_PointOnCurveOnSurface) POCS = new BRep_PointOnCurveOnSurface(p, PC, S, L);
    lpr.Append(POCS);
  }
}

// 更新顶点的点表示：在曲面上的点 (U, V)
static void UpdatePoints(BRep_ListOfPointRepresentation& lpr,
                         Standard_Real                   p1, // U
                         Standard_Real                   p2, // V
                         const Handle(Geom_Surface)&     S,
                         const TopLoc_Location&          L)
{
  BRep_ListIteratorOfListOfPointRepresentation itpr(lpr);
  while (itpr.More())
  {
    const Handle(BRep_PointRepresentation)& pr     = itpr.Value();
    Standard_Boolean                        ispons = pr->IsPointOnSurface(S, L);
    if (ispons)
      break;
    itpr.Next();
  }

  if (itpr.More())
  {
    Handle(BRep_PointRepresentation) pr = itpr.Value();
    pr->Parameter(p1);
    pr->Parameter2(p2);
  }
  else
  {
    Handle(BRep_PointOnSurface) POS = new BRep_PointOnSurface(p1, p2, S, L);
    lpr.Append(POS);
  }
}

//=================================================================================================
//  BRep_Builder 类方法实现
//=================================================================================================

// 创建面：绑定几何曲面
void BRep_Builder::MakeFace(TopoDS_Face&                F,
                            const Handle(Geom_Surface)& S,
                            const Standard_Real         Tol) const
{
  Handle(BRep_TFace) TF = new BRep_TFace(); // 1. 创建底层 TFace
  if (!F.IsNull() && F.Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::MakeFace");
  }
  TF->Surface(S);   // 2. 设置曲面
  TF->Tolerance(Tol); // 3. 设置公差
  MakeShape(F, TF); // 4. 封装为 TopoDS_Face
}

//=================================================================================================

// 创建面：绑定三角网格
void BRep_Builder::MakeFace(TopoDS_Face&                      theFace,
                            const Handle(Poly_Triangulation)& theTriangulation) const
{
  Handle(BRep_TFace) aTFace = new BRep_TFace();
  if (!theFace.IsNull() && theFace.Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::MakeFace");
  }
  aTFace->Triangulation(theTriangulation); // 设置网格
  MakeShape(theFace, aTFace);
}

//=================================================================================================

// 创建面：绑定网格列表
void BRep_Builder::MakeFace(TopoDS_Face&                      theFace,
                            const Poly_ListOfTriangulation&   theTriangulations,
                            const Handle(Poly_Triangulation)& theActiveTriangulation) const
{
  Handle(BRep_TFace) aTFace = new BRep_TFace();
  if (!theFace.IsNull() && theFace.Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::MakeFace");
  }
  aTFace->Triangulations(theTriangulations, theActiveTriangulation);
  MakeShape(theFace, aTFace);
}

//=================================================================================================

// 创建面：绑定曲面、位置和公差
void BRep_Builder::MakeFace(TopoDS_Face&                F,
                            const Handle(Geom_Surface)& S,
                            const TopLoc_Location&      L,
                            const Standard_Real         Tol) const
{
  Handle(BRep_TFace) TF = new BRep_TFace();
  if (!F.IsNull() && F.Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::MakeFace");
  }
  TF->Surface(S);
  TF->Tolerance(Tol);
  TF->Location(L); // 设置位置
  MakeShape(F, TF);
}

//=================================================================================================

// 更新面：修改曲面、位置和公差
void BRep_Builder::UpdateFace(const TopoDS_Face&          F,
                              const Handle(Geom_Surface)& S,
                              const TopLoc_Location&      L,
                              const Standard_Real         Tol) const
{
  // 获取底层 TFace 指针
  const Handle(BRep_TFace)& TF = *((Handle(BRep_TFace)*)&F.TShape());
  if (TF->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateFace");
  }
  TF->Surface(S);
  TF->Tolerance(Tol);
  // 计算相对位置：因为 TopoDS_Face 本身可能已有位置 F.Location()
  // 我们需要存入 TFace 的位置应该是 L 相对于 F.Location() 的部分
  TF->Location(L.Predivided(F.Location()));
  F.TShape()->Modified(Standard_True); // 标记为已修改
}

//=================================================================================================

// 更新面：修改三角网格
void BRep_Builder::UpdateFace(const TopoDS_Face&                theFace,
                              const Handle(Poly_Triangulation)& theTriangulation,
                              const Standard_Boolean            theToReset) const
{
  const Handle(BRep_TFace)& aTFace = *((Handle(BRep_TFace)*)&theFace.TShape());
  if (aTFace->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateFace");
  }
  aTFace->Triangulation(theTriangulation, theToReset);
  theFace.TShape()->Modified(Standard_True);
}

//=================================================================================================

// 更新面：修改公差
void BRep_Builder::UpdateFace(const TopoDS_Face& F, const Standard_Real Tol) const
{
  const Handle(BRep_TFace)& TF = *((Handle(BRep_TFace)*)&F.TShape());
  if (TF->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateFace");
  }
  TF->Tolerance(Tol);
  F.TShape()->Modified(Standard_True);
}

//=================================================================================================

// 设置面的自然边界标志
void BRep_Builder::NaturalRestriction(const TopoDS_Face& F, const Standard_Boolean N) const
{
  const Handle(BRep_TFace)& TF = (*((Handle(BRep_TFace)*)&F.TShape()));
  if (TF->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::NaturalRestriction");
  }
  TF->NaturalRestriction(N);
  F.TShape()->Modified(Standard_True);
}

//=================================================================================================

// 创建边：空边
void BRep_Builder::MakeEdge(TopoDS_Edge& E) const
{
  Handle(BRep_TEdge) TE = new BRep_TEdge();
  if (!E.IsNull() && E.Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::MakeEdge");
  }
  MakeShape(E, TE);
}

//=================================================================================================

// 更新边：设置 3D 曲线
void BRep_Builder::UpdateEdge(const TopoDS_Edge&        E,
                              const Handle(Geom_Curve)& C,
                              const TopLoc_Location&    L,
                              const Standard_Real       Tol) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  // 计算相对位置
  const TopLoc_Location l = L.Predivided(E.Location());

  // 调用辅助函数更新曲线列表
  UpdateCurves(TE->ChangeCurves(), C, l);

  TE->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：设置 PCurve
void BRep_Builder::UpdateEdge(const TopoDS_Edge&          E,
                              const Handle(Geom2d_Curve)& C,
                              const Handle(Geom_Surface)& S,
                              const TopLoc_Location&      L,
                              const Standard_Real         Tol) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  const TopLoc_Location l = L.Predivided(E.Location());

  UpdateCurves(TE->ChangeCurves(), C, S, l);

  TE->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=======================================================================
// function : UpdateEdge
// purpose  : for the second format (for XML Persistence)
//            (带 UV 点的 PCurve 更新)
//=======================================================================

void BRep_Builder::UpdateEdge(const TopoDS_Edge&          E,
                              const Handle(Geom2d_Curve)& C,
                              const Handle(Geom_Surface)& S,
                              const TopLoc_Location&      L,
                              const Standard_Real         Tol,
                              const gp_Pnt2d&             Pf,
                              const gp_Pnt2d&             Pl) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  const TopLoc_Location l = L.Predivided(E.Location());

  UpdateCurves(TE->ChangeCurves(), C, S, l, Pf, Pl);

  TE->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：设置闭合 PCurve (两条)
void BRep_Builder::UpdateEdge(const TopoDS_Edge&          E,
                              const Handle(Geom2d_Curve)& C1,
                              const Handle(Geom2d_Curve)& C2,
                              const Handle(Geom_Surface)& S,
                              const TopLoc_Location&      L,
                              const Standard_Real         Tol) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  const TopLoc_Location l = L.Predivided(E.Location());

  UpdateCurves(TE->ChangeCurves(), C1, C2, S, l);

  TE->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=======================================================================
// function : UpdateEdge
// purpose  : for the second format (for XML Persistence)
//            (带 UV 点的闭合 PCurve 更新)
//=======================================================================

void BRep_Builder::UpdateEdge(const TopoDS_Edge&          E,
                              const Handle(Geom2d_Curve)& C1,
                              const Handle(Geom2d_Curve)& C2,
                              const Handle(Geom_Surface)& S,
                              const TopLoc_Location&      L,
                              const Standard_Real         Tol,
                              const gp_Pnt2d&             Pf,
                              const gp_Pnt2d&             Pl) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  const TopLoc_Location l = L.Predivided(E.Location());

  UpdateCurves(TE->ChangeCurves(), C1, C2, S, l, Pf, Pl);

  TE->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：设置 3D 多边形
void BRep_Builder::UpdateEdge(const TopoDS_Edge&            E,
                              const Handle(Poly_Polygon3D)& P,
                              const TopLoc_Location&        L) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);

  // 查找并替换现有的 3D 多边形
  while (itcr.More())
  {
    if (itcr.Value()->IsPolygon3D())
    {
      if (P.IsNull())
        lcr.Remove(itcr); // 如果传入空，移除现有
      else
        itcr.Value()->Polygon3D(P); // 否则更新
      TE->Modified(Standard_True);
      return;
    }
    itcr.Next();
  }

  // 添加新的 3D 多边形
  const TopLoc_Location  l   = L.Predivided(E.Location());
  Handle(BRep_Polygon3D) P3d = new BRep_Polygon3D(P, l);
  lcr.Append(P3d);

  TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：设置三角网格上的多边形
void BRep_Builder::UpdateEdge(const TopoDS_Edge&                         E,
                              const Handle(Poly_PolygonOnTriangulation)& P,
                              const Handle(Poly_Triangulation)&          T,
                              const TopLoc_Location&                     L) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  const TopLoc_Location l = L.Predivided(E.Location());

  Standard_Boolean isModified = Standard_False;

  BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_CurveRepresentation)             cr;

  // 移除该网格上已有的多边形表示
  while (itcr.More())
  {
    if (itcr.Value()->IsPolygonOnTriangulation(T, l))
    {
      cr = itcr.Value();
      lcr.Remove(itcr);
      isModified = Standard_True;
      break;
    }
    itcr.Next();
  }

  // 添加新的
  if (!P.IsNull())
  {
    Handle(BRep_PolygonOnTriangulation) PT = new BRep_PolygonOnTriangulation(P, T, l);
    lcr.Append(PT);
    isModified = Standard_True;
  }

  if (isModified)
    TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：设置闭合三角网格上的多边形
void BRep_Builder::UpdateEdge(const TopoDS_Edge&                         E,
                              const Handle(Poly_PolygonOnTriangulation)& P1,
                              const Handle(Poly_PolygonOnTriangulation)& P2,
                              const Handle(Poly_Triangulation)&          T,
                              const TopLoc_Location&                     L) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  const TopLoc_Location l = L.Predivided(E.Location());

  Standard_Boolean isModified = Standard_False;

  BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_CurveRepresentation)             cr;

  while (itcr.More())
  {
    if (itcr.Value()->IsPolygonOnTriangulation(T, l)) 
    {
      cr = itcr.Value();
      lcr.Remove(itcr);
      isModified = Standard_True;
      break;
    }
    itcr.Next();
  }

  if (!P1.IsNull() && !P2.IsNull())
  {
    Handle(BRep_PolygonOnClosedTriangulation) PT =
      new BRep_PolygonOnClosedTriangulation(P1, P2, T, l);
    lcr.Append(PT);
    isModified = Standard_True;
  }

  if (isModified)
    TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：设置面上的 2D 多边形 (离散 PCurve)
void BRep_Builder::UpdateEdge(const TopoDS_Edge&            E,
                              const Handle(Poly_Polygon2D)& P,
                              const TopoDS_Face&            F) const
{
  TopLoc_Location             l;
  const Handle(Geom_Surface)& S = BRep_Tool::Surface(F, l);
  UpdateEdge(E, P, S, l);
}

//=================================================================================================

// 更新边：设置曲面上的 2D 多边形
void BRep_Builder::UpdateEdge(const TopoDS_Edge&            E,
                              const Handle(Poly_Polygon2D)& P,
                              const Handle(Geom_Surface)&   S,
                              const TopLoc_Location&        L) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  TopLoc_Location l = L.Predivided(E.Location());

  BRep_ListOfCurveRepresentation&  lcr = TE->ChangeCurves();
  Handle(BRep_CurveRepresentation) cr;

  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  while (itcr.More())
  {
    if (itcr.Value()->IsPolygonOnSurface(S, l))
      break;
    itcr.Next();
  }

  if (itcr.More())
  {
    cr = itcr.Value();
    lcr.Remove(itcr);
  }

  if (!P.IsNull())
  {
    Handle(BRep_PolygonOnSurface) PS = new BRep_PolygonOnSurface(P, S, l);
    lcr.Append(PS);
  }

  TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：设置面上的闭合 2D 多边形
void BRep_Builder::UpdateEdge(const TopoDS_Edge&            E,
                              const Handle(Poly_Polygon2D)& P1,
                              const Handle(Poly_Polygon2D)& P2,
                              const TopoDS_Face&            F) const
{
  TopLoc_Location             l;
  const Handle(Geom_Surface)& S = BRep_Tool::Surface(F, l);
  UpdateEdge(E, P1, P2, S, l);
}

//=================================================================================================

// 更新边：设置曲面上的闭合 2D 多边形
void BRep_Builder::UpdateEdge(const TopoDS_Edge&            E,
                              const Handle(Poly_Polygon2D)& P1,
                              const Handle(Poly_Polygon2D)& P2,
                              const Handle(Geom_Surface)&   S,
                              const TopLoc_Location&        L) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  TopLoc_Location l = L.Predivided(E.Location());

  BRep_ListOfCurveRepresentation&  lcr = TE->ChangeCurves();
  Handle(BRep_CurveRepresentation) cr;

  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  while (itcr.More())
  {
    if (itcr.Value()->IsPolygonOnSurface(S, l))
      break;
    itcr.Next();
  }

  if (itcr.More())
  {
    cr = itcr.Value();
    lcr.Remove(itcr);
  }

  if (!P1.IsNull() && !P2.IsNull())
  {
    Handle(BRep_PolygonOnClosedSurface) PS =
      new BRep_PolygonOnClosedSurface(P1, P2, S, TopLoc_Location());
    lcr.Append(PS);
  }

  TE->Modified(Standard_True);
}

//=================================================================================================

// 更新边：修改公差
void BRep_Builder::UpdateEdge(const TopoDS_Edge& E, const Standard_Real Tol) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateEdge");
  }
  TE->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=================================================================================================

// 设置边的连续性 (通过面)
void BRep_Builder::Continuity(const TopoDS_Edge&  E,
                              const TopoDS_Face&  F1,
                              const TopoDS_Face&  F2,
                              const GeomAbs_Shape C) const
{
  TopLoc_Location             l1, l2;
  const Handle(Geom_Surface)& S1 = BRep_Tool::Surface(F1, l1);
  const Handle(Geom_Surface)& S2 = BRep_Tool::Surface(F2, l2);
  Continuity(E, S1, S2, l1, l2, C);
}

//=================================================================================================

// 设置边的连续性 (通过曲面)
void BRep_Builder::Continuity(const TopoDS_Edge&          E,
                              const Handle(Geom_Surface)& S1,
                              const Handle(Geom_Surface)& S2,
                              const TopLoc_Location&      L1,
                              const TopLoc_Location&      L2,
                              const GeomAbs_Shape         C) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::Continuity");
  }
  const TopLoc_Location l1 = L1.Predivided(E.Location());
  const TopLoc_Location l2 = L2.Predivided(E.Location());

  UpdateCurves(TE->ChangeCurves(), S1, S2, l1, l2, C);

  TE->Modified(Standard_True);
}

//=================================================================================================

// 设置 SameParameter 标志
void BRep_Builder::SameParameter(const TopoDS_Edge& E, const Standard_Boolean S) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::SameParameter");
  }
  TE->SameParameter(S);
  TE->Modified(Standard_True);
}

//=================================================================================================

// 设置 SameRange 标志
void BRep_Builder::SameRange(const TopoDS_Edge& E, const Standard_Boolean S) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::SameRange");
  }
  TE->SameRange(S);
  TE->Modified(Standard_True);
}

//=================================================================================================

// 设置 Degenerated (退化) 标志
void BRep_Builder::Degenerated(const TopoDS_Edge& E, const Standard_Boolean D) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::Degenerated");
  }
  TE->Degenerated(D);
  if (D)
  {
    // 如果是退化边，设置一个空的 3D 曲线
    UpdateCurves(TE->ChangeCurves(), Handle(Geom_Curve)(), E.Location());
  }
  TE->Modified(Standard_True);
}

//=================================================================================================

// 设置边的参数范围
void BRep_Builder::Range(const TopoDS_Edge&     E,
                         const Standard_Real    First,
                         const Standard_Real    Last,
                         const Standard_Boolean Only3d) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::Range");
  }
  BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_GCurve)                          GC;

  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    // 如果 Only3d 为 True，则只更新 3D 曲线的范围
    // 否则更新所有曲线表示的范围
    if (!GC.IsNull() && (!Only3d || GC->IsCurve3D()))
      GC->SetRange(First, Last);
    itcr.Next();
  }

  TE->Modified(Standard_True);
}

//=================================================================================================

// 设置边在特定曲面上的参数范围
void BRep_Builder::Range(const TopoDS_Edge&          E,
                         const Handle(Geom_Surface)& S,
                         const TopLoc_Location&      L,
                         const Standard_Real         First,
                         const Standard_Real         Last) const
{
  const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
  if (TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::Range");
  }
  const TopLoc_Location l = L.Predivided(E.Location());

  BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_GCurve)                          GC;

  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull() && GC->IsCurveOnSurface(S, l))
    {
      GC->SetRange(First, Last);
      break;
    }
    itcr.Next();
  }

  if (!itcr.More())
    throw Standard_DomainError("BRep_Builder::Range, no pcurve");

  TE->Modified(Standard_True);
}

//=================================================================================================

// 转移几何表示：把 Ein 的所有几何信息复制给 Eout
void BRep_Builder::Transfert(const TopoDS_Edge& Ein, const TopoDS_Edge& Eout) const
{
  const Handle(BRep_TEdge)& TE  = *((Handle(BRep_TEdge)*)&Ein.TShape());
  const Standard_Real       tol = TE->Tolerance();

  const BRep_ListOfCurveRepresentation&        lcr = TE->Curves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& CR = itcr.Value();

    // 复制 PCurve
    if (CR->IsCurveOnSurface())
    {
      UpdateEdge(Eout, CR->PCurve(), CR->Surface(), Ein.Location() * CR->Location(), tol);
    }
    // 复制闭合 PCurve
    else if (CR->IsCurveOnClosedSurface())
    {
      UpdateEdge(Eout,
                 CR->PCurve(),
                 CR->PCurve2(),
                 CR->Surface(),
                 Ein.Location() * CR->Location(),
                 tol);
    }
    // 复制连续性
    if (CR->IsRegularity())
    {
      Continuity(Eout,
                 CR->Surface(),
                 CR->Surface2(),
                 Ein.Location() * CR->Location(),
                 Ein.Location() * CR->Location2(),
                 CR->Continuity());
    }

    itcr.Next();
  }
}

//=======================================================================
// function : UpdateVertex
// purpose  : update vertex with 3d point
//            (更新顶点的 3D 坐标)
//=======================================================================

void BRep_Builder::UpdateVertex(const TopoDS_Vertex& V,
                                const gp_Pnt&        P,
                                const Standard_Real  Tol) const
{
  const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V.TShape());
  if (TV->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateVertex");
  }
  // 注意：需要将点 P 变换到 TVertex 的局部坐标系中
  TV->Pnt(P.Transformed(V.Location().Inverted().Transformation()));
  TV->UpdateTolerance(Tol);
  TV->Modified(Standard_True);
}

//=======================================================================
// function : UpdateVertex
// purpose  : update vertex with parameter on edge
//            (更新顶点在边上的参数)
//=======================================================================

void BRep_Builder::UpdateVertex(const TopoDS_Vertex& V,
                                const Standard_Real  Par,
                                const TopoDS_Edge&   E,
                                const Standard_Real  Tol) const
{
  if (Precision::IsPositiveInfinite(Par) || Precision::IsNegativeInfinite(Par))
    throw Standard_DomainError("BRep_Builder::Infinite parameter");

  const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V.TShape());
  const Handle(BRep_TEdge)&   TE = *((Handle(BRep_TEdge)*)&E.TShape());

  if (TV->Locked() || TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateVertex");
  }

  TopLoc_Location L = E.Location().Predivided(V.Location());

  // 1. 在边 E 中寻找当前顶点 V，确定其方向 (Orientation)
  TopAbs_Orientation ori = TopAbs_INTERNAL;

  TopoDS_Iterator itv(E.Oriented(TopAbs_FORWARD));

  // 如果边没有顶点且是退化的，使用顶点本身的方向
  if (!itv.More() && TE->Degenerated())
    ori = V.Orientation();

  while (itv.More())
  {
    const TopoDS_Shape& Vcur = itv.Value();
    if (V.IsSame(Vcur))
    {
      ori = Vcur.Orientation();
      if (ori == V.Orientation())
        break; // 找到了方向匹配的
    }
    itv.Next();
  }

  // 2. 更新边 E 的所有几何表示中的参数
  BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_GCurve)                          GC;

  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull())
    {
      // 如果顶点方向是 FORWARD，设置起始参数
      if (ori == TopAbs_FORWARD)
        GC->First(Par);
      // 如果顶点方向是 REVERSED，设置终止参数
      else if (ori == TopAbs_REVERSED)
        GC->Last(Par);
      else
      {
        // 如果是 INTERNAL/EXTERNAL，则需要添加一个 PointOnCurve 表示到 TVertex
        BRep_ListOfPointRepresentation& lpr    = TV->ChangePoints();
        const TopLoc_Location&          GCloc  = GC->Location();
        TopLoc_Location                 LGCloc = L * GCloc;
        if (GC->IsCurve3D())
        {
          const Handle(Geom_Curve)& GC3d = GC->Curve3D();
          UpdatePoints(lpr, Par, GC3d, LGCloc);
        }
        else if (GC->IsCurveOnSurface())
        {
          const Handle(Geom2d_Curve)& GCpc = GC->PCurve();
          const Handle(Geom_Surface)& GCsu = GC->Surface();
          UpdatePoints(lpr, Par, GCpc, GCsu, LGCloc);
        }
      }
    }
    itcr.Next();
  }

  if ((ori != TopAbs_FORWARD) && (ori != TopAbs_REVERSED))
    TV->Modified(Standard_True);
  TV->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=======================================================================
// function : UpdateVertex
// purpose  : update vertex with parameter on edge on face
//            (更新顶点在面上的边上的参数)
//=======================================================================

void BRep_Builder::UpdateVertex(const TopoDS_Vertex&        V,
                                const Standard_Real         Par,
                                const TopoDS_Edge&          E,
                                const Handle(Geom_Surface)& S,
                                const TopLoc_Location&      L,
                                const Standard_Real         Tol) const
{
  if (Precision::IsPositiveInfinite(Par) || Precision::IsNegativeInfinite(Par))
    throw Standard_DomainError("BRep_Builder::Infinite parameter");

  TopLoc_Location l = L.Predivided(V.Location());

  const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V.TShape());
  const Handle(BRep_TEdge)&   TE = *((Handle(BRep_TEdge)*)&E.TShape());

  if (TV->Locked() || TE->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateVertex");
  }

  // 1. 寻找顶点方向
  TopAbs_Orientation ori = TopAbs_INTERNAL;
  TopoDS_Iterator itv(E.Oriented(TopAbs_FORWARD));

  if (!itv.More() && TE->Degenerated())
    ori = V.Orientation();

  while (itv.More())
  {
    const TopoDS_Shape& Vcur = itv.Value();
    if (V.IsSame(Vcur))
    {
      ori = Vcur.Orientation();
      if (ori == V.Orientation())
        break;
    }
    itv.Next();
  }

  // 2. 找到对应的 PCurve 并更新
  BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
  BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
  Handle(BRep_GCurve)                          GC;

  while (itcr.More())
  {
    GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (!GC.IsNull())
    {
      // 检查是否是指定曲面 S 上的曲线
      if (GC->IsCurveOnSurface(S, L))
      {
        if (ori == TopAbs_FORWARD)
          GC->First(Par);
        else if (ori == TopAbs_REVERSED)
          GC->Last(Par);
        else
        {
          BRep_ListOfPointRepresentation& lpr  = TV->ChangePoints();
          const Handle(Geom2d_Curve)&     GCpc = GC->PCurve();
          UpdatePoints(lpr, Par, GCpc, S, l);
          TV->Modified(Standard_True);
        }
        break;
      }
    }
    itcr.Next();
  }

  if (!itcr.More())
    throw Standard_DomainError("BRep_Builder:: no pcurve");

  TV->UpdateTolerance(Tol);
  TE->Modified(Standard_True);
}

//=======================================================================
// function : UpdateVertex
// purpose  : update vertex with parameters on face
//            (直接设置顶点在面上的 UV 参数)
//=======================================================================

void BRep_Builder::UpdateVertex(const TopoDS_Vertex& Ve,
                                const Standard_Real  U,
                                const Standard_Real  V,
                                const TopoDS_Face&   F,
                                const Standard_Real  Tol) const
{
  const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&Ve.TShape());

  if (TV->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateVertex");
  }

  TopLoc_Location             L;
  const Handle(Geom_Surface)& S       = BRep_Tool::Surface(F, L);
  L                                   = L.Predivided(Ve.Location());
  BRep_ListOfPointRepresentation& lpr = TV->ChangePoints();
  
  // 添加 PointOnSurface 表示
  UpdatePoints(lpr, U, V, S, L);

  TV->UpdateTolerance(Tol);
  TV->Modified(Standard_True);
}

//=======================================================================
// function : UpdateVertex
// purpose  : update vertex tolerance
//            (仅更新顶点公差)
//=======================================================================

void BRep_Builder::UpdateVertex(const TopoDS_Vertex& V, const Standard_Real Tol) const
{
  const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V.TShape());

  if (TV->Locked())
  {
    throw TopoDS_LockedShape("BRep_Builder::UpdateVertex");
  }

  TV->UpdateTolerance(Tol);
  TV->Modified(Standard_True);
}

//=================================================================================================

// 转移顶点参数
void BRep_Builder::Transfert(const TopoDS_Edge&   Ein,
                             const TopoDS_Edge&   Eout,
                             const TopoDS_Vertex& Vin,
                             const TopoDS_Vertex& Vout) const
{
  const Standard_Real tol   = BRep_Tool::Tolerance(Vin);
  const Standard_Real parin = BRep_Tool::Parameter(Vin, Ein);
  UpdateVertex(Vout, parin, Eout, tol);
}
