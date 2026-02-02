# BRepLProp 模块文件详细说明

## BRepLProp.hxx / BRepLProp.cxx
**文件作用：** BRepLProp 包的顶层接口与工具函数集合。
**详细说明：** 该文件定义了 `BRepLProp` 类（通常作为一个包/命名空间使用），主要提供静态工具函数。核心功能是 `Continuity` 方法，用于计算两条连接的 BRep 曲线（`BRepAdaptor_Curve`）在连接点处的连续性级别（如 C0, G1, C1, G2 等）。它处理了几何误差容忍度，是拓扑连接分析的基础组件。

## BRepLProp_CLProps.hxx / BRepLProp_CLProps_0.cxx
**文件作用：** 曲线局部属性（Curve Local Properties）计算的核心类。
**详细说明：** 该类是泛型类 `LProp_CLProps` 的实例化版本。它负责计算 BRep 曲线（基于 `BRepAdaptor_Curve`）在特定参数 $u$ 处的几何属性，包括点坐标、一阶导数（切线）、二阶导数（法线/曲率）、三阶导数以及曲率中心。它是分析边（Edge）几何特征的主要工具。

## BRepLProp_SLProps.hxx / BRepLProp_SLProps_0.cxx
**文件作用：** 曲面局部属性（Surface Local Properties）计算的核心类。
**详细说明：** 该类是泛型类 `LProp_SLProps` 的实例化版本。它负责计算 BRep 曲面（基于 `BRepAdaptor_Surface`）在特定参数 $(u, v)$ 处的几何属性。功能包括计算切平面（u/v 切线）、法向量、高斯曲率、平均曲率、主曲率（最小/最大曲率）及其方向，以及判断点是否为脐点（Umbilic）。它是分析面（Face）几何特征的主要工具。

## BRepLProp_CurveTool.hxx / BRepLProp_CurveTool.cxx
**文件作用：** 连接 BRep 曲线与泛型 LProp 算法的适配器工具类。
**详细说明：** 该类实现了泛型 `LProp` 算法所需的标准接口。它封装了 `BRepAdaptor_Curve`，提供了 `Value`（计算点）、`D1`/`D2`/`D3`（计算导数）、`Continuity`（获取连续性）等底层几何求值函数。它的存在使得通用的数学算法可以无缝应用于 BRep 的几何定义上。

## BRepLProp_SurfaceTool.hxx / BRepLProp_SurfaceTool.cxx
**文件作用：** 连接 BRep 曲面与泛型 LProp 算法的适配器工具类。
**详细说明：** 该类实现了泛型 `LProp` 算法所需的标准接口。它封装了 `BRepAdaptor_Surface`，提供了 `Value`、`D1`/`D2`（偏导数）、`DN`（高阶导数）、`Continuity` 和 `Bounds`（边界）等底层几何求值函数。它是 `BRepLProp_SLProps` 进行复杂曲面分析的底层数据提供者。

## FILES.cmake
**文件作用：** 构建系统的配置文件。
**详细说明：** 该文件列出了 `BRepLProp` 模块中包含的所有源文件和头文件，用于 CMake 构建系统识别编译目标和依赖关系。
