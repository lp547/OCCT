# BRepAdaptor 模块文件功能详解

本文件详细解释了 `src/ModelingData/TKBRep/BRepAdaptor` 目录下各个文件的作用。

## 1. BRepAdaptor_Curve.hxx / BRepAdaptor_Curve.cxx
**功能描述**：
该文件定义并实现了 `BRepAdaptor_Curve` 类。这是一个核心适配器类，用于将 BRep 拓扑结构中的 **Edge (边)** 适配为几何库可以通用的 3D 曲线接口 (`Adaptor3d_Curve`)。它的主要作用是让那些只接受标准几何曲线接口的算法（如求交、投影等）能够直接处理 BRep 的边，而无需关心边底层的具体实现（如是否被剪裁、是否有坐标变换、方向是否翻转）。它内部处理了边的几何变换（Trsf）和方向（Orientation）。

## 2. BRepAdaptor_Surface.hxx / BRepAdaptor_Surface.cxx
**功能描述**：
该文件定义并实现了 `BRepAdaptor_Surface` 类。类似于曲线适配器，它是将 BRep 拓扑结构中的 **Face (面)** 适配为 3D 曲面接口 (`Adaptor3d_Surface`) 的工具。它允许通用曲面算法直接作用于拓扑面。它考虑了面的定位（Location）、参数范围限制（Restriction）以及底层的几何曲面定义。

## 3. BRepAdaptor_CompCurve.hxx / BRepAdaptor_CompCurve.cxx
**功能描述**：
该文件定义并实现了 `BRepAdaptor_CompCurve` 类。这是一个复合曲线适配器，它将一个 **Wire (线框)** —— 即由多条边首尾相连组成的路径 —— 视为一条单一的连续 3D 曲线。这对于需要处理长路径的算法非常有用，它将底层的多段 Edge 抽象为一段参数连续（可能是分段的）的曲线。

## 4. BRepAdaptor_Curve2d.hxx / BRepAdaptor_Curve2d.cxx
**功能描述**：
该文件定义并实现了 `BRepAdaptor_Curve2d` 类。它用于访问 Edge 在 Face 上的 **参数空间曲线 (PCurve)**。在 BRep 中，边不仅有 3D 形式，在面上还有 2D 参数形式（u, v 坐标）。该类将这种 2D 曲线适配为 `Geom2dAdaptor_Curve`，使得 2D 几何算法可以在拓扑边的参数空间中运行。

## 5. BRepAdaptor_Array1OfCurve.hxx
**功能描述**：
定义了 `BRepAdaptor_Curve` 对象的一维数组容器。这是 OCCT `NCollection` 模板库的实例化，用于存储一组曲线适配器，通常用于批量处理或作为算法的输入数据结构。

## 6. BRepAdaptor_HArray1OfCurve.hxx
**功能描述**：
定义了 `BRepAdaptor_Array1OfCurve` 的句柄（Handle）类。在 OCCT 中，Handle 是一种智能指针机制，用于对象的引用计数和内存管理。这个文件允许数组对象在堆上创建并在不同模块间安全传递。

## 7. FILES.cmake
**功能描述**：
这是 CMake 构建系统的配置文件，列出了该模块中包含的所有源文件和头文件，用于指导编译器如何编译和链接 BRepAdaptor 模块。
