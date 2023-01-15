#include "Neonland.h"

#include "NeonScene.hpp"

#include <iostream>

namespace {

auto scene = NeonScene(MAX_INSTANCE_COUNT, TIMESTEP, GameClock(true));
    
}

void Neon_Start() {
    scene.clock.Paused(false);
}

FrameData Neon_Render(float aspectRatio) {
    scene.Update(aspectRatio);
    
    return scene.GetFrameData();
}

void Neon_UpdateCursorPosition(vector_float2 newPos) {
    scene.prevMousePos = scene.mousePos;
    
    scene.mousePos.x = std::clamp(newPos.x, -1.0f, 1.0f);
    scene.mousePos.y = std::clamp(newPos.y, -1.0f, 1.0f);
    scene.mouseDelta += scene.mousePos - scene.prevMousePos;
}

void Neon_UpdateMouseDown(bool down) {
    scene.mouseDown = down;
}

void Neon_UpdateDirectionalInput(vector_float2 newDir) {
    float length = simd_length(newDir);
    if (length > 0.0f) {
        newDir /= length;
    }
    scene.directionalInput = newDir;
}

void Neon_UpdateNumberKeyPressed(int num) {
    if (num == 0) {
        num = 10;
    }
    
    scene.SelectWeapon(num - 1);
}

