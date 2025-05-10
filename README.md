# PS1-Style 3D Engine

A C++ implementation of a PlayStation 1-style 3D engine that recreates the distinctive visual style of PS1 games, including texture quantization, dithering, and affine texture mapping.

## Features

- 3D rendering with PS1-style visual effects
- Texture mapping with quantization and dithering
- Depth buffer for proper 3D rendering
- Support for OBJ model loading
- Camera system with perspective projection
- AABB (Axis-Aligned Bounding Box) culling
- Basic lighting system
- Debug visualization (FPS counter, triangle count)
- SIMD optimizations for depth buffer clearing

## Technical Details

### Rendering Features
- Affine texture mapping (characteristic PS1-style texture warping)
- Texture quantization during loading
- Floyd-Steinberg dithering
- Depth buffer for proper 3D rendering
- Triangle sorting for transparency
- Fog effect for distance-based color blending

### Performance Optimizations
- SIMD instructions for depth buffer operations
- AABB culling to reduce unnecessary triangle processing
- Efficient triangle clipping against view frustum
- Optimized texture sampling

### Graphics Pipeline
1. Model loading and transformation
2. View frustum culling using AABB
3. Triangle transformation and lighting
4. Perspective projection
5. Triangle clipping
6. Rasterization with texture mapping
7. Dithering (optional)
8. Final frame composition

## Dependencies

- SFML (Simple and Fast Multimedia Library)
- C++17 or later
- SIMD support (SSE instructions)

## Building

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make
```

## Usage

```cpp
// Initialize engine
Engine* engine = new Engine(60, 4, "PS1 Engine Demo");

// Load textures
engine->LoadTexture("texture.png");

// Load 3D models
engine->components.createFromFile("model.obj", textureID);

// Main loop
while (engine->isOpen()) {
    engine->checkEvents();
    engine->calculateTriangles(camera->pos, camera->vTarget, camera->vUp);
    engine->renderAll();
}
```

## Configuration Options

- `targetFPS`: Target frames per second
- `scale`: Window scaling factor
- `useDither`: Enable/disable dithering
- `useSort`: Enable/disable triangle sorting

## Todo List

- [x] Texture metadata (width, height, data, dithered, size)
- [x] 2D arrays for pixels, update render only once per frame
- [x] AABB boxes
- [ ] Point lights
- [ ] Gouraud shading
- [ ] Precalculate sin/cos
- [ ] Use fixed point numbers
- [ ] Store triangle normals
- [ ] Shared resources

## Contributing

Feel free to submit issues and pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details. 