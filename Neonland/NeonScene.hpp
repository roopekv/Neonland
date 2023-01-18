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
#include "Level.hpp"

class NeonScene {
public:
    GameClock clock;
    
    float2 mouseDelta = {0, 0};
    float2 mousePos = {0, 0};
    float2 prevMousePos = {0, 0};
    
    bool mouseDown = false;
    bool prevMouseState = false;
    bool mousePressed = false;
    
    int currentWeaponIdx = 0;
    std::array<Weapon, 3> weapons = {
        Weapon(PlayerProjectile(1, 2.0f, 35.0f, true), 0.1f, 0.05f, 1, 1.0f),
        Weapon(PlayerProjectile(4, 1.5f, 30.0f, true), 0.4f, 0.1f, 5, 1.0f),
        Weapon(PlayerProjectile(1, 2.0f, 25.0f, false), 0.3f, 0.05f, 1, 1.5f)
    };
    
    float2 directionalInput = {0, 0};
    
    NeonScene(size_t maxInstanceCount, double timestep, GameClock clock);
    
    void SelectWeapon(int i);
    void Update(float aspectRatio);
    
    FrameData GetFrameData();
    
    double Timestep() const;
    size_t MaxInstanceCount() const;
    
    float RandomBetween(float a, float b);
    
    const Level& CurrentLevel();
    
    NumberField CreateField(float2 size = {0.005f, 0.01f}, float4 color = {1, 1, 1, 1});
    void UpdateField(const NumberField& field);
private:
    Scene _scene;
    
    float3 moveDir = {0, 0, 0};
    
    Entity player;
    Entity cam;
    Entity crosshair;
    Entity spreadCircle;
    
    Entity ground;
    
    float spreadMult = 1.0f;
    float prevSpreadMult = 1.0f;
    
    NumberField scoreField;
    NumberField healthField;
    
    float movementSpeed = 7.0f;
    
    float camDistance = 25;
    
    int levelIdx = 0;
    
    bool gameOver = false;
    bool levelWon = false;
    size_t currentWave = 0;
    size_t currentSubWave = 0;
    double nextSubWaveStartTime = 0;
    
    void LoadLevel(int i);
    
    void UpdateLevelProgress(double time);
    
    void SpawnSubWave(const Wave::SubWave& subWave);
    
    void Tick(double time);
    void Render(double time, double dt);
    void RenderUI();
    
    std::default_random_engine randomEngine;
    
    std::vector<Instance> _instances;
    std::vector<size_t> _groupSizes;
    std::vector<uint32_t> _groupMeshes;
    std::vector<uint32_t> _groupTextures;
    
    size_t _maxInstanceCount;
    
    double _timestep;
    double _nextTickTime;
    double _prevRenderTime;
};
