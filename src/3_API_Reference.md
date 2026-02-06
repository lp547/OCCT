# OCCT 建模数据层 - 关键模块 API 接口说明

## 1. 模块：TopAbs (拓扑抽象基础)

该模块定义了拓扑系统的基础词汇和逻辑运算规则。

### 1.1 枚举定义

#### TopAbs_ShapeEnum (形状类型)
描述拓扑实体的具体类型，按复杂度从高到低排列。

| 枚举值 | 含义 | 备注 |
| :--- | :--- | :--- |
| `TopAbs_COMPOUND` | 复合体 | 任意形状的集合 |
| `TopAbs_COMPSOLID`| 复合实体 | 共享面的实体集合 |
| `TopAbs_SOLID` | 实体 | 由壳包围的 3D 区域 |
| `TopAbs_SHELL` | 壳 | 面的集合 |
| `TopAbs_FACE` | 面 | 曲面的一部分 |
| `TopAbs_WIRE` | 线框 | 边的集合 |
| `TopAbs_EDGE` | 边 | 曲线的一部分 |
| `TopAbs_VERTEX` | 顶点 | 几何点 |
| `TopAbs_SHAPE` | 泛型形状 | 抽象基类 |

#### TopAbs_Orientation (方向)
描述拓扑元素相对于其几何定义的方向。

| 枚举值 | 含义 | 作用 |
| :--- | :--- | :--- |
| `TopAbs_FORWARD` | 正向 | 拓扑方向与几何参数方向一致 |
| `TopAbs_REVERSED`| 反向 | 拓扑方向与几何参数方向相反 |
| `TopAbs_INTERNAL` | 内部 | 形状完全位于父形状内部（无材质边界作用） |
| `TopAbs_EXTERNAL` | 外部 | 形状完全位于父形状外部 |

### 1.2 核心函数接口

#### `TopAbs::Compose`
计算组合方向。当一个形状 A 包含形状 B，且两者都有各自的方向时，计算 B 相对于 A 的最终方向。

*   **输入参数**:
    *   `Or1` (TopAbs_Orientation): 父形状的方向。
    *   `Or2` (TopAbs_Orientation): 子形状在父形状中的局部方向。
*   **输出参数**:
    *   (TopAbs_Orientation): 组合后的绝对方向。
*   **接口功能**: 实现方向的叠加逻辑。例如 `Compose(REVERSED, FORWARD)` 返回 `REVERSED`。

#### `TopAbs::Reverse`
反转方向。

*   **输入参数**:
    *   `Or` (TopAbs_Orientation): 原始方向。
*   **输出参数**:
    *   (TopAbs_Orientation): 反转后的方向。
*   **接口功能**: `FORWARD` <-> `REVERSED`，`INTERNAL` <-> `EXTERNAL`。常用于求补集操作。

## 2. 模块：Geom (TKG3d 核心几何类)

虽然 `TKG3d` 包含很多类，以下是几个最具代表性的核心类的 API 风格说明。

### 2.1 Geom_Curve (抽象曲线)

#### `Value`
计算曲线上参数 u 处的 3D 点。

*   **输入参数**:
    *   `U` (Standard_Real): 曲线参数。
*   **输出参数**:
    *   (gp_Pnt): 对应的 3D 点坐标。
*   **接口功能**: 曲线求值。

### 2.2 Geom_Surface (抽象曲面)

#### `D1` (First Derivative)
计算曲面上参数 (u, v) 处的点及其一阶偏导数。

*   **输入参数**:
    *   `U` (Standard_Real): U 方向参数。
    *   `V` (Standard_Real): V 方向参数。
    *   `P` (gp_Pnt&): [输出] 对应的 3D 点。
    *   `D1U` (gp_Vec&): [输出] U 方向的一阶偏导向量。
    *   `D1V` (gp_Vec&): [输出] V 方向的一阶偏导向量。
*   **接口功能**: 用于分析曲面的切平面和法向量。

### 2.3 Geom_BSplineCurve (B样条曲线)

#### `Constructor` (构造函数)
创建一个 B 样条曲线。

*   **输入参数**:
    *   `Poles` (Array1OfPnt): 控制点数组。
    *   `Weights` (Array1OfReal): 权重数组（可选）。
    *   `Knots` (Array1OfReal): 节点矢量。
    *   `Multiplicities` (Array1OfInteger): 节点重复度。
    *   `Degree` (Standard_Integer): 曲线阶数。
*   **接口功能**: 初始化一个数学上精确的自由曲线。

## 3. 数据结构关键字段说明

在 `TopAbs` 的设计中，没有任何成员变量，全是静态方法。数据主要体现在枚举值的定义上。
真正的复杂数据结构存在于 `TKBRep` 层的 `TopoDS_Shape` 中（虽然不在本目录下，但必须提及以便理解）：

*   **TopoDS_Shape**:
    *   `myTShape`: 指向实际几何/拓扑定义的指针（共享）。
    *   `myLocation`: 局部坐标系变换（允许移动物体而不修改底层几何）。
    *   `myOrientation`: 当前引用的方向（允许同一个面被正向或反向使用）。
