#import "@preview/touying:0.5.3": *
#import themes.metropolis: *
#import "template.typ": my-taltech-theme

// Apply the custom wrapper
#show: my-taltech-theme.with(
  title: [Gaussian Splatting],
  subtitle: [Overview of the method and implementation],
  author: [Matouš Krobot, Ditmar Hadera],
)

// Generate the title slide
#title-slide()

== Novel View Synthesis 

#figure(
  image("assets/novel_view_synthesis_generated.png", width: 100%)
)

== Other methods

#grid(
  rows: (1fr, 1fr), // Gap between columns

  [
    - *Photogrammetry* - reconstructing 3D mesh.

    - *Radiance Fields* - representing the scene as a voxel grid.

    - *Neural Radiance Fields* (NeRF) - radiance fields represented by a neural network
  ],
  
  [
    #grid(
  columns: (1fr, 1fr),
  gutter: 2em, // Gap between columns

  [
    #align(center + horizon)[
      #image("assets/bear_mesh.png", width: 100%)
    ]
  ],
  
  [
    #align(center + horizon)[
      #image("assets/radiance_field.png", width: 100%)
    ]
  ]
)
  ]
)


== What is a Gaussian Splat?

#grid(
  columns: (1fr, 1fr),
  gutter: 2em, // Gap between columns

  [
    *3D primitive defined by*:
    - Position (x, y, z)
    - Anisotropic covariance matrix
    - Color as spherical harmonics
    - Opacity
  ],
  
  [
    #align(center + horizon)[
      #image("assets/splat.png", width: 100%)
    ]
  ]
)

== Method Overview

*Comparable to training of Neural Networks*:

- *Initialization* - Camera poses and point cloud from Structure from Motion (SfM).

- *Forward pass* - Render the current set of Gaussian splats from the input viewpoints.

- *Backward pass* - Backpropagate the loss to update the parameters of the Gaussian splats.

== Point Cloud - Structure from Motion

#grid(
  columns: (1fr, 1fr),
  gutter: 2em, // Gap between columns

  [
    #align(center + horizon)[
      #image("assets/scene.png", width: 100%)
    ]
  ],
  
  [
    #align(center + horizon)[
      #image("assets/scene_cloud_point.png", width: 100%)
    ]
  ]
)

== Rendering


- *Sort the Gaussian splats* based on their *depth* from the camera.

- For each pixel:
 - loop through Gaussian splats *front-to-back*
  - project Gaussians *3D $->$ 2D*
  - sample *color* and *opacity* from the Gaussian
 - use *alpha blending* to combine the splat contributions




== Gaussian Optimization

#grid(
  columns: (1fr, 1fr),
  gutter: 2em, // Gap between columns

  [
    *Optimization iteration*:

    + *Render* Gaussian splats from input viewpoints.
    + *Compute loss* (MAE) between rendered and input images.
    + Backpropagate the loss to *update parameters* of the Gaussian splats.
     - Use *gradient descent*
    + Every $N$-th iteration:
      - *Prune* low-opacity splats
      - *Split* large splats
      - *Duplicate* splats in high-error regions
  ],
  
  [
    #align(center + horizon)[
      #image("assets/density_control.png")
    ]
  ]
)

== Comparison: Gaussian Splatting vs. NeRF

- *Real time* rendering
 - Optimal 3D primitives for graphics hardware
 - No Neural Network inference
- Better quality for same training time as NeRF
- Better handling of regions outside the training views
- Harder 3D mesh reconstruction

== Demo

#link(
  "https://github.com/Pebecko/GaussianSplatting"
)

