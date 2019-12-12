# Atrc Renderer Documentation

![pic](./gallery/0.png)

[TOC]

## Building

### Dependencies

**Prerequests**

* [cmake](https://cmake.org/) (>=3.10)
* MSVC or clang++ (and libc++) with supporting of major features of C++ 17 (g++ should be ok but is untested)

**Dependencies Contained in Project Source**

* [agz-utils](https://github.com/AirGuanZ/agz-utils) for mathematics and image loading
* [cxxopts](https://github.com/jarro2783/cxxopts) for parsing command-line arguments
* [nlohmann json](https://github.com/nlohmann/json) for parsing comfiguration file
* [stl reader](https://github.com/sreiter/stl_reader) for parsing STL model file
* [tiny obj loader](https://github.com/syoyo/tinyobjloader) for parsing OBJ model file
* [Embree 3.6.1](https://www.embree.org/) for better ray-mesh intersection test
* [glfw](https://www.glfw.org/) for managing OpenGL context of Material Explorer

**Dependencies Need to Be Prepared**

* [oidn](https://openimagedenoise.github.io/) for denoising (only when `USE_OIDN` is `ON`)

When using MSVC under Windows, copy all files in [oidn-1.1.0.x64.vc14.windows.zip](https://github.com/OpenImageDenoise/oidn/releases/download/v1.1.0/oidn-1.1.0. x64.vc14.windows.zip) to `lib/oidn/vc14`; when using linux, copy all files in [oidn-1.1.0.x86_64.linux.tar.gz](https://github.com/OpenImageDenoise/oidn/releases/download/v1.1.0/oidn-1.1.0.x86_64.linux.tar.gz) to `lib/oidn/linux`. Please refer to` cmake/ cmake-oidn` for details.

### CMake Options

| Name                    | Default Value | Explanation                        |
| ----------------------- | ------------- | ---------------------------------- |
| USE_EMBREE              | OFF           | use Embree library to tracing rays |
| USE_OIDN                | OFF           | use OIDN denoising library         |
| BUILD_MATERIAL_EXPLORER | OFF           | build Material Explorer            |

**Note**. OIDN is 64-bit only.

### Example

**Full-featured Building on Windows**

Run following command in `PowerShell`：

```powershell
git clone --recursive --depth=1 https://github.com/AirGuanZ/Atrc
cd Atrc
mkdir build
cd build
cmake -DUSE_EMBREE=ON -DUSE_OIDN=ON -DBUILD_MATERIAL_EXPLORER=ON -G "Visual Studio 15 2017 Win64" ..
```

`Embree`, `OIDN` and `glfw` will be automatically downloaded during the first build.

**Full-featured Building on *nix**

```shell
git clone --recursive --depth=1 https://github.com/AirGuanZ/Atrc
cd Atrc
mkdir build
cd build
export CC=clang
export CXX=clang++
cmake -DUSE_EMBREE=ON -DUSE_OIDN=ON -DBUILD_MATERIAL_EXPLORER=ON -G "Unix Makefiles" ..
```

## Usage

### Components

1. CLI, command-line interface of the off-line renderer
2. Tracer, off-line rendering library based on ray tracing
4. material_explorer, GPU-accelerated material explorer
5. CLI_RAS, renderer with rasterization pipeline (under development)

### CLI Usage

Use following command to print help information:

```shell
CLI --help
```

Typical usage looks like:

```shell
CLI -d render_config.json
```

in which `scene_config.json` is a configuration file describing scene information and rendering settings.

## Configuration

Atrc uses JSON to describe scene and rendering settings. The input JSON file must contains two parts:

```json
{
    "scene": {
        ...
    },
    "rendering": {
        ...
    }
}
```

where `scene` is a scene object with type `Scene` (interrupted later). `rendering` specifies rendering settings like camera, global illumination algorithm, progress reporter, and post processors.

![metal_sphere](./pictures/metal_sphere.png)

Here is a simple example, which results in the above image (a bit rough metal sphere):

```json
{
  "scene": {
    "type": "default",
    "entities": [
      {
        "type": "geometric",
        "geometry": {
          "type": "sphere",
          "radius": 1.6,
          "transform": []
        },
        "material": {
          "type": "disney",
          "base_color": {
            "type": "constant",
            "texel": [ 0.7 ]
          },
          "metallic": {
            "type": "constant",
            "texel": [ 1 ]
          },
          "roughness": {
            "type": "constant",
            "texel": [ 0.15 ]
          }
        }
      }
    ],
    "lights": [
      {
        "type": "ibl",
        "tex": {
          "type": "hdr",
          "filename": "${scene-directory}/gray_pier_4k.hdr"
        }
      }
    ]
  },
  "rendering": {
    "camera": {
      "type": "thin_lens",
      "pos": [ 0, -5, 1 ],
      "dst": [ 0, 0, 0 ],
      "up": [ 0, 0, 1 ],
      "fov": 64
    },
    "film": {
      "type": "native",
      "width": 640,
      "height": 640
    },
    "renderer": {
      "type": "pt",
      "worker_count": -1,
      "task_grid_size": 16,
      "integrator": {
        "type": "native"
      },
      "sampler": {
        "type": "native",
        "spp": 100
      }
    },
    "reporter": {
      "type": "stdout"
    },
    "post_processors": [
      {
        "type": "flip",
        "vertically": true
      },
      {
        "type": "save_to_img",
        "filename": "${scene-directory}/output.png",
        "inv_gamma": 2.2
      }
    ]
  }
}
```

Atrc's configuration file consists of a series of nested JSON objects, and each field has its type and value. The syntax of some common field types is listed below:

| field type | example                     | semantics                             |
| ---------- | --------------------------- | ------------------------------------- |
| int        | -1                          | integer `1`                           |
| real       | 1.2                         | floating number `1.2`                 |
| bool       | true                        | boolean value `true`                  |
| bool       | false                       | boolean value `false`                 |
| string     | "minecraft"                 | string `minecraft`                    |
| Spectrum   | [ 0.5 ]                     | RGB value `(0.5, 0.5, 0.5)`           |
| Spectrum   | [ 0.1, 0.2, 0.3 ]           | RGB value `(0.1, 0.2, 0.3)`           |
| Vec2       | [ 2 ]                       | 2D vector `(2, 2)`                    |
| Vec2       | [ 1.2, 2 ]                  | 2D vector `(1.2, 2)`                  |
| Vec3       | [ 9.9 ]                     | 3D vector `(9.9, 9.9, 9.9)`           |
| Vec3       | [ 1, 2.2, 3 ]               | 3D vector `(1, 2.2, 3)`               |
| Vec2i      | [ 1 ]                       | 2D integer vector `(1, 1)`            |
| Vec2i      | [ 1, 2 ]                    | 2D integer vector `(1, 2)`            |
| Vec3i      | [ 2 ]                       | 3D integer vector `(2, 2, 2)`         |
| Vec3i      | [ 3, 4, -6 ]                | 3D integer vector `(3, 4, -6)`        |
| [Type]     | [Instance0, Instance1, ...] | JSON array of objects with given Type |

When a string represents a path or file name, `${scene-directory}` represents the absolute path of the directory where the configuration file is located, and `${working-directory}` represents the absolute path of the working directory of the current executable program. Take `${scene-directory}/output.png` at the end of the above example configuration file as an example, it represents the `output.png` file in the directory where the configuration file is located.

Except for above types of fields, other fields appear in the syntax of a JSON object. Each object contains a field `type`, representing its type in Atrc renderer.

In the following description, I will use a simplified tabular form to indicate which fields an object should contain and the meaning of these fields. Taking the last post-processor `save_to_png` in the above configuration file as an example, its JSON representation is:

```json
{
	"type": "save_to_img",
	"filename": "${scene-directory}/output.png",
	"inv_gamma": 2.2
}
```

The corresponding tabular form is:

| Field Name         | Type   | Default Value | Explanation                                            |
| ------------------ | ------ | ------------- | ------------------------------------------------------ |
| filename           | string |               | where to save the output image                         |
| open               | bool   | true          | whether to open it with the default image browser      |
| inv_gamma          | real   | 1             | $1/\gamma$ for gamma correction (typical value is 2.2) |
| with_alpha_channel | bool   | false         | save G-Buffer::binary to the alpha channel             |
| ext                | string | png           | saved file type ("jpg" or "png")                       |

In the form, the "default value" column is blank to indicate that this is a required field, and not blank to indicate that it is an optional field.

### Rendering Settings

This section describes fields included in the `rendering` item.

| Field Name      | Type             | Default Value | Explanation                      |
| --------------- | ---------------- | ------------- | -------------------------------- |
| camera          | Camera           |               | camera for viewing the scene     |
| film            | Film             |               | used to store rendering results  |
| renderer        | Renderer         |               | rendering algorithm              |
| reporter        | ProgressReporter |               | how to output rendering progress |
| post_processors | [PostProcessor]  | []            | image post processors            |

### Scene

This section describes the possible type values for fields of type `Scene`.

**default**

| Field Name | Type            | Default Value    | Explanation                                                  |
| ---------- | --------------- | ---------------- | ------------------------------------------------------------ |
| entities   | [Entity]        | []               | entities in scene                                            |
| lights     | [NonareaLight]  | []               | non-area lights in scene (ibl, for example)                  |
| aggregate  | EntityAggregate | native aggregate | data structure for accelerating ray queries between entities (default is a brute-force one) |
| env        | NonareaLight    | null             | environment light                                            |

`env` is deprecated and exists only for compatibility. It (if presented) will be automatically added to the `lights` array.

### EntityAggregate

For the intersection test between rays and the entities, Atrc uses a two-level data structure (refer to the design of RTX), where the inside of the entity is one level and the one between entities is another level. `EntityAggregate` is the field type corresponding to the data structure between entities.

**native**

`native` doesn't contain any fields, meaning to traversing all entities.

**bvh**

Organize entities with a BVH tree.

| Field Name    | Type | Default Value | Explanation                               |
| ------------- | ---- | ------------- | ----------------------------------------- |
| max_leaf_size | int  | 5             | How many entities a leaf node can contain |

### Camera

This section describes the possible type values for fields of type `Camera`.

**thin_lens**

| Field Name     | Type | Default Value | Explanation                                 |
| -------------- | ---- | ------------- | ------------------------------------------- |
| pos            | Vec3 |               | eye position                                |
| dst            | Vec3 |               | position looked by the camera               |
| up             | Vec3 |               | see 'lookat' matrix in 3D computer graphics |
| fov            | real |               | field of view (in degree)                   |
| lens_radius    | real | 0             | lens radius (for DoF effect)                |
| focal_distance | real | 1             | distance between focal plane and lens       |

The rendering result of the `thin_lens` camera is upside down, so it usually works with a post processor of type `flip`.

### Entity

**diffuse**

![diffuse_light](./pictures/diffuse_light.png)

Geometric diffuse light source, that is, a light source with a geometric shape. Each point on the surface emits the same radiance in all directions.

| Field Name | Type  | Default Value | Explanation |
| -------- | -------- | ------ | -------- |
| geometry | Geometry |        | geometric shape |
| radiance | Spectrum |        | emitted radiance |
| med_in   | Medium   | void   | outer medium |
| med_out  | Medium   | void   | inner medium |
| no_denoise | bool | false | disable denoiser on this entity |

**geometric**

![geometric](./pictures/geometric_entity.png)

Ordinary entity with geometry shape, material and mediums.

| Field Name     | Type     | Default Value | Explanation                |
| -------------- | -------- | ------------- | -------------------------- |
| geometry       | Geometry |               | geometric shape            |
| material       | Material |               | surface material           |
| med_in         | Medium   | void          | outer medium               |
| med_out        | Medium   | void          | inner medium               |
| shadow_catcher | bool     | false         | marked as a shadow catcher |

### FilmFilter

The process of rendering the image is also a process of sampling-reconstructing the image plane. `FilmFilter` is the filter function used for reconstruction. For details, please refer to [this article](http://alvyray.com/Memos/CG/Microsoft/6_pixel. pdf). If you don't know much about this part, it is recommended to use the simplest (but not the best) reconstruction filter function:

```json
"type": "box",
"radius": 0.5
```

**box**

The simplest box filter function, coincides with a single pixel with a radius of 0.5.

| Field Name | Type | Default Value | Explanation             |
| ---------- | ---- | ------------- | ----------------------- |
| radius     | real |               | filter radius in pixels |

**gaussian**

Gaussian filter function

| Field Name | Type | Default Value | Explanation                   |
| ---------- | ---- | ------------- | ----------------------------- |
| radius     | real |               | filter radius in pixels       |
| alpha      | real |               | $\alpha$ in gaussian function |

### Film

**native**

The most basic and fastest film which doesn't support filter function other than box.

| Field Name | Type | Default Value | Explanation            |
| ---------- | ---- | ------------- | ---------------------- |
| width      | int  |               | image width in pixels  |
| height     | int  |               | image height in pixels |

**filtered**

Film supports various types of filter function.

| Field Name | Type       | Default Value | Explanation                            |
| ---------- | ---------- | ------------- | -------------------------------------- |
| width      | int        |               | image width in pixels                  |
| height     | int        |               | image height in pixels                 |
| filter     | FilmFilter |               | filter function for constructing image |

### Fresnel

Fresnel term in surface material BSDF

**always_one**

Fresnel term whose value is always 1.

**dielectric**

Fresnel term for dielectric

| Field Name | Type      | Default Value | Explanation                     |
| ---------- | --------- | ------------- | ------------------------------- |
| eta_out    | Texture2D | all_one       | outer ior (index of refraction) |
| eta_in     | Texture2D |               | inner ior                       |

**conductor**

Fresnel term of conductor

| Field Name | Type      | Default Value | Explanation         |
| ---------- | --------- | ------------- | ------------------- |
| eta_out    | Texture2D | all_one       | outer ior           |
| eta_in     | Texture2D |               | inner ior           |
| k          | Texture2D |               | index of absorption |

### Geometry

Used to describe the geometry of an object.

Most geometric shapes contain a `transform` field of type `[Transform] `, which is a sequence of `Transform`. It is worth noting that the `Transform` in the back of the sequence acts on the object first, and the `Transform` in front of the sequence acts on the object later.

**disk**

![pic](./pictures/disk.png)

| Field Name | Type        | Default Value | Explanation                               |
| ---------- | ----------- | ------------- | ----------------------------------------- |
| transform  | [Transform] |               | transform from local space to world space |
| radius     | real        |               | disk radius                               |

**double_sided**

Double-sided adapter, which converts a single-sided geometry to double-sided, and is mainly suitable for geometric shape in the form of disc, quad or triangle.

| Field Name | Type     | Default Value | Explanation                                       |
| ---------- | -------- | ------------- | ------------------------------------------------- |
| internal   | Geometry |               | geometry turned from single-sided to double-sided |

**quad**

![pic](./pictures/quad.png)

Quad $ABCD$, constructed by two triangles $ABC$ and $ACD$

| Field Name | Type        | Default Value | Explanation                               |
| ---------- | ----------- | ------------- | ----------------------------------------- |
| transform  | [Transform] |               | transform from local space to world space |
| A          | Vec3        |               | position of vertex $A$                    |
| B          | Vec3        |               | position of vertex $B$                    |
| C          | Vec3        |               | position of vertex $C$                    |
| D          | Vec3        |               | position of vertex $D$                    |
| tA         | Vec2        | (0, 0)        | texture coordinate of vertex $A$          |
| tB         | Vec2        | (0, 0)        | texture coordinate of vertex $B$          |
| tC         | Vec2        | (0, 0)        | texture coordinate of vertex $C$          |
| tD         | Vec2        | (0, 0)        | texture coordinate of vertex $D$          |

**sphere**

![pic](./pictures/sphere.png)

Single-sided sphere

| Field Name | Type        | Default Value | Explanation                               |
| ---------- | ----------- | ------------- | ----------------------------------------- |
| transform  | [Transform] |               | transform from local space to world space |
| radius     | real        |               | sphere radius                             |

**triangle**

![pic](./pictures/triangle.png)

Single-sided triangle

| Field Name | Type        | Default Value | Explanation                               |
| ---------- | ----------- | ------------- | ----------------------------------------- |
| transform  | [Transform] |               | transform from local space to world space |
| A          | Vec3        |               | position of vertex $A$                    |
| B          | Vec3        |               | position of vertex $B$                    |
| C          | Vec3        |               | position of vertex $C$                    |
| tA         | Vec2        | (0, 0)        | texture coordinate of vertex $A$          |
| tB         | Vec2        | (0, 0)        | texture coordinate of vertex $B$          |
| tC         | Vec2        | (0, 0)        | texture coordinate of vertex $C$          |

**triangle_bvh**

![pic](./pictures/triangle_bvh.png)

Triangle mesh

Embree is used when enabled, otherwise a simple BVH tree is used.

| Field Name | Type        | Default Value | Explanation                               |
| ---------- | ----------- | ------------- | ----------------------------------------- |
| transform  | [Transform] |               | transform from local space to world space |
| filename   | string      |               | model file path, supports OBJ/STL file    |

**triangle_bvh_embree**

Triangle mesh implemented using Embree. It has the same parameters as `triangle_bvh`.

Available only when cmake option `USE_EMBREE` is `ON`.

**triangle_bvh_noembree**

Triangle mesh implemented using simple BVH tree. It has the same parameters as `triangle_bvh`.

### Material

**Normal Mapping**

![pic](C:/Users/lenovo/Documents/Programming/Code/agz/doc/pictures/normal_mapping.png)

Some materials support normal mapping. The field list of these materials will include a `normal_map`.

**disney_reflection (deprecated)**

Complete implementation of [Disney Principled BRDF](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf)

| Field Name      | Type      | Default Value   | Explanation                                                  |
| --------------- | --------- | --------------- | ------------------------------------------------------------ |
| base_color      | Texture2D |                 | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| metallic        | Texture2D |                 | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| roughness       | Texture2D |                 | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| subsurface      | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| specular        | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| specular_tint   | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| anisotropic     | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| sheen           | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| sheen_tint      | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| clearcoat       | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| clearcoat_gloss | Texture2D | all_one         | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf) |
| normal_map      | Texture2D | all_{ 0, 0, 1 } | normal map                                                   |

*NOTE*: `disney_reflection` is basically a subset of `disney` and is already deprecated, so the latter is recommended.

**disney**

![pic](./pictures/disney_bsdf.png)

see [Disney Principled BSDF](https://blog.selfshadow.com/publications/s2015-shading-course/#course_content)

| Field Name             | Type      | Default Value   | Explanation                                                  |
| ---------------------- | --------- | --------------- | ------------------------------------------------------------ |
| base_color             | Texture2D |                 | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0, 1]^3$ |
| metallic               | Texture2D |                 | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| roughness              | Texture2D |                 | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| specular_scale         | Texture2D | all_one         | scale factor of non-metallic specular lobe; range: $[0,1]^3$ |
| specular_tint          | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| anisotropic            | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| sheen                  | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| sheen_tint             | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| clearcoat              | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| clearcoat_gloss        | Texture2D | all_one         | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| transmission           | Texture2D | all_zero        | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,1]$ |
| transmission_roughness | Texture2D | roughness       | roughness of transmitted specular lobe; range: $[0, 1]$      |
| ior                    | Texture2D | all_{1.5}       | see [original article](https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf); range: $[0,\infty)$ |
| normal_map             | Texture2D | all_{ 0, 0, 1 } | normal map                                                   |

**frosted_glass**

![pic](./pictures/frosted_glass.png)

Frosted glass material. Please refer to [Microfacet Models for Refraction through Rough Surfaces](https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html).

| Field Name | Type      | Default Value | Explanation       |
| ---------- | --------- | ------------- | ----------------- |
| color_map  | Texture2D |               | surface color     |
| fresnel    | Fresnel   |               | fresnel term      |
| roughness  | Texture2D |               | surface roughness |

**glass**

![pic](./pictures/glass.png)

Smooth glass

| Field Name           | Type      | Default Value | Explanation      |
| -------------------- | --------- | ------------- | ---------------- |
| color_map            | Texture2D |               | surface color    |
| color_reflection_map | Texture2D |               | reflection color |
| color_refraction_map | Texture2D |               | refraction color |
| fresnel              | Fresnel   |               | fresnel term     |

Two color assignment methods are provided:

1. Only provide `color_map`. Colors of reflection and refraction are given automatically filled with `color_map`.
2. Provide `color_reflection_map` and `color_refraction_map`, which give reflection and refraction colors respectively.

**ideal_black**

A mass of black

**ideal_diffuse**

![pic](./pictures/ideal_diffuse.png)

Ideal diffuse reflection, reflects the same radiance in all directions.

| Field Name | Type      | Default Value   | Explanation               |
| ---------- | --------- | --------------- | ------------------------- |
| albedo     | Texture2D |                 | surface color times $\pi$ |
| normal_map | Texture2D | all_{ 0, 0, 1 } | normal map                |

**mirror**

![pic](./pictures/mirror.png)

Ideal mirror reflection

| Field Name | Type  | Default Value | Explanation |
| ------- | ------- | ------ | --------- |
| color_map  | Texture2D |        | surface color |
| fresnel | Fresnel |        | fresnel term |

**mtl**

Standard reflection model, constructed with a diffuse component and a specular component.

| Field Name | Type      | Default Value | Explanation                           |
| ---------- | --------- | ------------- | ------------------------------------- |
| kd         | Texture2D |               | strength of diffuse component         |
| ks         | Fresnel   |               | strength of specular component        |
| ns         | Texture2D |               | specular gloss; range: $(0, +\infty)$ |

**add**

![pic](./pictures/adder.png)

Combine multiple materials together. The third image in the figure above is the effect obtained by summing the object materials in the first two images with a weight of $ 1/2 $.

| Field Name | Type       | Default Value | Explanation             |
| ---------- | ---------- | ------------- | ----------------------- |
| mats       | [Material] |               | combined material array |

**scale**

Scales the reflection/refraction component of a material linearly.

| Field Name | Type      | Default Value | Explanation     |
| ---------- | --------- | ------------- | --------------- |
| internal   | Material  |               | scaled material |
| scale      | Texture2D |               | scaling ratio   |

**mirror_varnish**

![pic](./pictures/mirror_varnish.png)

Absolutely smooth varnish, where the interior material must be opaque.

| Field Name | Type      | Default Value | Explanation                      |
| ---------- | --------- | ------------- | -------------------------------- |
| internal   | Material  |               | material under the varnish layer |
| eta_in     | Texture2D |               | inner ior of varnish             |
| eta_out    | Texture2D | all_one       | outer ior of varnish             |
| color      | Texture2D |               | varnish color                    |

**invisible_surface**

Surface material which is totally invisible

### Medium

**void**

Vacuum, which means nothing.

**absorb**

Non-scattering medium.

| Field Name | Type     | Default Value | Explanation     |
| ---------- | -------- | ------------- | --------------- |
| sigma_a    | Spectrum |               | absorption rate |

**heterogeneous**

![pic](C:/Users/lenovo/Documents/Programming/Code/agz/doc/pictures/heterogeneous_medium.png)

Heterogeneous media defined based on 3D textures

| Field Name | Type        | Default Value | Explanation                                     |
| ---------- | ----------- | ------------- | ----------------------------------------------- |
| transform  | [Transform] |               | from texture space ($[0,1]^3$) to world space   |
| density    | Texture3D   |               | medium density, i.e. $\sigma_s + \sigma_a$      |
| albedo     | Texture3D   |               | albedo, i.e. $\sigma_s / (\sigma_s + \sigma_a)$ |
| g          | Texture3D   |               | asymmetry of scattering                         |

**homogeneous**

![pic](./pictures/homogeneous.png)

| Field Name | Type     | Default Value | Explanation             |
| ---------- | -------- | ------------- | ----------------------- |
| sigma_a    | Spectrum |               | absorption rate         |
| sigma_s    | Spectrum |               | scattering rate         |
| g          | real     |               | asymmetry of scattering |

### NonareaLight

**ibl**

![pic](./pictures/env_light.png)

Image based lighting

| Field Name | Type      | Default Value | Explanation                                 |
| ---------- | --------- | ------------- | ------------------------------------------- |
| tex        | Texture2D |               | texture object describing radiance          |
| up         | Vec3      | [ 0, 0, 1 ]   | which direction is "above", default is $+z$ |

**hdri**

![pic](./pictures/hdri.png)

Inner glowing sphere which is not visible on the outer side

| Field Name | Type      | Default Value | Explanation                                 |
| ---------- | --------- | ------------- | ------------------------------------------- |
| tex        | Texture2D |               | texture object describing radiance          |
| up         | Vec3      | [ 0, 0, 1 ]   | which direction is "above", default is $+z$ |
| radius     | real      | 100           | sphere radius                               |
| offset     | Vec3      | [0]           | sphere center position                      |

*NOTE*. The `hdri` light source does not support light transport algorithms requiring backward tracing at the moment. It can only be used in path tracing.

**native_sky**

![pic](./pictures/native_sky.png)

Environment light that represents a sky with a gradient of color from top to bottom.

| Field Name | Type     | Default Value | Explanation                                                  |
| ---------- | -------- | ------------- | ------------------------------------------------------------ |
| top        | Spectrum |               | top radiance                                                 |
| bottom     | Spectrum |               | bottom radiance                                              |
| up         | Vec3     | [ 0, 0, 1 ]   | describe which direction in the world space is "above", default is $+z$ |

### Post Processor

**flip**

Flip the image

| Field Name   | Type | Default Value | Explanation                            |
| ------------ | ---- | ------------- | -------------------------------------- |
| vertically   | bool | false         | whether to flip the image vertically   |
| horizontally | bool | false         | whether to flip the image horizontally |

**gamma**

Perform gamma correction on the image

| Field Name | Type | Default Value | Explanation |
| ---------- | ---- | ------------- | ----------- |
| gamma      | real |               | $\gamma$    |
| inv_gamma  | real |               | $1/\gamma$  |

Only one of `gamma` and `inv_gamma` need to be given.

**oidn_denoiser**

Use OIDN to denoise the image

| Field Name | Type | Default Value | Explanation                                                  |
| ---------- | ---- | ------------- | ------------------------------------------------------------ |
| clamp      | bool | false         | whether to clamp the image color to $[0, 1]^3$ before denoising |

**save_gbuffer_to_png**

![pic](./pictures/gbuffer.png)

Save the G-Buffer to png files

| Field Name | Type   | Default Value | Explanation                   |
| ---------- | ------ | ------------- | ----------------------------- |
| albedo     | string | ""            | where to save material colors |
| normal     | string | ""            | where to save normal image    |
| depth      | string | ""            | where to save depth image     |

**save_to_img**

Save the rendered image to a file

| Field Name | Type   | Default Value | Explanation                                            |
| ---------- | ------ | ------------- | ------------------------------------------------------ |
| filename   | string |               | where to save the output image                         |
| open       | bool   | true          | whether to open it with the default image browser      |
| inv_gamma  | real   | 1             | $1/\gamma$ for gamma correction (typical value is 2.2) |
| ext        | string | png           | saved file type ("jpg" or "png")                       |

**resize**

Resize the image and G-Buffer to the specified resolution

| Field Name | Type       | Default Value | Explanation       |
| ---------- | ---------- | ------------- | ----------------- |
| size       | [int, int] |               | target resolution |

### Renderer

**pt**

Traditional path tracing. You can specify the tracing strategy by `integrator`.

| Field Name     | Type                  | Default Value | Explanation               |
| -------------- | --------------------- | ------------- | ------------------------- |
| integrator     | PathTracingIntegrator |               | tracing strategy          |
| task_grid_size | int                   | 32            | rendering task pixel size |
| worker_count   | int                   | 0             | rendering thread count    |
| sampler        | Sampler               |               | random number generator   |

The entire image is divided into multiple square pixel blocks (rendering tasks), and each pixel block is assigned to a worker thread for execution as a subtask.

When the number of worker threads $n$ is less or equal to 0 and the number of hardware threads is $ k $, then $\max\{1, k + n \} $ worker threads will be used. For example, you can set `worker_count` to -2, which means that you leave two hardware threads and use all other hardware threads.

**particle**

Adjoint particle tracer. `particle` builds path from light source to camera, making the convergence very slow.

| Field Name             | Type    | Default Value | Explanation                               |
| ---------------------- | ------- | ------------- | ----------------------------------------- |
| worker_count           | int     | 0             | rendering thread count                    |
| particle_task_count    | int     |               | particle tracing task count               |
| backward_sampler       | Sampler |               | sampler used in backward pass             |
| min_depth              | int     | 5             | min path depth before using RR policy     |
| max_depth              | int     | 10            | max depth of the path                     |
| cont_prob              | real    | 0.9           | pass probability when using RR strategy   |
| forward_task_grid_size | int     | 32            | rendering task pixel size in forward pass |
| forward_sampler        | Sampler |               | sampler used in forward pass              |

`particle` uses the strategy of starting from a light source to construct a light path, called backward pass; for paths of length 1 (that is, the light source is directly seen from the camera), however, `particle` builds them from the camera to light sources, called forward pass. The two passes are independent executed and are combined to render the final image.

Backward pass consists of `particle_task_count` particle tracing tasks. Each task contains `spp` of `backward_sampler`, so a total of `particle_task_count * backward_sampler.spp` paths are traced in backward pass.

### PathTracingIntegrator

**native**

Violent path tracing without any optimization

| Field Name | Type | Default Value | Explanation                               |
| ---------- | ---- | ------------- | ----------------------------------------- |
| min_depth  | int  | 5             | minimum path depth before using RR policy |
| max_depth  | int  | 10            | maximum depth of the path                 |
| cont_prob  | real | 0.9           | pass probability when using RR strategy   |

**mis**

Path tracing with Multiple Importance Sampling

| Field Name | Type | Default Value | Explanation                               |
| ---------- | ---- | ------------- | ----------------------------------------- |
| min_depth  | int  | 5             | minimum path depth before using RR policy |
| max_depth  | int  | 10            | maximum depth of the path                 |
| cont_prob  | real | 0.9           | pass probability when using RR strategy   |

### ProgressReporter

**stdout**

Print to standard output

**noout**

No progress output

### Sampler

Random number generator

**native**

The most basic random number sampler, where all samples are independent

| Field Name | Type | Default Value | Explanation                                  |
| ---------- | ---- | ------------- | -------------------------------------------- |
| seed       | int  | by time       | Seed value, default seed is the running time |
| spp        | int  |               | samples per pixel                            |

### Texture2D

![pic](./pictures/texture.png)

All 2d textures contain the following fields (these fields are not listed in the subsequent textures):

| Field Name | Type         | Default Value | Explanation                                                  |
| ---------- | ------------ | ------------- | ------------------------------------------------------------ |
| inv_v      | bool         | false         | turn $v$ to $1 - v$ (flip the texture vertically)            |
| inv_u      | bool         | false         | turn $u$ to $1 - u$ (flip the texture horizontally)          |
| swap_uv    | bool         | false         | swap $u$ and $v$                                             |
| transform  | [Transform2] | []            | transform sequence applied to the texture coordinate         |
| wrap_u     | string       | "clamp"       | ways to deal with $u$ that is out of $[0, 1]$; range: clamp/mirror/repeat |
| wrap_v     | string       | "clamp"       | ways to deal with $v$ that is out of $[0, 1]$; range: clamp/mirror/repeat |
| inv_gamma  | real         | 1             | used to perform inverse gamma correction to the texture; typical value is 2.2 |

Note that `inv_v, inv_u, swap_uv` and `transform` are all transformations to uv, where `transform` applies first, then `swap_uv` , and `inv_u, inv_v` applies last. In the `transform` sequence, the `Transform2` in the back of the sequence applies first, and the `Transform2` in the front of the sequence applies later.

**checker_board**

checker board texture

| Field Name | Type     | Default Value | Explanation                         |
| ---------- | -------- | ------------- | ----------------------------------- |
| grid_count | real     |               | how many grids the texture contains |
| color1     | Spectrum | [ 0 ]         | the first grid color                |
| color2     | Spectrum | [ 1 ]         | the second grid color               |

**constant**

Constant-valued texture, that is, sampling always results in the same value

| Field Name | Type     | Default Value | Explanation |
| ---------- | -------- | ------------- | ----------- |
| texel      | Spectrum |               | texel value |

**hdr**

Texture loaded from `.hdr` file

| Field Name | Type   | Default Value | Explanation                              |
| ---------- | ------ | ------------- | ---------------------------------------- |
| filename   | string |               | `.hdr` filename                          |
| sample     | string | "linear"      | sampling strategy; range: linear/nearest |

**image**

Textures loaded from common image file formats (`.bmp, .jpg, .png, .tga`, etc)

| Field Name | Type   | Default Value | Explanation                              |
| ---------- | ------ | ------------- | ---------------------------------------- |
| filename   | string |               | image filename                           |
| sample     | string | "linear"      | sampling strategy; range: linear/nearest |

**scale**

A wrapper that linearly scales another texture

| Field Name | Type      | Default Value | Explanation    |
| ---------- | --------- | ------------- | -------------- |
| scale      | Spectrum  |               | scaling ratio  |
| internal   | Texture2D |               | scaled texture |

**gradient**

Linear gradient texture along the `u` direction. You can achieve gradients in other directions through texture coordinate transformation.

| Field Name | Type     | Default Value | Explanation    |
| ---------- | -------- | ------------- | -------------- |
| color1     | Spectrum |               | color at $u=0$ |
| color2     | Spectrum |               | color at $u=1$ |

**add**

Add two textures together

| Field Name | Type      | Default Value | Explanation               |
| ---------- | --------- | ------------- | ------------------------- |
| lhs        | Texture2D |               | the first added Texture2D |
| rhs        | Texture2D |               | the second added texture  |

**mul**

Multiply two textures together

| Field Name | Type      | Default Value | Explanation                   |
| ---------- | --------- | ------------- | ----------------------------- |
| lhs        | Texture2D |               | the first multiplied texture  |
| rhs        | Texture2D |               | the second multiplied texture |

**lum_classify**

Use $T(u, v)$ to represent the result of sampling texture $T$ at $(u, v)$, then `lum_classify` is defined as:
$$
\mathrm{lum\_classify}(u, v) := \begin{cases}\begin{aligned}
    &\mathrm{high}(u,v), &\mathrm{internal}(u, v) \ge \mathrm{threshold}(u, v) \\
    &\mathrm{low}(u, v), &\text{otherwise}
\end{aligned}\end{cases}
$$

| Field Name | Type      | Default Value | Explanation |
| ---------- | --------- | ------------- | ----------- |
| internal   | Texture2D |               |             |
| threshold  | Texture2D |               |             |
| high       | Texture2D |               |             |
| low        | Texture2D |               |             |

**reverse**

Invert the color of a texture

| Field Name | Type      | Default Value | Explanation      |
| ---------- | --------- | ------------- | ---------------- |
| internal   | Texture2D |               | inverted texture |

$$
\mathrm{reverse}(u, v) := \mathrm{clamp}(1 - \mathrm{internal}(u, v), 0, 1)
$$

### Texture3D

All 3d textures contain the following fields (these fields are not listed in the subsequent textures):

| Field Name | Type         | Default Value | Explanation                                                  |
| ---------- | ------------ | ------------- | ------------------------------------------------------------ |
| inv_v      | bool         | false         | turn $v$ to $1 - v$                                          |
| inv_u      | bool         | false         | turn $u$ to $1 - u$                                          |
| inv_w      | bool         | false         | turn $w$ to $1 - w$                                          |
| uvw_perm   | Vec3i        | [ 0, 1, 2 ]   | permutation of uvw                                           |
| transform  | [Transform3] | []            | transform sequence applied to the texture coordinate         |
| wrap_u     | string       | "clamp"       | wrapping method to deal with $u$ that is out of $[0, 1]$; range: clamp/mirror/repeat |
| wrap_v     | string       | "clamp"       | wrapping method to deal with $v$ that is out of $[0, 1]$; range: clamp/mirror/repeat |
| wrap_w     | string       | "clamp"       | wrapping method to deal with $w$ that is out of $[0, 1]$; range: clamp/mirror/repeat |
| inv_gamma  | real         | 1             | used to perform inverse gamma correction to the texture; typical value is 2.2 |

Note that `inv_v, inv_u, inv_w, uvw_perm` and `transform` are all transformations to uv, where `transform` applies first, then `uvw_perm` , and `inv_u, inv_v, inv_w` applies last. In the `transform` sequence, the `Transform3` in the back of the sequence applies first, and the `Transform3` in the front of the sequence applies later.

**constant**

Constant-valued texture, that is, sampling always results in the same value

| Field Name | Type     | Default Value | Explanation |
| ---------- | -------- | ------------- | ----------- |
| texel      | Spectrum |               | texel value |

**gray_grid**

3D gray-scale grids. the value between grid points is obtained by trilinear interpolation

| Field Name      | Type     | Default Value | Explanation                              |
| --------------- | -------- | ------------- | ---------------------------------------- |
| ascii_filename  | string   |               | file name of voxel data in text format   |
| binary_filename | string   |               | file name of voxel data in binary format |
| image_filenames | [string] |               | array of filenames for image slices      |

There are three ways to provide voxel data to `gray_grid`, so you only need to fill in one of the three fields in the table above.

The format of text voxel data is:

```
int32 value (texture width)
int32 value (texture height)
int32 value (texture depth)
for z in 0 to texture depth
    for y in 0 to texture height
        for x in 0 to texture width
            float value (voxel value at { x, y, z })
```

The data arrangement of binary voxel data is similar to the text format, except that all data is stored as binary data in little-endian order.

The file name array of image slices refers to the file names of a series of two-dimensional images obtained by decomposing the voxels in the depth direction. These two-dimensional images must be the same size, the number of which corresponds to the depth value of the texture.

**spectrum_grid**

3D RGB grids. the value between grid points is obtained by trilinear interpolation

| Field Name      | Type     | Default | Explanation                              |
| --------------- | -------- | ------- | ---------------------------------------- |
| ascii_filename  | string   |         | file name of voxel data in text format   |
| binary_filename | string   |         | file name of voxel data in binary format |
| image_filenames | [string] |         | array of filenames for image slices      |

There are three ways to provide voxel data to `spectrum_grid`, so you only need to fill in one of the three fields in the table above.

The format of text voxel data is:

```
int32 value (texture width)
int32 value (texture height)
int32 value (texture depth)
for z in 0 to texture depth
    for y in 0 to texture height
        for x in 0 to texture width
            float value (r value at { x, y, z })
            float value (g value at { x, y, z })
            float value (b value at { x, y, z })
```

The data arrangement of binary voxel data is similar to the text format, except that all data is stored as binary data in little-endian order.

The file name array of image slices refers to the file names of a series of two-dimensional images obtained by decomposing the voxels in the depth direction. These two-dimensional images must be the same size, the number of which corresponds to the depth value of the texture.

**add**

Add two textures together

| Field Name | Type      | Default Value | Explanation               |
| ---------- | --------- | ------------- | ------------------------- |
| lhs        | Texture3D |               | the first added Texture2D |
| rhs        | Texture3D |               | the second added texture  |

**mul**

Multiply two textures together

| Field Name | Type      | Default Value | Explanation                   |
| ---------- | --------- | ------------- | ----------------------------- |
| lhs        | Texture3D |               | the first multiplied texture  |
| rhs        | Texture3D |               | the second multiplied texture |

**scale**

A wrapper that linearly scales another texture

| Field Name | Type      | Default Value | Explanation    |
| ---------- | --------- | ------------- | -------------- |
| scale      | Spectrum  |               | scaling ratio  |
| internal   | Texture3D |               | scaled texture |

**lum_classify**

| Field Name    | Type      | Default Value | Explanation |
| ------------- | --------- | ------------- | ----------- |
| lhs           | Texture3D |               |             |
| rhs           | Texture3D |               |             |
| less_or_equal | Texture3D |               |             |
| greater       | Texture3D |               |             |

Use $T(u, v)$ to represent the result of sampling texture $T$ at $(u, v)$, then `lum_classify` is defined as:
$$
\mathrm{lum\_classify}(u, v) := \begin{cases}\begin{aligned}    &\mathrm{less\_or\_equal}(u,v), &\mathrm{lhs}(u, v) \ge \mathrm{rhs}(u, v) \\    &\mathrm{greater}(u, v), &\text{otherwise}\end{aligned}\end{cases}
$$

### Transform

Affine transformation on three-dimensional coordinates

**translate**

| Field Name | Type | Default Value | Explanation      |
| ---------- | ---- | ------------- | ---------------- |
| offset     | Vec3 |               | transtion offset |

**rotate**

| Field Name | Type | Default Value | Explanation     |
| ---------- | ---- | ------------- | --------------- |
| axis       | Vec3 |               | rotation axis   |
| rad        | real |               | rotation radian |
| deg        | real |               | rotation degree |

Only one of `rad` and `deg` can be provided.

**rotate_x**

Rotate around the $x$ axis.

| Field Name | Type | Default Value | Explanation     |
| ---------- | ---- | ------------- | --------------- |
| rad        | real |               | rotation radian |
| deg        | real |               | rotation degree |

Only one of `rad` and `deg` can be provided.

**rotate_y**

Rotate around the $y$ axis.

| Field Name | Type | Default Value | Explanation     |
| ---------- | ---- | ------------- | --------------- |
| rad        | real |               | rotation radian |
| deg        | real |               | rotation degree |

Only one of `rad` and `deg` can be provided.

**rotate_z**

Rotate around the $z$ axis.

| Field Name | Type | Default Value | Explanation     |
| ---------- | ---- | ------------- | --------------- |
| rad        | real |               | rotation radian |
| deg        | real |               | rotation degree |

Only one of `rad` and `deg` can be provided.

**scale**

Isotropic scaling

| Field Name | Type | Default Value | Explanation   |
| ---------- | ---- | ------------- | ------------- |
| ratio      | real |               | scaling ratio |

### Transform2

Affine transformation on two-dimensional coordinates

**translate**

| Field Name | Type | Default Value | Explanation        |
| ---------- | ---- | ------------- | ------------------ |
| offset     | Vec2 |               | translation offset |

**rotate**

| Field Name | Type | Default Value | Explanation     |
| ---------- | ---- | ------------- | --------------- |
| rad        | real |               | rotation radian |
| deg        | real |               | rotation degree |

Only one of `rad` and `deg` can be provided.

**scale**

Isotropic scaling

| Field Name | Type | Default Value | Explanation   |
| ---------- | ---- | ------------- | ------------- |
| ratio      | real |               | scaling ratio |

## Shared Scene Description

Atrc supports rendering the same scene with different rendering parameters, such as rendering the same scene with multiple camera perspectives. Compared with using multiple configuration files, this solution avoids the overhead of repeatedly loading scene data and constructing data structures. Let the original content of rendering configuration is:

```json
{
    "scene": { SceneDescription },
    "rendering": { RenderingDescription }
}
```

Now we need to use multiple different `RenderDescription` to render the same `SceneDescription`. We can write the configuration file as:

```json
{
    "scene": { SceneDescription },
    "rendering": [
        { RenderingDescription0 },
        { RenderingDescription1 },
        ...,
        { RenderingDescriptionN }
    ]
}
```


