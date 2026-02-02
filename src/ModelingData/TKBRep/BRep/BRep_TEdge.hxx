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

#ifndef _BRep_TEdge_HeaderFile
#define _BRep_TEdge_HeaderFile // 头文件保护：防止重复包含

#include <Standard.hxx>      // OCCT 基础定义：导出宏、基础类型等
#include <Standard_Type.hxx> // OCCT RTTI：Standard_Type / DynamicType 支持

#include <Standard_Integer.hxx>               // Standard_Integer：OCCT 统一整数类型
#include <BRep_ListOfCurveRepresentation.hxx> // 边的曲线表示列表（3D 曲线、PCurve、多边形等）
#include <TopoDS_TEdge.hxx>                   // TopoDS_TEdge：拓扑边的基础 TShape 实现
class TopoDS_TShape;
// 前置声明：减少头文件依赖，缩短编译时间

class BRep_TEdge;
DEFINE_STANDARD_HANDLE(BRep_TEdge, TopoDS_TEdge)
// DEFINE_STANDARD_HANDLE：为 BRep_TEdge 声明 Handle(BRep_TEdge) 智能句柄类型

//! The TEdge from BRep is  inherited from  the  TEdge
//! from TopoDS. It contains the geometric data.
//!
//! The TEdge contains :
//!
//! * A tolerance. (公差：用于判定顶点是否在边上，或者边是否在面上)
//!
//! * A same parameter flag.
//!   (SameParameter 标志：如果为 True，表示边的 3D 曲线和所有 2D 参数曲线 (PCurve)
//!    在参数化上是同步的。即 C3d(t) 与 Surface(C2d(t)) 是同一点。)
//!
//! * A same range flag.
//!   (SameRange 标志：表示所有曲线表示共用相同的参数范围 [First, Last]。)
//!
//! * A Degenerated flag.
//!   (Degenerated 标志：表示这是一条退化边，几何上缩退为一个点。
//!    例如球面的极点处，虽然拓扑上是一条边，但几何上只是一个点。)
//!
//! *  A  list   of curve representation.
//!   (曲线表示列表：存储该边所有的几何形态，如 3D 曲线、面上的 2D 曲线、多边形等。)
class BRep_TEdge : public TopoDS_TEdge
{

public:
  //! Creates an empty TEdge.
  Standard_EXPORT BRep_TEdge(); // 构造：初始化默认公差与标志位（SameParameter/SameRange）

  //! Returns the tolerance.
  Standard_Real Tolerance() const; // 获取边公差（用于几何-拓扑一致性判断）

  //! Sets the tolerance.
  void Tolerance(const Standard_Real T); // 设置边公差（直接覆盖）

  //! Sets the tolerance  to the   max  of <T>  and  the
  //! current  tolerance.
  //! @note 更新公差。新公差取原公差和输入值 T 中的较大者。
  void UpdateTolerance(const Standard_Real T); // 更新公差：myTolerance = max(myTolerance, T)

  //! Returns SameParameter flag.
  Standard_EXPORT Standard_Boolean SameParameter() const; // 是否“同参”：3D 曲线与 2D 曲线参数同步

  //! Sets SameParameter flag.
  Standard_EXPORT void SameParameter(const Standard_Boolean S); // 设置“同参”标志

  //! Returns SameRange flag.
  Standard_EXPORT Standard_Boolean SameRange() const; // 是否“同范围”：所有表示共享同一参数范围

  //! Sets SameRange flag.
  Standard_EXPORT void SameRange(const Standard_Boolean S); // 设置“同范围”标志

  //! Returns Degenerated flag.
  Standard_EXPORT Standard_Boolean Degenerated() const; // 是否退化边：几何上缩退为点

  //! Sets Degenerated flag.
  Standard_EXPORT void Degenerated(const Standard_Boolean S); // 设置退化标志

  //! Returns the list of curve representations.
  //! @return 曲线表示列表的引用
  const BRep_ListOfCurveRepresentation& Curves() const; // 只读访问：曲线表示列表

  //! Returns the list of curve representations for modification.
  BRep_ListOfCurveRepresentation& ChangeCurves(); // 可修改访问：用于追加/替换曲线表示

  //! Returns a copy  of the  TShape  with no sub-shapes.
  Standard_EXPORT Handle(TopoDS_TShape) EmptyCopy() const Standard_OVERRIDE; // 复制：仅复制本体数据，不复制子形状

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;
  // DumpJson：用于调试/诊断，把对象内容以 JSON 形式写到输出流

  DEFINE_STANDARD_RTTIEXT(BRep_TEdge, TopoDS_TEdge)
  // RTTI：OCCT 运行时类型识别（IsKind / DynamicType 等）

protected:
private:
  Standard_Real                  myTolerance; // 边公差（几何误差允许范围）
  Standard_Integer               myFlags;     // 位标志：SameParameter/SameRange/Degenerated 等
  BRep_ListOfCurveRepresentation myCurves;    // 曲线表示列表：3D 曲线、PCurve、多边形等
};

#include <BRep_TEdge.lxx> // 内联方法实现（提高性能，遵循 OCCT 组织方式）

#endif // _BRep_TEdge_HeaderFile // 头文件保护结束
