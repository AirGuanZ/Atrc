# Atrc

Atrc是[Zhuang Guan](https://github.com/AirGuanZ)的离线渲染实验室，主要以C++编写，仅依赖于辅助库[Utils](https://github.com/AirGuanZ/Utils)。Atrc（将会）包含以下组件：

- [x] Lib 离线渲染核心组件库，包含各种实体、材质模型和常见渲染算法，会随着我的学习不断扩展/重构
- [x] Mgr 用于管理核心组件的辅助库，根据配置字符串创建各种类型的核心组件中的对象（如模型、摄像机、灯光等）
- [x] Launcher 渲染器启动器
- [x] SH2D 将场景、灯光投影到1~5阶球谐函数系数，以及旋转球谐系数、根据球谐系数重建图像的工具

## Build

Atrc使用了大量C++17特性，因此只能用版本较新的编译器构建。`./Build/VS2017/Atrc`中包含了可以用VS2017打开的解决方案；在*nix下也可使用clang/gcc编译，终端中输入`make all`即可。我的测试环境为：

```
VisualStudioVersion = 15.0.28010.2048
g++ 8.2.0
clang 8.0.0
```

## Screenshots

![SS1](./Diary/1920x1080/30_2018_12_07_TriangleMirror.png)

![SS2](./Diary/Misc/2018_12_25_ShowTime.png)
