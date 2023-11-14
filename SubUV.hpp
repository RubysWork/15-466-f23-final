#include "Scene.hpp"
#include <vector>

class SubUV
{
public:
    std::vector<Scene::Drawable> subdrawables;
    uint32_t bitmask = 1;
    float scale = 1.0f;
};