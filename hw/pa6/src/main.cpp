#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    Scene scene(1280, 960);
    BVHAccel::SplitMethod splitMethod = BVHAccel::SplitMethod::SAH;

    MeshTriangle bunny("../src/models/bunny/bunny.obj", splitMethod);

    scene.Add(&bunny);
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 1));
    scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), 1));
    scene.buildBVH(splitMethod);

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene, splitMethod);
    auto stop = std::chrono::system_clock::now();

    double diff = (stop - start).count();
    std::cout << "Render complete: \n";
    printf("Time Taken: %f ns\n\n", diff);
    return 0;
}
