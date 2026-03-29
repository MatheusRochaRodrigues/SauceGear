# C++ OpenGL Graphics Engine

A modular **graphics engine built from scratch in C++ using OpenGL**, focused on real-time rendering, procedural terrain generation, and modern GPU pipelines.

This project explores low-level graphics programming concepts such as **voxel terrain generation, mesh extraction algorithms, physically based rendering, GPU compute pipelines, and custom editor tooling**.

---

## Demo Video

Click the image below to watch a demonstration of the engine:

[![Engine Demo](https://github.com/user-attachments/assets/7e5c251d-bd4c-4e54-bae3-f941efa0ec71)](https://www.linkedin.com/posts/mathewrocha_here-are-some-of-the-features-ive-been-implementing-activity-7423646120695951360-YUm9)

---

# Overview

This engine was built as a **personal research and learning project** focused on understanding the inner workings of modern real-time rendering pipelines.

The goal is to experiment with advanced graphics techniques while maintaining a **clean modular architecture** suitable for experimentation and expansion.

Key focus areas include:

- GPU driven rendering
- procedural terrain generation
- voxel mesh extraction algorithms
- modern shading pipelines
- editor tooling

---

# Engine Features

## Rendering

- Modern **OpenGL rendering pipeline**
- **Physically Based Rendering (PBR)**
- **Post-processing pipeline**
- HDR rendering
- Modular render passes
- GPU compute shaders

---

## Procedural Terrain

Procedural terrain generation using voxel techniques.

Implemented algorithms include:

- **Surface Nets**
- **Dual Contouring**
- **Octree LOD terrain**
- Signed Distance Fields (SDF)

Focus on **real-time mesh extraction and level-of-detail optimization**.

---

## GPU Compute

The engine heavily uses **compute shaders** for GPU side computation.

Examples include:

- mesh generation
- terrain evaluation
- SDF sampling
- GPU based processing pipelines

---

## Engine Architecture

The engine is built with a modular architecture designed for flexibility.

Core systems include:

- **Entity Component System (ECS)**
- modular render pipeline
- asset management
- scene system
- editor integration

---

## Editor

The engine includes a custom **ImGui-based editor**.

Features include:

- scene inspection
- runtime parameter editing
- debug visualization
- rendering settings control

---

## Text Rendering

Font rendering implemented using **Signed Distance Field (SDF) fonts** for scalable and high quality text.

---

# Technologies Used

- **C++**
- **OpenGL**
- **GLSL**
- **Compute Shaders**
- **ImGui**
- **SDF Font Rendering**

---

# Project Goals

This project aims to explore:

- modern rendering architecture
- real-time procedural terrain
- GPU based pipelines
- graphics engine design

---

# Future Work

Planned improvements include:

- GPU driven rendering
- Vulkan backend
- expanded editor features

---

# Author

Matheus Rocha  

GitHub: https://github.com/MatheusRochaRodrigues  
LinkedIn: www.linkedin.com/in/mathewrocha
