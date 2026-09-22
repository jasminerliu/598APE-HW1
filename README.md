# 598APE-HW1

This repository contains code for homework 1 of 598APE.

In particular, this repository is an implementation of a Raytracer, which we sped up.

To compile the program run:
```bash
make -j
```

To clean existing build artifacts run:
```bash
make clean
```

To profile any run, you can do:
`perf stat -- <command>`
`perf record -- <command>`
`perf report`
`perd report --stdio`

For example:
```bash
perf stat -- ./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
perf record -- ./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
perf report
```
To reproduce our baseline numbers, check out the last commit before our's in this repo's history, rebuild, and run:
```bash
git checkout 19bbc813a55d19354196bf9887d3786adfe2f6c0
make clean && make all
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
```

Our history has three checkpoint commits, each bundling one or more related optimizations. To isolate an individual optimization's effect, check out the commit before it changed, diff against the commit that introduced it (git diff [PARENT] [COMMIT] -- <file>), and re-benchmark before/after applying just that file's change.

### After calcColor + Box::getIntersection fixes
```bash
git checkout aef8813
make clean && make all
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
```

### After solveScalers rewrite
```bash
git checkout db5249b
make clean && make all
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
python3 scripts/verify_orthonormal.py   # re-check the correctness precondition this optimization relies on
```

### After OpenMP parallelization + compiler flags
```bash
git checkout 62e31ef
make clean && make all
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
```
### To isolate the parallelization speedup specifically from the flag changes, compare single-thread vs. all core runs at this same commit:
```bash
OMP_NUM_THREADS=1 ./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500.
```

Repeat the same sequence with inputs/globe.ray, inputs/sphere.ray, and inputs/elephant.ray. Commands are in the Input Programs section.

This program assumes the following are installed on your machine:
* A working C++ compiler (g++ is assumed in the Makefile)
* make
* ImageMagick (for importing and exporting non-ppm images)
* FFMpeg (for exporting movies from image sequences)
* perf (not required to build or run the raytracer, for profiling only)

The raytracer program here is general and can be used to generate any number of different potential scenes.

Once compiled, one can call the raytracer program as follows:
```bash
./main.exe --help
# Prints the following
# Usage ./main.exe [-H <height>] [-W <width>] [-F <framecount>] [--movie] [--no-movie] [--png] [--ppm] [--help] [-o <outfile>] [-i <infile>] [-a <animationfile>]
```

The raytracer program takes a scene file (a text file ending in .ray) and generates an image or sequence of images corresponding to the specified scene.

One can tune the height, width, and format of the image being generated with optional command line arguments. For example, let's generate an 500x500 image corresponding to the scene in `inputs/pianoroom.ray`, in PPM format.

```bash
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
```

As we run the program, we see the following output:
```
Done Frame       0|
Total time to create images=1.334815 seconds
```

## Input Programs
This project contains three (arguably four) input programs for you to optimize.

### PianoRoom

A simple room with a reflecting checkerboard floor, a stairwell, a sphere, a circular rug, and a mirror ref.

Here we want to produce the highest resolution single image of this format, as fast as possible. The relevant command for producing an output is:

```bash
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
```

### Globe

A video of the Earth floating on top of a sea with a sky in the background. The Earth and clouds are rotating (in opposite directions), and the sea beneath reflects the scene above, and moves.

Here we want to produce the highest resolution video, as fast as possible. The relevant command for producing an output is:

```bash
./main.exe -i inputs/globe.ray --ppm  -a inputs/globe.animate --movie -F 24 
```

Here, as we are generating multiple frames, the extra command `-a <animationfile>` is used to pass in a sequence of commands to generate subsequent frames.

The number of frames we wish to generate (24) is passed in as `-F <numframes>`.

Here we will produce 24 individual images for each frame. To produce a playable movie out of these images, the `--movie` command will call a program called FFMpeg to produce a playable video.

### Elephant

A mesh of objects. In practical graphics applications, designing a primitive for each possible object is too complex. Instead, one builds up a mesh of triangles to represent the object being shown. Given sufficiently many triangles, we can represent arbitrarily complex structures. Here, we wish to make a video circling around a Mesh object which we import.

The simple version of this program is generated by the following command:
```bash
./main.exe -i inputs/elephant.ray --ppm  -a inputs/elephant.animate --movie -F 24 -W 100 -H 100 -o output/sphere.mp4 
```

Note the reduced resolution (as the initial unoptimized code can be somewhat slow).

This initial mesh represents a sphere with 3168 triangles.

Here we produce a video in which we have the camera circles around the object.

If we inspect the input file `inputs/elephant.ray` we see that it loads the mesh from two files, as defined by the line
```
data/x.txt 1586 data/f.txt 3168 -1.58 -.43 2.7
```

The goal here is to speed up the program sufficiently to make a high resolution circle of the elephant mesh (found in `data/elepx.txt` and `data/elepf.txt`), which contains 111748 triangles. One can edit the `.ray` file and comment out the sphere mesh and replace it with `data/elepx.txt 62779 data/elepf.txt 111748 -1.58 -.43 2.7`.

## Code Overview

The raytracer contains several core utilities, defined in different files.

### Camera

The Camera class contains information about the position and direction we are facing. An image is constructed by creating a grid of points and sending out rays from each of these points, and determining what objects they collide with. Each result becomes an individual pixel in our resulting image.

### Shape

Each object in our scene is defined as a shape. There are several shapes subclasses in the application. This includes a plane (an infinitely long flat surface), a sphere (a collection of points equidistant from a center), a disk (a flat surface whose points are within a given distance of a center), a box (a flat rectangle), and a triangle.

Shapes have a position in space, and potentially an orientation (i.e. direction they face, as defined with the angles yaw pitch and roll).

Shapes also have a texture defining what color of each point of the shape, and optionally a "normalMap" texture which defines how light bounces off each point.

Core methods within shape include:
* `getIntersection`, which defines whether a given light ray will hit the shape, and if so returns time it takes the light to hit it (otherwise infinity).
* `getLightIntersection`: Given that a ray hits the shape, determine how a light source will illuminate the shape at that point based off of the color of the object, and its spectral properties (i.e. opaque, reflective, aminent lighting).
* `getNormal` determine the normal axis to the point of collision, in order to compute the direction in which light will bounce off the object.

### Texture

A texture object defines what color will be applied at a point in space. There are two textures implemented: a single color for all points, and one loaded from an image. Textures are used to define both the color of an object, and also can optionally be used to define normal axes for an object (using data stored in rgb to define the xyz axis).

### Light

Light objects illuminate a scene, resulting in differences in gradients of colors on an object and shadows. Lights have a color and a position.

### Autonoma

An Autonoma is a base class used to hold all of the shapes in scope, the camera, and all lights.


## Docker

For ease of use and installation, we provide a docker image capable of running and building code here. The source docker file is in /docker (which is essentially a list of commands to build an OS state from scratch). It contains the dependent compilers, and some other nice things.

You can build this yourself manually by running `cd docker && docker build -t <myusername>/598ape .`. Alternatively we have pushed a pre-built version to `wsmoses/598ape` on Dockerhub.

You can then use the Docker container to build and run your code.

To start up the docker container,
`docker run -it \
  --cap-add=PERFMON \
  --security-opt seccomp=unconfined \
  -v "$PWD:/host" \
  -w /host \
  --name 598ape \
  jrliu2/598ape`

replacing jrliu2 with whatever your container is called.

The first time you do this, inside the container, do `apt update` and `apt install linux-tools-7.0.0-31-generic`. You need this to run perf.

The current directory (aka this folder) is mounted within `/host`. Any files you create on your personal machine will be available there, and anything you make in the container in that folder will be available on your personal machine.
