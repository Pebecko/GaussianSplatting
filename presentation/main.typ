#import "@preview/touying:0.5.3": *
#import themes.metropolis: *
#import "template.typ": my-taltech-theme

// Apply the custom wrapper
#show: my-taltech-theme.with(
  title: [Gaussian Splatting],
  subtitle: [Owerview of the method and implementation],
  author: [Matous Krobot, Ditmar Hadera],
)

// Generate the title slide
#title-slide()

== Usage 

- creating a 3D scene from a set of images

== Other methods

- NeRF
- Point-based rendering
- 3D reconstruction

== Advantages of Gaussian Splatting

- TODO

== Method overview

Trhee main steps:
0. Creating a pont cloud from a set of images (using Structure from Motion)
1. Converting the point cloud into a set of Gaussian splats and optimizing their parameters
2. Rendering the scene from novel viewpoints

== Gaussian optimization

TODO

== Rendering

TODO

== Demo