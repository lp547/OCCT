# BinTools 模块文件详细说明

## BinTools.cxx / BinTools.hxx
**二进制工具核心类**
这是 BinTools 模块的入口和核心工具类。它提供了一系列静态方法，用于将基本数据类型（如 Real, Integer, Boolean, ExtCharacter 等）以二进制格式写入流（OStream）或从流（IStream）中读取。此外，它还提供了高层的 `Write` 静态函数，允许用户将 `TopoDS_Shape`（拓扑形状）以指定的格式版本直接写入到输出流中。它是整个模块对外提供二进制读写能力的主要接口。

## BinTools_ShapeSet.cxx / BinTools_ShapeSet.hxx
**形状集合管理类（经典模式）**
该类用于管理和读写一组拓扑形状（Shapes）。它是 `BinTools_ShapeSetBase` 的子类，采用一种“分段”的策略来存储数据：首先将形状中用到的所有几何对象（曲线、曲面）、位置变换（Locations）等分别收集到对应的子集合（SurfaceSet, CurveSet 等）中，然后按照特定的顺序（Locations -> Geometry -> Shapes）将它们写入文件。这种设计确保了文件中相同的几何对象只被存储一次（通过索引引用），从而优化了存储空间。

## BinTools_ShapeReader.cxx / BinTools_ShapeReader.hxx
**流式形状读取器（新版）**
这是一个较新的类（2021年引入），用于从二进制流中读取拓扑形状。与 `BinTools_ShapeSet` 不同，它设计用于处理没有按类型分组的对象流，支持通过文件中的相对位置作为引用来读取对象。它利用 `BinTools_IStream` 进行底层数据读取，能够解析包括几何体、位置、多边形等在内的各种复杂数据结构，适合处理更灵活或流式的二进制数据格式。

## BinTools_ShapeWriter.cxx / BinTools_ShapeWriter.hxx
**流式形状写入器（新版）**
与 `BinTools_ShapeReader` 对应，该类用于将拓扑形状以流式二进制格式写入输出流。它不需要预先收集所有对象并分组，而是可以在遍历形状时直接写入对象，并利用内部的映射表（Maps）来管理对象的引用（如果对象已写入，则只写入引用ID）。这种方式对于某些流式传输或不需要预处理整个模型的场景更加高效。

## BinTools_ShapeSetBase.cxx / BinTools_ShapeSetBase.hxx
**形状集合基类**
这是 `BinTools_ShapeSet`、`BinTools_ShapeReader` 和 `BinTools_ShapeWriter` 的共同基类。它定义了读写过程中的一些通用配置和状态，例如是否保存三角网格（Triangulations）、是否保存法线（Normals）以及使用的格式版本（FormatVersion）。它为派生类提供了统一的接口规范和基础数据成员。

## BinTools_FormatVersion.hxx
**格式版本定义**
这是一个枚举定义文件，列出了 BinTools 支持的所有二进制格式版本。从 VERSION_1 到 VERSION_4（当前版本），每个版本通常引入了对新数据特性的支持（例如支持 CurveOnSurface 的 UV 点，或者支持仅三角网格面的法线信息）。这使得读写工具能够向后兼容旧版本的数据文件。

## BinTools_Curve2dSet.cxx / BinTools_Curve2dSet.hxx
**2D曲线集合工具**
该类专门用于管理一组 2D 曲线（Geom2d_Curve）。它通常被 `BinTools_ShapeSet` 使用，用于收集模型中所有的 2D 曲线，去重后建立索引。在写入文件时，它会将这些曲线统一写入；在读取时，它会重建这些曲线并提供通过索引访问的接口。

## BinTools_CurveSet.cxx / BinTools_CurveSet.hxx
**3D曲线集合工具**
类似于 `BinTools_Curve2dSet`，但该类用于管理 3D 曲线（Geom_Curve）。它负责将 3D 曲线对象序列化到二进制流中，并维护曲线与索引之间的映射关系，确保文件结构的紧凑性。

## BinTools_SurfaceSet.cxx / BinTools_SurfaceSet.hxx
**曲面集合工具**
该类用于管理一组几何曲面（Geom_Surface）。在保存复杂的 BRep 模型时，曲面数据往往占用较大空间。该类通过建立索引表，确保同一曲面在文件中只存储一次，并负责曲面参数的二进制序列化和反序列化。

## BinTools_LocationSet.cxx / BinTools_LocationSet.hxx
**位置变换集合工具**
拓扑形状通常包含位置变换（Location），例如平移、旋转等。该类用于收集和管理这些位置对象（TopLoc_Location）。由于位置变换在模型装配结构中会被大量复用，该类通过去重和索引机制，极大地减少了文件体积。

## BinTools_IStream.cxx / BinTools_IStream.hxx
**二进制输入流封装**
这是对标准 `Standard_IStream` 的一层封装，专门为 `BinTools_ShapeReader` 优化。它提供了读取特定对象类型（BinTools_ObjectType）、引用、以及基础几何数据（如 gp_Pnt）的方法。它还支持在流中进行定位（GoTo）和状态查询，是新版流式读取的核心底层工具。

## BinTools_OStream.cxx / BinTools_OStream.hxx
**二进制输出流封装**
这是对标准 `Standard_OStream` 的一层封装，专门为 `BinTools_ShapeWriter` 优化。它提供了写入对象标识符、引用、变换矩阵、三角形数据等底层二进制数据的方法。它通过缓冲区优化了部分数据的写入性能。

## BinTools_ObjectType.hxx
**对象类型枚举**
该文件定义了二进制流中各种对象的标识符（ID），例如 `BinTools_ObjectType_Curve`、`BinTools_ObjectType_Location` 等。这些标识符在 `BinTools_ShapeReader` 和 `BinTools_ShapeWriter` 中使用，用于在二进制流中标记接下来要存储或读取的数据块类型。

## FILES.cmake
**构建配置文件**
这是 CMake 构建系统的配置文件，列出了 BinTools 模块包含的所有源文件。它告诉构建系统在编译该模块时需要处理哪些 .cxx 和 .hxx 文件。
