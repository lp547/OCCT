// Created on: 1993-07-07
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

#include <BRep_Curve3D.hxx>
#include <BRep_CurveOnClosedSurface.hxx>
#include <BRep_CurveOnSurface.hxx>
#include <BRep_CurveRepresentation.hxx>
#include <BRep_Polygon3D.hxx>
#include <BRep_PolygonOnSurface.hxx>
#include <BRep_PolygonOnTriangulation.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_TFace.hxx>
#include <BRep_Tool.hxx>
#include <BRep_TVertex.hxx>
#include <ElSLib.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor.hxx>
#include <Geom_Curve.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomProjLib.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <NCollection_IncAllocator.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <ProjLib_ProjectedCurve.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_NullObject.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <BRep_GCurve.hxx>

// ======================== 新手速记（先当成一句话）========================
// 这份 .cxx 里实现的都是 BRep_Tool 的“读接口”，核心套路基本就两类：
//
// A) Face / Edge / Vertex  ->  取出内部实现对象（BRep_TFace / BRep_TEdge / BRep_TVertex）
//    然后读它里面存的 Surface/Curve/Point/Polygon/Tolerance 等数据。
//
// B) 处理 Location / Orientation（位置变换与方向）：
//    - Location：TopoDS_* 自己有一个 Location；内部 BRep_T* 也可能有一个 Location；
//      真正使用几何时通常要把它们“乘起来”组合成最终变换。
//    - Orientation：尤其是 Edge 在 Face 上的 2D 曲线(PCurve)，当 Face 反向时，
//      同一条 Edge 在参数空间里的方向也要对应处理，所以会看到 Reverse() 的逻辑。
//
// 看到 static_cast<const BRep_TFace*>(F.TShape().get()) 这种写法别慌：
// 你可以先把它当成一句话：“拿到 Face 背后真正存数据的对象 BRep_TFace”。
// =======================================================================

// modified by NIZNHY-PKV Fri Oct 17 14:13:29 2008f
static Standard_Boolean IsPlane(const Handle(Geom_Surface)& aS);

// modified by NIZNHY-PKV Fri Oct 17 14:13:33 2008t
//
//=======================================================================
// function : Surface
// purpose  : Returns the geometric surface of the face. Returns
//            in <L> the location for the surface.
//=======================================================================

const Handle(Geom_Surface)& BRep_Tool::Surface(const TopoDS_Face& F, TopLoc_Location& L)
{
  // 1) 从拓扑 Face(F) 中取出它背后真正存数据的实现对象：BRep_TFace
  const BRep_TFace* TF = static_cast<const BRep_TFace*>(F.TShape().get());

  // 2) 组合最终的放置变换(Location)：
  //    - F.Location()        ：这个 Face 自己携带的变换（通常来自装配/实例化）
  //    - TF->Location()      ：BRep_TFace 内部保存的局部变换（曲面自己的变换）
  //    - 两者相乘得到“曲面在世界坐标系里的最终变换”
  L = F.Location() * TF->Location();

  // 3) 返回几何曲面句柄（注意：此版本不创建副本，只返回原句柄 + Location 让你自己用）
  return TF->Surface();
}

//=======================================================================
// function : Surface
// purpose  : Returns the geometric  surface of the face. It can
//           be a copy if there is a Location.
//=======================================================================

Handle(Geom_Surface) BRep_Tool::Surface(const TopoDS_Face& F)
{
  // 取出 Face 的内部实现对象（真正存 Surface / Tolerance / Triangulation 的地方）
  const BRep_TFace*           TF = static_cast<const BRep_TFace*>(F.TShape().get());

  // 取出几何曲面句柄（Handle 类似“带引用计数的智能指针”）
  const Handle(Geom_Surface)& S  = TF->Surface();

  if (S.IsNull())
    return S;

  // 组合最终的 Location（同上一个重载）
  TopLoc_Location L = F.Location() * TF->Location();
  if (!L.IsIdentity())
  {
    // 如果存在非单位变换，这个版本会“创建一个应用了变换的几何副本”
    // 新手先记一句话：这个版本返回的 Surface 已经在世界坐标里了
    Handle(Geom_Geometry) aCopy = S->Transformed(L.Transformation());
    Geom_Surface*         aGS   = static_cast<Geom_Surface*>(aCopy.get());
    return Handle(Geom_Surface)(aGS);
  }
  return S;
}

//=================================================================================================

const Handle(Poly_Triangulation)& BRep_Tool::Triangulation(const TopoDS_Face&     theFace,
                                                           TopLoc_Location&       theLocation,
                                                           const Poly_MeshPurpose theMeshPurpose)
{
  // Triangulation（网格）通常和 Face 的 Location 一起使用（用于显示/加速）
  theLocation              = theFace.Location();
  const BRep_TFace* aTFace = static_cast<const BRep_TFace*>(theFace.TShape().get());
  return aTFace->Triangulation(theMeshPurpose);
}

//=================================================================================================

const Poly_ListOfTriangulation& BRep_Tool::Triangulations(const TopoDS_Face& theFace,
                                                          TopLoc_Location&   theLocation)
{
  theLocation              = theFace.Location();
  const BRep_TFace* aTFace = static_cast<const BRep_TFace*>(theFace.TShape().get());
  return aTFace->Triangulations();
}

//=======================================================================
// function : Tolerance
// purpose  : Returns the tolerance of the face.
//=======================================================================

Standard_Real BRep_Tool::Tolerance(const TopoDS_Face& F)
{
  const BRep_TFace*       TF   = static_cast<const BRep_TFace*>(F.TShape().get());
  Standard_Real           p    = TF->Tolerance();
  constexpr Standard_Real pMin = Precision::Confusion();
  if (p > pMin)
    return p;
  else
    return pMin;
}

//=======================================================================
// function : NaturalRestriction
// purpose  : Returns the  NaturalRestriction  flag of the  face.
//=======================================================================

Standard_Boolean BRep_Tool::NaturalRestriction(const TopoDS_Face& F)
{
  const BRep_TFace* TF = static_cast<const BRep_TFace*>(F.TShape().get());
  return TF->NaturalRestriction();
}

//=======================================================================
// function : Curve
// purpose  : Returns the 3D curve  of the edge.  May be  a Null
//           handle. Returns in <L> the location for the curve.
//           In <First> and <Last> the parameter range.
//=======================================================================

static const Handle(Geom_Curve) nullCurve;

const Handle(Geom_Curve)& BRep_Tool::Curve(const TopoDS_Edge& E,
                                           TopLoc_Location&   L,
                                           Standard_Real&     First,
                                           Standard_Real&     Last)
{
  // 一条 Edge 可能有多种“表示”(representation)：
  // - 3D 曲线 (BRep_Curve3D)
  // - 曲面上的 2D 曲线 (BRep_CurveOnSurface)
  // - 3D/2D 离散多边形 (Polygon3D / PolygonOnSurface)
  // 它们都塞在 BRep_TEdge::Curves() 的一个列表里，所以这里需要遍历查找。
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurve3D())
    {
      // 找到 3D 曲线表示后，把它向下转型为 BRep_Curve3D 来读取数据
      const BRep_Curve3D* GC = static_cast<const BRep_Curve3D*>(cr.get());

      // Edge 自己的 Location 也要和曲线表示自身的 Location 组合
      L                      = E.Location() * GC->Location();

      // 这条边对应曲线上的参数范围
      GC->Range(First, Last);
      return GC->Curve3D();
    }
    itcr.Next();
  }
  // 没找到 3D 曲线：按约定返回一个“空句柄引用”，并把输出参数清零
  L.Identity();
  First = Last = 0.;
  return nullCurve;
}

//=======================================================================
// function : Curve
// purpose  : Returns the 3D curve  of the edge. May be a Null handle.
//           In <First> and <Last> the parameter range.
//           It can be a copy if there is a Location.
//=======================================================================

Handle(Geom_Curve) BRep_Tool::Curve(const TopoDS_Edge& E, Standard_Real& First, Standard_Real& Last)
{
  TopLoc_Location           L;
  const Handle(Geom_Curve)& C = Curve(E, L, First, Last);
  if (!C.IsNull())
  {
    if (!L.IsIdentity())
    {
      // 这个重载会创建一个“应用了 Location 的几何副本”
      Handle(Geom_Geometry) aCopy = C->Transformed(L.Transformation());
      Geom_Curve*           aGC   = static_cast<Geom_Curve*>(aCopy.get());
      return Handle(Geom_Curve)(aGC);
    }
  }
  return C;
}

//=======================================================================
// function : IsGeometric
// purpose  : Returns True if <F> has a surface.
//=======================================================================
Standard_Boolean BRep_Tool::IsGeometric(const TopoDS_Face& F)
{
  const BRep_TFace*           TF = static_cast<const BRep_TFace*>(F.TShape().get());
  const Handle(Geom_Surface)& S  = TF->Surface();
  return !S.IsNull();
}

//=======================================================================
// function : IsGeometric
// purpose  : Returns True if <E> is a 3d curve or a curve on
//           surface.
//=======================================================================

Standard_Boolean BRep_Tool::IsGeometric(const TopoDS_Edge& E)
{
  // 遍历 Edge 的所有表示，只要找到“有几何”的表示就认为它是 geometric
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurve3D())
    {
      Handle(BRep_Curve3D) GC(Handle(BRep_Curve3D)::DownCast(cr));
      if (!GC.IsNull() && !GC->Curve3D().IsNull())
        return Standard_True;
    }
    else if (cr->IsCurveOnSurface())
      return Standard_True;
    itcr.Next();
  }
  return Standard_False;
}

//=======================================================================
// function : Polygon3D
// purpose  : Returns the 3D polygon of the edge. May be   a Null
//           handle. Returns in <L> the location for the polygon.
//=======================================================================

static const Handle(Poly_Polygon3D) nullPolygon3D;

const Handle(Poly_Polygon3D)& BRep_Tool::Polygon3D(const TopoDS_Edge& E, TopLoc_Location& L)
{
  // 和 Curve() 类似：遍历 Edge 的表示列表，找 Polygon3D 表示
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygon3D())
    {
      const BRep_Polygon3D* GC = static_cast<const BRep_Polygon3D*>(cr.get());
      L                        = E.Location() * GC->Location();
      return GC->Polygon3D();
    }
    itcr.Next();
  }
  L.Identity();
  return nullPolygon3D;
}

//=======================================================================
// function : CurveOnSurface
// purpose  : Returns the curve  associated to the  edge in  the
//           parametric  space of  the  face.  Returns   a NULL
//           handle  if this curve  does not exist.  Returns in
//           <First> and <Last> the parameter range.
//=======================================================================

Handle(Geom2d_Curve) BRep_Tool::CurveOnSurface(const TopoDS_Edge& E,
                                               const TopoDS_Face& F,
                                               Standard_Real&     First,
                                               Standard_Real&     Last,
                                               Standard_Boolean*  theIsStored)
{
  // 1) 先取出 Face 的几何曲面 + Face 对应的 Location
  TopLoc_Location             l;
  const Handle(Geom_Surface)& S          = BRep_Tool::Surface(F, l);

  // 2) 处理 Face 方向：Face 反向时，Edge 在这个 Face 上的“有效方向”也要反过来
  //    （新手先记：这一步是为了让取到的 PCurve 与当前 Face 的方向约定一致）
  TopoDS_Edge                 aLocalEdge = E;
  if (F.Orientation() == TopAbs_REVERSED)
  {
    aLocalEdge.Reverse();
  }

  // 3) 调用更底层的重载：在指定曲面 S + Location l 上寻找 Edge 的 PCurve
  return CurveOnSurface(aLocalEdge, S, l, First, Last, theIsStored);
}

//=======================================================================
// function : CurveOnSurface
// purpose  : Returns the  curve associated to   the edge in the
//           parametric  space of the   surface. Returns a NULL
//           handle  if this curve does  not exist.  Returns in
//           <First> and <Last> the parameter range.
//=======================================================================

static const Handle(Geom2d_Curve) nullPCurve;

Handle(Geom2d_Curve) BRep_Tool::CurveOnSurface(const TopoDS_Edge&          E,
                                               const Handle(Geom_Surface)& S,
                                               const TopLoc_Location&      L,
                                               Standard_Real&              First,
                                               Standard_Real&              Last,
                                               Standard_Boolean*           theIsStored)
{
  // 这里要构造“相对变换”loc：
  // - 输入的 L 代表“曲面所在的坐标系”
  // - E.Location() 代表“Edge 所在的坐标系”
  // - Predivided 可以先当成一句话：“把 Edge 的变换从 L 里除掉，得到相对关系”
  TopLoc_Location  loc         = L.Predivided(E.Location());
  Standard_Boolean Eisreversed = (E.Orientation() == TopAbs_REVERSED);
  if (theIsStored)
    *theIsStored = Standard_True;

  // 遍历 Edge 的表示列表，找“在指定曲面 S、指定相对 Location loc 上”的那条 PCurve
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface(S, loc))
    {
      const BRep_GCurve* GC = static_cast<const BRep_GCurve*>(cr.get());
      GC->Range(First, Last);
      if (GC->IsCurveOnClosedSurface() && Eisreversed)
        return GC->PCurve2();
      else
        return GC->PCurve();
    }
    itcr.Next();
  }

  // 没找到已存储的 PCurve：如果是平面，就尝试“把 3D 曲线投影到平面”临时生成
  if (theIsStored)
    *theIsStored = Standard_False;
  return CurveOnPlane(E, S, L, First, Last);
}

//=======================================================================
// function : CurveOnPlane
// purpose  : For planar surface returns projection of the edge on the plane
//=======================================================================
Handle(Geom2d_Curve) BRep_Tool::CurveOnPlane(const TopoDS_Edge&          E,
                                             const Handle(Geom_Surface)& S,
                                             const TopLoc_Location&      L,
                                             Standard_Real&              First,
                                             Standard_Real&              Last)
{
  First = Last = 0.;

  // 1) 判断 S 是否“平面/平面修剪面”
  Handle(Geom_Plane)                     GP;
  Handle(Geom_RectangularTrimmedSurface) GRTS;
  GRTS = Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
  if (!GRTS.IsNull())
    GP = Handle(Geom_Plane)::DownCast(GRTS->BasisSurface());
  else
    GP = Handle(Geom_Plane)::DownCast(S);

  if (GP.IsNull())
    // 不是平面：无法用投影的方式构造 PCurve
    return nullPCurve;

  // 2) 需要 Edge 有 3D 曲线才能投影
  Standard_Real      f, l;
  TopLoc_Location    aCurveLocation;
  Handle(Geom_Curve) C3D = BRep_Tool::Curve(E, aCurveLocation, f, l);

  if (C3D.IsNull())
    // Edge 没有 3D 曲线：也没法投影
    return nullPCurve;

  // 3) 把 3D 曲线放到平面对应坐标系里
  aCurveLocation = aCurveLocation.Predivided(L);
  First          = f;
  Last           = l;

  // 4) 如有需要，对曲线做几何变换，同时修正参数（缩放变换会影响参数）
  if (!aCurveLocation.IsIdentity())
  {
    const gp_Trsf& aTrsf = aCurveLocation.Transformation();
    C3D                  = Handle(Geom_Curve)::DownCast(C3D->Transformed(aTrsf));
    f                    = C3D->TransformedParameter(f, aTrsf);
    l                    = C3D->TransformedParameter(l, aTrsf);
  }

  // 5) 把 3D 曲线（截取到 [f, l] 范围）投影到平面，得到“落在平面上的 3D 曲线”
  Handle(Geom_Curve) ProjOnPlane =
    GeomProjLib::ProjectOnPlane(new Geom_TrimmedCurve(C3D, f, l, Standard_True, Standard_False),
                                GP,
                                GP->Position().Direction(),
                                Standard_True);

  // 6) 把“平面上的 3D 曲线”转换成“平面参数空间里的 2D 曲线”
  Handle(GeomAdaptor_Surface) HS = new GeomAdaptor_Surface(GP);
  Handle(GeomAdaptor_Curve)   HC = new GeomAdaptor_Curve(ProjOnPlane);

  ProjLib_ProjectedCurve Proj(HS, HC);
  Handle(Geom2d_Curve)   pc = Geom2dAdaptor::MakeCurve(Proj);

  if (pc->DynamicType() == STANDARD_TYPE(Geom2d_TrimmedCurve))
  {
    // 新手理解：如果结果是“修剪曲线”，我们只取它的基曲线（去掉外层壳）
    Handle(Geom2d_TrimmedCurve) TC = Handle(Geom2d_TrimmedCurve)::DownCast(pc);
    pc                             = TC->BasisCurve();
  }

  return pc;
}

//=================================================================================================

void BRep_Tool::CurveOnSurface(const TopoDS_Edge&    E,
                               Handle(Geom2d_Curve)& C,
                               Handle(Geom_Surface)& S,
                               TopLoc_Location&      L,
                               Standard_Real&        First,
                               Standard_Real&        Last)
{
  // 这是“把找到的内容拆开输出”的版本：
  // - C：2D 曲线(PCurve)
  // - S：对应的 Surface
  // - L：最终 Location
  // - First/Last：参数范围
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface())
    {
      const BRep_GCurve* GC = static_cast<const BRep_GCurve*>(cr.get());
      C                     = GC->PCurve();
      S                     = GC->Surface();
      L                     = E.Location() * GC->Location();
      GC->Range(First, Last);
      return;
    }
    itcr.Next();
  }

  // 没找到就清空输出
  C.Nullify();
  S.Nullify();
  L.Identity();
  First = Last = 0.;
}

//=================================================================================================

void BRep_Tool::CurveOnSurface(const TopoDS_Edge&     E,
                               Handle(Geom2d_Curve)&  C,
                               Handle(Geom_Surface)&  S,
                               TopLoc_Location&       L,
                               Standard_Real&         First,
                               Standard_Real&         Last,
                               const Standard_Integer Index)
{
  if (Index < 1)
    return;

  Standard_Integer i = 0;
  // Index 版本：第 Index 个“曲面上的表示”
  // 新手注意一个坑：闭合曲面上的 closing edge 可能对应两条 PCurve（GC->PCurve 与 GC->PCurve2）
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());
  for (; itcr.More(); itcr.Next())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface())
    {
      const BRep_GCurve* GC = static_cast<const BRep_GCurve*>(cr.get());
      ++i;
      // Compare index taking into account the fact that for the curves on
      // closed surfaces there are two PCurves
      if (i == Index)
        C = GC->PCurve();
      else if (GC->IsCurveOnClosedSurface() && (++i == Index))
        C = GC->PCurve2();
      else
        continue;

      S = GC->Surface();
      L = E.Location() * GC->Location();
      GC->Range(First, Last);
      return;
    }
  }

  // 越界/没找到：清空输出
  C.Nullify();
  S.Nullify();
  L.Identity();
  First = Last = 0.;
}

//=======================================================================
// function : PolygonOnSurface
// purpose  : Returns the polygon associated to the  edge in  the
//           parametric  space of  the  face.  Returns   a NULL
//           handle  if this polygon  does not exist.
//=======================================================================

Handle(Poly_Polygon2D) BRep_Tool::PolygonOnSurface(const TopoDS_Edge& E, const TopoDS_Face& F)
{
  // 取 Face 的曲面 + Location，然后找 Edge 在这个曲面上的 2D 折线(离散)
  TopLoc_Location             l;
  const Handle(Geom_Surface)& S          = BRep_Tool::Surface(F, l);
  TopoDS_Edge                 aLocalEdge = E;
  if (F.Orientation() == TopAbs_REVERSED)
  {
    aLocalEdge.Reverse();
    //    return PolygonOnSurface(E,S,l);
  }
  //    return PolygonOnSurface(TopoDS::Edge(E.Reversed()),S,l);
  //  else
  //    return PolygonOnSurface(E,S,l);
  return PolygonOnSurface(aLocalEdge, S, l);
}

//=======================================================================
// function : PolygonOnSurface
// purpose  : Returns the polygon associated to the  edge in  the
//           parametric  space of  the surface. Returns   a NULL
//           handle  if this polygon  does not exist.
//=======================================================================

static const Handle(Poly_Polygon2D) nullPolygon2D;

Handle(Poly_Polygon2D) BRep_Tool::PolygonOnSurface(const TopoDS_Edge&          E,
                                                   const Handle(Geom_Surface)& S,
                                                   const TopLoc_Location&      L)
{
  // 计算“曲面坐标系”相对“Edge 坐标系”的变换（用于匹配存储在 Edge 里的表示）
  TopLoc_Location  l           = L.Predivided(E.Location());
  Standard_Boolean Eisreversed = (E.Orientation() == TopAbs_REVERSED);

  // 遍历 Edge 的所有表示，找到“在指定曲面 S + 相对变换 l 上”的 2D 折线
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygonOnSurface(S, l))
    {
      // 如果这是“闭合曲面上的缝合边”，并且 Edge 方向反了，就返回第二套折线
      if (cr->IsPolygonOnClosedSurface() && Eisreversed)
        return cr->Polygon2();
      else
        return cr->Polygon();
    }
    itcr.Next();
  }

  return nullPolygon2D;
}

//=================================================================================================

void BRep_Tool::PolygonOnSurface(const TopoDS_Edge&      E,
                                 Handle(Poly_Polygon2D)& P,
                                 Handle(Geom_Surface)&   S,
                                 TopLoc_Location&        L)
{
  // 输出拆分版本：把“折线/曲面/Location”分别输出
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygonOnSurface())
    {
      const BRep_PolygonOnSurface* PS = static_cast<const BRep_PolygonOnSurface*>(cr.get());
      P                               = PS->Polygon();
      S                               = PS->Surface();
      L                               = E.Location() * PS->Location();
      return;
    }
    itcr.Next();
  }

  // 没找到：清空输出
  L.Identity();
  P.Nullify();
  S.Nullify();
}

//=================================================================================================

void BRep_Tool::PolygonOnSurface(const TopoDS_Edge&      E,
                                 Handle(Poly_Polygon2D)& P,
                                 Handle(Geom_Surface)&   S,
                                 TopLoc_Location&        L,
                                 const Standard_Integer  Index)
{
  Standard_Integer i = 0;

  // Index 版本：取第 Index 条“PolygonOnSurface”表示
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygonOnSurface())
    {
      const BRep_PolygonOnSurface* PS = static_cast<const BRep_PolygonOnSurface*>(cr.get());
      i++;
      if (i > Index)
        break;
      if (i == Index)
      {
        P = PS->Polygon();
        S = PS->Surface();
        L = E.Location() * PS->Location();
        return;
      }
    }
    itcr.Next();
  }

  L.Identity();
  P.Nullify();
  S.Nullify();
}

//=======================================================================
// function : PolygonOnTriangulation
// purpose  : Returns the polygon associated to the  edge in  the
//           parametric  space of  the  face.  Returns   a NULL
//           handle  if this polygon  does not exist.
//=======================================================================

static const Handle(Poly_PolygonOnTriangulation) nullArray;

const Handle(Poly_PolygonOnTriangulation)& BRep_Tool::PolygonOnTriangulation(
  const TopoDS_Edge&                E,
  const Handle(Poly_Triangulation)& T,
  const TopLoc_Location&            L)
{
  // 计算“网格坐标系”相对“Edge 坐标系”的变换，用于匹配存储的 PolygonOnTriangulation 表示
  TopLoc_Location  l           = L.Predivided(E.Location());
  Standard_Boolean Eisreversed = (E.Orientation() == TopAbs_REVERSED);

  // 遍历 Edge 的表示，寻找“在指定网格 T + 相对变换 l 上”的边界折线索引
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygonOnTriangulation(T, l))
    {
      // 闭合网格上的 closing edge 与反向逻辑同理：可能有两套索引数组
      if (cr->IsPolygonOnClosedTriangulation() && Eisreversed)
        return cr->PolygonOnTriangulation2();
      else
        return cr->PolygonOnTriangulation();
    }
    itcr.Next();
  }

  return nullArray;
}

//=================================================================================================

void BRep_Tool::PolygonOnTriangulation(const TopoDS_Edge&                   E,
                                       Handle(Poly_PolygonOnTriangulation)& P,
                                       Handle(Poly_Triangulation)&          T,
                                       TopLoc_Location&                     L)
{
  // 输出拆分版本：把“边在网格上的折线索引/网格本体/Location”分别输出
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygonOnTriangulation())
    {
      const BRep_PolygonOnTriangulation* PT =
        static_cast<const BRep_PolygonOnTriangulation*>(cr.get());
      P = PT->PolygonOnTriangulation();
      T = PT->Triangulation();
      L = E.Location() * PT->Location();
      return;
    }
    itcr.Next();
  }

  // 没找到：清空输出
  L.Identity();
  P.Nullify();
  T.Nullify();
}

//=================================================================================================

void BRep_Tool::PolygonOnTriangulation(const TopoDS_Edge&                   E,
                                       Handle(Poly_PolygonOnTriangulation)& P,
                                       Handle(Poly_Triangulation)&          T,
                                       TopLoc_Location&                     L,
                                       const Standard_Integer               Index)
{
  Standard_Integer i = 0;

  // find the representation
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygonOnTriangulation())
    {
      const BRep_PolygonOnTriangulation* PT =
        static_cast<const BRep_PolygonOnTriangulation*>(cr.get());
      i++;
      if (i > Index)
        break;
      if (i == Index)
      {
        T = PT->Triangulation();
        P = PT->PolygonOnTriangulation();
        L = E.Location() * PT->Location();
        return;
      }
    }
    itcr.Next();
  }

  L.Identity();
  P.Nullify();
  T.Nullify();
}

//=======================================================================
// function : IsClosed
// purpose  : Returns  True  if  <E>  has  two  PCurves  in  the
//           parametric space of <F>. i.e.  <F>  is on a closed
//           surface and <E> is on the closing curve.
//=======================================================================

Standard_Boolean BRep_Tool::IsClosed(const TopoDS_Edge& E, const TopoDS_Face& F)
{
  // 优先用“曲面上的 PCurve”判断 closing edge
  TopLoc_Location             l;
  const Handle(Geom_Surface)& S = BRep_Tool::Surface(F, l);
  if (IsClosed(E, S, l))
    return Standard_True;

  // 如果曲面信息不足或没有 PCurve，则退化到“网格上的折线索引”判断
  const Handle(Poly_Triangulation)& T = BRep_Tool::Triangulation(F, l);
  return IsClosed(E, T, l);
}

//=======================================================================
// function : IsClosed
// purpose  : Returns  True  if  <E>  has  two  PCurves  in  the
//           parametric space  of <S>.  i.e.   <S>  is a closed
//           surface and <E> is on the closing curve.
//=======================================================================

Standard_Boolean BRep_Tool::IsClosed(const TopoDS_Edge&          E,
                                     const Handle(Geom_Surface)& S,
                                     const TopLoc_Location&      L)
{
  // modified by NIZNHY-PKV Fri Oct 17 12:16:58 2008f
  if (IsPlane(S))
  {
    // 平面不是“闭合曲面”，因此不可能出现“closing edge 需要两条 PCurve”的情况
    return Standard_False;
  }
  // modified by NIZNHY-PKV Fri Oct 17 12:16:54 2008t
  //
  TopLoc_Location l = L.Predivided(E.Location());

  // 找“在这个曲面 S 上，并且标记为 CurveOnClosedSurface”的表示
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface(S, l) && cr->IsCurveOnClosedSurface())
      return Standard_True;
    itcr.Next();
  }
  return Standard_False;
}

//=======================================================================
// function : IsClosed
// purpose  : Returns  True  if <E> has two arrays of indices in
//           the triangulation <T>.
//=======================================================================

Standard_Boolean BRep_Tool::IsClosed(const TopoDS_Edge&                E,
                                     const Handle(Poly_Triangulation)& T,
                                     const TopLoc_Location&            L)
{
  if (T.IsNull())
  {
    return Standard_False;
  }

  // 网格坐标系相对 Edge 坐标系的变换
  TopLoc_Location l = L.Predivided(E.Location());

  // 找“在这个网格 T 上，并且标记为 PolygonOnClosedTriangulation”的表示
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsPolygonOnTriangulation(T, l) && cr->IsPolygonOnClosedTriangulation())
      return Standard_True;
    itcr.Next();
  }
  return Standard_False;
}

//=======================================================================
// function : Tolerance
// purpose  : Returns the tolerance for <E>.
//=======================================================================

Standard_Real BRep_Tool::Tolerance(const TopoDS_Edge& E)
{
  const BRep_TEdge*       TE   = static_cast<const BRep_TEdge*>(E.TShape().get());
  Standard_Real           p    = TE->Tolerance();
  constexpr Standard_Real pMin = Precision::Confusion();
  if (p > pMin)
    return p;
  else
    return pMin;
}

//=======================================================================
// function : SameParameter
// purpose  : Returns the SameParameter flag for the edge.
//=======================================================================

Standard_Boolean BRep_Tool::SameParameter(const TopoDS_Edge& E)
{
  // SameParameter 标志存放在 BRep_TEdge 里，BRep_Tool 只是“读出来”
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  return TE->SameParameter();
}

//=======================================================================
// function : SameRange
// purpose  : Returns the SameRange flag for the edge.
//=======================================================================

Standard_Boolean BRep_Tool::SameRange(const TopoDS_Edge& E)
{
  // SameRange 标志同理
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  return TE->SameRange();
}

//=======================================================================
// function : Degenerated
// purpose  : Returns True  if the edge is degenerated.
//=======================================================================

Standard_Boolean BRep_Tool::Degenerated(const TopoDS_Edge& E)
{
  // Degenerated 标志同理（“退化边”的标记）
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  return TE->Degenerated();
}

//=================================================================================================

void BRep_Tool::Range(const TopoDS_Edge& E, Standard_Real& First, Standard_Real& Last)
{
  // 取 Edge 的参数范围：
  // - 如果有 3D 曲线，优先用 3D 曲线的范围
  // - 否则用曲面上的表示(PCurve)的范围
  // - 都没有就返回 0
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurve3D())
    {
      const BRep_Curve3D* CR = static_cast<const BRep_Curve3D*>(cr.get());
      if (!CR->Curve3D().IsNull())
      {
        First = CR->First();
        Last  = CR->Last();
        return;
      }
    }
    else if (cr->IsCurveOnSurface())
    {
      const BRep_GCurve* CR = static_cast<const BRep_GCurve*>(cr.get());
      First                 = CR->First();
      Last                  = CR->Last();
      return;
    }
    itcr.Next();
  }
  First = Last = 0.;
}

//=================================================================================================

void BRep_Tool::Range(const TopoDS_Edge&          E,
                      const Handle(Geom_Surface)& S,
                      const TopLoc_Location&      L,
                      Standard_Real&              First,
                      Standard_Real&              Last)
{
  // 在指定曲面 S 上查找 Edge 的 CurveOnSurface 表示，然后取它的范围
  TopLoc_Location l = L.Predivided(E.Location());

  // find the representation
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface(S, l))
    {
      const BRep_CurveOnSurface* CR = static_cast<const BRep_CurveOnSurface*>(cr.get());
      CR->Range(First, Last);
      break;
    }
    itcr.Next();
  }
  if (!itcr.More())
  {
    // 没找到这个曲面上的表示：退回到“通用 Range(E,...)”
    Range(E, First, Last);
  }

  // 这里把 Edge 的内部实现对象标记为“已修改”
  // 新手可以先忽略：它更多是 OCCT 内部的缓存/一致性管理需求
  E.TShape()->Modified(Standard_True);
}

//=================================================================================================

void BRep_Tool::Range(const TopoDS_Edge& E,
                      const TopoDS_Face& F,
                      Standard_Real&     First,
                      Standard_Real&     Last)
{
  TopLoc_Location             L;
  const Handle(Geom_Surface)& S = BRep_Tool::Surface(F, L);
  Range(E, S, L, First, Last);
}

//=================================================================================================

void BRep_Tool::UVPoints(const TopoDS_Edge&          E,
                         const Handle(Geom_Surface)& S,
                         const TopLoc_Location&      L,
                         gp_Pnt2d&                   PFirst,
                         gp_Pnt2d&                   PLast)
{
  // 目标：拿到 Edge 在这个曲面 S 的参数空间(UV)里，两端点的 UV 坐标
  // 优先路径：如果 Edge 存有 CurveOnSurface 表示，则里面直接缓存了 UV 端点
  TopLoc_Location  l           = L.Predivided(E.Location());
  Standard_Boolean Eisreversed = (E.Orientation() == TopAbs_REVERSED);

  // find the representation
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface(S, l))
    {
      if (cr->IsCurveOnClosedSurface() && Eisreversed)
      {
        // 闭合曲面上的 closing edge：如果方向反了，用 UVPoints2（第二套 UV 端点）
        const BRep_CurveOnClosedSurface* CR =
          static_cast<const BRep_CurveOnClosedSurface*>(cr.get());
        CR->UVPoints2(PFirst, PLast);
      }
      else
      {
        // 普通情况：CurveOnSurface 里直接有 UV 端点缓存
        const BRep_CurveOnSurface* CR = static_cast<const BRep_CurveOnSurface*>(cr.get());
        CR->UVPoints(PFirst, PLast);
      }
      return;
    }
    itcr.Next();
  }

  // 没找到已存的 CurveOnSurface：如果是平面，就用“把 3D 顶点投影到平面”来算 UV
  // modif 21-05-97 : for RectangularTrimmedSurface, project the vertices
  Handle(Geom_Plane)                     GP;
  Handle(Geom_RectangularTrimmedSurface) GRTS;
  GRTS = Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
  if (!GRTS.IsNull())
    GP = Handle(Geom_Plane)::DownCast(GRTS->BasisSurface());
  else
    GP = Handle(Geom_Plane)::DownCast(S);
  // fin modif du 21-05-97
  if (!GP.IsNull())
  {
    // 1) 先取 Edge 的两个端点（拓扑意义上的起点/终点）
    TopoDS_Vertex Vf, Vl;
    TopExp::Vertices(E, Vf, Vl);

    // 2) 把顶点先移到曲面所在坐标系（用 L 的逆把全局影响“消掉”）
    TopLoc_Location Linverted = L.Inverted();
    Vf.Move(Linverted, Standard_False);
    Vl.Move(Linverted, Standard_False);
    Standard_Real u, v;
    gp_Pln        pln = GP->Pln();

    // 3) 对每个端点求它在平面上的参数 (u,v)
    u = v = 0.;
    if (!Vf.IsNull())
    {
      gp_Pnt PF = BRep_Tool::Pnt(Vf);
      ElSLib::Parameters(pln, PF, u, v);
    }
    PFirst.SetCoord(u, v);

    u = v = 0.;
    if (!Vl.IsNull())
    {
      gp_Pnt PL = BRep_Tool::Pnt(Vl);
      ElSLib::Parameters(pln, PL, u, v);
    }
    PLast.SetCoord(u, v);
  }
  else
  {
    // 不是平面：这里没法猜 UV，返回 (0,0) 作为兜底
    PFirst.SetCoord(0., 0.);
    PLast.SetCoord(0., 0.);
  }
}

//=================================================================================================

void BRep_Tool::UVPoints(const TopoDS_Edge& E,
                         const TopoDS_Face& F,
                         gp_Pnt2d&          PFirst,
                         gp_Pnt2d&          PLast)
{
  // Face 版本：先取 Surface + Location，再处理 Face 的方向问题，最后调用底层版本
  TopLoc_Location             L;
  const Handle(Geom_Surface)& S          = BRep_Tool::Surface(F, L);
  TopoDS_Edge                 aLocalEdge = E;
  if (F.Orientation() == TopAbs_REVERSED)
  {
    aLocalEdge.Reverse();
    //    UVPoints(E,S,L,PFirst,PLast);
  }
  //    UVPoints(TopoDS::Edge(E.Reversed()),S,L,PFirst,PLast);
  //  else
  //    UVPoints(E,S,L,PFirst,PLast);
  UVPoints(aLocalEdge, S, L, PFirst, PLast);
}

//=================================================================================================

void BRep_Tool::SetUVPoints(const TopoDS_Edge&          E,
                            const Handle(Geom_Surface)& S,
                            const TopLoc_Location&      L,
                            const gp_Pnt2d&             PFirst,
                            const gp_Pnt2d&             PLast)
{
  // SetUVPoints 会修改 Edge 内部存储的 CurveOnSurface 表示（如果存在的话）
  TopLoc_Location  l           = L.Predivided(E.Location());
  Standard_Boolean Eisreversed = (E.Orientation() == TopAbs_REVERSED);

  // find the representation
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    Handle(BRep_CurveRepresentation) cr = itcr.Value();
    if (cr->IsCurveOnSurface(S, l))
    {
      if (cr->IsCurveOnClosedSurface() && Eisreversed)
      {
        // 闭合曲面 + 反向：写第二套 UV 端点
        BRep_CurveOnClosedSurface* CS = static_cast<BRep_CurveOnClosedSurface*>(cr.get());
        CS->SetUVPoints2(PFirst, PLast);
      }
      else
      {
        // 普通：写第一套 UV 端点
        BRep_CurveOnSurface* CS = static_cast<BRep_CurveOnSurface*>(cr.get());
        CS->SetUVPoints(PFirst, PLast);
      }
    }
    itcr.Next();
  }
}

//=================================================================================================

void BRep_Tool::SetUVPoints(const TopoDS_Edge& E,
                            const TopoDS_Face& F,
                            const gp_Pnt2d&    PFirst,
                            const gp_Pnt2d&    PLast)
{
  TopLoc_Location             L;
  const Handle(Geom_Surface)& S          = BRep_Tool::Surface(F, L);
  TopoDS_Edge                 aLocalEdge = E;
  if (F.Orientation() == TopAbs_REVERSED)
  {
    aLocalEdge.Reverse();
    //    SetUVPoints(TopoDS::Edge(E.Reversed()),S,L,PFirst,PLast);
  }
  //  else
  //    SetUVPoints(E,S,L,PFirst,PLast);
  SetUVPoints(aLocalEdge, S, L, PFirst, PLast);
}

//=======================================================================
// function : HasContinuity
// purpose  : Returns True if the edge is on the surfaces of the
//           two faces.
//=======================================================================

Standard_Boolean BRep_Tool::HasContinuity(const TopoDS_Edge& E,
                                          const TopoDS_Face& F1,
                                          const TopoDS_Face& F2)
{
  TopLoc_Location             l1, l2;
  const Handle(Geom_Surface)& S1 = BRep_Tool::Surface(F1, l1);
  const Handle(Geom_Surface)& S2 = BRep_Tool::Surface(F2, l2);
  return HasContinuity(E, S1, S2, l1, l2);
}

//=======================================================================
// function : Continuity
// purpose  : Returns the continuity.
//=======================================================================

GeomAbs_Shape BRep_Tool::Continuity(const TopoDS_Edge& E,
                                    const TopoDS_Face& F1,
                                    const TopoDS_Face& F2)
{
  TopLoc_Location             l1, l2;
  const Handle(Geom_Surface)& S1 = BRep_Tool::Surface(F1, l1);
  const Handle(Geom_Surface)& S2 = BRep_Tool::Surface(F2, l2);
  return Continuity(E, S1, S2, l1, l2);
}

//=======================================================================
// function : HasContinuity
// purpose  : Returns True if the edge is on the surfaces.
//=======================================================================

Standard_Boolean BRep_Tool::HasContinuity(const TopoDS_Edge&          E,
                                          const Handle(Geom_Surface)& S1,
                                          const Handle(Geom_Surface)& S2,
                                          const TopLoc_Location&      L1,
                                          const TopLoc_Location&      L2)
{
  // 连续性/正则性(regularity)记录在 Edge 的某些表示里：
  // 给定两侧曲面 S1/S2（以及它们相对 Edge 的变换），判断是否存在对应的 regularity 记录。
  const TopLoc_Location& Eloc = E.Location();
  TopLoc_Location        l1   = L1.Predivided(Eloc);
  TopLoc_Location        l2   = L2.Predivided(Eloc);

  // find the representation
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsRegularity(S1, S2, l1, l2))
      return Standard_True;
    itcr.Next();
  }
  return Standard_False;
}

//=======================================================================
// function : Continuity
// purpose  : Returns the continuity.
//=======================================================================

GeomAbs_Shape BRep_Tool::Continuity(const TopoDS_Edge&          E,
                                    const Handle(Geom_Surface)& S1,
                                    const Handle(Geom_Surface)& S2,
                                    const TopLoc_Location&      L1,
                                    const TopLoc_Location&      L2)
{
  // Continuity：返回具体的连续性等级（GeomAbs_C0 / GeomAbs_C1 / ...）
  TopLoc_Location l1 = L1.Predivided(E.Location());
  TopLoc_Location l2 = L2.Predivided(E.Location());

  // find the representation
  BRep_ListIteratorOfListOfCurveRepresentation itcr(
    (*((Handle(BRep_TEdge)*)&E.TShape()))->ChangeCurves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsRegularity(S1, S2, l1, l2))
      return cr->Continuity();
    itcr.Next();
  }
  return GeomAbs_C0;
}

//=======================================================================
// function : HasContinuity
// purpose  : Returns True if the edge is on some two surfaces.
//=======================================================================

Standard_Boolean BRep_Tool::HasContinuity(const TopoDS_Edge& E)
{
  const BRep_TEdge* TE = static_cast<const BRep_TEdge*>(E.TShape().get());
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());

  for (; itcr.More(); itcr.Next())
  {
    const Handle(BRep_CurveRepresentation)& CR = itcr.Value();
    if (CR->IsRegularity())
      return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

GeomAbs_Shape BRep_Tool::MaxContinuity(const TopoDS_Edge& theEdge)
{
  // MaxContinuity：在 Edge 的所有 regularity 记录里，取“连续性等级最大的那个”
  // （把 C0/C1/C2... 当成从小到大的枚举值比较即可）
  GeomAbs_Shape aMaxCont = GeomAbs_C0;
  for (BRep_ListIteratorOfListOfCurveRepresentation aReprIter(
         (*((Handle(BRep_TEdge)*)&theEdge.TShape()))->ChangeCurves());
       aReprIter.More();
       aReprIter.Next())
  {
    const Handle(BRep_CurveRepresentation)& aRepr = aReprIter.Value();
    if (aRepr->IsRegularity())
    {
      const GeomAbs_Shape aCont = aRepr->Continuity();
      if ((Standard_Integer)aCont > (Standard_Integer)aMaxCont)
      {
        aMaxCont = aCont;
      }
    }
  }
  return aMaxCont;
}

//=================================================================================================

gp_Pnt BRep_Tool::Pnt(const TopoDS_Vertex& V)
{
  // Vertex 的几何点坐标存放在 BRep_TVertex 里；如果 Vertex 自己有 Location，需要再做变换
  const BRep_TVertex* TV = static_cast<const BRep_TVertex*>(V.TShape().get());

  if (TV == 0)
  {
    throw Standard_NullObject("BRep_Tool:: TopoDS_Vertex hasn't gp_Pnt");
  }

  // TV->Pnt() 是“局部坐标系”的点
  const gp_Pnt& P = TV->Pnt();
  if (V.Location().IsIdentity())
  {
    return P;
  }

  // Vertex 的 Location 非单位：返回“应用了变换后的世界坐标点”
  return P.Transformed(V.Location().Transformation());
}

//=======================================================================
// function : Tolerance
// purpose  : Returns the tolerance.
//=======================================================================

Standard_Real BRep_Tool::Tolerance(const TopoDS_Vertex& V)
{
  const BRep_TVertex* aTVert = static_cast<const BRep_TVertex*>(V.TShape().get());

  if (aTVert == 0)
  {
    throw Standard_NullObject("BRep_Tool:: TopoDS_Vertex hasn't gp_Pnt");
  }

  Standard_Real           p    = aTVert->Tolerance();
  constexpr Standard_Real pMin = Precision::Confusion();
  if (p > pMin)
    return p;
  else
    return pMin;
}

//=======================================================================
// function : Parameter
// purpose  : Returns the parameter of <V> on <E>.
//=======================================================================

Standard_Boolean BRep_Tool::Parameter(const TopoDS_Vertex& theV,
                                      const TopoDS_Edge&   theE,
                                      Standard_Real&       theParam)
{
  // 目标：求“顶点 theV 在边 theE 上的参数值”
  //
  // 新手先把它当成一句话：
  // - 如果 theV 是边的起点/终点：参数就是边范围的 First/Last（要考虑方向）
  // - 否则：去顶点的 PointRepresentation 列表里找“它在这条曲线上的参数缓存”

  Standard_Boolean   rev = Standard_False;
  TopoDS_Shape       VF;
  TopAbs_Orientation orient = TopAbs_INTERNAL;

  // 用 FORWARD 方向遍历边的子形状（通常是两个顶点：起点/终点）
  TopoDS_Iterator itv(theE.Oriented(TopAbs_FORWARD));

  // if the edge has no vertices
  // and is degenerated use the vertex orientation
  // RLE, june 94

  if (!itv.More() && BRep_Tool::Degenerated(theE))
  {
    // 退化边可能没有显式端点：用输入顶点自身的方向作为兜底
    orient = theV.Orientation();
  }

  while (itv.More())
  {
    const TopoDS_Shape& Vcur = itv.Value();
    if (theV.IsSame(Vcur))
    {
      if (VF.IsNull())
      {
        // 第一次匹配到这个顶点：先记下来
        VF = Vcur;
      }
      else
      {
        // 同一个 Edge 上可能出现两次“同一个顶点”（例如闭合边/退化情况）
        // 这里通过方向与 Edge 方向(rev)来选择更匹配的那一个
        rev = theE.Orientation() == TopAbs_REVERSED;
        if (Vcur.Orientation() == theV.Orientation())
        {
          VF = Vcur;
        }
      }
    }
    itv.Next();
  }

  if (!VF.IsNull())
    orient = VF.Orientation();

  Standard_Real f, l;

  if (orient == TopAbs_FORWARD)
  {
    // 顶点是“边的起点”：参数取 First（如果 Edge 本身是 REVERSED，则起点/终点对调）
    BRep_Tool::Range(theE, f, l);
    theParam = (rev) ? l : f;
    return Standard_True;
  }

  else if (orient == TopAbs_REVERSED)
  {
    // 顶点是“边的终点”：参数取 Last（同样要考虑 Edge 是否 REVERSED）
    BRep_Tool::Range(theE, f, l);
    theParam = (rev) ? f : l;
    return Standard_True;
  }

  else
  {
    // 顶点不是边的显式端点：尝试在“顶点的点表示列表”里找到它在这条曲线上的参数
    TopLoc_Location           L;
    const Handle(Geom_Curve)& C = BRep_Tool::Curve(theE, L, f, l);
    L                           = L.Predivided(theV.Location());
    if (!C.IsNull() || BRep_Tool::Degenerated(theE))
    {
      const BRep_TVertex* TV = static_cast<const BRep_TVertex*>(theV.TShape().get());
      BRep_ListIteratorOfListOfPointRepresentation itpr(TV->Points());

      while (itpr.More())
      {
        const Handle(BRep_PointRepresentation)& pr = itpr.Value();
        if (pr->IsPointOnCurve(C, L))
        {
          // pr->Parameter() 就是缓存的“顶点在这条曲线上的参数”
          Standard_Real p   = pr->Parameter();
          Standard_Real res = p; // SVV 4 nov 99 - to avoid warnings on Linux
          if (!C.IsNull())
          {
            // Closed curves RLE 16 june 94
            if (Precision::IsNegativeInfinite(f))
            {
              theParam = pr->Parameter(); // p;
              return Standard_True;
            };
            if (Precision::IsPositiveInfinite(l))
            {
              theParam = pr->Parameter(); // p;
              return Standard_True;
            }
            gp_Pnt        Pf  = C->Value(f).Transformed(L.Transformation());
            gp_Pnt        Pl  = C->Value(l).Transformed(L.Transformation());
            Standard_Real tol = BRep_Tool::Tolerance(theV);
            if (Pf.Distance(Pl) < tol)
            {
              // 如果曲线两端点在公差内几乎重合（闭合曲线的典型情况），
              // 并且这个顶点也落在重合点附近，则把参数“钉死”到 f 或 l，避免歧义
              if (Pf.Distance(BRep_Tool::Pnt(theV)) < tol)
              {
                if (theV.Orientation() == TopAbs_FORWARD)
                  res = f; // p = f;
                else
                  res = l; // p = l;
              }
            }
          }
          theParam = res; // p;
          return Standard_True;
        }
        itpr.Next();
      }
    }
    else
    {
      // 没有 3D 曲线：退一步，尝试用“曲面上的 2D 曲线(PCurve)”来找参数
      Handle(Geom2d_Curve) PC;
      Handle(Geom_Surface) S;
      BRep_Tool::CurveOnSurface(theE, PC, S, L, f, l);
      L                      = L.Predivided(theV.Location());
      const BRep_TVertex* TV = static_cast<const BRep_TVertex*>(theV.TShape().get());
      BRep_ListIteratorOfListOfPointRepresentation itpr(TV->Points());

      while (itpr.More())
      {
        const Handle(BRep_PointRepresentation)& pr = itpr.Value();
        if (pr->IsPointOnCurveOnSurface(PC, S, L))
        {
          Standard_Real p = pr->Parameter();
          // Closed curves RLE 16 june 94
          if (PC->IsClosed())
          {
            if ((p == PC->FirstParameter()) || (p == PC->LastParameter()))
            {
              if (theV.Orientation() == TopAbs_FORWARD)
                p = PC->FirstParameter();
              else
                p = PC->LastParameter();
            }
          }
          theParam = p;
          return Standard_True;
        }
        itpr.Next();
      }
    }
  }

  return Standard_False;
}

//=======================================================================
// function : Parameter
// purpose  : Returns the parameter of <V> on <E>.
//=======================================================================

Standard_Real BRep_Tool::Parameter(const TopoDS_Vertex& V, const TopoDS_Edge& E)
{
  Standard_Real p;
  if (Parameter(V, E, p))
    return p;
  throw Standard_NoSuchObject("BRep_Tool:: no parameter on edge");
}

//=======================================================================
// function : Parameter
// purpose  : Returns the  parameters  of   the  vertex   on the
//           pcurve of the edge on the face.
//=======================================================================

Standard_Real BRep_Tool::Parameter(const TopoDS_Vertex& V,
                                   const TopoDS_Edge&   E,
                                   const TopoDS_Face&   F)
{
  TopLoc_Location             L;
  const Handle(Geom_Surface)& S = BRep_Tool::Surface(F, L);
  return BRep_Tool::Parameter(V, E, S, L);
}

//=======================================================================
// function : Parameter
// purpose  : Returns the  parameters  of   the  vertex   on the
//           pcurve of the edge on the surface.
//=======================================================================

Standard_Real BRep_Tool::Parameter(const TopoDS_Vertex&        V,
                                   const TopoDS_Edge&          E,
                                   const Handle(Geom_Surface)& S,
                                   const TopLoc_Location&      L)
{
  // 目标：求“顶点 V 在边 E 的 PCurve（位于曲面 S 上）里的参数值”
  //
  // 新手先记一句话：如果 V 是边端点 -> 返回边在该曲面上的 Range 端点；
  // 否则 -> 去顶点的 PointRepresentation 里找“PointOnCurveOnSurface”缓存；
  // 再不行 -> 用 3D 曲线的 PointOnCurve 缓存兜底。

  Standard_Boolean rev = Standard_False;
  TopoDS_Shape     VF;
  TopoDS_Iterator  itv(E.Oriented(TopAbs_FORWARD));

  while (itv.More())
  {
    if (V.IsSame(itv.Value()))
    {
      if (VF.IsNull())
        VF = itv.Value();
      else
      {
        rev = E.Orientation() == TopAbs_REVERSED;
        if (itv.Value().Orientation() == V.Orientation())
          VF = itv.Value();
      }
    }
    itv.Next();
  }

  TopAbs_Orientation orient = TopAbs_INTERNAL;
  if (!VF.IsNull())
    orient = VF.Orientation();

  Standard_Real f, l;

  if (orient == TopAbs_FORWARD)
  {
    // 起点：取该曲面上的 Range.First（考虑 Edge 是否 REVERSED）
    BRep_Tool::Range(E, S, L, f, l);
    return (rev) ? l : f;
  }

  else if (orient == TopAbs_REVERSED)
  {
    // 终点：取该曲面上的 Range.Last（考虑 Edge 是否 REVERSED）
    BRep_Tool::Range(E, S, L, f, l);
    return (rev) ? f : l;
  }

  else
  {
    // 非端点：尝试用 PCurve + PointOnCurveOnSurface 的缓存来取参数
    Handle(Geom2d_Curve) PC = BRep_Tool::CurveOnSurface(E, S, L, f, l);
    const BRep_TVertex*  TV = static_cast<const BRep_TVertex*>(V.TShape().get());
    BRep_ListIteratorOfListOfPointRepresentation itpr(TV->Points());

    while (itpr.More())
    {
      if (itpr.Value()->IsPointOnCurveOnSurface(PC, S, L))
        return itpr.Value()->Parameter();
      itpr.Next();
    }
  }

  //----------------------------------------------------------

  TopLoc_Location           L1;
  const Handle(Geom_Curve)& C = BRep_Tool::Curve(E, L1, f, l);
  L1                          = L1.Predivided(V.Location());
  if (!C.IsNull() || Degenerated(E))
  {
    // 如果存在 3D 曲线：再用 PointOnCurve 的缓存兜底（尤其对非流形/特殊情况更稳）
    const BRep_TVertex* TV = static_cast<const BRep_TVertex*>(V.TShape().get());
    BRep_ListIteratorOfListOfPointRepresentation itpr(TV->Points());

    while (itpr.More())
    {
      const Handle(BRep_PointRepresentation)& pr = itpr.Value();
      if (pr->IsPointOnCurve(C, L1))
      {
        Standard_Real p   = pr->Parameter();
        Standard_Real res = p;
        if (!C.IsNull())
        {
          // Closed curves RLE 16 june 94
          if (Precision::IsNegativeInfinite(f))
            return res;
          if (Precision::IsPositiveInfinite(l))
            return res;
          gp_Pnt        Pf  = C->Value(f).Transformed(L1.Transformation());
          gp_Pnt        Pl  = C->Value(l).Transformed(L1.Transformation());
          Standard_Real tol = BRep_Tool::Tolerance(V);
          if (Pf.Distance(Pl) < tol)
          {
            if (Pf.Distance(BRep_Tool::Pnt(V)) < tol)
            {
              if (V.Orientation() == TopAbs_FORWARD)
                res = f;
              else
                res = l;
            }
          }
        }
        return res;
      }
      itpr.Next();
    }
  }

  //----------------------------------------------------------

  throw Standard_NoSuchObject("BRep_Tool:: no parameter on edge");
}

//=======================================================================
// function : Parameters
// purpose  : Returns the parameters of the vertex on the face.
//=======================================================================

gp_Pnt2d BRep_Tool::Parameters(const TopoDS_Vertex& V, const TopoDS_Face& F)
{
  // 目标：求“顶点 V 在面 F 的参数空间(U,V)里的坐标”
  // 优先：直接在顶点的 PointRepresentation 列表里找“PointOnSurface”记录
  TopLoc_Location             L;
  const Handle(Geom_Surface)& S = BRep_Tool::Surface(F, L);
  L                             = L.Predivided(V.Location());
  const BRep_TVertex* TV        = static_cast<const BRep_TVertex*>(V.TShape().get());
  BRep_ListIteratorOfListOfPointRepresentation itpr(TV->Points());

  // It is checked if there is PointRepresentation (case non Manifold)
  while (itpr.More())
  {
    if (itpr.Value()->IsPointOnSurface(S, L))
    {
      return gp_Pnt2d(itpr.Value()->Parameter(), itpr.Value()->Parameter2());
    }
    itpr.Next();
  }

  // 兜底：如果顶点没有直接的“在面上的参数点”记录，就去遍历面的边，
  // 找到连接到该顶点的那条边，然后用 UVPoints(E, F, ...) 来拿端点 UV。
  TopoDS_Vertex Vf, Vl;
  TopoDS_Edge   E;
  // Otherwise the edges are searched (PMN 4/06/97) It is not possible to succeed 999/1000!
  // even if often there is a way to make more economically than above...
  TopExp_Explorer exp;
  for (exp.Init(F, TopAbs_EDGE); exp.More(); exp.Next())
  {
    E = TopoDS::Edge(exp.Current());
    TopExp::Vertices(E, Vf, Vl);
    if ((V.IsSame(Vf)) || (V.IsSame(Vl)))
    {
      gp_Pnt2d Pf, Pl;
      UVPoints(E, F, Pf, Pl);
      if (V.IsSame(Vf))
        return Pf;
      else
        return Pl; // Ambiguity (natural) for degenerated edges.
    }
  }
  throw Standard_NoSuchObject("BRep_Tool:: no parameters on surface");
}

//=================================================================================================

Standard_Boolean BRep_Tool::IsClosed(const TopoDS_Shape& theShape)
{
  if (theShape.ShapeType() == TopAbs_SHELL)
  {
    // Shell 闭合判断：统计所有“边界边”出现次数
    // - 内部共享边会出现两次（被两个面共享）
    // - 裸露边界边只出现一次（只被一个面用到）
    // 下面这个 Add/Remove 的技巧等价于“对出现次数做奇偶计数”
    NCollection_Map<TopoDS_Shape, TopTools_ShapeMapHasher> aMap(101, new NCollection_IncAllocator);
    TopExp_Explorer  exp(theShape.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
    Standard_Boolean hasBound = Standard_False;
    for (; exp.More(); exp.Next())
    {
      const TopoDS_Edge& E = TopoDS::Edge(exp.Current());
      if (BRep_Tool::Degenerated(E) || E.Orientation() == TopAbs_INTERNAL
          || E.Orientation() == TopAbs_EXTERNAL)
        continue;
      hasBound = Standard_True;
      if (!aMap.Add(E))
        aMap.Remove(E);
    }
    return hasBound && aMap.IsEmpty();
  }
  else if (theShape.ShapeType() == TopAbs_WIRE)
  {
    // Wire 闭合判断：统计所有“端点顶点”出现次数
    // - 内部顶点会出现两次（两条边连接）
    // - 自由端点只出现一次
    NCollection_Map<TopoDS_Shape, TopTools_ShapeMapHasher> aMap(101, new NCollection_IncAllocator);
    TopExp_Explorer  exp(theShape.Oriented(TopAbs_FORWARD), TopAbs_VERTEX);
    Standard_Boolean hasBound = Standard_False;
    for (; exp.More(); exp.Next())
    {
      const TopoDS_Shape& V = exp.Current();
      if (V.Orientation() == TopAbs_INTERNAL || V.Orientation() == TopAbs_EXTERNAL)
        continue;
      hasBound = Standard_True;
      if (!aMap.Add(V))
        aMap.Remove(V);
    }
    return hasBound && aMap.IsEmpty();
  }
  else if (theShape.ShapeType() == TopAbs_EDGE)
  {
    // Edge 闭合判断：起点与终点是否是同一个顶点
    TopoDS_Vertex aVFirst, aVLast;
    TopExp::Vertices(TopoDS::Edge(theShape), aVFirst, aVLast);
    return !aVFirst.IsNull() && aVFirst.IsSame(aVLast);
  }
  return theShape.Closed();
}

// modified by NIZNHY-PKV Fri Oct 17 14:09:58 2008 f
//=================================================================================================

Standard_Boolean IsPlane(const Handle(Geom_Surface)& aS)
{
  // 新手理解：判断“是不是平面”要考虑几种常见包装层：
  // - RectangularTrimmedSurface：矩形修剪面（内部有一个 BasisSurface）
  // - OffsetSurface：偏置曲面（内部也有一个 BasisSurface）
  // 所以这里会把这些壳子剥掉后再尝试 DownCast 到 Geom_Plane。
  Standard_Boolean                       bRet;
  Handle(Geom_Plane)                     aGP;
  Handle(Geom_RectangularTrimmedSurface) aGRTS;
  Handle(Geom_OffsetSurface)             aGOFS;
  //
  aGRTS = Handle(Geom_RectangularTrimmedSurface)::DownCast(aS);
  aGOFS = Handle(Geom_OffsetSurface)::DownCast(aS);
  //
  if (!aGOFS.IsNull())
  {
    aGP = Handle(Geom_Plane)::DownCast(aGOFS->BasisSurface());
  }
  else if (!aGRTS.IsNull())
  {
    aGP = Handle(Geom_Plane)::DownCast(aGRTS->BasisSurface());
  }
  else
  {
    aGP = Handle(Geom_Plane)::DownCast(aS);
  }
  //
  bRet = !aGP.IsNull();
  //
  return bRet;
}

//=================================================================================================

Standard_Real BRep_Tool::MaxTolerance(const TopoDS_Shape&    theShape,
                                      const TopAbs_ShapeEnum theSubShape)
{
  // MaxTolerance：在 shape 的所有子形状里取“最大公差”
  // 注意：只支持 Face / Edge / Vertex 三类子形状。
  Standard_Real aTol = 0.0;

  // Explorer Shape-Subshape.
  TopExp_Explorer anExpSS(theShape, theSubShape);
  if (theSubShape == TopAbs_FACE)
  {
    for (; anExpSS.More(); anExpSS.Next())
    {
      const TopoDS_Shape& aCurrentSubShape = anExpSS.Current();
      aTol                                 = Max(aTol, Tolerance(TopoDS::Face(aCurrentSubShape)));
    }
  }
  else if (theSubShape == TopAbs_EDGE)
  {
    for (; anExpSS.More(); anExpSS.Next())
    {
      const TopoDS_Shape& aCurrentSubShape = anExpSS.Current();
      aTol                                 = Max(aTol, Tolerance(TopoDS::Edge(aCurrentSubShape)));
    }
  }
  else if (theSubShape == TopAbs_VERTEX)
  {
    for (; anExpSS.More(); anExpSS.Next())
    {
      const TopoDS_Shape& aCurrentSubShape = anExpSS.Current();
      aTol                                 = Max(aTol, Tolerance(TopoDS::Vertex(aCurrentSubShape)));
    }
  }

  return aTol;
}
