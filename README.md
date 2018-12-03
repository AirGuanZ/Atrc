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
    type        = PathTracingRenderer; # 路径追踪
    workerCount = 11;                  # 工作线程数量
    spp         = 1000;                # 每像素采样数
    integrator  = {
        type     = PathTracer; # 使用普通的路径追踪（而非支持介质传播等的高级版本）
        maxDepth = 50;         # 最大追踪深度
    };
};
# 输出文件名
outputFilename = "./Build/Output.png";
# 使用何种方式展示渲染进度
reporter = { type = Default; };
# 摄像机模型
camera = {
    type = Pinhole; # 小孔摄像机
    film = {
        width  = 640; # 图像宽度
        height = 480; # 图像高度
    };
    sensor = {
        width    = 2.0; # 图像平面在场景中的宽度
                        # 高度可以省略，此时会根据图像宽高于图像平面宽度自动计算
        distance = 1;   # 小孔和图像平面间的距离
    };
    pinholePos = (-7, 0, 0);  # 小孔位置
    lookAt     = (0, 0, 1.5); # 摄像机看向何处
    up         = (0, 0, 1);   # 用于辅助设置摄像机水平倾斜角度的向量
};
# 实体列表
entities = (
{
    type = GeometricEntity;
    geometry = {
        type = TriangleBVH;                                        # 以三角形为图元的BVH树
        path = "./Assets/Model.obj";                               # 模型文件路径
        transform = (Translate(0.0, 0.02, -0.945), Scale(0.0050)); # 先缩放，再平移
    };
    material = {
        type = Metal;         # 金属材质
        rc = (0.9, 0.4, 0.2); # 颜色
        etaI = 1;             # 外部折射率
        etaT = 0.1;           # 内部折射率
        k = 0.2;              # 吸收率
        roughness = 0.02;     # 粗糙度
    };
},
{
    type = GeometricEntity;
    geometry = {
        type = Cube;                                    # 内建的立方体模型
        sidelen = 1.4;                                  # 边长
        transform = (Translate(0, -2, 0.123),           # 先旋转再平移
                     Rotate((1.0, 1.1, 1.2), Deg(47)));
    };
    material = {
        type = TextureScaler;          # 在其他材质基础上叠加一个由纹理指定的乘数因子
        sampler = Linear;              # 双线性采样
        path = "./Assets/CubeTex.png"; # 纹理路径
        internal = {
            type = DiffuseMaterial;   # 内部采用理想漫反射材质
            albedo = (0.2, 0.4, 0.8); # 反照率
        };
    };
}
);
# 光源列表
lights = (
{
    type = SphereEnvironmentLight;   # 使用球面纹理映射的环境光
    tex = "./Assets/PineForest.png"; # 纹理路径
}
);
# 后处理流程，从上往下依次执行
postProcessors = (
    { type = HorizontalFlipper; },           # 小孔摄像机成像是上下颠倒的，因此要水平、垂直各翻转一次
    { type = VerticalFlipper; },
    { type = ACESFilm; },                    # Tone mapping
    { type = GammaCorrection; gamma = 1.0; } # Gamma矫正
);

```

## Screenshots

![SS1](./Gallery/show-time-1.png)

![SS2](./Gallery/show-time-2.png)
