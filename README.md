# Atrc

Offline rendering lab based on ray tracing

## Features

- [x] volumetric path tracing, adjoint particle tracing, bidirectional path tracing
- [x] materials: disney bsdf, mirror, varnish...
- [x] hdr texture
- [x] (optional) integrated [Embree](https://github.com/embree/embree) engine
- [x] (optional) integrated [OIDN](https://github.com/OpenImageDenoise/oidn) denoiser

## Roadmap

- [x] refactor area light source as entity material
- [x] highlighting selected entity
- [x] ply loading
- [x] mouse picking
- [x] material thumbnail in editor
- [x] saving/loading in editor
- [ ] rendering camera setting in editor
- [ ] JSON exporter in editor
- [ ] scene tree
- [ ] variance buffer
- [ ] more rendering algorithms: sppm, vcm, mlt, ...

## Documentation

[doc](https://airguanz.github.io/atrc_doc/doc.html)

[doc-cn](https://airguanz.github.io/atrc_doc/doc-cn.html) (in Traditional Chinese)

## Gallery

Scene Editor (under development) (scene ref [here](https://luxcorerender.org/download/)):

![0](./doc/gallery/editor.png)

Materials:

![1](./doc/gallery/materials.png)

Fireplace Room (scene ref [here](http://casual-effects.com/data/index.html)):

![2](./doc/gallery/fireplace.png)

DoF:

![3](./doc/gallery/dof.png)
