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
    bool appShouldQuit = false;
    
    std::array<TexSize, TextureTypeCount> textureSizes;
    
    float2 mouseDelta = {0, 0};
    float2 mousePos = {0, 0};
    float2 prevMousePos = {0, 0};
    
    bool mouseDown = false;
    bool prevMouseState = false;
    bool mouseClicked = false;
    
    float2 directionalInput = {0, 0};
    
    int weaponIdx = 0;
    
    NeonScene(size_t maxInstanceCount, double timestep);
    
    void TogglePause();
    
    void Start();
    
    void ClearLevel();
    
    float3 CameraPosition();
    
    void SelectWeapon(int i);
    void Update(float aspectRatio);
    
    FrameData GetFrameData();
    
    double Timestep() const;
    size_t MaxInstanceCount() const;
    
    float RandomBetween(float a, float b);
    
    const Level& CurrentLevel();
    Weapon& CurrentWeapon();
    
    NumberField CreateField(float2 screenPos, float scale, TextureType tex, bool texFirst, bool keepCamTilt, float4 color = {1, 1, 1, 1});
    void UpdateField(const NumberField& field, float2 extraMargin = {0, 0});
    
    Entity CreateButton(float2 screenPos, float scale, TextureType tex, std::function<void()> action);
    Entity CreateImage(float2 screenPos, float scale, TextureType tex);
private:
    Scene _scene;
    GameClock _clock;
    
    float3 moveDir = {0, 0, 0};
    
    Entity player;
    Entity cam;
    Entity crosshair;
    Entity spreadCircle;
    
    Entity ground;
    
    float spreadMult = 1.0f;
    float prevSpreadMult = 1.0f;
    
    bool threeSixtyShots = false;
    double threeSixtyShotsEndTime = 0;
    
    NumberField enemiesRemainingField;
    NumberField hpField;
    NumberField waveField;
    
    float movementSpeed = 7.0f;
    
    float camDistance = 25;
    float camTilt = 0;
    
    int levelIdx = 0;

    GameState _gameState = GameState::Menu;
    
    int currentWave = 0;
    int currentSubWave = 0;
    double nextSubWaveStartTime = 0;
    
    std::default_random_engine randomEngine;
    
    std::vector<Instance> _instances;
    std::vector<size_t> _groupSizes;
    std::vector<uint32_t> _groupMeshes;
    std::vector<uint32_t> _groupTextures;
    std::vector<uint32_t> _groupShaders;
    
    std::vector<uint32_t> _audios;
    
    std::vector<Entity> _mainMenuUI;
    std::vector<Entity> _pauseMenuUI;
    std::vector<Entity> _gameplayUI;
    std::vector<Entity> _levelClearedUI;
    std::vector<Entity> _gameOverUI;
    
    size_t _maxInstanceCount;
    
    double _timestep;
    double _nextTickTime;
    double _prevRenderTime;
    
    bool _musicPlaying = false;
    
    std::array<Weapon, 3> weapons = {
        Weapon(PlayerProjectile(1, 1.0f, 50.0f, true),
               0.1f,
               0.02f,
               1,
               1.0f,
               Mesh(SHARD_MESH, Material(LIT_SHADER, NO_TEX, float4{0, 1, 0, 1})),
               LASER1_AUDIO),
        Weapon(PlayerProjectile(1, 1.5f, 30.0f, true),
               0.5f,
               0.15f,
               8,
               1.0f,
               Mesh(SHARD_MESH, Material(LIT_SHADER, NO_TEX, float4{1, 0, 0, 1})),
               LASER2_AUDIO),
        
        Weapon(PlayerProjectile(3, 2.0f, 25.0f, false),
               0.8f,
               0.01f,
               1,
               1.5f,
               Mesh(SHARD_MESH, Material(LIT_SHADER, NO_TEX, float4{0, 0.6, 1, 1})),
               LASER3_AUDIO)
    };
    
    void SetGameState(GameState state);
    
    Entity CreatePickup(float3 pos);
    
    void LoadLevel(int i);
    
    void UpdateLevelProgress(double time);
    
    void SpawnSubWave(const Wave::SubWave& subWave);
    
    void Tick(double time);
    void Render(double time, double dt);
    void RenderUI();
};
