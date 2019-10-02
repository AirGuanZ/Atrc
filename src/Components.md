## Components

`./src/core/common`：核心组件的共用基本定义，如`real`等

`./src/core/rasterizer`：光栅化渲染管线

`./src/core/tracer`：光线追踪对象

`./src/app/material_explorer`：材质实时预览工具

`./src/app/obj_to_scene`：将带`mtl`的`obj`文件转换为JSON表示的`entity`列表

`./src/app/cli`：离线渲染器命令行启动器

`./src/app/cli_ras`：光栅化离线渲染器命令行启动器

## Dependencies

| 组件名            | 组件形式   | 依赖名     |
| ----------------- | ---------- | ---------- |
| agz-utils         | 静态库     |            |
| common            | 头文件     | AGZUtils   |
| rasterizer        | 头文件     | common     |
| tracer            | 静态库     | common     |
| material_explorer | 可执行文件 |            |
| obj_to_scene      | 可执行文件 |            |
| cli               | 可执行文件 | tracer     |
| cli_ras           | 可执行文件 | rasterizer |

## Cached Varibles

**common**: `AGZCommon_INCLUDE_DIRS`

**rasterizer**: `AGZRasterizer_INCLUDE_DIRS`

**tracer**: `AGZTracer_INCLUDE_DIRS, AGZTracer_LIBRARIES`

