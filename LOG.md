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

刚开始写材质系统，完成了BxDFAggregate，DiffuseBRDF和PerfectSpecularReflection

## 2018.10.30

居然好几天忘了写日志。这几天主要完成了Cube类型的Geometry Object，三角形网格的BVH树，OBJ文件解析以及Torrance-Sparrow反射模型，添加了塑料、金属之类的材质。哦对，还支持了纹理，包括nearest和bilinear采样，Mipmap就先不管了。

## 2018.10.31

今天完成了Global BVH和Utils中的Config组件。下一步目标是写介质，想想牛奶玉石还有丁达尔效应就让人开心。

先稍微整理一下思路。任何光线的传播过程都要把介质考虑进来，因为必须跟踪任何light-carrying ray所处的介质。任何可以产生ray的东西都要指定它产生的ray所处的介质是什么，ray和表面发生intersection的时候也要做介质切换。目前，应该是要让entity指定in和out两种介质，scene指定虚空中的介质。Camera不指定介质，因为camera出来的ray的介质要么由首个交点指定，要么是虚空介质。

传播过程也有两类：light-surface传播，以及surface-surface传播，这两个过程都由integrator完成。

## 2018.11.2

本来是想写个GPU加速的BVH的，但是装了CUDA过后只要写的cu代码一有什么问题，直接死机，我觉得这样下去我是调不出来的，所以暂且放下吧。

尽管如此，还是死怼了一番性能，大幅优化了BVH求交速度——其实就是消除了关键路径上的一个分支，效果好得出乎我的意料了——即使是在profiler的帮助下，你永远也想不到代码的性能瓶颈在哪。

Wait，说好的介质渲染呢？我白天推导了一番带介质的path tracer的估计量，只是还没动手写代码。

## 2018.11.3

今天比较失败。想用SSE/AVX加速BVH，然后喜闻乐见地负优化了。唯一的“进度”是画了张最高精度的Stanford Bunny（之前画的都是低模）。