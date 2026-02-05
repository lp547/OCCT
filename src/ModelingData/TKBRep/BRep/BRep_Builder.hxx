// Created on: 1991-07-01
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

#ifndef _BRep_Builder_HeaderFile
#define _BRep_Builder_HeaderFile

// 引入必要的头文件
#include <GeomAbs_Shape.hxx>           // 定义几何形状的基本类型（如 C0, G1 等连续性）
#include <Poly_ListOfTriangulation.hxx> // 三角剖分列表，用于处理网格数据
#include <Standard.hxx>                // 标准基础定义
#include <Standard_DefineAlloc.hxx>    // 内存分配宏定义
#include <Standard_Handle.hxx>         // 智能指针 Handle 定义
#include <TopoDS_Builder.hxx>          // 父类 TopoDS_Builder，提供了基础的拓扑构建功能

// 前置声明 (Forward declarations)
// 告诉编译器这些类存在，但具体细节在其他地方定义
class TopoDS_Face;                    // 拓扑面
class Geom_Surface;                   // 几何曲面（如平面、球面、B样条曲面）
class TopLoc_Location;                // 位置/坐标变换（平移、旋转）
class Poly_Triangulation;             // 三角网格数据（点和三角形的集合）
class TopoDS_Edge;                    // 拓扑边
class Geom_Curve;                     // 3D 几何曲线
class Poly_Polygon3D;                 // 3D 多边形（离散的线段集合）
class Poly_PolygonOnTriangulation;    // 三角网上的多边形（用于网格上的线）
class Geom2d_Curve;                   // 2D 几何曲线（参数空间中的曲线，PCurve）
class gp_Pnt2d;                       // 2D 点
class Poly_Polygon2D;                 // 2D 多边形
class TopoDS_Vertex;                  // 拓扑顶点
class gp_Pnt;                         // 3D 点

//! A framework providing advanced tolerance control.
//! 一个提供高级公差控制的框架。
//!
//! It is used to build Shapes.
//! 它专门用于构建 BRep (Boundary Representation, 边界表示) 形状。
//!
//! If tolerance control is required, you are advised to:
//! 如果需要控制公差，建议按以下步骤操作：
//! 1. build a default precision for topology, using the
//!    classes provided in the BRepAPI package
//!    使用 BRepAPI 包提供的类构建默认精度的拓扑结构。
//! 2. update the tolerance of the resulting shape.
//!    更新生成形状的公差。
//!
//! Note that only vertices, edges and faces have
//! meaningful tolerance control.
//! 注意：只有顶点(Vertex)、边(Edge)和面(Face)具有有意义的公差控制。
//!
//! The tolerance value must always comply with the condition that face
//! tolerances are more restrictive than edge tolerances
//! which are more restrictive than vertex tolerances.
//! 公差值必须始终遵守以下条件：面的公差比边的公差更严格（更小），
//! 而边的公差比顶点的公差更严格。
//!
//! In other words: Tol(Vertex) >= Tol(Edge) >= Tol(Face).
//! 换句话说：顶点公差 >= 边公差 >= 面公差。
//!
//! Other rules in setting tolerance include:
//! 设置公差的其他规则包括：
//! - you can open up tolerance but should never restrict it
//!   你可以放宽（增大）公差，但永远不应该收紧（减小）它。
//!   (因为一旦数据不精确了，你就不能假装它变精确了)
//! - an edge cannot be included within the fusion of the
//!   tolerance spheres of two vertices
//!   一条边不能完全被其两个顶点的公差球融合区域所包含。
//!   (否则这条边就太短了，在公差范围内退化成了一个点)
class BRep_Builder : public TopoDS_Builder
{
public:
  // OCCT 标准内存分配宏
  DEFINE_STANDARD_ALLOC

  // ========================================================================
  //  Face (面) 的构建方法
  //  新手理解：BRep_Face = 几何曲面 + 边界(Wires) + 公差 + 位置
  // ========================================================================

  //! Makes an undefined Face.
  //! 创建一个未定义的面（没有几何信息的空面）。
  //! @param F [out] 要构建的面
  void MakeFace(TopoDS_Face& F) const;

  //! Makes a Face with a surface.
  //! 使用几何曲面创建一个面。
  //! @param F   [out] 构建生成的面
  //! @param S   [in]  几何曲面句柄 (如平面、圆柱面等)
  //! @param Tol [in]  公差值 (用于判断点是否在面上)
  //! @note 这是最常用的创建面的方法。
  Standard_EXPORT void MakeFace(TopoDS_Face&                F,
                                const Handle(Geom_Surface)& S,
                                const Standard_Real         Tol) const;

  //! Makes a Face with a surface and a location.
  //! 使用几何曲面和位置变换创建一个面。
  //! @param F   [out] 构建生成的面
  //! @param S   [in]  几何曲面
  //! @param L   [in]  位置变换 (Location)，定义曲面在空间中的位置
  //! @param Tol [in]  公差
  Standard_EXPORT void MakeFace(TopoDS_Face&                F,
                                const Handle(Geom_Surface)& S,
                                const TopLoc_Location&      L,
                                const Standard_Real         Tol) const;

  //! Makes a theFace with a single triangulation.
  //! 使用单个三角网格创建一个面。
  //! @param theFace          [out] 构建生成的面
  //! @param theTriangulation [in]  三角网格数据
  //! @note 这种面没有解析几何曲面，只有离散的网格数据。
  Standard_EXPORT void MakeFace(TopoDS_Face&                      theFace,
                                const Handle(Poly_Triangulation)& theTriangulation) const;

  //! Makes a Face with a list of triangulations and active one.
  //! 使用三角网格列表创建一个面，并指定当前激活的网格。
  //! @param theFace                [out] 构建生成的面
  //! @param theTriangulations      [in]  三角网格列表
  //! @param theActiveTriangulation [in]  当前激活的网格 (默认为空，表示使用列表中的第一个)
  Standard_EXPORT void MakeFace(
    TopoDS_Face&                      theFace,
    const Poly_ListOfTriangulation&   theTriangulations,
    const Handle(Poly_Triangulation)& theActiveTriangulation = Handle(Poly_Triangulation)()) const;

  //! Updates the face F using the tolerance value Tol, surface S and location Location.
  //! 更新面 F 的几何信息：曲面 S、位置 L 和公差 Tol。
  //! @note 如果面已经创建但需要修改其几何定义，使用此方法。
  Standard_EXPORT void UpdateFace(const TopoDS_Face&          F,
                                  const Handle(Geom_Surface)& S,
                                  const TopLoc_Location&      L,
                                  const Standard_Real         Tol) const;

  //! Changes a face triangulation.
  //! 修改或设置面的三角网格。
  //! @param theFace          [in] 要更新的面
  //! @param theTriangulation [in] 新的三角网格 (如果为 NULL，则移除网格)
  //! @param theToReset       [in] 是否重置网格列表 (True: 替换所有网格; False: 添加或更新)
  Standard_EXPORT void UpdateFace(const TopoDS_Face&                theFace,
                                  const Handle(Poly_Triangulation)& theTriangulation,
                                  const Standard_Boolean            theToReset = true) const;

  //! Updates the face Tolerance.
  //! 仅更新面的公差。
  Standard_EXPORT void UpdateFace(const TopoDS_Face& F, const Standard_Real Tol) const;

  //! Sets the NaturalRestriction flag of the face.
  //! 设置面的 "自然边界限制" 标志。
  //! @param N [in] 如果为 True，表示面的边界就是其几何曲面的参数边界 (Umin, Umax, Vmin, Vmax)。
  Standard_EXPORT void NaturalRestriction(const TopoDS_Face& F, const Standard_Boolean N) const;

  // ========================================================================
  //  Edge (边) 的构建方法
  //  新手理解：BRep_Edge = 3D曲线 + 2D参数曲线(PCurve) + 顶点 + 公差
  // ========================================================================

  //! Makes an undefined Edge (no geometry).
  //! 创建一个未定义的边（没有几何信息的空边）。
  //! @param E [out] 要构建的边
  void MakeEdge(TopoDS_Edge& E) const;

  //! Makes an Edge with a curve.
  //! 使用 3D 曲线创建一条边。
  //! @param E   [out] 构建生成的边
  //! @param C   [in]  3D 几何曲线 (如直线、圆弧、B样条曲线)
  //! @param Tol [in]  公差 (覆盖曲线与实际顶点的偏差)
  Standard_EXPORT void MakeEdge(TopoDS_Edge&              E,
                                const Handle(Geom_Curve)& C,
                                const Standard_Real       Tol) const;

  //! Makes an Edge with a curve and a location.
  //! 使用 3D 曲线和位置变换创建一条边。
  //! @param E   [out] 构建生成的边
  //! @param C   [in]  3D 几何曲线
  //! @param L   [in]  位置变换
  //! @param Tol [in]  公差
  Standard_EXPORT void MakeEdge(TopoDS_Edge&              E,
                                const Handle(Geom_Curve)& C,
                                const TopLoc_Location&    L,
                                const Standard_Real       Tol) const;

  //! Makes an Edge with a polygon 3d.
  //! 使用 3D 多边形创建一条边。
  //! @param E [out] 构建生成的边
  //! @param P [in]  3D 多边形 (离散的点序列)
  //! @note 这种边没有连续的解析曲线，只有折线段。
  Standard_EXPORT void MakeEdge(TopoDS_Edge&                  E,
                                const Handle(Poly_Polygon3D)& P) const;

  //! Makes an Edge with a polygon on triangulation.
  //! 在三角网格上创建一条边（使用多边形索引）。
  //! @param E [out] 构建生成的边
  //! @param N [in]  多边形在三角网上的节点索引
  //! @param T [in]  关联的三角网格
  Standard_EXPORT void MakeEdge(TopoDS_Edge&                           E,
                                const Handle(Poly_PolygonOnTriangulation)& N,
                                const Handle(Poly_Triangulation)&          T) const;

  //! Makes an Edge with a polygon on triangulation and a location.
  //! 在三角网格上创建一条边，带位置变换。
  Standard_EXPORT void MakeEdge(TopoDS_Edge&                           E,
                                const Handle(Poly_PolygonOnTriangulation)& N,
                                const Handle(Poly_Triangulation)&          T,
                                const TopLoc_Location&                     L) const;

  //! Makes an Edge with a closed polygon on triangulation.
  //! 在三角网格上创建一条闭合边（例如圆柱的接缝）。
  //! @param N [in] 闭合多边形定义（包含两个对应的节点索引序列）
  Standard_EXPORT void MakeEdge(TopoDS_Edge&                                 E,
                                const Handle(Poly_PolygonOnClosedTriangulation)& N,
                                const Handle(Poly_Triangulation)&                T) const;

  //! Makes an Edge with a closed polygon on triangulation and a location.
  //! 在三角网格上创建一条闭合边，带位置变换。
  Standard_EXPORT void MakeEdge(TopoDS_Edge&                                 E,
                                const Handle(Poly_PolygonOnClosedTriangulation)& N,
                                const Handle(Poly_Triangulation)&                T,
                                const TopLoc_Location&                           L) const;

  //! Sets a 3D curve for the edge.
  //! 更新边，设置其 3D 曲线。
  //! @param E   [in] 要更新的边
  //! @param C   [in] 新的 3D 曲线
  //! @param Tol [in] 新的公差
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&        E,
                                  const Handle(Geom_Curve)& C,
                                  const Standard_Real       Tol) const;

  //! Sets a 3D curve for the edge with a location.
  //! 更新边，设置其 3D 曲线和位置。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&        E,
                                  const Handle(Geom_Curve)& C,
                                  const TopLoc_Location&    L,
                                  const Standard_Real       Tol) const;

  //! Sets a pcurve for the edge on the face.
  //! 在面上为边设置 PCurve (参数曲线)。
  //! @param E   [in] 要更新的边
  //! @param C   [in] 2D 参数曲线 (定义在面 F 的参数空间 UV 中)
  //! @param F   [in] 边所在的参照面
  //! @param Tol [in] 公差
  //! @note 核心概念：PCurve 将 3D 边映射到 2D 面上。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&          E,
                                  const Handle(Geom2d_Curve)& C,
                                  const TopoDS_Face&          F,
                                  const Standard_Real         Tol) const;

  //! Sets pcurves for the edge on the closed face.
  //! 在闭合面上为边设置 PCurves (例如圆柱面的接缝，需要两条 2D 曲线)。
  //! @param C1 [in] 第一条 2D 曲线 (对应接缝的一侧)
  //! @param C2 [in] 第二条 2D 曲线 (对应接缝的另一侧)
  void UpdateEdge(const TopoDS_Edge&          E,
                  const Handle(Geom2d_Curve)& C1,
                  const Handle(Geom2d_Curve)& C2,
                  const TopoDS_Face&          F,
                  const Standard_Real         Tol) const;

  //! Sets a pcurve for the edge on the face.
  //! 在面上为边设置 PCurve (使用曲面 S 和位置 L 直接定义，不通过 Face 对象)。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&          E,
                                  const Handle(Geom2d_Curve)& C,
                                  const Handle(Geom_Surface)& S,
                                  const TopLoc_Location&      L,
                                  const Standard_Real         Tol) const;

  //! Sets a pcurve for the edge on the face.
  //! 在面上为边设置 PCurve，并指定 UV 边界点。
  //! @param Pf [in] 起点在 UV 空间中的坐标
  //! @param Pl [in] 终点在 UV 空间中的坐标
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&          E,
                                  const Handle(Geom2d_Curve)& C,
                                  const Handle(Geom_Surface)& S,
                                  const TopLoc_Location&      L,
                                  const Standard_Real         Tol,
                                  const gp_Pnt2d&             Pf,
                                  const gp_Pnt2d&             Pl) const;

  //! Sets pcurves for the edge on the closed surface.
  //! 在闭合曲面上为边设置 PCurves (直接使用曲面对象)。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&          E,
                                  const Handle(Geom2d_Curve)& C1,
                                  const Handle(Geom2d_Curve)& C2,
                                  const Handle(Geom_Surface)& S,
                                  const TopLoc_Location&      L,
                                  const Standard_Real         Tol) const;

  //! Sets pcurves for the edge on the closed surface.
  //! 在闭合曲面上为边设置 PCurves，并指定 UV 边界点。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&          E,
                                  const Handle(Geom2d_Curve)& C1,
                                  const Handle(Geom2d_Curve)& C2,
                                  const Handle(Geom_Surface)& S,
                                  const TopLoc_Location&      L,
                                  const Standard_Real         Tol,
                                  const gp_Pnt2d&             Pf,
                                  const gp_Pnt2d&             Pl) const;

  //! Changes an Edge 3D polygon.
  //! 更新边的 3D 多边形表示。
  //! @param P [in] 新的 3D 多边形 (如果是 NULL，则移除)
  void UpdateEdge(const TopoDS_Edge& E, const Handle(Poly_Polygon3D)& P) const;

  //! Changes an Edge 3D polygon.
  //! 更新边的 3D 多边形表示，带位置变换。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&            E,
                                  const Handle(Poly_Polygon3D)& P,
                                  const TopLoc_Location&        L) const;

  //! Changes an Edge polygon on Triangulation.
  //! 更新边在三角网格上的多边形表示。
  void UpdateEdge(const TopoDS_Edge&                         E,
                  const Handle(Poly_PolygonOnTriangulation)& N,
                  const Handle(Poly_Triangulation)&          T) const;

  //! Changes an Edge polygon on Triangulation.
  //! 更新边在三角网格上的多边形表示，带位置变换。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&                         E,
                                  const Handle(Poly_PolygonOnTriangulation)& N,
                                  const Handle(Poly_Triangulation)&          T,
                                  const TopLoc_Location&                     L) const;

  //! Changes an Edge polygon on Triangulation.
  //! 更新闭合边在三角网格上的多边形表示 (需要两个多边形索引 N1, N2)。
  void UpdateEdge(const TopoDS_Edge&                         E,
                  const Handle(Poly_PolygonOnTriangulation)& N1,
                  const Handle(Poly_PolygonOnTriangulation)& N2,
                  const Handle(Poly_Triangulation)&          T) const;

  //! Changes an Edge polygon on Triangulation.
  //! 更新闭合边在三角网格上的多边形表示，带位置变换。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&                         E,
                                  const Handle(Poly_PolygonOnTriangulation)& N1,
                                  const Handle(Poly_PolygonOnTriangulation)& N2,
                                  const Handle(Poly_Triangulation)&          T,
                                  const TopLoc_Location&                     L) const;

  //! Changes Edge polygon on a face.
  //! 更新边在面上的 2D 多边形表示 (离散的 PCurve)。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&            E,
                                  const Handle(Poly_Polygon2D)& P,
                                  const TopoDS_Face&            S) const;

  //! Changes Edge polygon on a face.
  //! 更新边在曲面上的 2D 多边形表示。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&            E,
                                  const Handle(Poly_Polygon2D)& P,
                                  const Handle(Geom_Surface)&   S,
                                  const TopLoc_Location&        T) const;

  //! Changes Edge polygons on a face.
  //! 更新闭合边在面上的 2D 多边形表示 (需要两个 2D 多边形)。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&            E,
                                  const Handle(Poly_Polygon2D)& P1,
                                  const Handle(Poly_Polygon2D)& P2,
                                  const TopoDS_Face&            S) const;

  //! Changes Edge polygons on a face.
  //! 更新闭合边在曲面上的 2D 多边形表示。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge&            E,
                                  const Handle(Poly_Polygon2D)& P1,
                                  const Handle(Poly_Polygon2D)& P2,
                                  const Handle(Geom_Surface)&   S,
                                  const TopLoc_Location&        L) const;

  //! Updates the edge tolerance.
  //! 仅更新边的公差。
  Standard_EXPORT void UpdateEdge(const TopoDS_Edge& E, const Standard_Real Tol) const;

  //! Sets the geometric continuity on the edge.
  //! 设置边在两个面 F1, F2 之间的几何连续性 (如 C0, G1, C2)。
  //! @param C [in] 连续性级别 (GeomAbs_C0, GeomAbs_G1, etc.)
  Standard_EXPORT void Continuity(const TopoDS_Edge&  E,
                                  const TopoDS_Face&  F1,
                                  const TopoDS_Face&  F2,
                                  const GeomAbs_Shape C) const;

  //! Sets the geometric continuity on the edge.
  //! 设置边在两个曲面 S1, S2 之间的几何连续性。
  Standard_EXPORT void Continuity(const TopoDS_Edge&          E,
                                  const Handle(Geom_Surface)& S1,
                                  const Handle(Geom_Surface)& S2,
                                  const TopLoc_Location&      L1,
                                  const TopLoc_Location&      L2,
                                  const GeomAbs_Shape         C) const;

  //! Sets the same parameter flag for the edge <E>.
  //! 设置 "SameParameter" 标志。
  //! @param S [in] 如果为 True，表示边在 3D 空间和 2D 参数空间中的参数化是一致的。
  //!               即：Curve3D(t) == Surface(PCurve(t)) (在公差范围内)。
  Standard_EXPORT void SameParameter(const TopoDS_Edge& E, const Standard_Boolean S) const;

  //! Sets the same range flag for the edge <E>.
  //! 设置 "SameRange" 标志。
  //! @param S [in] 如果为 True，表示边的所有几何表示（3D曲线、PCurves）共享相同的参数范围 [First, Last]。
  Standard_EXPORT void SameRange(const TopoDS_Edge& E, const Standard_Boolean S) const;

  //! Sets the degenerated flag for the edge <E>.
  //! 设置 "Degenerated" (退化) 标志。
  //! @param D [in] 如果为 True，表示这条边在 3D 空间中几何上是一个点（例如球面的极点）。
  //!               但在参数空间中，它可能仍然是一条线（例如球面的顶部边界 V=PI/2）。
  Standard_EXPORT void Degenerated(const TopoDS_Edge& E, const Standard_Boolean D) const;

  //! Sets the range of the 3d curve if Only3d=TRUE,
  //! otherwise sets the range to all the representations.
  //! 设置边的参数范围 [First, Last]。
  //! @param Only3d [in] 如果为 True，只更新 3D 曲线的范围；否则更新所有几何表示的范围。
  Standard_EXPORT void Range(const TopoDS_Edge&     E,
                             const Standard_Real    First,
                             const Standard_Real    Last,
                             const Standard_Boolean Only3d = Standard_False) const;

  //! Sets the range of the edge on the pcurve on the surface.
  //! 仅设置边在指定曲面 S 上的 PCurve 的参数范围。
  Standard_EXPORT void Range(const TopoDS_Edge&          E,
                             const Handle(Geom_Surface)& S,
                             const TopLoc_Location&      L,
                             const Standard_Real         First,
                             const Standard_Real         Last) const;

  //! Sets the range of the edge on the pcurve on the face.
  //! 仅设置边在指定面 F 上的 PCurve 的参数范围。
  void Range(const TopoDS_Edge&  E,
             const TopoDS_Face&  F,
             const Standard_Real First,
             const Standard_Real Last) const;

  //! Add to <Eout> the geometric representations of <Ein>.
  //! 将 Ein 的所有几何表示转移（复制）给 Eout。
  //! @note 用于合并或重建边。
  Standard_EXPORT void Transfert(const TopoDS_Edge& Ein, const TopoDS_Edge& Eout) const;

  // ========================================================================
  //  Vertex (顶点) 的构建方法
  //  新手理解：BRep_Vertex = 3D点 + 公差 + (可选的)在边/面上的参数位置
  // ========================================================================

  //! Makes an udefined vertex without geometry.
  //! 创建一个未定义的顶点（没有 3D 点信息）。
  void MakeVertex(TopoDS_Vertex& V) const;

  //! Makes a vertex from a 3D point.
  //! 使用 3D 点创建顶点。
  //! @param V   [out] 构建生成的顶点
  //! @param P   [in]  3D 点坐标
  //! @param Tol [in]  公差 (表示点的不确定范围)
  void MakeVertex(TopoDS_Vertex& V, const gp_Pnt& P, const Standard_Real Tol) const;

  //! Sets a 3D point on the vertex.
  //! 更新顶点的 3D 点坐标。
  Standard_EXPORT void UpdateVertex(const TopoDS_Vertex& V,
                                    const gp_Pnt&        P,
                                    const Standard_Real  Tol) const;

  //! Sets the parameter for the vertex on the edge curves.
  //! 设置顶点在边 E 上的参数值 P。
  //! @param V   [in] 顶点
  //! @param P   [in] 参数值 (t)
  //! @param E   [in] 边
  //! @param Tol [in] 公差
  //! @note 这告诉系统：Vertex = Edge.Curve(t)
  Standard_EXPORT void UpdateVertex(const TopoDS_Vertex& V,
                                    const Standard_Real  P,
                                    const TopoDS_Edge&   E,
                                    const Standard_Real  Tol) const;

  //! Sets the parameter for the vertex on the edge pcurve on the face.
  //! 设置顶点在边 E 的 PCurve（位于面 F 上）上的参数值 P。
  void UpdateVertex(const TopoDS_Vertex& V,
                    const Standard_Real  P,
                    const TopoDS_Edge&   E,
                    const TopoDS_Face&   F,
                    const Standard_Real  Tol) const;

  //! Sets the parameter for the vertex on the edge pcurve on the surface.
  //! 设置顶点在边 E 的 PCurve（位于曲面 S 上）上的参数值 P。
  Standard_EXPORT void UpdateVertex(const TopoDS_Vertex&        V,
                                    const Standard_Real         P,
                                    const TopoDS_Edge&          E,
                                    const Handle(Geom_Surface)& S,
                                    const TopLoc_Location&      L,
                                    const Standard_Real         Tol) const;

  //! Sets the parameters for the vertex on the face.
  //! 设置顶点在面 F 上的参数坐标 (U, V)。
  //! @note 用于直接指定顶点在面上的位置，不一定非要在边上。
  Standard_EXPORT void UpdateVertex(const TopoDS_Vertex& Ve,
                                    const Standard_Real  U,
                                    const Standard_Real  V,
                                    const TopoDS_Face&   F,
                                    const Standard_Real  Tol) const;

  //! Updates the vertex tolerance.
  //! 仅更新顶点的公差。
  Standard_EXPORT void UpdateVertex(const TopoDS_Vertex& V, const Standard_Real Tol) const;

  //! Transfert the parameters of Vin on Ein as the parameter of Vout on Eout.
  //! 将 Vin 在 Ein 上的参数信息转移给 Vout 在 Eout 上的参数信息。
  //! @note 假设 Vin/Ein 和 Vout/Eout 几何上是对应的。
  Standard_EXPORT void Transfert(const TopoDS_Edge&   Ein,
                                 const TopoDS_Edge&   Eout,
                                 const TopoDS_Vertex& Vin,
                                 const TopoDS_Vertex& Vout) const;

protected:
private:
};

#include <BRep_Builder.lxx>

#endif // _BRep_Builder_HeaderFile
