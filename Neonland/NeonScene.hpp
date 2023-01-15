#pragma once

#include <array>
#include <random>

#include "Scene.hpp"
#include "GameClock.hpp"
#include "NeonConstants.h"

#include "Transform.hpp"
#include "Physics.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "HP.hpp"
#include "Enemy.hpp"
#include "PlayerProjectile.hpp"
#include "Weapon.hpp"

#include "FrameData.h"
#include "NumberField.hpp"

class NeonScene {
public:
    GameClock clock;
    
    float2 mouseDelta = {0, 0};
    float2 mousePos = {0, 0};
    float2 prevMousePos = {0, 0};
    
    bool mouseDown = false;
    bool prevMouseState = false;
    bool mousePressed = false;
    
    std::array<Weapon, 3> weapons = {
        Weapon(PlayerProjectile(1, 1.5f, 35.0f, true), 0.1f, 0.02f, 1, 1.0f),
        Weapon(PlayerProjectile(1, 1.5f, 30.0f, true), 0.5f, 0.2f, 5, 1.0f),
        Weapon(PlayerProjectile(1, 2.0f, 25.0f, false), 0.5f, 0.01f, 1, 1.5f)
    };
    
    int weaponIdx = 0;
    
    std::default_random_engine randomEngine;
    
    double movementSpeed = 7.0f;
    
    float2 directionalInput = {0, 0};
    float3 moveDir = {0, 0, 0};
    
    Entity player;
    Entity cam;
    Entity crosshair;
    
    NumberField scoreField;
    
    float2 mapSize = {50, 50};
    
    bool gameOver = false;
    
    float camDistance = 25;
    
    NeonScene(size_t maxInstanceCount, double timestep, GameClock clock);
    
    void SelectWeapon(int i);
    
    void Update(float aspectRatio);
    
    void EarlyRender(double time, double dt);
    void Tick(double time);
    void LateRender(double time, double dt);
    void RenderUI();
    
    FrameData GetFrameData();
    
    double Timestep() const;
    
    size_t MaxInstanceCount() const;
    
    float RandomValue();
private:
    Scene _scene;
    
    std::vector<Instance> _instances;
    std::vector<size_t> _groupSizes;
    std::vector<uint32_t> _groupMeshes;
    std::vector<uint32_t> _groupTextures;
    
    size_t _maxInstanceCount;
    
    double _timestep;
    double _nextTickTime;
    double _prevRenderTime;
};
