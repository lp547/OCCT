// Created on: 1992-05-27
// Created by: Remi LEQUETTE
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _BRep_TVertex_HeaderFile
#define _BRep_TVertex_HeaderFile // 头文件保护：防止重复包含

#include <Standard.hxx> // OCCT 基础定义：导出宏、基础类型、断言等

#include <gp_Pnt.hxx>                         // gp_Pnt：三维点坐标（X,Y,Z）
#include <Standard_Real.hxx>                  // Standard_Real：OCCT 统一的实数类型（通常为 double）
#include <BRep_ListOfPointRepresentation.hxx> // 顶点的点表示列表（在曲线/曲面上的参数等）
#include <TopoDS_TVertex.hxx>                 // TopoDS_TVertex：拓扑顶点的基础 TShape 实现
class TopoDS_TShape;
// 前置声明：避免在头文件中引入不必要的依赖（减少编译开销）

class BRep_TVertex;
DEFINE_STANDARD_HANDLE(BRep_TVertex, TopoDS_TVertex)
// DEFINE_STANDARD_HANDLE：为 BRep_TVertex 声明 OCCT 智能句柄 Handle(BRep_TVertex)

//! The TVertex from  BRep inherits  from  the TVertex
//! from TopoDS. It contains the geometric data.
//!
//! The  TVertex contains a 3d point, location and a tolerance.
//! (TVertex 包含一个 3D 点坐标，以及该点的公差。)
class BRep_TVertex : public TopoDS_TVertex
{

public:
  Standard_EXPORT BRep_TVertex(); // 构造：创建一个空顶点（默认公差为 RealEpsilon）

  //! Returns the tolerance.
  //! @return 公差值
  Standard_Real Tolerance() const; // 获取顶点公差（用于几何-拓扑一致性判断）

  //! Sets the tolerance.
  void Tolerance(const Standard_Real T); // 设置顶点公差（直接覆盖）

  //! Sets the tolerance  to the   max  of <T>  and  the
  //! current  tolerance.
  //! @note 更新公差，取较大值。
  void UpdateTolerance(const Standard_Real T); // 更新公差：myTolerance = max(myTolerance, T)

  //! Returns the 3D point.
  //! @return 3D 点坐标 (gp_Pnt)
  const gp_Pnt& Pnt() const; // 获取顶点的 3D 坐标（几何位置）

  //! Sets the 3D point.
  void Pnt(const gp_Pnt& P); // 设置顶点的 3D 坐标（几何位置）

  //! Returns the list of point representations.
  //! @return 点表示列表 (BRep_ListOfPointRepresentation)
  //! @note 该列表存储了顶点在边或面上的参数信息。
  const BRep_ListOfPointRepresentation& Points() const; // 只读访问：顶点在不同曲线/曲面上的参数表示

  //! Returns the list of point representations for modification.
  BRep_ListOfPointRepresentation& ChangePoints(); // 可修改访问：用于追加/删除点表示

  //! Returns a copy  of the  TShape  with no sub-shapes.
  Standard_EXPORT Handle(TopoDS_TShape) EmptyCopy() const Standard_OVERRIDE; // 复制：仅复制本体数据，不复制子形状

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;
  // DumpJson：用于调试/诊断，把对象内容以 JSON 形式写到输出流

  DEFINE_STANDARD_RTTIEXT(BRep_TVertex, TopoDS_TVertex)
  // RTTI：OCCT 运行时类型识别（IsKind / DynamicType 等）

protected:
private:
  gp_Pnt                         myPnt;       // 顶点的 3D 坐标（几何数据）
  Standard_Real                  myTolerance; // 顶点公差（允许的几何误差范围）
  BRep_ListOfPointRepresentation myPoints;    // 顶点在边/面上的参数表示集合（可有多个）
};

#include <BRep_TVertex.lxx> // 内联方法实现（为了性能与模板/内联约定）

#endif // _BRep_TVertex_HeaderFile // 头文件保护结束
