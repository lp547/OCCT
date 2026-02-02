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

#ifndef _BRep_TFace_HeaderFile
#define _BRep_TFace_HeaderFile // 头文件保护：防止重复包含

#include <Standard.hxx> // OCCT 基础定义：导出宏、基础类型等

#include <Poly_ListOfTriangulation.hxx> // 面的三角网格列表容器
#include <TopLoc_Location.hxx>          // TopLoc_Location：几何位置变换（平移/旋转等）
#include <Standard_Real.hxx>            // Standard_Real：OCCT 统一的实数类型
#include <TopoDS_TFace.hxx>             // TopoDS_TFace：拓扑面的基础 TShape 实现
class Geom_Surface;                                                                                                    
class TopoDS_TShape;
// 前置声明：减少头文件耦合，加快编译

class BRep_TFace;
DEFINE_STANDARD_HANDLE(BRep_TFace, TopoDS_TFace)
// DEFINE_STANDARD_HANDLE：为 BRep_TFace 声明 Handle(BRep_TFace) 智能句柄类型

//! The Tface from BRep  is  based  on the TFace  from
//! TopoDS. The TFace contains :
//!
//! * A surface, a tolerance and a Location.
//!   (一个曲面，一个公差和一个位置。)
//!
//! * A NaturalRestriction flag,   when this  flag  is
//! True the  boundary of the  face is known to be the
//! parametric space (Umin, UMax, VMin, VMax).
//! (自然边界标志。如果为真，表示该面的边界就是其底层曲面的参数范围边界。)
//!
//! * An optional list of triangulations. If there are any
//! triangulations the surface can be absent.
//! (可选的三角网格列表。如果存在三角网格，曲面对象可以是空的（例如仅用于显示的网格数据）。)
//!
//! The  Location is  used   for the Surface.
//! (Location 字段仅作用于 Surface。)
//!
//! The triangulation  is in the same reference system
//! than the TFace.     A point on mySurface must   be
//! transformed with myLocation,  but  not a point  on
//! the triangulation.
//! (重要：三角网格的坐标已经包含了变换，或者是定义在最终坐标系下的，不需要再应用 Location。
//!  而 mySurface 是原始定义的曲面，使用时必须应用 myLocation 进行变换。)
//!
//! The Surface may  be shared by different TFaces but
//! not the  Triangulation, because the  Triangulation
//! may be modified by  the edges.
//! (曲面对象可以被多个 TFace 共享，但三角网格通常不共享。)
class BRep_TFace : public TopoDS_TFace
{

public:
  //! Creates an empty TFace.
  Standard_EXPORT BRep_TFace(); // 构造：初始化默认公差与 NaturalRestriction 标志

  //! Returns face surface.
  //! @return 几何曲面句柄
  const Handle(Geom_Surface)& Surface() const { return mySurface; } // 获取底层曲面（可能为空）

  //! Sets surface for this face.
  //! @param theSurface 新的几何曲面
  void Surface(const Handle(Geom_Surface)& theSurface) { mySurface = theSurface; } // 设置底层曲面

  //! Returns the face location.
  //! @return 位置变换
  const TopLoc_Location& Location() const { return myLocation; } // 获取曲面位置变换（仅作用于曲面）

  //! Sets the location for this face.
  //! @param theLocation 新的位置变换
  void Location(const TopLoc_Location& theLocation) { myLocation = theLocation; } // 设置曲面位置变换

  //! Returns the face tolerance.
  //! @return 公差值
  Standard_Real Tolerance() const { return myTolerance; } // 获取面公差

  //! Sets the tolerance for this face.
  //! @param theTolerance 新的公差值
  void Tolerance(const Standard_Real theTolerance) { myTolerance = theTolerance; } // 设置面公差

  //! Returns TRUE if the boundary of this face is known to be the parametric space (Umin, UMax,
  //! VMin, VMax).
  Standard_Boolean NaturalRestriction() const { return myNaturalRestriction; } // 是否自然边界（边界等于参数域边界）

  //! Sets the flag that is TRUE if the boundary of this face is known to be the parametric space.
  void NaturalRestriction(const Standard_Boolean theRestriction)
  {
    myNaturalRestriction = theRestriction; // 保存自然边界标志
  }

  //! Returns the triangulation of this face according to the mesh purpose.
  //! @param[in] thePurpose a mesh purpose to find appropriate triangulation (NONE by default).
  //! @return an active triangulation in case of NONE purpose,
  //!         the first triangulation appropriate for the input purpose,
  //!         just the first triangulation if none matching other criteria and input purpose is
  //!         AnyFallback or null handle if there is no any suitable triangulation.
  Standard_EXPORT const Handle(Poly_Triangulation)& Triangulation(
    const Poly_MeshPurpose thePurpose = Poly_MeshPurpose_NONE) const;

  //! Sets input triangulation for this face.
  //! @param[in] theTriangulation  triangulation to be set
  //! @param[in] theToReset  flag to reset triangulations list to new list with only one input
  //! triangulation. If theTriangulation is NULL internal list of triangulations will be cleared and
  //! active triangulation will be nullified. If theToReset is TRUE internal list of triangulations
  //! will be reset to new list with only one input triangulation that will be active. Else if input
  //! triangulation is contained in internal triangulations list it will be made active,
  //!      else the active triangulation will be replaced to input one.
  Standard_EXPORT void Triangulation(const Handle(Poly_Triangulation)& theTriangulation,
                                     const Standard_Boolean            theToReset = true);

  //! Returns a copy  of the  TShape  with no sub-shapes.
  //! The new Face has no triangulation.
  Standard_EXPORT virtual Handle(TopoDS_TShape) EmptyCopy() const Standard_OVERRIDE;
  // EmptyCopy：复制一个“空壳”面，仅复制基础字段，不复制子形状/三角网格

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;
  // DumpJson：用于调试/诊断，把对象内容以 JSON 形式写到输出流

public:
  //! Returns the list of available face triangulations.
  const Poly_ListOfTriangulation& Triangulations() const { return myTriangulations; } // 获取三角网格列表（只读）

  //! Sets input list of triangulations and currently active triangulation for this face.
  //! If list is empty internal list of triangulations will be cleared and active triangulation will
  //! be nullified. Else this list will be saved and the input active triangulation be saved as
  //! active. Use NULL active triangulation to set the first triangulation in list as active. Note:
  //! the method throws exception if there is any NULL triangulation in input list or
  //!       if this list doesn't contain input active triangulation.
  Standard_EXPORT void Triangulations(const Poly_ListOfTriangulation&   theTriangulations,
                                      const Handle(Poly_Triangulation)& theActiveTriangulation);

  //! Returns number of available face triangulations.
  Standard_Integer NbTriangulations() const { return myTriangulations.Size(); } // 返回三角网格数量

  //! Returns current active triangulation.
  const Handle(Poly_Triangulation)& ActiveTriangulation() const { return myActiveTriangulation; } // 返回当前活动网格

  DEFINE_STANDARD_RTTIEXT(BRep_TFace, TopoDS_TFace)
  // RTTI：OCCT 运行时类型识别（IsKind / DynamicType 等）

private:
  Poly_ListOfTriangulation   myTriangulations;      // 所有三角网格（可能用于不同用途）
  Handle(Poly_Triangulation) myActiveTriangulation; // 当前活动三角网格（Purpose=NONE 时返回它）
  Handle(Geom_Surface)       mySurface;             // 底层几何曲面（可共享；也可能为空）
  TopLoc_Location            myLocation;            // 曲面位置变换（仅对曲面生效，不作用于网格）
  Standard_Real              myTolerance;           // 面公差（几何误差允许范围）
  Standard_Boolean           myNaturalRestriction;  // 自然边界标志（边界等于参数域矩形边界）
};

#endif // _BRep_TFace_HeaderFile // 头文件保护结束
