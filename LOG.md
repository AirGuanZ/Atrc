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

尽管如此，还是死怼了一番性能，大幅优化了BVH求交速度——其实就是消除了关键路径上的一个分支，效果好得出乎我的意料了——即使是在profiler的帮助下，你也未必能发现代码的性能瓶颈在哪。

Wait，说好的介质渲染呢？我白天推导了一番带介质的path tracer的估计量，只是还没动手写代码。

## 2018.11.3

今天比较失败。想用SSE/AVX加速BVH，然后喜闻乐见地负优化了。唯一的“进度”是画了张最高精度的Stanford Bunny（之前画的都是低模）。

## 2018.11.4

发现一个[非常良心的网站](http://threedscans.com/)，上面有一大堆扫描出来的精模，LICENSE是你爱怎么用就怎么用的那种。我立刻在上面下了个人像，简直是素材宝库啊！

这上面的模型动不动就是几十上百万面，我跑的时候果不其然BVH遍历爆栈了。我先是把BVH用`std::stack`改成非递归版本，然后嫌弃它动态内存分配太慢，自己写分配又太麻烦，干脆开了个`thread_local`的静态数组。但是我把数组开到1024依然会溢出，这时我开始怀疑是不是自己的SAH写炸了，不然BVH不应该这么深才对。于是我写了个简单粗暴的BVH划分策略——按图元数量二等分，结果效率瞬间上天，这说明我之前的SAH肯定是有问题的。至于问题是什么，暂时不想查了。

之前觉得跑得太慢，使劲怼代码上的常数；现在发现算法有问题，改了之后，啧啧，真快。

## 2018.11.5

把VolumetricPathTracer写了大半，真是在考试的边缘疯狂作死。

## 2018.11.6

VolumetricPathTracer done！开心！刚做出来的时候噪点多得可怕，检查了一番发现是采样介质自发光和表面自发光时没有截断计算透射比用的距离，改掉就好了。目前还没看出其他问题。

## 2018.11.7

实现了场景描述脚本，不用每次改场景都要改代码了，爽到！

## 2018.11.8

I wrote this record at 04:12 on my ubuntu VM. Fuck `std::wifstream`!

现在切回Windows了，吐槽一下，我跑一个小场景，编译器优化全往运行速度上开，MSVC用六个线程，耗时5-7秒；clang++在只占四个硬件线程的虚拟机上跑，只需要3秒……这效率我有点看不懂，明天再测试一波。

## 2018.11.10

昨天一天都在车上，没空写代码；今天把RendererManager和SubareaRendererManager补全了，现在就差Independent Scene Object Definition了，有了这个就能用脚本描述Scene中包括Participating Medium在内的一切元素。

为personal homepage添加了一个atrc-gallery页面。

## 2018.11.11

如计划所述，允许在脚本中引用之前定义的场景元素，目前已经可以用脚本描述任何可以渲染的情景了，立刻画了个琥珀材质的雕塑，想拿来当头像又觉得太丑 Orz

接下来做什么还没完全想好，先画个丁达尔效应吧，然后做BSSRDF或者BDPT。

## 2018.11.17

连续划了好几天的水，一直不能很好地理解BDPT，真是太丢人了……网上很多博客完全没用，这些人以为自己懂了其实他们连无偏性/一致性都证明不来。目前只能零零星星看各种thesis和paper。

## 2018.11.19

昨天一天都在赶路，今天基本都在办手续，啥都没做。不过搞清楚了一点：$\delta$从一个定义域经过变量代换变换到另一个定义域时是需要放缩的，而这并不是什么定理，而是为了让它的性质足够良好而作出的定义。我以前在《信号与系统》中怕是学了个假$\delta$函数——我竟然不知道这一点！就是这个性质让我好几天没法理解为什么我推出来的理想透视摄像机的和其他文献中的都不一样……

划水写了个AO Integrator，不过好歹可以作为毕设的一部分。

## 2018.11.21

这几天各种跑腿办手续，真是累啊。今天就只写了个CubeMapper，明天基于此搞个Environment Mapping。

## 2018.11.22

Environment Mapping done！不过Cube Map用起来有点蛋疼，每次都要去核对六张图需不需要翻转、旋转什么的。

## 2018.11.23

写了EntityProjector和EnvironmentLightProjector，也就是可以求球谐函数的投影系数了。

把ObjectManager这一大坨给抽离出来成为单独的项目，免得每个项目都要写一遍基本的参数解析。

等等，我是不是把BDPT给忘了？……咳咳……

## 2018.11.24

写了个Sphere Environment Light。

简单地抽出来了一个并行任务调度器，放进了AGZ Utils中，从而简化了ParallelRenderer的编写。其实做这个主要是因为SHTool也要做类似的并行化，偏偏又因为接口原因没法纳入Atrc原本的体系中。本着DRY的原则，就做了这么个局部重构。

## 2018.11.25

今天主要是去上海搬东西，没怎么写代码。

晚上回来过后看了看SHTool，意识到自己犯了个愚蠢的错误——Entity的球谐系数是一系列Spectrum构成的矩阵，Light的球谐系数是一个向量，求两者时需要的输入也不一样，这两个东西的Projector怎么能共用一套接口呢？于是二话不说把它们拆成了两个不相关的东西。

## 2018.11.26

完成了EntityProjector、LightProjector以及EntityRenderer，3阶球谐函数的重建效果意外地很不错。

整理了项目文件结构——把SHTool重命名为SH，并将其代码挪动到/Source/Tools中。

新建项目C2S，用来把立方环境贴图转换成球面环境贴图。

## 2018.11.27

实现了SH系数旋转。

晚上发现在Ubuntu上跑得好好的根据SH系数重建图像的程序在Windows上无法加载保存好的SH系数。调了一个多小时，发现是因为保存和加载文件的时候没有加`std::ios::binary`，Fuck！

## 2018.11.28

在AGZ Utils中写了个比较通用的FileCache工具，以此在Atrc中添加了TriangleBVH的磁盘cache功能，大幅减小了渲染前的预处理时间。这东西调了我老久，最后居然是一个std::optional类型忘了解引用，心好累……

## 2018.11.29

之前在ubuntu上用的编译器一直是clang++，今天尝试在g++7.3.0上通过编译。一开始是模板参数处无法推导出函数指针类型，我手动加上了。这也就算了，通过编译后在渲染时直接卡死，暂停+打断点一看，卧槽，卡在标准库随机数生成器内部出不来……同样是使用libstdc++，clang++就不会出问题，真是奇怪了。

晚上继续调试，总结出来一个最小用例：

```cpp
#include <iostream>
#include <random>
template<typename Engine>
struct EngineWrapper { Engine eng; };
template<typename Engine>
thread_local EngineWrapper<Engine> engineWrapper;
int main()
{
    int s = std::uniform_int_distribution<int>(0, 1)(
            engineWrapper<std::default_random_engine>.eng);
    std::cout << s << std::endl;
}
```

最后发现是gcc thread_local的bug，沃日，告辞！

## 2018.11.30

要做BDPT，现有的Atrc在Core Interface上必须要做修改——我默认把任务调度策略和LTE求解策略分离了，但是BDPT中有类似Light Tracing的部分必须要访问整个Film。为此我把过去的Integrator、SubareaRenderer和Renderer三者合成了一个Renderer，过去的这三个组件现在变成了一个特殊的Renderer的实现。

## 2018.12.1

一口气把SH的投影、重建和系数旋转做到了5阶，话说我基本看不出低频光源下用4阶和用5阶的区别……

此外，原来的Integrator是核心接口的一部分，现在只适用于PathTracingRenderer，因此仅应作为后者的组件出现。所以我把Integrator开除出了Core Interface，而只是作为PathTracingRenderer的下属。

## 2018.12.2

让SH支持了任意bouces number的间接照明，其实这东西和path tracing差不多，不知道为什么那么多文章将它作为“拓展”、“advanced topic”来介绍。

## 2018.12.3

修改了Camera相关的Core Interface以支持摄像机采样以及measure function，丢掉了原来的PerspectiveCamera，重写了一个比较严格的PinholeCamera。PinholeCamera画出来的东西是上下左右颠倒的，为此又添加了两个用来翻转图像的post-processor stage。

## 2018.12.4

没做什么东西，只把Camera相关的local-world变换从PinholeCamera中抽取出来，使得其他类型的Camera也可以使用这部分代码。

## 2018.12.5

实现了薄凸透镜摄像机模型，途中还纠正了一个AGZ Utils中计算UnitDisk上均匀采样的pdf的bug，终于又可以画景深效果了。

把PathTracingRenderer划分任务时用的grid大小做进了脚本参数，随便改了改，效率瞬间提升15%，这是为什么？Cache的原因吗？不科学啊……

原来的代码在许多地方直接使用了AGZ::Texture2D，采样方式、坐标wrap方式各自为政，混乱不堪；这回在Core Interface中添加了Texture，也把它加入了Creator/Manager体系，用起来舒服多了。

## 2018.12.6

添加了Environment Camera，这东西的film上的均匀采样并不对应sensor上的均匀采样，于是我不得不干掉原来的We体系，换成基于film的Qe体系，而推导Qe和We间的关系又花费了一些时间。

## 2018.12.7

想加Normal mapping，于是写了个简单的NormalMapper，由于没有单独的Triangle Geometry Object，一时居然找不到好的测试场景。于是写了个单独的Triangle类，结果测试的时候物体下面的阴影怎么看都很奇怪。一开始以为是Triangle这边local coord system有问题，通过换成mirror材质否认了这一猜想，然后看了半天diffuse材质的wi采样，也没什么错误。我甚至怀疑到path tracer上去了，可是换成volumetric path tracer，问题依然。也设想了是has intersection不对，可是看代码怎么也看不出差错。最后我认为可能是shadow ray测试不对，于是把path tracer的MIS of direct illumination给拆开，结果bsdf sampling的结果是对的，而light sampling的结果就不对，果然问题在这！然而这个MIS我已经用了很久，没道理会有这么明显的bug啊。看了半天，我突然想起triangle不同于以往的geometry object，它的bound可能在某一维度上宽度为0，检查了一番相关代码，也改了一两个不合理之处，而bug变得更加玄学了——地面阴影会随着三角形形状变化而变化，而这个变化显得丝毫没有规律和道理。最后，我也不知道是为什么，检查了一下environment light中计算world radius的代码，结果发现自己不慎把一个+写成了-……

## 2018.12.8

实现了Diffuse Material的Normal Mapping。

## 2018.12.9

发现用的人物模型法线有问题，手动加入了纠正代码；局部重构了BSDF对shading coord sys的处理。

## 2018.12.10

前两天做normal mapping是在MSVC上做的，今天一切换到ubuntu，clang编译的版本马上挂了一大堆assertion，东拆西补好不热闹。搞了大半个上午，烦不胜烦，决定启动一次重构，主要针对：

1. World space中表示方向的向量不再被假设是归一化的。这个假设一开始减小了我的心智负担，现在却变得难以维护。
2. BSDF对几何局部坐标系与着色局部坐标系的处理，这里需要好好想想。
3. ObjMgr的架构导致无法共享全局参数、无法处理预定义对象间引用的问题。
4. 早就看那堆Integrator基于递归的写法不顺眼了。
5. Material体系混乱。
6. 全局BVH没法用，TriangleBVH代码丑。

以及一些其它的细枝末节的问题。

## 2018.12.11

看到Alvy Ray Smith 1995年的一篇文章，把将像素看作一个矩形的做法严肃地批判了一番，吓得我赶紧按照他的建议做了个基于sampling-reconstruction体系。

## 2018.12.12

实现了CubeEnvironmentLight、SkyLight以及NativePathTracingIntegrator，之间夹杂了许许多多的接口调整。终于又画出了两个理想漫反射球体叠在一起的场景。

## 2018.12.13

完成了解析脚本创建场景的框架。

## 2018.12.14

重新写了一个Triangle BVH，速度比重构前的版本不知高到哪里去了。值得一提的是我精心编写的SIMD加速代码被MSVC生成的代码吊起来打 = =

## 2018.12.15

实现了基于Torrance-Sparrow微表面模型的金属材质。

## 2018.12.16

实现了Oren-Nayar模型和理想镜面反射。

## 2018.12.17

实现了用多重重要性采样技术计算直接光照的Path Tracer。

## 2018.12.18

实现了基于TriangleBVH的漫射光源，以及对TriangleBVH的磁盘cache。

## 2019.03.01

好久没写过LOG了，连Editor都写出来了。现在有这么几个问题：

1. 用imgui撸复杂界面还是有点让我吃不消，特别是做layout，心智负担太大了。
2. Mgr负责从配置文件到渲染对象的转换，Editor有编辑对象和配置文件间的互转，两边完全是隔开的，每次添加或者修改一种渲染对象，就需要改这好几个地方，各自为政有些蛋疼。
3. Editor的资源管理体系有点乱，虽然目前都在一个抽象体系下相依无事，但心里总觉得这里面有一堆坑。

对此我打算做一次局部重构，Mgr和Editor完全重写，Lib中渲染对象的创建方式也会改变。采用以下方案：

1. 学学Qt并用来做Editor的界面。
2. 每个渲染对象对应一个Parameter type，其构造函数接受Parameter type类型的参数，Mgr负责Parameter type和配置文件间的转换，Editor负责Parameter type的展示和编辑。也就是说，一个实体的中心不再是渲染对象，而是Parameter type，渲染对象只是渲染器可以用该type来创建的边缘对象而已。