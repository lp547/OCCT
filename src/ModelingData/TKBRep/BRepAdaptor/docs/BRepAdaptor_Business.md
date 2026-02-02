# BRepAdaptor 业务功能与架构设计

本文件详细阐述了 `BRepAdaptor` 模块在 Open CASCADE Technology (OCCT) 中的业务价值、功能定位及高层架构设计。

## 1. 业务功能 (Business Function)

### 1.1 背景
在 CAD/CAM 软件开发中，存在两个核心概念域：
*   **几何域 (Geometry)**：纯数学定义的形状，如直线、圆、贝塞尔曲线、B样条曲面等。它们不包含“边界”或“连接”的概念。
*   **拓扑域 (Topology)**：描述物体边界关系的数据结构，如点 (Vertex)、边 (Edge)、面 (Face)、体 (Solid)。拓扑对象引用几何对象，并添加了方向 (Orientation) 和位置 (Location) 等属性。

### 1.2 核心痛点
许多通用几何算法（如计算曲线长度、求两曲面交线、计算点到曲面距离）是基于纯数学定义的。如果直接针对拓扑对象（如 `TopoDS_Edge`）编写这些算法，需要处理极其繁琐的细节：
*   **坐标变换**：边可能位于一个被移动或旋转过的组件中。
*   **方向翻转**：边的逻辑方向可能与底层几何曲线的参数方向相反。
*   **参数剪裁**：边通常只使用几何曲线的一小段（Trimmed Curve）。

### 1.3 解决方案
`BRepAdaptor` 充当了 **“翻译官”** 或 **“适配器”** 的角色。它的业务功能是：
*   **统一接口**：为上层算法提供统一的 `Adaptor3d_Curve` 和 `Adaptor3d_Surface` 接口，屏蔽底层是“原生几何”还是“拓扑边界”的差异。
*   **自动处理上下文**：自动应用拓扑结构中的坐标变换和方向标志，调用者看到的曲线就是“世界坐标系”下、“逻辑方向”正确的曲线。
*   **简化开发**：算法开发者只需编写一次代码，即可同时支持纯几何对象和复杂的 BRep 拓扑对象。

---

## 2. 架构设计 (Architecture Design)

### 2.1 设计模式
该模块主要采用了 **适配器模式 (Adapter Pattern)**，具体为对象适配器。

*   **Target (目标接口)**：`Adaptor3d_Curve`, `Adaptor3d_Surface` (定义了求值 `Value(u)`、导数 `D1(u)` 等标准几何操作)。
*   **Adaptee (被适配者)**：`TopoDS_Edge`, `TopoDS_Face` (包含几何引用、Location、Orientation 的拓扑对象)。
*   **Adapter (适配器)**：`BRepAdaptor_Curve`, `BRepAdaptor_Surface`。

### 2.2 模块定位
`BRepAdaptor` 位于 OCCT 架构的 **Modeling Data** 层。
*   **下层依赖**：依赖 `TKBRep` (拓扑定义), `TKMath` (数学库), `TKGeomBase` (几何基础)。
*   **上层应用**：被 `TKGeomAlgo` (几何算法), `TKTopAlgo` (拓扑算法), `TKHLR` (消隐算法) 等高级模块广泛调用。

### 2.3 架构视图
```mermaid
graph TD
    subgraph "Generic Geometry Algorithms (通用算法)"
        Algo[通用算法 (如 GProp, IntCurveSurface)]
        Interface[<<Interface>> Adaptor3d_Curve]
    end

    subgraph "BRepAdaptor Layer (适配层)"
        Adapter[BRepAdaptor_Curve]
    end

    subgraph "Topology Data (拓扑数据)"
        Edge[TopoDS_Edge]
        Geom[Geom_Curve]
        Loc[TopLoc_Location]
    end

    Algo --> Interface
    Adapter -- inherits --> Interface
    Adapter -- wraps --> Edge
    Edge -- references --> Geom
    Edge -- contains --> Loc
```

### 2.4 设计优势
*   **解耦**：算法与数据结构解耦。
*   **性能**：适配器通常是轻量级对象，建立在栈上，不涉及深拷贝，仅持有引用，开销极小。
*   **一致性**：保证了所有基于适配器的算法对方向和变换的处理是一致的，减少了 Bug。
