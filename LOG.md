## 2018.10.17

尝试复杂材质的重要性采样但失败了。

## 2018.10.18

总结了一番项目存在的问题并决定重构。

## 2018.10.19

重构完成了大体框架，对之前注意到的问题有以下对策：

```
系统地做一套Common Distribution Transformer，这个做进Utils好了
    已经做进Utils了，这东西零零散散地写容易出问题，集中精力来搞还真不困难
缺乏一致的处理无限光源、无实体光源、有实体光源的方法
    分了AreaLe和Le用于处理有实体和无实体光源，无限光源当成无实体光源处理
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
没有引入时间参数，没法做动态模糊这种炫酷效果
    暂时不考虑
```

约定world space中wi、wo以及r.dir都是单位向量，并且在用到的地方随手assert之。

另外，仍然没有加participating medium的接口，要把这东西和其他组件的交互想清楚太烧脑了。

## 2018.10.21

完成了用MIS计算直接光照的PathTracer。其实公式推导到完成代码一条龙只花了小半天，但是和重构前的PathTracer相比，画出来的东西总是特别暗。调了大半天，什么手段都试过了，最后发现是因为测试代码里最后输出图像的时候$\gamma$校正的1/2.2写成了2.2，尼玛……

## 2018.10.22

刚开始写材质系统，离能用还有十万八千里呢。