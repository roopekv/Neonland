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
#include "GameState.hpp"

class NeonScene {
public:
    GameClock clock;
    std::array<TexSize, TextureTypeCount> textureSizes;
    
    float2 mouseDelta = {0, 0};
    float2 mousePos = {0, 0};
    float2 prevMousePos = {0, 0};
    
    bool mouseDown = false;
    bool prevMouseState = false;
    bool mouseClicked = false;
    
    float2 directionalInput = {0, 0};
    
    int weaponIdx = 0;
    
    NeonScene(size_t maxInstanceCount, double timestep, GameClock clock);
    
    void Start();
    
    void SelectWeapon(int i);
    void Update(float aspectRatio);
    
    FrameData GetFrameData();
    
    double Timestep() const;
    size_t MaxInstanceCount() const;
    
    float RandomBetween(float a, float b);
    
    const Level& CurrentLevel();
    Weapon& CurrentWeapon();
    
    NumberField CreateField(float scale, TextureType tex, bool keepCamTilt = false, float4 color = {1, 1, 1, 1});
    Entity CreateButton(float3 pos, float scale, TextureType tex, std::function<void()> action, bool keepCamTilt, bool enabled);
    void UpdateField(const NumberField& field, float3 center);
private:
    Scene _scene;
    
    float3 moveDir = {0, 0, 0};
    
    Entity player;
    Entity cam;
    Entity crosshair;
    Entity spreadCircle;
    
    Entity ground;
    
    Entity numKeysImg;
    
    float spreadMult = 1.0f;
    float prevSpreadMult = 1.0f;
    
    NumberField enemiesRemainingField;
    NumberField healthField;
    NumberField waveField;
    
    float movementSpeed = 7.0f;
    
    float camDistance = 25;
    float camTilt = 0;
    
    int levelIdx = 0;

    GameState gameState = GameState::Menu;
    
    int currentWave = 0;
    int currentSubWave = 0;
    double nextSubWaveStartTime = 0;
    
    std::default_random_engine randomEngine;
    
    std::vector<Instance> _instances;
    std::vector<size_t> _groupSizes;
    std::vector<uint32_t> _groupMeshes;
    std::vector<uint32_t> _groupTextures;
    std::vector<uint32_t> _groupShaders;
    
    size_t _maxInstanceCount;
    
    double _timestep;
    double _nextTickTime;
    double _prevRenderTime;
    
    std::array<Weapon, 3> weapons = {
        Weapon(PlayerProjectile(1, 2.0f, 35.0f, true),
               0.1f,
               0.025f,
               1,
               1.0f,
               Mesh(SHARD_MESH, Material(LIT_SHADER, NO_TEX, float4{0, 1, 0, 1}))),
        Weapon(PlayerProjectile(4, 1.5f, 30.0f, true),
               0.4f,
               0.1f,
               5,
               1.0f,
               Mesh(SHARD_MESH, Material(LIT_SHADER, NO_TEX, float4{1, 0, 0, 1}))),
        
        Weapon(PlayerProjectile(1, 2.0f, 25.0f, false),
               0.3f,
               0.025f,
               1,
               1.5f,
               Mesh(SHARD_MESH, Material(LIT_SHADER, NO_TEX, float4{0, 0.6, 1, 1})))
    };
    
    void LoadLevel(int i);
    
    void UpdateLevelProgress(double time);
    
    void SpawnSubWave(const Wave::SubWave& subWave);
    
    void Tick(double time);
    void Render(double time, double dt);
    void RenderUI();
};
