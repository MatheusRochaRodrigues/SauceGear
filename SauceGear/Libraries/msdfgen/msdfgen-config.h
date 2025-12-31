#pragma once

// Build configuration for MSDFGen

// Disable OpenMP (mais simples no Windows / MSVC)
#define MSDFGEN_USE_OPENMP 0

// Use double precision (recomendado)
#define MSDFGEN_USE_DOUBLE 1

// Disable Skia integration (não usamos)
#define MSDFGEN_USE_SKIA 0

// Disable FreeImage (não usamos)
#define MSDFGEN_USE_FREEIMAGE 0

// We use FreeType for font loading
#define MSDFGEN_USE_FREETYPE 1
