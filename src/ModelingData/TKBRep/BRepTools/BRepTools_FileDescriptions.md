# BRepTools 文件夹文件作用详解

本文档详细解释了 `src\ModelingData\TKBRep\BRepTools` 路径下每一份文件的作用。

### BRepTools.hxx / BRepTools.cxx
**BRepTools 核心工具包**
提供了一系列静态工具函数，用于处理 BRep 数据结构，包括计算面（Face）和边（Edge）的 UV 范围、更新拓扑结构中的缺失数据、比较顶点或边、寻找外轮廓线（Outer Wire）以及 BRep 对象的序列化（读写和 Dump）等基础操作。

### BRepTools_ShapeSet.hxx / BRepTools_ShapeSet.cxx
**形状集合管理工具**
用于管理一个形状（Shape）及其所有的子形状、位置信息和几何信息。它继承自 TopTools_ShapeSet，专门负责 BRep 格式的持久化，提供了将几何体写入流（Stream）或从流中读取的功能，是 OCCT 文件存取（.brep 格式）的核心类。

### BRepTools_ReShape.hxx / BRepTools_ReShape.cxx
**形状重构与替换工具**
允许用户对形状进行预定义的替换或删除操作。它分两个阶段工作：首先记录替换/删除请求，然后将这些请求批量应用到任意形状上，并支持操作历史记录（BRepTools_History），常用于复杂的拓扑修改。

### BRepTools_WireExplorer.hxx / BRepTools_WireExplorer.cxx
**轮廓线（Wire）有序遍历工具**
专门用于按照几何连接顺序（头尾相接）遍历轮廓线（Wire）中的边（Edge）。它可以处理闭合或非闭合的轮廓，并能根据给定的面（Face）参数空间来确定遍历方向，解决了普通遍历无法保证顺序的问题。

### BRepTools_Quilt.hxx / BRepTools_Quilt.cxx
**形状缝合（“被子”）工具**
用于将散乱的面（Face）通过共同的边（Edge）缝合起来，重新构建成外壳（Shell）。用户可以手动绑定共同边，该工具会自动处理边和顶点的替换、拷贝以及几何连续性，最终生成闭合或非闭合的壳体。

### BRepTools_Modification.hxx / BRepTools_Modification.cxx
**几何修改抽象基类**
定义了一个用于描述形状几何修改的抽象接口。通过派生此类，可以定义如何改变面、边和顶点的几何支撑（如修改曲面方程、曲线方程或顶点坐标），它是 BRepTools_Modifier 的核心驱动。

### BRepTools_Modifier.hxx / BRepTools_Modifier.cxx
**形状几何修改执行器**
根据给定的 BRepTools_Modification 描述，实际执行对形状的修改。它会递归地遍历形状，应用几何变换，并保持拓扑结构的完整性，生成一个新的修改后的形状。

### BRepTools_History.hxx / BRepTools_History.cxx
**操作历史追踪工具**
用于记录和管理形状在操作过程中的演变关系（生成、修改、删除）。它定义了输入形状与输出形状之间的映射，支持历史记录的合并（Merge），让用户能够追踪某个子形状是由哪些原始形状演变而来的。

### BRepTools_PurgeLocations.hxx / BRepTools_PurgeLocations.cxx
**位置信息清理工具**
用于移除形状及其子形状中不符合特定条件（如负缩放、非单位缩放等）的变换位置信息（Location）。它通过 BRepTools_ReShape 实现形状的重构，以净化数据结构。

### BRepTools_NurbsConvertModification.hxx / BRepTools_NurbsConvertModification.cxx
**NURBS 转换修改器**
一种具体的几何修改实现，用于将形状中的解析几何（如圆柱面、圆环等）转换为 NURBS（非均匀有理 B 样条）几何。它继承自 BRepTools_CopyModification，保留了原有的拓扑结构。

### BRepTools_TrsfModification.hxx / BRepTools_TrsfModification.cxx
**仿射变换修改器**
通过应用 gp_Trsf（如平移、旋转、缩放）来修改形状的几何。它是 BRepTools_Modification 的具体实现，能确保所有子形状的几何支撑都经过相应的坐标变换。

### BRepTools_GTrsfModification.hxx / BRepTools_GTrsfModification.cxx
**广义变换修改器**
类似于 TrsfModification，但使用更通用的 gp_GTrsf（支持非均匀缩放和剪切变换）。它负责处理那些标准仿射变换无法覆盖的复杂几何变形。

### BRepTools_Substitution.hxx / BRepTools_Substitution.cxx
**子形状批量替换工具**
提供了一种直接替换子形状的机制。用户可以指定某个旧形状被一组新形状替换，然后调用 Build 方法重建整个拓扑树，常用于局部的拓扑修补。

### BRepTools_CopyModification.hxx / BRepTools_CopyModification.cxx
**形状拷贝修改器**
实现了对几何和三角网格的深拷贝。它可以作为其他修改器的基类，确保在修改过程中不会影响原始形状的几何数据。

### BRepTools_DataMapIteratorOfMapOfVertexPnt2d.hxx / BRepTools_MapOfVertexPnt2d.hxx
**顶点与 2D 点映射工具**
定义了顶点（Vertex）到 2D 坐标点（Pnt2d）的映射容器及其迭代器，主要用于在处理面的参数空间时存储临时关联数据。

### BRepTools_Debug.cxx
**调试辅助工具**
包含了用于调试 BRepTools 内部状态的辅助函数，通常不直接在业务代码中使用。

### FILES.cmake
**项目构建配置文件**
CMake 构建系统文件，定义了 BRepTools 模块包含的源文件列表，用于自动化编译和链接过程。
