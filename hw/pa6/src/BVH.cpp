#include <algorithm>
#include <cassert>
#include <chrono>
#include <vector>
#include "BVH.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    auto start = std::chrono::high_resolution_clock::now();
    if (primitives.empty())
        return;

    if (splitMethod == SplitMethod::NAIVE) {
        root = recursiveBuild(primitives);
    } else if (splitMethod == SplitMethod::SAH) {
        root = recursiveBuildSAH(primitives);
    } 

    auto stop = std::chrono::high_resolution_clock::now();
    double diff = (stop - start).count();
    // int hrs = (int)diff / 3600;
    // int mins = ((int)diff / 60) - (hrs * 60);
    // int secs = (int)diff - (hrs * 3600) - (mins * 60);
    // printf("BVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
    //     hrs, mins, secs);    
    printf("BVH Generation complete: \nTime Taken: %f ns\n\n", diff);
}

BVHBuildNode* BVHAccel::recursiveBuildSAH(std::vector<Object*> objects) {

    if (objects.size() < bucketNum) {
        return recursiveBuild(objects);
    }

    BVHBuildNode* node = new BVHBuildNode();

    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());

    Bounds3 centroidBounds;
    for (int i = 0; i < objects.size(); ++i)
        centroidBounds = Union(centroidBounds, objects[i]->getBounds().Centroid());
    int dim = centroidBounds.maxExtent();

    float minX = bounds.pMin[dim];
    float maxX = bounds.pMax[dim];
    double minSAH = std::numeric_limits<double>::max();
    int M = bucketNum / 2;
    for (int i = 1; i < bucketNum; ++i) {
        float truncX = minX + i * (maxX - minX) / bucketNum;
        auto middling = std::partition(objects.begin(), objects.end(), [dim, truncX](Object* object) {
            return object->getBounds().Centroid()[dim] < truncX;
        });
        Bounds3 left, right;
        for (auto obj = objects.begin(); obj != middling; ++obj) {
            left = Union(left, (*obj)->getBounds());
        }
        for (auto obj = middling; obj != objects.end(); ++obj) {
            right = Union(right, (*obj)->getBounds());
        }
        double SAH = (middling - objects.begin()) * left.SurfaceArea() + (objects.end() - middling) * right.SurfaceArea();
        if (SAH < minSAH) {
            M = i;
            minSAH = SAH;
        }
    }

    float truncX = minX + M * (maxX - minX) / bucketNum;
    auto middling = std::partition(objects.begin(), objects.end(), [dim, truncX](Object* object) {
        return object->getBounds().Centroid()[dim] < truncX;
    });
    auto leftshapes =  std::vector<Object*>(objects.begin(), middling);
    auto rightshapes = std::vector<Object*>(middling, objects.end());

    node->left = recursiveBuildSAH(leftshapes);
    node->right = recursiveBuildSAH(rightshapes);
    node->bounds = Union(node->left->bounds, node->right->bounds);

    return node;
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    // Bounds3 bounds;
    // for (int i = 0; i < objects.size(); ++i)
    //     bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // traverse the BVH to find intersection
    std::array<int, 3> dirIsNeg = {int(ray.direction.x < 0), 
                                   int(ray.direction.y < 0), 
                                   int(ray.direction.z < 0)};
    if (!node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg)) {
        return Intersection();
    }

    if (node->object) {
        return node->object->getIntersection(ray);
    }
    
    Intersection isect1 = getIntersection(node->left, ray);
    Intersection isect2 = getIntersection(node->right, ray);
    if (!isect1.happened) {
        return isect2;
    }

    if (!isect2.happened) {
        return isect1;
    }
    return (isect1.distance < isect2.distance) ? isect1 : isect2;
}
