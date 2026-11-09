# Minecraft 2D

## About

This is one of my first proper projects using [CPL](https://github.com/ColinIndieDev/CPL) - a gamedev library I made from scratch with its first prototype released ~1 year ago which is now written entirely in C even though 3D is not added yet.

## Screenshots
![image](img/minecraft_2D.png)

## Features / "Gameplay"
For now only these things:

- World Generation by Seeds
  - Terrain
  - Ocean
  - Trees
  - Caves
  - Ores
- Block breaking, collecting & placing
- Basic inventory system
- Collisions and player movement

## Dependencies

This project uses third party libraries from CPL (f.e. GLFW) but 
for Perlin Noise, it uses FastNoiseLite, a simple header only library which
is inside `src/` with the file name `noise.h`.
