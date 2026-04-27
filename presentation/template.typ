#import "@preview/touying:0.5.3": *
#import themes.metropolis: *

// Define a custom wrapper function for reuse across multiple presentations
#let my-taltech-theme(
  title: "",
  subtitle: "",
  author: "",
  body
) = {
  // Enforce strong typing/validation where possible
  assert(title != "", message: "Title must strictly be provided.")
  
  // Initialize the Touying state with the metropolis theme
  show: metropolis-theme.with(
    aspect-ratio: "16-9",
    config-info(
      title: title,
      subtitle: subtitle,
      author: author,
      date: datetime.today(),
    ),
    config-colors(
      // Customize primary theme color to match your organization
      primary: rgb("#0055A4"), 
    )
  )

  // Inject the actual content
  body
}