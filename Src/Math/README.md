系统层面的三维坐标系有三个：

1. 世界坐标系$S_W$；

2. 本地坐标系$S_L$，每个Geometry Object都有一个自己的；

3. 表面坐标系$S_S$，每个Surface上的每个点都有一个这样的坐标系，三个坐标轴分别是本地坐标系中的：

   $$
   \begin{aligned}
   \boldsymbol x_S &= \frac{\partial\boldsymbol p}{\partial\boldsymbol u} \\
   \boldsymbol y_S &= \boldsymbol n \times \frac{\partial\boldsymbol p}{\partial\boldsymbol u} \\
   \boldsymbol z_S &= \boldsymbol n
   \end{aligned}
   $$



每个Entity持有自己的Geometry、Material和Transform，Ray $r_W$来的时候先$S_W \to S_L$得到$r_L$，由Geometry给出交点以及$S_S$，把交点参数喂给Material得到BxDF，把$r_S$交给BxDF得到一系列后续采样的Ray，然后一路逆变换到$S_W$。所有权结构是这样的：

```
struct Entity {
    geoObj : GeometryObject,
    mat    : Material,
    trans  : Transform
}
```

GeometryObject原则上不持有$S_W \leftrightarrow S_L$的Transform。

