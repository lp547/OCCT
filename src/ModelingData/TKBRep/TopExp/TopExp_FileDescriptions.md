# TopExp 文件夹文件作用详解

本文档详细解释了 `src\ModelingData\TKBRep\TopExp` 路径下每一份文件的作用。

### TopExp.hxx / TopExp.cxx
**拓扑结构映射与遍历工具包**
提供了一系列静态工具函数，用于将复杂形状（Shape）分解为子形状并存储到映射表（Map）中。它支持查找特定类型的子形状（如获取一个实体中的所有面），以及建立形状与祖先（Ancestor）之间的反向索引关系（如查找一条边被哪些面共享），是拓扑分析的基础工具。

### TopExp_Explorer.hxx / TopExp_Explorer.cxx
**拓扑结构深度优先遍历器**
实现了一个灵活的迭代器，用于深度优先遍历（DFS）拓扑图。用户可以指定要查找的目标形状类型（ToFind）和需要避开的形状类型（ToAvoid），从而高效地提取所需的子形状（例如：在不进入 Compound 的情况下遍历所有 Solid）。

### TopExp_Stack.hxx
**遍历堆栈定义**
定义了一个简单的类型别名 `TopExp_Stack`，它是指向 `TopoDS_Iterator` 的指针。该堆栈结构被 `TopExp_Explorer` 用于在递归或迭代过程中维护当前的遍历状态，确保算法能够正确回溯。

### FILES.cmake
**项目构建配置文件**
CMake 构建系统文件，定义了 TopExp 模块包含的源文件列表，用于自动化编译和链接过程。
