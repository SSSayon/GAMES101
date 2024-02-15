//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"
#include "global.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray) const {  // Ray.direction is always normalized
    Intersection isect = intersect(ray);  // Intersection.direction is always normalized
    if (!isect.happened) {
        return Vector3f(0.f);
    }

    Vector3f p = isect.coords;
    
    Vector3f L_dir(0.f);
    Intersection light_isect;
    float light_pdf;
    sampleLight(light_isect, light_pdf);  // light_isect does not has `distance` attr.
    if (!(isect.obj == light_isect.obj)) {  // obj not emitting / obj emitting while sampled light != obj
        Vector3f wi = (light_isect.coords - p).normalized();
        Intersection ray_to_light = intersect(Ray(p, wi));
        float delta_dist = ray_to_light.distance - (light_isect.coords - p).norm();
        if (delta_dist > -EPSILON && delta_dist < EPSILON) {  // not blocked
            double r2 = (light_isect.coords - p).normSquared();
            L_dir = light_isect.emit * isect.m->eval(-ray.direction, wi, isect.normal)
                    * dotProduct(isect.normal, wi) * dotProduct(light_isect.normal, -wi)
                    / r2 / light_pdf;
        }
    }

    Vector3f L_indir(0.f);
    if (get_random_float() < RussianRoulette) {
        Vector3f wi = isect.m->sample(-ray.direction, isect.normal).normalized();
        if (dotProduct(isect.normal, wi) > 0.f) {
        // if (dotProduct(isect.normal, wi) > 0.f && dotProduct(isect.normal, -ray.direction) > 0.f) {
            Intersection ray_to_obj = intersect(Ray(p, wi));
            if (ray_to_obj.happened && !ray_to_obj.m->hasEmission()) {  // ray cast to a non-emitting obj
                L_indir = castRay(Ray(p, wi)) * isect.m->eval(-ray.direction, wi, isect.normal)
                        * dotProduct(isect.normal, wi)
                        / isect.m->pdf(-ray.direction, wi, isect.normal) / RussianRoulette;
            }
        }
    }

    return isect.m->m_emission + L_dir + L_indir;
}
