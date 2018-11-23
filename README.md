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
# LTE求解策略
integrator = { type = VolumetricPathTracer; maxDepth = 10; };
# 任务调度策略
renderer = { type = ParallelRenderer; workerCount = 6; };
# 图像局部采样策略
subareaRenderer = { type = JitteredSubareaRenderer; spp = 10; };
# 输出文件属性
output = { filename = "./Build/Output.png"; width = 640; height = 480; };
# 进度显示配
reporter = { type = Default; };
# 摄像机属性
camera = {
    eye  = (-7, 0, 0);
    dst  = (0, 0, 0);
    up   = (0, 0, 1);
    FOVz = (Deg, 40);
};
# 实体列表
entities = ({
    type = GeometricEntity;
    geometry = { # 几何属性
        type      = Sphere;                    # 内建球体模型
        radius    = 0.7;                       # 半径为0.7
        transform = ((Translate, 0, 2, -0.3)); # 平移
    };
    material = { # 材质属性
        type      = Metal; # 金属材质
        rc        = (0.5); # 颜色
        etaI      = (1);   # 外部折射率，这里为空气
        etaT      = (0.1); # 金属折射率
        k         = (0.1); # 吸收率
        roughness = 0.003; # 粗糙度
    };
}, {
    type = GeometricEntity;
    geometry = {
        type = TriangleBVH; # 建立三角形BVH树
        path = "./Assets/Statue_Bressant_1M_Poly.obj"; # 模型文件路径
        transform = ((Translate, 0.0, 0.02, -1.1), (RotateZ, Deg, -90), (Scale, 0.0050)); # 缩放之后旋转，最后平移
    };
    material = {
        type      = FresnelSpecular;  # 理想反射/折射表面
        rc        = (0.9, 0.4, 0.2);  # 颜色
        fresnel = { # Fresnel项
            type = FresnelDielectric; # 使用绝缘体fresnel公式
            etaI = 1.0;               # 外部折射率
            etaT = 1.6;               # 材料折射率
        };
    };
    mediumInterface = { # 介质属性
        in = { # 这里只指定了内部介质，外部默认为真空
            type = Homogeneous; # 均匀介质
            sigmaA = (7.0);     # 吸收率
            sigmaS = (0.3);     # 散射率
            le = (0.0);         # 自发光
            g = 0.3;            # 不对称系数
        };
    };
});
# 光源列表
lights = ({
    type   = SkyLight;        # 垂直渐变环境光
    top    = (0.4, 0.7, 0.9); # 顶部亮度
    bottom = (1.0);           # 底部亮度
});
# 后处理流程
postProcessors = (
{ type = ACESFilm; },                       # tone mapping
{ type = GammaCorrection; gamma = 0.4545; } # gamma校正
);

```

## Screenshots

![SS1](./Gallery/show-time-1.png)

![SS2](./Gallery/show-time-2.png)
