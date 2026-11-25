#pragma once

#include <numbers>
#include <Geode/Geode.hpp>
#include "../BlurAPI.hpp"
#include "GLUtils.hpp"

using namespace geode::prelude;

void setupPostProcess();
void cleanupPostProcess();

namespace Blur
{
    void drawBlurredNode(CCNode* node, BlurAPI::BlurOptions* options);
    asp::fs::PathView getShaderPath(bool vertexShader);
};
