## 2018.10.17

尝试复杂材质的重要性采样但失败了。

## 2018.10.18

总结了一番项目存在的问题并决定重构：

```
系统地做一套Common Distribution Transformer，这个做进Utils好了
缺乏一致的处理无限光源、无实体光源、有实体光源的方法
缺乏光源采样策略的指导性原则
BxDF的局部坐标系各自为政
wi和wo混乱不堪，面对非对称BSDF太烧脑
缺乏GeometryTemplate的接口规范
几何参数坐标和着色参数坐标的框架不明朗
交点的几何信息和着色信息应该分开存储和传递，现在的Intersection简直是混沌恶
要支持动态模糊就得引入时间参数
```

## 2018.10.19

重构完成了大体框架，对上述问题有以下对策：

```
系统地做一套Common Distribution Transformer，这个做进Utils好了
    已经做进Utils了，这东西零零散散地写容易出问题，集中精力来搞还真不困难
缺乏一致的处理无限光源、无实体光源、有实体光源的方法
    分了Le和AreaLe用于处理有实体和无实体光源
缺乏光源采样策略的指导性原则
    没啥好指导的，接口统一就行，各自想办法
BxDF的局部坐标系各自为政
    BSDF记录local coordinate system
wi和wo混乱不堪，面对非对称BSDF太烧脑
    统一命名，wi和wo表示radiance的传播方向而不是importance的
缺乏GeometryTemplate的接口规范
    统一做成Geometry基类，慢点没关系，不再用模板了，SFINAE写得心累
几何参数坐标和着色参数坐标的框架不明朗
    分离了geoLocal和shdLocal，前者由求交过程给出，后者由Material在SurfacePoint的基础上给出
交点的几何信息和着色信息应该分开存储和传递，现在的Intersection简直是混沌恶
    分开做了SurfacePoint和ShadingPoint
要支持动态模糊就得引入时间参数
    暂时不考虑
```

约定world space中wi、wo以及r.dir都是单位向量，并且在用到的地方随手assert之。

另外，仍然没有加participating medium的接口，要把这东西和其他组件的交互想清楚太烧脑了。