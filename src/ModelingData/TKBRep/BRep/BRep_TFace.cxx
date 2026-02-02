// Created on: 1992-08-25
// Created by: Remi Lequette
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

#include <BRep_TFace.hxx>         // 本类声明
#include <Geom_Surface.hxx>       // Geom_Surface：几何曲面基类
#include <Poly_Triangulation.hxx> // Poly_Triangulation：三角网格数据结构
#include <Standard_Type.hxx>      // OCCT RTTI 支持
#include <TopLoc_Location.hxx>    // TopLoc_Location：位置变换
#include <TopoDS_Shape.hxx>       // TopoDS_TShape / Handle(TopoDS_TShape)

IMPLEMENT_STANDARD_RTTIEXT(BRep_TFace, TopoDS_TFace)
// IMPLEMENT_STANDARD_RTTIEXT：为 OCCT RTTI 生成必要代码（动态类型识别）

//=================================================================================================

BRep_TFace::BRep_TFace()
    : TopoDS_TFace(),
      myTolerance(RealEpsilon()),
      myNaturalRestriction(Standard_False)
{
  // 说明：
  // - mySurface / myActiveTriangulation 默认为空句柄（Null Handle）
  // - myLocation 使用默认构造（单位变换）
  // - myTolerance 使用 RealEpsilon() 作为非常小的默认公差
}

//=================================================================================================

Handle(TopoDS_TShape) BRep_TFace::EmptyCopy() const
{
  Handle(BRep_TFace) TF = new BRep_TFace(); // 创建同类型的新对象（空壳）
  TF->Surface(mySurface);                   // 复制曲面句柄（句柄共享同一曲面对象）
  TF->Location(myLocation);                 // 复制曲面位置变换
  TF->Tolerance(myTolerance);               // 复制面公差
  return TF;                                // 不复制三角网格（保持“EmptyCopy”的语义）
}

//=================================================================================================

const Handle(Poly_Triangulation)& BRep_TFace::Triangulation(const Poly_MeshPurpose thePurpose) const
{
  if (thePurpose == Poly_MeshPurpose_NONE)
  {
    return ActiveTriangulation(); // NONE：直接返回当前活动网格
  }
  for (Poly_ListOfTriangulation::Iterator anIter(myTriangulations); anIter.More(); anIter.Next())
  {
    const Handle(Poly_Triangulation)& aTriangulation = anIter.Value();
    if ((aTriangulation->MeshPurpose() & thePurpose) != 0)
    {
      return aTriangulation; // 找到第一个满足用途掩码的网格就返回
    }
  }
  if ((thePurpose & Poly_MeshPurpose_AnyFallback) != 0 && !myTriangulations.IsEmpty())
  {
    // if none matching other criteria was found return the first defined triangulation
    return myTriangulations.First(); // AnyFallback：没有匹配用途时退回到列表第一个
  }
  static const Handle(Poly_Triangulation) anEmptyTriangulation;
  return anEmptyTriangulation; // 没有合适网格：返回空句柄引用
}

//=================================================================================================

void BRep_TFace::Triangulation(const Handle(Poly_Triangulation)& theTriangulation,
                               const Standard_Boolean            theToReset)
{
  if (theToReset || theTriangulation.IsNull())
  {
    if (!myActiveTriangulation.IsNull())
    {
      // Reset Active bit
      myActiveTriangulation->SetMeshPurpose(myActiveTriangulation->MeshPurpose()
                                            & ~Poly_MeshPurpose_Active);
      myActiveTriangulation.Nullify();
      // 先清除旧活动网格的 Active 标志，再把活动句柄置空
    }
    myTriangulations.Clear();
    // 清空网格列表（重置模式）
    if (!theTriangulation.IsNull())
    {
      // Reset list of triangulations to new list with only one input triangulation that will be
      // active
      myTriangulations.Append(theTriangulation);
      myActiveTriangulation = theTriangulation;
      // Set Active bit
      theTriangulation->SetMeshPurpose(theTriangulation->MeshPurpose() | Poly_MeshPurpose_Active);
      // 把输入网格设为唯一网格并标记为 Active
    }
    return;
  }
  for (Poly_ListOfTriangulation::Iterator anIter(myTriangulations); anIter.More(); anIter.Next())
  {
    // Make input triangulation active if it is already contained in list of triangulations
    if (anIter.Value() == theTriangulation)
    {
      if (!myActiveTriangulation.IsNull())
      {
        // Reset Active bit
        myActiveTriangulation->SetMeshPurpose(myActiveTriangulation->MeshPurpose()
                                              & ~Poly_MeshPurpose_Active);
      }
      myActiveTriangulation = theTriangulation;
      // Set Active bit
      theTriangulation->SetMeshPurpose(theTriangulation->MeshPurpose() | Poly_MeshPurpose_Active);
      // 如果输入网格已在列表中：切换活动网格到它
      return;
    }
  }
  for (Poly_ListOfTriangulation::Iterator anIter(myTriangulations); anIter.More(); anIter.Next())
  {
    // Replace active triangulation to input one
    if (anIter.Value() == myActiveTriangulation)
    {
      // Reset Active bit
      myActiveTriangulation->SetMeshPurpose(myActiveTriangulation->MeshPurpose()
                                            & ~Poly_MeshPurpose_Active);
      anIter.ChangeValue()  = theTriangulation;
      myActiveTriangulation = theTriangulation;
      // Set Active bit
      theTriangulation->SetMeshPurpose(theTriangulation->MeshPurpose() | Poly_MeshPurpose_Active);
      // 如果输入网格不在列表中：用它替换当前活动网格，并设为 Active
      return;
    }
  }
}

//=================================================================================================

void BRep_TFace::Triangulations(const Poly_ListOfTriangulation&   theTriangulations,
                                const Handle(Poly_Triangulation)& theActiveTriangulation)
{
  if (theTriangulations.IsEmpty())
  {
    myActiveTriangulation.Nullify();
    myTriangulations.Clear();
    // 输入为空：清空内部列表并取消活动网格
    return;
  }
  Standard_Boolean anActiveInList = false;
  for (Poly_ListOfTriangulation::Iterator anIter(theTriangulations); anIter.More(); anIter.Next())
  {
    const Handle(Poly_Triangulation)& aTriangulation = anIter.Value();
    Standard_ASSERT_RAISE(!aTriangulation.IsNull(),
                          "Try to set list with NULL triangulation to the face");
    if (aTriangulation == theActiveTriangulation)
    {
      anActiveInList = true;
    }
    // Reset Active bit
    aTriangulation->SetMeshPurpose(aTriangulation->MeshPurpose() & ~Poly_MeshPurpose_Active);
    // 逐个网格清除 Active 位：确保内部状态由本类统一设置
  }
  Standard_ASSERT_RAISE(theActiveTriangulation.IsNull() || anActiveInList,
                        "Active triangulation isn't part of triangulations list");
  myTriangulations = theTriangulations;
  if (theActiveTriangulation.IsNull())
  {
    // Save the first one as active
    myActiveTriangulation = myTriangulations.First();
    // 未指定活动网格：默认取列表第一个
  }
  else
  {
    myActiveTriangulation = theActiveTriangulation;
    // 指定活动网格：直接使用
  }
  myActiveTriangulation->SetMeshPurpose(myActiveTriangulation->MeshPurpose()
                                        | Poly_MeshPurpose_Active);
  // 设置最终活动网格的 Active 位
}

//=================================================================================================

void BRep_TFace::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)
  // OCCT_DUMP_TRANSIENT_CLASS_BEGIN：输出对象类型等通用信息（调试用途）

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TopoDS_TFace)
  // OCCT_DUMP_BASE_CLASS：递归输出基类字段（如果 theDepth 允许）

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myActiveTriangulation.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, mySurface.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myLocation)
  // 说明：mySurface / myActiveTriangulation 是句柄，Dump 时传入底层指针；myLocation 传地址

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myTolerance)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myNaturalRestriction)
  // 输出数值字段：公差与自然边界标志

  for (Poly_ListOfTriangulation::Iterator anIter(myTriangulations); anIter.More(); anIter.Next())
  {
    const Handle(Poly_Triangulation)& aTriangulation = anIter.Value();
    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, aTriangulation.get())
    // 遍历输出所有网格（便于定位“存在多个用途网格”时的状态）
  }
}
