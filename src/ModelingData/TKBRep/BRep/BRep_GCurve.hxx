// Created on: 1995-03-09
// Created by: Laurent PAINNOT
// Copyright (c) 1995-1999 Matra Datavision
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

#ifndef _BRep_GCurve_HeaderFile
#define _BRep_GCurve_HeaderFile
// 你先把它当成一句话就够了：
// BRep_GCurve = “曲线表示（BRep_CurveRepresentation）” + “参数范围 [First, Last]”。
// 它是一个抽象基类：要求子类至少能做一件事——给定参数 U 计算 3D 点 P（D0）。

#include <Standard.hxx> // OCCT 基础宏与基础类型

#include <Standard_Real.hxx>            // Standard_Real：OCCT 统一实数类型（通常为 double）
#include <BRep_CurveRepresentation.hxx> // 曲线表示基类：提供 Location 等
class TopLoc_Location;
class gp_Pnt;
// 前置声明：这里只需要指针/引用类型，避免引入更多头文件

class BRep_GCurve;
DEFINE_STANDARD_HANDLE(BRep_GCurve, BRep_CurveRepresentation)
// DEFINE_STANDARD_HANDLE：定义 Handle(BRep_GCurve) 智能句柄类型

//! Root   class    for    the    geometric     curves
//! representation. Contains a range.
//! Contains a first and a last parameter.
class BRep_GCurve : public BRep_CurveRepresentation
{

public:
  void SetRange(const Standard_Real First, const Standard_Real Last);
  // 设置参数范围 [First, Last]。
  // 注意：SetRange() 内部会调用 Update()，让子类在范围变化后更新派生数据（例如缓存 UV 端点）。

  void Range(Standard_Real& First, Standard_Real& Last) const;
  // 读取参数范围到输出参数 First/Last。

  Standard_Real First() const;
  // 返回起始参数。

  Standard_Real Last() const;
  // 返回结束参数。

  void First(const Standard_Real F);
  // 单独设置起始参数，并调用 Update()。

  void Last(const Standard_Real L);
  // 单独设置结束参数，并调用 Update()。

  //! Computes the point at parameter U.
  Standard_EXPORT virtual void D0(const Standard_Real U, gp_Pnt& P) const = 0;
  // 纯虚函数：子类必须实现。
  // 含义：给定曲线参数 U，计算曲线上的 3D 点 P（只算点，不算一阶/二阶导数）。

  //! Recomputes any derived data after a modification.
  //! This is called when the range is modified.
  Standard_EXPORT virtual void Update();
  // Update()：当范围改变时被调用。
  // 基类默认实现为空（见 .cxx），但某些子类会重写它去更新缓存信息。

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;
  // DumpJson：调试用，将对象内容输出为 JSON（不影响几何功能）。

  DEFINE_STANDARD_RTTIEXT(BRep_GCurve, BRep_CurveRepresentation)
  // RTTI：OCCT 运行时类型识别（DynamicType/IsKind）

protected:
  Standard_EXPORT BRep_GCurve(const TopLoc_Location& L,
                              const Standard_Real    First,
                              const Standard_Real    Last);
  // 受保护构造：只允许子类构造。
  // 输入：
  // - L：本曲线表示的 Location（坐标系变换）
  // - First/Last：参数范围

private:
  Standard_Real myFirst; // 起始参数
  Standard_Real myLast;  // 结束参数
};

#include <BRep_GCurve.lxx>

#endif // _BRep_GCurve_HeaderFile
