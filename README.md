# Atrc

Offline rendering lab based on ray tracing

**Note**. [GPU version](https://github.com/AirGuanZ/Btrc) is in development...

## Features

- [x] Volumetric path tracing with next event estimation
- [x] Volumetric particle tracing
- [x] Volumetric bidirectional path tracing
- [x] Stochastic progressive photon mapping
- [x] Primary sample space MLT on path tracing
- [x] ReSTIR renderer
- [x] Various material models: Disney principled BSDF, phong, DreamWorks fabric ...
- [x] Homogeneous/heterogeneous participating medium
- [x] Normalized diffusion BSSRDF
- [x] Almost all material properties can be specified with 2D/3D textures
- [x] Various geometry models: sphere, quad, triangle, disk, mesh
- [x] Two-level accelerating structure (SAH based BVH)
- [x] Importance sampled environment light
- [x] G-buffer output (albedo, normal, ...)
- [x] Depth of field
- [x] ACES tone mapping
- [x] Image sample space low-pass filter
- [x] (Optional) Integrated OIDN library
- [x] (Optional) Integrated Embree library
- [x] Interactive scene editor

## Known Issues

- [ ] Relation between inv gamma correction & linear sampler of image texture

## Documentation

[doc](https://airguanz.github.io/atrc_doc/doc.html)

## Gallery

Editor:

![pic](./doc/gallery/editor.png)

Food (rendered with bdpt) (scene ref [here](https://luxcorerender.org/download/)):

![0](./doc/gallery/food.png)

Juice (rendered with vol_bdpt) (scene ref [here](https://luxcorerender.org/download/)):

![pic](./doc/gallery/juice.png)

Classroom (rendered with vol_bdpt) (scene ref [here](https://www.blender.org/download/demo-files/)):

![pic](./doc/gallery/classroom.png)

Materials: (rendered with pt):

![pic](./doc/gallery/materials.png)

Fog (rendered with vol_bdpt):

![pic](./doc/gallery/fog.png)

BSSRDF (rendered with pt):

![pic](./doc/gallery/dragon.png)

Bedroom (rendered with pt) (scene ref [here](https://benedikt-bitterli.me/resources/)):

![1](./doc/gallery/bedroom.png)

Dining Room (rendered with bdpt) (scene ref [here](https://www.blendswap.com/blends/view/86457)):

![2](./doc/gallery/dining.png)

