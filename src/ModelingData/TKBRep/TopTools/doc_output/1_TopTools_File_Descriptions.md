# TopTools 包文件描述

本文档详细解释了 Open CASCADE Technology (OCCT) 库中 `TopTools` 包内的每个文件。`TopTools` 包是 Modeling Data（建模数据）模块的基础组件，提供了用于管理边界表示（B-Rep）形状的基本数据结构和实用工具。

## 1. TopTools.cxx / TopTools.hxx
**包定义与全局工具**
这些文件定义了 `TopTools` 包本身。它们包含整个包中使用的全局函数和枚举。
- **TopTools.hxx**: 声明包类 `TopTools` 的头文件。它通常包含诸如 `Dump()` 之类的实用函数（用于调试形状），以及包中类的 typedef 或前置声明。
- **TopTools.cxx**: 包方法的实现文件。它通常包含 `Dump()` 函数的逻辑，该函数将形状的拓扑结构输出到流以供检查。

## 2. TopTools_ShapeSet.cxx / TopTools_ShapeSet.hxx
**形状序列化与持久化**
这些文件实现了 `TopTools_ShapeSet` 类，负责读取和写入 `TopoDS_Shape` 对象集。
- **TopTools_ShapeSet.hxx**: 声明 `ShapeSet` 类。它提供了 `Add()` 方法将形状添加到集合中，`Write()` 方法将其写入文件或流，以及 `Read()` 方法将其读回。它处理子形状（面、边、顶点）的递归存储，以确保在持久化过程中保留拓扑连接性。
- **TopTools_ShapeSet.cxx**: 实现序列化逻辑。它遍历形状，以紧凑的格式写入其几何信息（通过 `BRepTools`）和拓扑结构（TShapes）。

## 3. TopTools_LocationSet.cxx / TopTools_LocationSet.hxx
**位置序列化与持久化**
这些文件实现了 `TopTools_LocationSet` 类，负责管理 `TopLoc_Location` 对象的存储。
- **TopTools_LocationSet.hxx**: 声明 `LocationSet` 类。由于 OCCT 中的形状通常共享相同的底层几何体但位置（变换）不同，该类创建了一组唯一的位置集合，以避免在将形状写入文件时重复数据。
- **TopTools_LocationSet.cxx**: 实现收集唯一坐标参考系和变换，并将它们写入/读取到流的逻辑。

## 4. TopTools_MutexForShapeProvider.cxx / TopTools_MutexForShapeProvider.hxx
**线程安全机制**
这些文件为形状操作提供了同步原语。
- **TopTools_MutexForShapeProvider.hxx**: 声明专门为形状提供者设计的互斥锁机制。在多线程环境中，当多个算法可能尝试同时访问或修改共享的拓扑数据时，这至关重要。
- **TopTools_MutexForShapeProvider.cxx**: 实现互斥锁的锁定和解锁逻辑。

## 5. 集合实例化 (列表、映射、数组)
`TopTools` 包的大部分内容由 OCCT 泛型集合类（来自 `NCollection` 或 `TCollection`）的实例化组成，专门用于 `TopoDS_Shape` 及其相关类型。这些提供了建模算法广泛使用的类型安全容器。

### Lists (链表)
用于顺序很重要且不需要随机访问的动态集合。
- **TopTools_ListOfShape.hxx**: 定义 `TopTools_ListOfShape`，即 `TopoDS_Shape` 对象的列表。普遍用于返回面、边等的列表。
- **TopTools_ListOfListOfShape.hxx**: 定义形状列表的列表。用于复杂的数据结构，如将形状分组为壳（Shells）或实体（Solids）。
- **TopTools_ListIteratorOfListOfShape.hxx**: 定义遍历 `TopTools_ListOfShape` 的迭代器。

### Maps (哈希集合)
用于快速查找以检查形状是否存在于集合中。
- **TopTools_MapOfShape.hxx**: `TopoDS_Shape` 对象的集合。使用哈希实现 O(1) 的平均查找时间。
- **TopTools_MapOfOrientedShape.hxx**: 与 `MapOfShape` 类似，但区分具有不同方向（FORWARD 与 REVERSED）的形状。
- **TopTools_MapIteratorOfMapOfShape.hxx**: `TopTools_MapOfShape` 的迭代器。
- **TopTools_MapIteratorOfMapOfOrientedShape.hxx**: `TopTools_MapOfOrientedShape` 的迭代器。

### Indexed Maps (索引映射 - 哈希集合+数组)
当你既需要快速查找（像 Map）又需要通过索引访问（像 Array）时使用。对于需要将形状映射到整数（索引）的算法至关重要。
- **TopTools_IndexedMapOfShape.hxx**: 存储唯一的形状并为每个形状分配一个索引（1 到 N）。
- **TopTools_IndexedMapOfOrientedShape.hxx**: 同上，但通过方向区分形状。

### Data Maps (数据映射 - 哈希表/字典)
用于将数据（值）与形状（键）关联。
- **TopTools_DataMapOfShapeInteger.hxx**: 将 `TopoDS_Shape` 映射到 `Standard_Integer`。用于计数、索引或标记形状。
- **TopTools_DataMapOfShapeReal.hxx**: 将 `TopoDS_Shape` 映射到 `Standard_Real` (浮点数)。用于为形状分配公差、面积或其他物理属性。
- **TopTools_DataMapOfShapeShape.hxx**: 将 `TopoDS_Shape` 映射到另一个 `TopoDS_Shape`。在建模历史记录中极其常见（例如，将生成的面映射回其生成边）。
- **TopTools_DataMapOfShapeListOfShape.hxx**: 将 `TopoDS_Shape` 映射到形状列表。用于邻接图（例如，边 -> 共享它的面的列表）。
- **TopTools_DataMapOfShapeListOfInteger.hxx**: 将形状映射到整数列表。
- **TopTools_DataMapOfShapeBox.hxx**: 将形状映射到其包围盒 (`Bnd_Box`)。用于缓存碰撞检测数据。
- **TopTools_DataMapOfShapeSequenceOfShape.hxx**: 将形状映射到形状序列。
- **TopTools_DataMapOfIntegerShape.hxx**: 反向映射：整数 -> 形状。
- **TopTools_DataMapOfIntegerListOfShape.hxx**: 整数 -> 形状列表。
- **TopTools_DataMapOfOrientedShapeShape.hxx**: 将定向形状映射到形状。
- **TopTools_DataMapOfOrientedShapeInteger.hxx**: 将定向形状映射到整数。

### Indexed Data Maps (索引数据映射)
结合了索引映射和数据映射的特性。
- **TopTools_IndexedDataMapOfShapeShape.hxx**: 映射 Shape -> Shape，但也允许按索引迭代。
- **TopTools_IndexedDataMapOfShapeListOfShape.hxx**: 映射 Shape -> 形状列表，可通过索引访问。
- **TopTools_IndexedDataMapOfShapeAddress.hxx**: 映射 Shape -> Address (指针)。用于低级关联。
- **TopTools_IndexedDataMapOfShapeReal.hxx**: 映射 Shape -> Real，可通过索引访问。

### Arrays (数组)
固定大小的集合。
- **TopTools_Array1OfShape.hxx**: 形状的一维数组（索引通常为 1 到 N）。
- **TopTools_Array2OfShape.hxx**: 形状的二维数组（矩阵）。
- **TopTools_HArray1OfShape.hxx**: `Array1OfShape` 的 Handle（智能指针）版本。当数组需要共享或通过引用计数传递时使用。
- **TopTools_HArray2OfShape.hxx**: `Array2OfShape` 的 Handle 版本。
- **TopTools_Array1OfListOfShape.hxx**: 数组，其中每个元素都是形状列表。
- **TopTools_HArray1OfListOfShape.hxx**: 上述内容的 Handle 版本。

### Sequences (序列)
具有索引访问权限的有序集合（类似 C++ 的 `std::vector` 或 `std::deque`）。
- **TopTools_SequenceOfShape.hxx**: `TopoDS_Shape` 的序列。
- **TopTools_HSequenceOfShape.hxx**: `SequenceOfShape` 的 Handle 版本。

## 6. Iterators (迭代器)
这些文件定义了上述特定映射类型的迭代器。它们允许遍历集合。
- **TopTools_DataMapIteratorOfDataMapOfShapeInteger.hxx**
- **TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx**
- **TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx**
- ... (以此类推，对应每种 DataMap 类型)。

## 7. Hashing Utilities (哈希工具)
- **TopTools_ShapeMapHasher.hxx**: 定义如何计算 `TopoDS_Shape` 的哈希码以及如何检查相等性。它通常对底层的 `TopoDS_TShape` 指针和 `TopLoc_Location` 进行哈希处理。
- **TopTools_FormatVersion.hxx**: 定义 `ShapeSet` 使用的文件格式的版本常量，确保向后兼容性。

## 8. LocationSet Pointer (位置集指针)
- **TopTools_LocationSetPtr.hxx**: 定义 `LocationSet` 的指针类型，可能用于内部使用或前置声明。

## 9. FILES.cmake
**构建配置**
- **FILES.cmake**: 一个 CMake 配置文件，列出了该包中的所有源文件。构建系统使用它来了解哪些文件需要编译并链接到 `TKBRep` 库中。
