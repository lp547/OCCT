# TopExp 详细设计文档

本文档深入解析 `TopExp` 模块中关键组件的算法流程、数据结构设计以及函数接口说明。

## 1. 深度优先遍历器 (TopExp_Explorer)

### 1.1 算法流程设计
`TopExp_Explorer` 使用非递归的方式实现深度优先搜索（DFS），避免了在深层拓扑结构中可能导致的栈溢出问题。它维护一个显式的堆栈（Stack）。

```mermaid
graph TD
    %% --- 样式定义区 ---
    %% 1. 基础流程节点 (蓝色)
    classDef process fill:#e1f5fe,stroke:#01579b,stroke-width:2px,rx:5,ry:5;
    %% 2. 判断节点 (黄色)
    classDef decision fill:#fff9c4,stroke:#fbc02d,stroke-width:2px,stroke-dasharray: 5 5;
    %% 3. 堆栈/数据操作 (紫色)
    classDef stack fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px;
    %% 4. 开始/结束 (绿色/红色)
    classDef start fill:#e8f5e9,stroke:#2e7d32,stroke-width:3px;
    classDef endNode fill:#ffebee,stroke:#c62828,stroke-width:3px;

    %% --- 流程图主体 ---
    
    %% 开始节点：使用圆角括号
    Start(["🚀 Init(Shape, ToFind, ToAvoid)"]):::start 
    
    %% 堆栈操作：使用圆柱体表示数据存储
    Start --> PushRoot[("📥 将 Shape 迭代器压栈")]:::stack
    
    %% 逻辑连接
    PushRoot --> CheckStack{{"堆栈为空?"}}:::decision
    
    %% 分支逻辑
    CheckStack -- Yes --> Finish(["🏁 遍历结束 More=False"]):::endNode
    CheckStack -- No --> Peek[/"👀 获取栈顶迭代器"/]:::stack
    
    Peek --> HasMore{{"迭代器有更多元素?"}}:::decision
    
    %% 循环逻辑：弹出
    HasMore -- No --> Pop[("📤 弹出栈顶迭代器")]:::stack
    Pop --> CheckStack
    
    %% 循环逻辑：处理元素
    HasMore -- Yes --> GetCurrent["📦 获取当前子形状 SubShape"]:::process
    
    %% 类型判断
    GetCurrent --> IsAvoid{{"🚫 类型 == ToAvoid?"}}:::decision
    
    IsAvoid -- Yes --> NextIter["⏭️ 迭代器步进 Next"]:::process
    NextIter --> CheckStack
    
    IsAvoid -- No --> IsFind{{"✅ 类型 == ToFind?"}}:::decision
    
    %% 找到目标
    IsFind -- Yes --> SetCurrent["🎯 设置当前结果"]:::process
    SetCurrent --> ProcessChildren
    
    %% 未找到目标，继续下探
    IsFind -- No --> ProcessChildren["⚙️ 准备处理子节点"]:::process
    
    %% 压栈新层级
    ProcessChildren --> PushChild[("📥 为 SubShape 创建新迭代器并压栈")]:::stack
    PushChild --> Return(["✨ 返回 More=True"]):::start
```

### 1.2 数据结构
*   **`myStack`**: `TopExp_Stack` (即 `TopoDS_Iterator*` 的链表或数组)。
    *   存储当前遍历路径上每一层的迭代器。
*   **`myShape`**: `TopoDS_Shape`
    *   当前找到的符合条件的形状。
*   **`toFind`**: `TopAbs_ShapeEnum`
    *   目标形状类型。
*   **`toAvoid`**: `TopAbs_ShapeEnum`
    *   避开形状类型。

### 1.3 接口说明

#### `Init`
*   **功能**: 初始化遍历器。
*   **输入参数**:
    *   `S`: `TopoDS_Shape` - 根形状。
    *   `ToFind`: `TopAbs_ShapeEnum` - 要查找的子形状类型。
    *   `ToAvoid`: `TopAbs_ShapeEnum` (默认 `SHAPE`) - 遇到此类型停止向下递归。

#### `More`
*   **功能**: 检查是否还有更多符合条件的形状。
*   **返回值**: `Standard_Boolean`。

#### `Next`
*   **功能**: 继续搜索下一个形状。
*   **逻辑**: 驱动内部堆栈进行下一步 DFS 搜索，直到找到下一个匹配项或堆栈清空。

#### `Current`
*   **功能**: 获取当前找到的形状。
*   **返回值**: `const TopoDS_Shape&`。

---

## 2. 拓扑映射工具 (TopExp Package Methods)

### 2.1 MapShapes (扁平化映射)

#### 算法逻辑
递归遍历给定的形状 `S`。对于访问到的每一个子形状，将其添加到一个 `IndexedMap` 中。`IndexedMap` 会自动处理去重（通过哈希值）。

#### 接口说明
*   **功能**: 将形状 `S` 中的所有子形状（或指定类型的子形状）存储到 `M` 中。
*   **输入参数**:
    *   `S`: `TopoDS_Shape` - 待处理形状。
    *   `T`: `TopAbs_ShapeEnum` (可选) - 仅映射此类型的子形状。
*   **输出参数**:
    *   `M`: `TopTools_IndexedMapOfShape&` - 结果容器。索引从 1 开始。

### 2.2 MapShapesAndAncestors (祖先映射)

#### 算法逻辑
这是一个两层遍历算法：
1.  首先调用 `MapShapes` 收集所有类型为 `TS` (SubShape Type) 的形状，建立索引。
2.  然后使用 `TopExp_Explorer` 遍历 `S` 查找类型为 `TA` (Ancestor Type) 的形状（例如 Face）。
3.  对于找到的每个 Ancestor `A`，再使用 `TopExp_Explorer` 遍历其内部类型为 `TS` 的子形状 `s`。
4.  在 Map 中查找 `s`，将 `A` 添加到 `s` 对应的 Ancestor 列表中。

```mermaid
graph TD
    %% 样式定义
    classDef step fill:#e1f5fe,stroke:#01579b,stroke-width:2px;
    classDef loop fill:#fff9c4,stroke:#fbc02d,stroke-width:2px,stroke-dasharray: 5 5;
    classDef action fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px;
    classDef storage fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px;

    Start([开始: MapShapesAndAncestors]) --> Step1
    
    Step1["Step 1: 预处理\n收集所有的 TS (子形状)"]:::step
    Step1 --> Index[("🗂️ 建立索引表 Map\n(此时只有Key, Value为空)")]:::storage
    
    Index --> Step2{{"Step 2: 外层循环\n遍历所有 TA (父形状)"}}:::loop
    
    Step2 -- 拿到一个父形状 A --> Step3{{"Step 3: 内层循环\n查看 A 里面有哪些 TS"}}:::loop
    
    Step3 -- 发现子形状 s --> Match["Step 4: 查表与关联"]:::action
    
    Match -- "在 Map 中找到 s" --> Link["📝 记录:\n s 的家长列表 += A"]:::storage
    
    Link --> Step3
    Step3 -- A 内部找完了 --> Step2
    Step2 -- 所有父形状找完了 --> Finish([完成: 返回 Map])
```



#### 接口说明

*   **功能**: 建立从“子形状”到“祖先形状”的映射。例如，查找每条边被哪些面使用。
*   **输入参数**:
    *   `S`: `TopoDS_Shape` - 范围形状。
    *   `TS`: `TopAbs_ShapeEnum` - 子形状类型（Key）。
    *   `TA`: `TopAbs_ShapeEnum` - 祖先形状类型（Value List 中的元素）。
*   **输出参数**:
    *   `M`: `TopTools_IndexedDataMapOfShapeListOfShape&` - 结果容器。Key 是 TS 类型的形状，Value 是 TA 类型的形状列表。

### 2.3 MapShapesAndUniqueAncestors

#### 业务差异
与 `MapShapesAndAncestors` 类似，但强调**唯一性**。如果一个 Ancestor `A` 多次包含同一个子形状 `s`（例如一条缝合边在同一个面中出现两次），在普通 Ancestors Map 中 `A` 会出现两次。而在 UniqueAncestors Map 中，`A` 只会被记录一次。

#### 接口说明
*   **功能**: 建立子形状到唯一祖先形状的映射。
*   **输入/输出参数**: 同上。
*   **逻辑**: 在添加 `A` 到列表前，检查列表末尾是否已经等于 `A`，或者使用 Set 进行去重。

```mermaid
graph TD
    %% 样式定义
    classDef process fill:#e1f5fe,stroke:#01579b,stroke-width:2px;
    classDef decision fill:#fff9c4,stroke:#fbc02d,stroke-width:2px,stroke-dasharray: 5 5;
    classDef storage fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px;
    classDef start fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px;

    Start([开始: UniqueAncestors]) --> Step1["Step 1: 预处理 Map 索引"]
    Step1 --> LoopTA{{"遍历父形状 TA (如 Face)"}}:::decision
    
    LoopTA -- 拿到父形状 A --> LoopTS{{"遍历其子形状 TS (如 Edge)"}}:::decision
    
    LoopTS -- 找到子形状 s --> Check{{"去重检查❓\nList(s) 的最后一个元素 == A ?"}}:::decision
    
    Check -- Yes (重复了) --> Skip["🚫 跳过 (不记录)"]:::process
    Skip --> LoopTS
    
    Check -- No (新的) --> Record["📝 记录: List(s).Append(A)"]:::storage
    Record --> LoopTS
    
    LoopTS -- 找完了 --> LoopTA
    LoopTA -- 找完了 --> Finish([完成])
```
