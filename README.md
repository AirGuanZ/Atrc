# Atrc

一些实现过程中的思路记录在[Atrc](https://airguanz.github.io/all.html?tag=Atrc)中。已支持：

* Multiple Importance Sampling
* Torrance-Sparrow Mircofacet Model
* Participating Medium Rendering

## Script

允许用脚本来指定场景元素，示例如下：

```
# 渲染方程求解策略
integrator = { type = VolumetricPathTracer; maxDepth = 10; };
# 任务调度策略
renderer = { type = ParallelRenderer; workerCount = 6; };
# 图像平面采样策略
subareaRenderer = { type = JitteredSubareaRenderer; spp = 400; };
# 输出图像属性
output = { width = 640; height = 480; };
# 摄像机
camera = {
    eye  = (-7, 0, 0); # 视点位置
    dst  = (0, 0, 0);  # lookAt位置
    up   = (0, 0, 1);  # 用于描述横向倾斜的辅助向量
    FOVz = (Deg, 40);  # 垂直方向视野角
};
# 实体列表，实体即场景中的物体
entities = (
{
    type = GeometricEntity;
    # 几何模型
    geometry = {
        type = TriangleBVH;	# 构建为BVH树
        path = "./Assets/Statue_Bressant_1M_Poly.obj";
        # 绕Z轴旋转-90度，然后向下平移0.95个单位
        transform = ((Translate, 0, 0, -0.95), (RotateZ, Deg, -90));
    };
    # 表面材质
    material = {
        type      = FresnelSpecular;   # 绝缘体表面
        rc        = (0.9, 0.4, 0.2);   # 颜色
        fresnel = {
            type = FresnelDielectric;
            etaI = 1.0;				   # 外部折射率
            etaT = 1.6;				   # 内部折射率
        };
    };
    # 内外介质
    mediumInterface = {
        # 这里只指定了内部介质，外部默认为真空
        in = {
            type = Homogeneous;
            sigmaA = (7.0);     # 吸收率
            sigmaS = (0.3);     # 散射率
            le = (0.0);         # 自发光
            g = 0.6;            # 不对称系数
        };
    };
},
{
    type = GeometricEntity;
    geometry = {
        type = Cube;   # 内建的立方体模型
        sidelen = 1.4; # 立方体边长
        transform = ((Translate, 0, -2, 0.123), (Rotate, 1.0, 1.1, 1.2, Deg, 47));
    };
    material = {
        # 在其他材质上乘上一个由二维纹理描述的因子
        type = TextureScaler;
        sampler = Linear;                  # 双线性采样
        filename = "./Assets/CubeTex.png"; # 纹理路径
        internal = {
            type = DiffuseMaterial;   # “其他材质”为理想漫反射材质
            albedo = (0.2, 0.4, 0.8); # 反照率
        };
    };
});
# 光源列表
lights = ({
    type   = SkyLight;        # 从顶部到底部线性渐变的天光
    top    = (0.4, 0.7, 0.9); # 顶部辐射亮度
    bottom = (1.0);           # 底部辐射亮度
});

```

## Screenshots

见[Gallery](https://airguanz.github.io/atrc-gallery.html)。

![SS0](./Gallery/show-time.png)
