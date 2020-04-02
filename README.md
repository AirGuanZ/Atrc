# Atrc

Offline rendering lab based on ray tracing

## Features

- [x] Volumetric path tracing with next event estimation
- [x] Adjoint particle tracing
- [x] Bidirectional path tracing
- [x] Stochastic progressive photon mapping
- [x] Primary sample space MLT on path tracing
- [x] Various material models: Disney Principled BSDF, (frosted) glass, Phong, ...
- [x] Homogeneous/Heterogeneous Participating Medium
- [x] Normalized Diffusion BSSRDF
- [x] Almost all material properties can be specified with 2D/3D textures
- [x] Various geometry models: sphere, quad, triangle, disk, mesh
- [x] Two-level accelerating structure (SAH based BVH)
- [x] Importance sampled environment light
- [x] G-Buffer output (albedo, normal, ...)
- [x] Depth of field
- [x] ACES tone mapping
- [x] Image sample space low-pass filter
- [x] (Optional) Integrated OIDN library
- [x] (Optional) Integrated Embree library
- [x] Interactive scene editor

## Roadmap

- [ ] Light portal
- [ ] Component based bsdf
- [ ] Light sampling hints (sample count, user-specified power, env light portal)

## Documentation

[doc](https://airguanz.github.io/atrc_doc/doc.html)

## Gallery

Food (scene ref [here](https://luxcorerender.org/download/)):

![0](./doc/gallery/food.png)

Classroom (scene ref [here](https://www.blender.org/download/demo-files/)):

![pic](./doc/gallery/classroom.png)

BSSRDF:

![pic](./doc/gallery/dragon.png)

Materials:

![1](./doc/gallery/materials.png)

Dining Room (scene ref [here](https://www.blendswap.com/blends/view/86457)):

![2](./doc/gallery/dining.png)


