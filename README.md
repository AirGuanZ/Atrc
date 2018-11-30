# Atrc

一些实现过程中的思路记录在[Atrc](https://airguanz.github.io/all.html?tag=Atrc)中。已支持：

* 直接光照的多重重要性采样
* Torrance-Sparrow微表面模型
* 带介质的路径追踪器
* 多边形模型加载
* BVH等加速求交数据结构
* 用脚本描述所有的场景元素和渲染配置

## Script

允许用脚本来指定场景元素，示例如下：

```
# 渲染策略
renderer = {
    type        = ParallelRenderer; # 基本的并行渲染
    workerCount = 11;               # 开启11个工作线程
    spp         = 100;              # 每像素采样数
    integrator  = {
        type     = PathTracer; # 使用路径追踪
        maxDepth = 50;         # 最大追踪深度
    };
};
# 输出图像的属性
output = { filename = "./Build/Output.png"; width = 640; height = 480; };
# 用何种方式展示进度
reporter = { type = Default; };
# 摄像机
camera = {
    type = Perspective;        # 透视摄像机
    aspectRatio = 1.333333333; # 视图宽高之比
    eye  = (-7, 0, 0);         # 视点位置
    dst  = (0, 0, 1.5);        # lookAt位置
    up   = (0, 0, 1);          # 用来水平倾斜摄像机的辅助向量
    FOVz = Deg(70);            # 垂直方向视角
};
# 实体列表
entities = ({
    type = GeometricEntity;
    geometry = {
        type = TriangleBVH;          # 由三角形构成的BVH树
        path = "./Assets/Model.obj"; # Wavefront模型路径
        transform = (                # 先缩小为0.005倍大小，再向下平移1单位
            Translate(0, 0, -1),
            Scale(0.0050));
    };
    material = {
        type = Metal;         # 使用金属材质
        rc = (0.9, 0.4, 0.2); # 颜色
        etaI = 1;             # 外部折射率
        etaT = 0.1;           # 内部折射率
        k = 0.2;              # 吸收率
        roughness = 0.02;     # 粗糙度
    };
}, {
    type = GeometricEntity;
    geometry = {
        type = Cube;                           # 内建的立方体模型
        sidelen = 1.4;                         # 边长1.4
        transform = (                          # 先旋转再平移
            Translate(0, -2, 0.123),
            Rotate((1.0, 1.1, 1.2), Deg(47)));
    };
    material = {
        type = TextureScaler;          # 用纹理作为颜色的乘积因子
        sampler = Linear;              # 双线性采样
        path = "./Assets/CubeTex.png"; # 纹理路径
        internal = {
            type = DiffuseMaterial;   # 纹理之下采用漫反射模型
            albedo = (0.2, 0.4, 0.8); # 反照率
        };
    };
});
# 光源列表
lights = ({
    type = SphereEnvironmentLight;   # 使用球面映射的环境光
    tex = "./Assets/PineForest.png"; # 纹理路径
});
# 后处理流程
postProcessors = (
    { type = ACESFilm; },                       # Tone mapping
    { type = GammaCorrection; gamma = 0.4545; } # Gamma correction
);
```

## Screenshots

![SS1](./Gallery/show-time-1.png)

![SS2](./Gallery/show-time-2.png)
