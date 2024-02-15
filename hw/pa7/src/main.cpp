#include "Material.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{

    // Change the definition here to change resolution
    Scene scene(1024, 1024);

    Material* red = new Material(DIFFUSE, Vector3f(0.0f));
    red->m_color = Vector3f(0.63f, 0.065f, 0.05f);
    Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    green->m_color = Vector3f(0.14f, 0.45f, 0.091f);
    Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    white->m_color = Vector3f(0.725f, 0.71f, 0.68f);
    Material* light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * Vector3f(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *Vector3f(0.737f+0.642f,0.737f+0.159f,0.737f)));
    light->m_color = Vector3f(0.65f);

    // Material* white2 = new Material(MICROFACET, Vector3f(0.0f));
    // white2->m_color = Vector3f(0.725f, 0.71f, 0.68f);
    // white2->Kd = 0.5f;
    // white2->alpha = 0.5f;
    // white2->F0 = Vector3f(0.725f, 0.71f, 0.68f);

    MeshTriangle floor("../src/models/cornellbox/floor.obj", white);
    MeshTriangle shortbox("../src/models/cornellbox/shortbox.obj", white);
    // MeshTriangle shortbox("../src/models/cornellbox/shortbox.obj", white2);
    MeshTriangle tallbox("../src/models/cornellbox/tallbox.obj", white);
    MeshTriangle left("../src/models/cornellbox/left.obj", red);
    MeshTriangle right("../src/models/cornellbox/right.obj", green);
    MeshTriangle light_("../src/models/cornellbox/light.obj", light);

    scene.Add(&floor);
    scene.Add(&shortbox);
    scene.Add(&tallbox);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    scene.buildBVH();

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}