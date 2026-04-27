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

== Gaussian splatting

- Latency reduced by 1

== Implementation details

#figure(
  image("assets/test.jpg", width: 80%),
  caption: [Vizualizace Gaussova rozostření v čase]
)