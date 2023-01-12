#include "Neonland.h"

#include "NeonScene.hpp"

#include <iostream>

namespace {

auto scene = NeonScene(MAX_ENTITY_COUNT, TIMESTEP, GameClock(true));

constexpr MeshType MeshForGroup(uint32_t groupIdx) {
    MeshType meshesForGroups[NeonScene::EntityGroupCount];
    
    meshesForGroups[NeonScene::PLAYER] = PLAYER_MESH;
    meshesForGroups[NeonScene::CROSSHAIR] = PLAYER_MESH;
    meshesForGroups[NeonScene::ENEMY] = ENEMY_MESH;
    
    return meshesForGroups[groupIdx];
}
    
}

uint32_t Neon_MeshForGroup(uint32_t groupIdx) {
    return MeshForGroup(groupIdx);
}

void Neon_Start() {
    scene.clock.Paused(false);
}

FrameData Neon_Render(float aspectRatio) {
    if (aspectRatio != scene.camera.GetAspectRatio()) {
        scene.camera.SetAspectRatio(aspectRatio);
    }
    
    scene.Update();
    
    return scene.GetFrameData();
}

void Neon_UpdateCursorPosition(vector_float2 newPos) {
    scene.prevMousePos = scene.mousePos;
    
    scene.mousePos.x = std::clamp(newPos.x, -1.0f, 1.0f);
    scene.mousePos.y = std::clamp(newPos.y, -1.0f, 1.0f);
    scene.mouseDelta += scene.mousePos - scene.prevMousePos;
}

void Neon_UpdateMoveDirection(vector_float2 newDir) {
    scene.moveDir += {newDir.x, newDir.y, 0.0f};
    float length = simd_length(scene.moveDir);
    if (length > 0.0f) {
        scene.moveDir /= length;
    }
}

