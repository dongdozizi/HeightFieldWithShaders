# HeightFieldWithShaders

## Homework Website

https://viterbi-web.usc.edu/~jbarbic/cs420-s24/assign1/index.html

## Introduction
Height fields may be found in many applications of computer graphics. They are used to represent terrain in video games and simulations, and also often utilized to represent data in three dimensions. This assignment asks you to create a height field based on the data from an image which the user specifies at the command line, and to allow the user to manipulate the height field in three dimensions by rotating, translating, or scaling it. You also have to implement a vertex shader that performs smoothing of the geometry, and re-adjusts the geometry color. After the completion of your program, you will use it to create an animation. You will program the assignment using OpenGL's core profile.

This assignment is intended as a hands-on introduction to OpenGL and programming in three dimensions. It teaches the OpenGL's core profile and shader-based programming. The provided starter gives the functionality to initialize GLUT, read and write a JPEG image, handle mouse and keyboard input, and display one triangle to the screen. You must write code to handle camera transformations, transform the landscape (translate/rotate/scale), and render the heightfield. You must also write a shader to perform geometry smoothing and re-color the terrain accordingly. Please see the OpenGL Programming Guide for information, or OpenGL.org.

## Basic features

- Render the points mode.
- Render the Lines mode.
- Render the Triangles mode.
- Render the smooth triangle mode and can scale and exponentiate the height in this mode.
- Scale, translate and rotate the heightfield.

## Extra Credits

- Use element arrays and glDrawElements to save the memory (Write helper class ebo.h and ebo.cpp in openGLHelper).
- Support color (ImageIO::getBytesPerPixel == 3) in input heightfield images (Convert the RGB to grayscale by using formula in https://docs.opencv.org/4.x/de/d25/imgproc_color_conversions.html).
- Render wireframe on top of solid triangles (use glPolygonOffset to avoid z-buffer fighting).
- Color the vertices based on color values taken from another image of equal size (The color of the pixel in heightfield is the same pixel in the heightmap).
- Texturemap the surface with an arbitrary image (Using glTexImage2D).
- When using key "4", color the surface using the JetColorMap function, in the vertex shader. Speficially, change the grayscale color x to JetColorMap(x).
- Can not only move the object but also moving the camera position and change the focus vector.
- Can support image size that height and width are not the same.

## Main Files Description
 - README.md - Introduction of the project
 - animations - Restore the JPEG frames required for animation
 - openGLHelper - Helper files for OpenGL
 - Examples - Example output for the heightfield
 - hw1\heightmap - File for the input images
 - hw1\hw1.cpp - Main opengl program for this homework
 - hw1\Makefile - Make file for MacOS and Linux.
 - hw1\hw1 - Compiled executable for MacOS
 - hw1\Bin\Release\hw1.exe - Compiled executable for Windows

## To run the program

### MacOS

For compile, enter the hw1 folder and make

    // Enter hw1
    cd hw1

    // make the file
    make

For only heightmap, execute

    ./hw1 <heightmap_file>

For heightmap and enable texture mapping, execute

    ./hw1 <heightmap_file> <texturemap_file>

The heightmap_file should be only grayscale or RGB image and texturemap_file should be only RGB image.

Examples

    ./hw1 heightmap/Heightmap.jpg

    ./hw1 heightmap/color.jpg

    ./hw1 heightmap/Heightmap.jpg heightmap/Heightmap.jpg/color.jpg
    
    // Mount Fuji Fuji-0001 to Fuji-0009 in Examples
    ./hw1 heightmap/mtFujiHeight-128.jpg heightmap/mtFujiHeight-1000.jpg
    // Oahu Island Oahu-0001 to Oahu-0009 in Examples  
    ./hw1 heightmap/Oahu-160x128.jpg heightmap/Oahu.jpg

### Windows

For windows, just open the hw1.sln with visual studio, and change the command arguments for custom input:

    ./heightmap/Oahu-160x128.jpg ./heightmap/Oahu.jpg

Also you can just use the command prompt (cmd.exe), such as:

    hw1\Bin\Release\hw1 hw1\heightmap\Oahu-160x128.jpg hw1\heightmap\Oahu.jpg
    

## Instructions

- **Points Mode**: Press `1`
- **Lines Mode**: Press `2`
- **Triangles Mode**: Press `3`
- **Smooth Mode**: Press `4`. In this mode:
  - Press `+` to multiply "scale" by 2x
  - Press `-` to divide "scale" by 2x
  - Press `9` to multiply "exponent" by 2x
  - Press `0` to divide "exponent" by 2x

**Rotation**
- X and Y axes: Press and hold left mouse button and move the mouse.
- Z axis: Press and hold middle mouse button and move the mouse.

**Scaling**
- X and Y axes: Press and hold `Shift` + left mouse button and move.
- Z axis: Press and hold `Shift` + middle mouse button and move.

**Translation (Windows)**
- X and Y axes: Press and hold `Control` + left mouse button and move.
- Z axis: Press and hold `Control` + middle mouse button and move.

**Translation (MacOS)**
Press `t` to enable\disable the Translate mode
- X and Y axes: In translation mode, press and hold left mouse button and move.
- Z axis: In translation mode, press and hold middle mouse button and move.

## Instructions (For Extra Credit)

- **Smooth Mode With JetColorMap**: Press `5`
- **Triangles Mode with wireframe**: Press `6`
- **Texturemap the surface with an arbitrary image (If compatible)**: Press `7`
- **Texturemap the surface with an image with image at same size (If compatible)**: Press `&` (which is `shift`+`7`)
- **Reset the heightmap to initial view(Undo all the rotation, scaling and translation)**: Press `c`
- **Enable moving camera**: Press `v` to enable\disable. In this mode:
  - `w` to move the camera position ahead.
  - `s` to move the camera position backward.
  - `a` to move the camera position left.
  - `d` to move the camera position right.
  - Move the mouse to change the direction of the mouse (Look up, down, left, right).
