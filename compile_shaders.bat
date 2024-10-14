@echo off

cd src/client/shaders

glslc simple.vert -o simple.vert.spv
glslc simple.frag -o simple.frag.spv

PAUSE