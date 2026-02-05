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

#ifndef _BRep_Tool_HeaderFile
#define _BRep_Tool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <GeomAbs_Shape.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Pnt.hxx>
#include <Poly_ListOfTriangulation.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <TopAbs_ShapeEnum.hxx>

class TopoDS_Shape;
class TopoDS_Face;
class TopLoc_Location;
class TopoDS_Edge;
class TopoDS_Vertex;

//! ======================== 新手速记（先当成一句话）========================
//! BRep_Tool = “从拓扑形状(TopoDS_*)里，把几何(Geom_*) / 网格(Poly_*) / 公差拿出来的工具箱”。
//!
//! 1) TopoDS_* 是“拓扑壳子”
//!    - TopoDS_Face / TopoDS_Edge / TopoDS_Vertex 是你在建模/布尔/倒角时操作的“拓扑对象”
//!    - 它们负责：连接关系（谁连着谁、方向、子形状），但通常不直接存几 
//  何数据
//!
//! 2) 真正的数据在 TShape 里（BRep_TFace / BRep_TEdge / BRep_TVertex）
//!    - TopoDS_* 内部有一个指向“真正实现对象”的指针：TShape()
//!    - 在 BRep 模型里，这个实现对象是 BRep_TFace / BRep_TEdge / BRep_TVertex
//!    - 所以你经常会看到：static_cast<const BRep_TFace*>(F.TShape().get()) 这种写法
//!      你可以先把它当成一句话：“把 Face 的内部数据对象取出来”
//!
//! 3) Location(TopLoc_Location) = “放置变换”
//!    - 形状可以带一个位置变换：平移/旋转/缩放（以及它们的组合）
//!    - BRep_Tool 很多接口会同时返回：几何对象 + 该几何对象对应的 Location
//!    - 新手先记住：几何本体(Geom_*)通常是在“局部坐标系”，Location 负责把它放到“全局坐标系”
//!
//! 4) Tolerance(公差) = “允许的误差半径”
//!    - 例如两个点距离 < 公差，就可能被认为“等价/可合并”
//!    - Face/Edge/Vertex 都有公差，算法会用它决定判断的松紧
//!
//! 5) PCurve(2D 曲线) = “边在面参数空间(UV)里的影子”
//!    - Edge 在 3D 空间里有一条 Geom_Curve（可选）
//!    - Edge 在某个 Face 的参数空间(U,V)里还可能有一条 Geom2d_Curve（这就是 PCurve）
//!    - 做很多曲面相关算法（修剪、求交、缝合）经常更依赖 PCurve
//! =======================================================================

//! Provides class methods  to  access to the geometry
//! of BRep shapes.
//! [新手理解] 你可以把 BRep_Tool 当作“读数据的工具类”：
//! - 给我一个 Face/Edge/Vertex，我帮你把它背后的 Surface/Curve/Point/UV/网格/公差等取出来
//! - 很多接口分两种：一种返回 (几何 + Location)，另一种直接返回“应用了 Location 的几何副本”
//!   一般更推荐用“带 Location 输出参数”的版本：更快、少拷贝
class BRep_Tool
{
public:
  DEFINE_STANDARD_ALLOC

  //! If S is Shell, returns True if it has no free boundaries (edges).
  //! If S is Wire, returns True if it has no free ends (vertices).
  //! (Internal and External sub-shepes are ignored in these checks)
  //! If S is Edge, returns True if its vertices are the same.
  //! For other shape types returns S.Closed().
  //! [新手理解] 判断“闭合/封闭”的通用接口：
  //! - Shell：看有没有“裸露边界边”（外轮廓没封住）
  //! - Wire：看有没有“裸露端点”（线没连成环）
  //! - Edge：看起点终点是否同一个顶点（退化成环）
  //! - 其他：退回到 TopoDS_Shape::Closed() 标志
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoDS_Shape& S);

  //! Returns the geometric surface of the face. Returns
  //! in <L> the location for the surface.
  //! @param F [in] 输入的面 (Input Face)
  //! @param L [out] 输出的位置变换 (Output Location)
  //! @return 几何曲面句柄 (Handle to Geom_Surface)
  //! @note 这是获取面几何信息的最常用方法。它返回底层的几何曲面以及
  //!       该曲面相对于面的位置变换。
  Standard_EXPORT static const Handle(Geom_Surface)& Surface(const TopoDS_Face& F,
                                                             TopLoc_Location&   L);

  //! Returns the geometric  surface of the face. It can
  //! be a copy if there is a Location.
  //! @note 这是一个便捷方法，如果面有位置变换，它可能会返回曲面的副本（应用了变换）。
  //!       为了性能，通常推荐使用带 Location 参数的版本。
  Standard_EXPORT static Handle(Geom_Surface) Surface(const TopoDS_Face& F);

  //! Returns the triangulation of the face according to the mesh purpose.
  //! @param[in] theFace  the input face to find triangulation.
  //! @param[out] theLocation  the face location.
  //! @param[in] theMeshPurpose  a mesh purpose to find appropriate triangulation (NONE by default).
  //! @return an active triangulation in case of NONE purpose,
  //!         the first triangulation appropriate for the input purpose,
  //!         just the first triangulation if none matching other criteria and input purpose is
  //!         AnyFallback or null handle if there is no any suitable triangulation.
  //! @note 获取面的三角网格表示（用于显示或网格算法）。
  //! [新手理解] Face 除了“几何曲面(Geom_Surface)”，还可能有“离散网格(Poly_Triangulation)”：
  //! - 网格常用于：显示、快速拾取、某些网格算法
  //! - 一个 Face 可能同时存多份网格（不同用途/不同精度），用 theMeshPurpose 选择
  Standard_EXPORT static const Handle(Poly_Triangulation)& Triangulation(
    const TopoDS_Face&     theFace,
    TopLoc_Location&       theLocation,
    const Poly_MeshPurpose theMeshPurpose = Poly_MeshPurpose_NONE);

  //! Returns all triangulations of the face.
  //! @param[in] theFace  the input face.
  //! @param[out] theLocation  the face location.
  //! @return list of all available face triangulations.
  Standard_EXPORT static const Poly_ListOfTriangulation& Triangulations(
    const TopoDS_Face& theFace,
    TopLoc_Location&   theLocation);

  //! Returns the tolerance of the face.
  //! [新手理解] 面的公差：算法用它来判断“点/边是否贴合到这个面上”以及各种几何判断的容忍度。
  Standard_EXPORT static Standard_Real Tolerance(const TopoDS_Face& F);

  //! Returns the  NaturalRestriction  flag of the  face.
  //! [新手理解] NaturalRestriction 可以先当成一句话：
  //! “这个面是否使用几何曲面自身的天然边界作为限制边界”
  //! 例如：平面/圆柱面等几何曲面本身可能是无限的，Face 才是真正的“裁剪后区域”。
  Standard_EXPORT static Standard_Boolean NaturalRestriction(const TopoDS_Face& F);

  //! Returns True if <F> has a surface, false otherwise.
  //! [新手理解] 这个 Face 是否真的绑定了一个几何曲面（有些拓扑可能是空壳或特殊构造）。
  Standard_EXPORT static Standard_Boolean IsGeometric(const TopoDS_Face& F);

  //! Returns True if <E> is a 3d curve or a curve on
  //! surface.
  //! [新手理解] 这个 Edge 是否有几何信息：
  //! - 有 3D 曲线（Geom_Curve），或者
  //! - 有曲面上的 2D 曲线（PCurve / CurveOnSurface）
  Standard_EXPORT static Standard_Boolean IsGeometric(const TopoDS_Edge& E);

  //! Returns the 3D curve  of the edge.  May be  a Null
  //! handle. Returns in <L> the location for the curve.
  //! In <First> and <Last> the parameter range.
  //! [新手理解] 取 Edge 的“空间曲线”（如果有）：
  //! - 返回的是曲线对象句柄（可能为空）
  //! - First/Last 是曲线参数范围（可以理解为“从哪到哪这一段曲线属于这条边”）
  //! - L 是把曲线放到世界坐标的变换
  Standard_EXPORT static const Handle(Geom_Curve)& Curve(const TopoDS_Edge& E,
                                                         TopLoc_Location&   L,
                                                         Standard_Real&     First,
                                                         Standard_Real&     Last);

  //! Returns the 3D curve  of the edge. May be a Null handle.
  //! In <First> and <Last> the parameter range.
  //! It can be a copy if there is a Location.
  Standard_EXPORT static Handle(Geom_Curve) Curve(const TopoDS_Edge& E,
                                                  Standard_Real&     First,
                                                  Standard_Real&     Last);

  //! Returns the 3D polygon of the edge. May be   a Null
  //! handle. Returns in <L> the location for the polygon.
  //! [新手理解] 取 Edge 的“离散折线(Poly_Polygon3D)”：
  //! - 这是离散表示，不是精确几何
  //! - 常用于显示或快速近似计算
  Standard_EXPORT static const Handle(Poly_Polygon3D)& Polygon3D(const TopoDS_Edge& E,
                                                                 TopLoc_Location&   L);

  //! Returns the curve  associated to the  edge in  the
  //! parametric  space of  the  face.  Returns   a NULL
  //! handle  if this curve  does not exist.  Returns in
  //! <First> and <Last> the parameter range.
  //! If the surface is a plane the curve can be not stored but created a new
  //! each time. The flag pointed by <theIsStored> serves to indicate storage status.
  //! It is valued if the pointer is non-null.
  //! [新手理解] 取 Edge 在 Face 的 UV 平面上的那条 2D 曲线（PCurve）：
  //! - 这条曲线描述“边在这个面的参数空间里怎么走”
  //! - 有些情况下（尤其是平面），PCurve 可能不存储，而是每次临时算出来
  Standard_EXPORT static Handle(Geom2d_Curve) CurveOnSurface(const TopoDS_Edge& E,
                                                             const TopoDS_Face& F,
                                                             Standard_Real&     First,
                                                             Standard_Real&     Last,
                                                             Standard_Boolean*  theIsStored = NULL);

  //! Returns the  curve associated to   the edge in the
  //! parametric  space of the   surface. Returns a NULL
  //! handle  if this curve does  not exist.  Returns in
  //! <First> and <Last> the parameter range.
  //! If the surface is a plane the curve can be not stored but created a new
  //! each time. The flag pointed by <theIsStored> serves to indicate storage status.
  //! It is valued if the pointer is non-null.
  //! [新手理解] 这是更底层的重载：你自己提供 Surface + Location，用它去找 PCurve。
  Standard_EXPORT static Handle(Geom2d_Curve) CurveOnSurface(const TopoDS_Edge&          E,
                                                             const Handle(Geom_Surface)& S,
                                                             const TopLoc_Location&      L,
                                                             Standard_Real&              First,
                                                             Standard_Real&              Last,
                                                             Standard_Boolean* theIsStored = NULL);

  //! For the planar surface builds the 2d curve for the edge
  //! by projection of the edge on plane.
  //! Returns a NULL handle if the surface is not planar or
  //! the projection failed.
  //! [新手理解] 如果找不到已存的 PCurve，并且 Surface 是平面：
  //! - 就把 3D 曲线投影到平面，得到一条 2D 曲线作为“临时 PCurve”
  Standard_EXPORT static Handle(Geom2d_Curve) CurveOnPlane(const TopoDS_Edge&          E,
                                                           const Handle(Geom_Surface)& S,
                                                           const TopLoc_Location&      L,
                                                           Standard_Real&              First,
                                                           Standard_Real&              Last);

  //! Returns in <C>, <S>, <L> a 2d curve, a surface and
  //! a location for the edge <E>. <C> and <S>  are null
  //! if the  edge has no curve on  surface.  Returns in
  //! <First> and <Last> the parameter range.
  Standard_EXPORT static void CurveOnSurface(const TopoDS_Edge&    E,
                                             Handle(Geom2d_Curve)& C,
                                             Handle(Geom_Surface)& S,
                                             TopLoc_Location&      L,
                                             Standard_Real&        First,
                                             Standard_Real&        Last);

  //! Returns in <C>, <S>, <L> the 2d curve, the surface
  //! and the location for the edge <E> of rank <Index>.
  //! <C> and <S> are null if the index is out of range.
  //! Returns in <First> and <Last> the parameter range.
  Standard_EXPORT static void CurveOnSurface(const TopoDS_Edge&     E,
                                             Handle(Geom2d_Curve)&  C,
                                             Handle(Geom_Surface)&  S,
                                             TopLoc_Location&       L,
                                             Standard_Real&         First,
                                             Standard_Real&         Last,
                                             const Standard_Integer Index);

  //! Returns the polygon associated to the  edge in  the
  //! parametric  space of  the  face.  Returns   a NULL
  //! handle  if this polygon  does not exist.
  //! [新手理解] 取 Edge 在 Face 的参数空间里的“离散折线(2D)”：
  //! - 这是离散的 UV 折线，不是连续 2D 曲线
  Standard_EXPORT static Handle(Poly_Polygon2D) PolygonOnSurface(const TopoDS_Edge& E,
                                                                 const TopoDS_Face& F);

  //! Returns the polygon associated to the  edge in  the
  //! parametric  space of  the surface. Returns   a NULL
  //! handle  if this polygon  does not exist.
  Standard_EXPORT static Handle(Poly_Polygon2D) PolygonOnSurface(const TopoDS_Edge&          E,
                                                                 const Handle(Geom_Surface)& S,
                                                                 const TopLoc_Location&      L);

  //! Returns in <C>, <S>, <L> a 2d curve, a surface and
  //! a location for the edge <E>. <C> and <S>  are null
  //! if the  edge has no polygon on  surface.
  Standard_EXPORT static void PolygonOnSurface(const TopoDS_Edge&      E,
                                               Handle(Poly_Polygon2D)& C,
                                               Handle(Geom_Surface)&   S,
                                               TopLoc_Location&        L);

  //! Returns in <C>, <S>, <L> the 2d curve, the surface
  //! and the location for the edge <E> of rank <Index>.
  //! <C> and <S> are null if the index is out of range.
  Standard_EXPORT static void PolygonOnSurface(const TopoDS_Edge&      E,
                                               Handle(Poly_Polygon2D)& C,
                                               Handle(Geom_Surface)&   S,
                                               TopLoc_Location&        L,
                                               const Standard_Integer  Index);

  //! Returns the polygon associated to the  edge in  the
  //! parametric  space of  the  face.  Returns   a NULL
  //! handle  if this polygon  does not exist.
  //! [新手理解] 取 Edge 在三角网格(Poly_Triangulation)上的“折线索引”：
  //! - 这通常用于把边与网格对齐，或者在网格上快速走边界
  Standard_EXPORT static const Handle(Poly_PolygonOnTriangulation)& PolygonOnTriangulation(
    const TopoDS_Edge&                E,
    const Handle(Poly_Triangulation)& T,
    const TopLoc_Location&            L);

  //! Returns in <P>, <T>, <L> a polygon on triangulation, a
  //! triangulation and a location for the edge <E>.
  //! <P>  and  <T>  are null  if  the  edge has no
  //! polygon on  triangulation.
  Standard_EXPORT static void PolygonOnTriangulation(const TopoDS_Edge&                   E,
                                                     Handle(Poly_PolygonOnTriangulation)& P,
                                                     Handle(Poly_Triangulation)&          T,
                                                     TopLoc_Location&                     L);

  //! Returns   in   <P>,  <T>,    <L> a     polygon  on
  //! triangulation,   a triangulation  and a  location for
  //! the edge <E> for the range index.  <C> and <S> are
  //! null if the edge has no polygon on triangulation.
  Standard_EXPORT static void PolygonOnTriangulation(const TopoDS_Edge&                   E,
                                                     Handle(Poly_PolygonOnTriangulation)& P,
                                                     Handle(Poly_Triangulation)&          T,
                                                     TopLoc_Location&                     L,
                                                     const Standard_Integer               Index);

  //! Returns  True  if  <E>  has  two  PCurves  in  the
  //! parametric space of <F>. i.e.  <F>  is on a closed
  //! surface and <E> is on the closing curve.
  //! [新手理解] Edge 在同一个 Face 上有两条 PCurve 的典型场景：
  //! - Face 的底层 Surface 是闭合的（例如圆柱面）
  //! - Edge 正好在“缝合线/闭合线”上（例如圆柱的经线），两侧的 UV 需要两条表示
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoDS_Edge& E, const TopoDS_Face& F);

  //! Returns  True  if  <E>  has  two  PCurves  in  the
  //! parametric space  of <S>.  i.e.   <S>  is a closed
  //! surface and <E> is on the closing curve.
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoDS_Edge&          E,
                                                   const Handle(Geom_Surface)& S,
                                                   const TopLoc_Location&      L);

  //! Returns  True  if <E> has two arrays of indices in
  //! the triangulation <T>.
  //! [新手理解] 在三角网格上“闭合”的边：同一条边在网格索引上可能对应两套索引（两侧）。
  Standard_EXPORT static Standard_Boolean IsClosed(const TopoDS_Edge&                E,
                                                   const Handle(Poly_Triangulation)& T,
                                                   const TopLoc_Location&            L);

  //! Returns the tolerance for <E>.
  Standard_EXPORT static Standard_Real Tolerance(const TopoDS_Edge& E);

  //! Returns the SameParameter flag for the edge.
  //! [新手理解] SameParameter 可以先当成一句话：
  //! “这条边的 3D 曲线参数和 2D(PCurve) 参数是同一套参数标定”
  //! 这个标志对很多算法的稳健性/效率很重要。
  Standard_EXPORT static Standard_Boolean SameParameter(const TopoDS_Edge& E);

  //! Returns the SameRange flag for the edge.
  //! [新手理解] SameRange 表示：
  //! “边的 3D 曲线范围和它在曲面上的 2D 曲线范围一致”
  Standard_EXPORT static Standard_Boolean SameRange(const TopoDS_Edge& E);

  //! Returns True  if the edge is degenerated.
  //! [新手理解] Degenerated 边：可以理解成“长度几乎为 0 的边”，常见于球面顶点处的退化经线等。
  Standard_EXPORT static Standard_Boolean Degenerated(const TopoDS_Edge& E);

  //! Gets the range of the 3d curve.
  Standard_EXPORT static void Range(const TopoDS_Edge& E,
                                    Standard_Real&     First,
                                    Standard_Real&     Last);

  //! Gets the range  of the edge  on the pcurve on  the
  //! surface.
  Standard_EXPORT static void Range(const TopoDS_Edge&          E,
                                    const Handle(Geom_Surface)& S,
                                    const TopLoc_Location&      L,
                                    Standard_Real&              First,
                                    Standard_Real&              Last);

  //! Gets the range of the edge on the pcurve on the face.
  Standard_EXPORT static void Range(const TopoDS_Edge& E,
                                    const TopoDS_Face& F,
                                    Standard_Real&     First,
                                    Standard_Real&     Last);

  //! Gets the UV locations of the extremities of the edge.
  //! [新手理解] UVPoints：把边的两端点投到面参数空间，得到 (U,V) 起点/终点。
  Standard_EXPORT static void UVPoints(const TopoDS_Edge&          E,
                                       const Handle(Geom_Surface)& S,
                                       const TopLoc_Location&      L,
                                       gp_Pnt2d&                   PFirst,
                                       gp_Pnt2d&                   PLast);

  //! Gets the UV locations of the extremities of the edge.
  Standard_EXPORT static void UVPoints(const TopoDS_Edge& E,
                                       const TopoDS_Face& F,
                                       gp_Pnt2d&          PFirst,
                                       gp_Pnt2d&          PLast);

  //! Sets the UV locations of the extremities of the edge.
  //! [新手理解] SetUVPoints：直接写入“边端点在 UV 空间的参数值”。
  //! 注意：这属于修改拓扑/几何关联的数据，通常只在修复/重建 PCurve 时使用。
  Standard_EXPORT static void SetUVPoints(const TopoDS_Edge&          E,
                                          const Handle(Geom_Surface)& S,
                                          const TopLoc_Location&      L,
                                          const gp_Pnt2d&             PFirst,
                                          const gp_Pnt2d&             PLast);

  //! Sets the UV locations of the extremities of the edge.
  Standard_EXPORT static void SetUVPoints(const TopoDS_Edge& E,
                                          const TopoDS_Face& F,
                                          const gp_Pnt2d&    PFirst,
                                          const gp_Pnt2d&    PLast);

  //! Returns True if the edge is on the surfaces of the
  //! two faces.
  //! [新手理解] HasContinuity / Continuity：判断两侧面沿着这条边的连续性（C0/C1/C2...）。
  //! - C0：只是接触（位置连续）
  //! - C1：切向连续
  //! - C2：曲率连续
  Standard_EXPORT static Standard_Boolean HasContinuity(const TopoDS_Edge& E,
                                                        const TopoDS_Face& F1,
                                                        const TopoDS_Face& F2);

  //! Returns the continuity.
  Standard_EXPORT static GeomAbs_Shape Continuity(const TopoDS_Edge& E,
                                                  const TopoDS_Face& F1,
                                                  const TopoDS_Face& F2);

  //! Returns True if the edge is on the surfaces.
  Standard_EXPORT static Standard_Boolean HasContinuity(const TopoDS_Edge&          E,
                                                        const Handle(Geom_Surface)& S1,
                                                        const Handle(Geom_Surface)& S2,
                                                        const TopLoc_Location&      L1,
                                                        const TopLoc_Location&      L2);

  //! Returns the continuity.
  Standard_EXPORT static GeomAbs_Shape Continuity(const TopoDS_Edge&          E,
                                                  const Handle(Geom_Surface)& S1,
                                                  const Handle(Geom_Surface)& S2,
                                                  const TopLoc_Location&      L1,
                                                  const TopLoc_Location&      L2);

  //! Returns True if the edge has regularity on some
  //! two surfaces
  Standard_EXPORT static Standard_Boolean HasContinuity(const TopoDS_Edge& E);

  //! Returns the max continuity of edge between some surfaces or GeomAbs_C0 if there no such
  //! surfaces.
  Standard_EXPORT static GeomAbs_Shape MaxContinuity(const TopoDS_Edge& theEdge);

  //! Returns the 3d point.
  //! @param V [in] 顶点
  //! @return gp_Pnt (3D 笛卡尔坐标点)
  //! @note 获取顶点的几何坐标。
  //! [新手理解] Vertex 的“几何点坐标”，这是你最常用的取点接口之一。
  Standard_EXPORT static gp_Pnt Pnt(const TopoDS_Vertex& V);

  //! Returns the tolerance.
  //! @note 获取顶点的公差。在这个公差半径内，该顶点被认为是单一的点。
  Standard_EXPORT static Standard_Real Tolerance(const TopoDS_Vertex& V);

  //! Finds the parameter of <theV> on <theE>.
  //! @param[in] theV  input vertex
  //! @param[in] theE  input edge
  //! @param[out] theParam   calculated parameter on the curve
  //! @return TRUE if done
  Standard_EXPORT static Standard_Boolean Parameter(const TopoDS_Vertex& theV,
                                                    const TopoDS_Edge&   theE,
                                                    Standard_Real&       theParam);

  //! Returns the parameter of <V> on <E>.
  //! @param V [in] 顶点
  //! @param E [in] 边
  //! @return Standard_Real (参数值)
  //! @note 获取顶点 V 在边 E 上的参数值。如果 V 不是 E 的顶点，行为未定义。
  Standard_EXPORT static Standard_Real Parameter(const TopoDS_Vertex& V, const TopoDS_Edge& E);

  //! Returns the  parameters  of   the  vertex   on the
  //! pcurve of the edge on the face.
  Standard_EXPORT static Standard_Real Parameter(const TopoDS_Vertex& V,
                                                 const TopoDS_Edge&   E,
                                                 const TopoDS_Face&   F);

  //! Returns the  parameters  of   the  vertex   on the
  //! pcurve of the edge on the surface.
  Standard_EXPORT static Standard_Real Parameter(const TopoDS_Vertex&        V,
                                                 const TopoDS_Edge&          E,
                                                 const Handle(Geom_Surface)& S,
                                                 const TopLoc_Location&      L);

  //! Returns the parameters of <V> on <F>.
  //! @param V [in] 顶点
  //! @param F [in] 面
  //! @return gp_Pnt2d (UV 坐标)
  //! @note 获取顶点 V 在面 F 上的 UV 参数坐标。
  //! [新手理解] Parameters(V, F) 返回的是 (U,V) 而不是 (X,Y,Z)：
  //! - (X,Y,Z) 在 3D 空间
  //! - (U,V) 是在曲面参数域里
  Standard_EXPORT static gp_Pnt2d Parameters(const TopoDS_Vertex& V, const TopoDS_Face& F);

  //! Returns the maximum tolerance of input shape subshapes.
  //@param theShape    - Shape to search tolerance.
  //@param theSubShape - Search subshape, only Face, Edge or Vertex are supported.
  Standard_EXPORT static Standard_Real MaxTolerance(const TopoDS_Shape&    theShape,
                                                    const TopAbs_ShapeEnum theSubShape);
};

#endif // _BRep_Tool_HeaderFile
