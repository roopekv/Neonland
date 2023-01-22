#include "NeonScene.hpp"

#include <iostream>
#include <atomic>
#include <map>
#include <bit>

#include "Material.hpp"
#include "Button.hpp"
#include "Anchor.hpp"
#include "Pickup.hpp"

NeonScene::NeonScene(size_t maxInstanceCount, double timestep)
: _maxInstanceCount{maxInstanceCount}
, _timestep{timestep}
, _instances(maxInstanceCount)
, _clock(GameClock(true))
, _nextTickTime{_clock.Time()}
, _prevRenderTime{_clock.Time()} {
    std::random_device device;
    randomEngine = std::default_random_engine(device());
}

void NeonScene::Start() {
    waveField = CreateField(float2{0, 0.9f}, 1, WAVE_TEX, true, false);
    enemiesRemainingField = CreateField(float2{0, 0.8f}, 1, ENEMIES_REMAINING_TEX, false, false);
    hpField = CreateField(float2{0, 0}, 0.75f, HP_TEX, false, true);
    
    ground = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 0}, float3{0, 0, 1}),
                                 Mesh(PLANE_MESH, Material(LIT_SHADER, GROUND1_TEX)));
    
    auto playerTf = Transform(float3{0, 0, -0.5f}, float3{0, 0, 0}, float3{1, 1, 1});
    player = _scene.CreateEntity(Physics(playerTf),
                                 std::move(playerTf),
                                 Mesh(SPHERE_MESH, Material(LIT_SHADER)),
                                 HP(100));
    
    cam = _scene.CreateEntity<Camera>();
    auto& camera = _scene.Get<Camera>(cam);
    camera.SetPosition({0, 0, -camDistance});
    camera.SetRotation({camTilt, 0, 0});
    
    float farPlane = camTilt == 0 ? (camDistance + 100) : (camDistance + 1) * (std::sin(camera.GetVerticalFoV() * DegToRad) / std::sin(camTilt * DegToRad));
    
    camera.SetFarClipPlane(farPlane);
    
    _scene.Get<Mesh>(ground).material.texture = CurrentLevel().groundTexture;
    _scene.Get<Transform>(ground).scale = {CurrentLevel().mapSize.x, CurrentLevel().mapSize.y, 1};
    
    crosshair = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 0}, float3{0.3, 0.3, 0.3}),
                                    Mesh(CROSSHAIR_MESH, Material(UI_SHADER, NO_TEX, float4{1, 1, 1, 1})));
    
    spreadCircle = _scene.CreateEntity(Transform(),
                                       Mesh(SPREAD_MESH, Material(UI_SHADER, NO_TEX)));
    
    Entity numKeysImg = CreateImage(float2{-1, -1}, 4, NUM_KEYS_TEX);
    _scene.Get<Anchor>(numKeysImg).margin = float3{_scene.Get<Transform>(numKeysImg).scale.x / 2 + 0.5f, _scene.Get<Transform>(numKeysImg).scale.y / 2 + 0.5f, 0};
    
    _gameplayUI = {
        spreadCircle,
        numKeysImg
    };
    
    _levelButtons = {
        CreateButton(float2{0, 0.25f}, 2.5f, LEVEL1_BT_TEX, [this]{LoadLevel(0);}),
        CreateButton(float2{0, 0}, 2.5f, LEVEL2_BT_TEX, [this]{LoadLevel(1);}),
        CreateButton(float2{0, -0.25f}, 2.5f, LEVEL3_BT_TEX, [this]{LoadLevel(2);}),
    };
    
    for (auto bt : _levelButtons) {
        auto lockImg = CreateImage(_scene.Get<Anchor>(bt).screenPosition, 2.0f, LOCK_TEX);
        _scene.Get<Anchor>(lockImg).margin.x = _scene.Get<Transform>(bt).scale.x / 2 + _scene.Get<Transform>(lockImg).scale.x / 2;
        _levelLocks.push_back(lockImg);
    }
    
    _mainMenuUI.insert(_mainMenuUI.end(), _levelButtons.begin(), _levelButtons.end());
    _mainMenuUI.insert(_mainMenuUI.end(), _levelLocks.begin(), _levelLocks.end());
    
    _mainMenuUI.push_back(CreateImage(float2{0, 0.65}, 3.75f, NEONLAND_TEX));
    _mainMenuUI.push_back(CreateButton(float2{0, -0.5f}, 2.5f, QUIT_BT_TEX, [this]{ appShouldQuit = true; }));
    
    Entity byImg = CreateImage(float2{1, -1}, 0.5f, BY_TEX);
    _scene.Get<Anchor>(byImg).margin = float3{-_scene.Get<Transform>(byImg).scale.x / 2 - 0.5f,
        _scene.Get<Transform>(byImg).scale.y / 2 + 0.5f, 0};
    
    _mainMenuUI.push_back(byImg);
    
    _pauseMenuUI = {
        CreateImage(float2{0, 0.375}, 2.5f, PAUSED_TEX),
        CreateButton(float2{0, 0.125}, 2.5f, RESUME_BT_TEX, [this]{TogglePause();}),
        CreateButton(float2{0, -0.125f}, 2.5f, EXIT_BT_TEX, [this]{SetGameState(GameState::Menu);})
    };
    
    _gameOverUI = {
        CreateImage(float2{0, 0.375}, 2.5f, GAME_OVER_TEX),
        CreateButton(float2{0, -0.125f}, 2.5f, EXIT_BT_TEX, [this]{SetGameState(GameState::Menu);})
    };
    
    _levelClearedUI = {
        CreateImage(float2{0, 0.375}, 2.5f, LEVEL_CLEARED_TEX),
        CreateButton(float2{0, -0.125f}, 2.5f, EXIT_BT_TEX, [this]{SetGameState(GameState::Menu);})
    };
    
    SelectWeapon(weaponIdx);
    SetGameState(GameState::Menu);
}

NumberField NeonScene::CreateField(float2 screenPos, float scale, TextureType tex, bool texFirst, bool keepCamTilt, float4 color) {
    NumberField field;
    field.screenPos = screenPos;
    field.texFirst = texFirst;
    
    float rotX = keepCamTilt ? 0 : -camTilt;
    float numScaleX = (static_cast<float>(textureSizes[ZERO_TEX].width) / textureSizes[ZERO_TEX].height) * scale;
    float texScaleX = (static_cast<float>(textureSizes[tex].width) / textureSizes[tex].height) * scale;
    
    field.tex = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{rotX, 0, 180}, float3{texScaleX, scale, 1}),
                                    Anchor(screenPos),
                                    Mesh(PLANE_MESH, Material(UI_SHADER, tex, color)));
    
    for (size_t i = 0; i < 10; i++) {
        field.digitEntities[i] = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{rotX, 0, 180}, float3{numScaleX, scale, 1}),
                                                     Anchor(screenPos),
                                                     Mesh(PLANE_MESH, Material(UI_SHADER, ZERO_TEX, color), true));
    }
    
    UpdateField(field);
    
    return field;
}

void NeonScene::SetUnlockLevel(int lvl) {
    _unlockLevel = lvl;
    
    if (_gameState == GameState::Menu) {
        for (int i = 0; i < _levelLocks.size(); i++) {
            _scene.Get<Mesh>(_levelLocks[i]).hidden = i <= _unlockLevel;
            _scene.Get<Button>(_levelButtons[i]).disabled = i > _unlockLevel;
        }
    }
}

Entity NeonScene::CreateImage(float2 screenPos, float scale, TextureType tex) {
    float rotX = -camTilt;
    float scaleX = (static_cast<float>(textureSizes[tex].width) / textureSizes[tex].height) * scale;
    return _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{rotX, 0, 180}, float3{scaleX, scale, 1}),
                               Anchor(screenPos),
                               Mesh(PLANE_MESH, Material(UI_SHADER, tex)));
}

Entity NeonScene::CreateButton(float2 screenPos, float scale, TextureType tex, std::function<void()> action) {
    float rotX = -camTilt;
    float scaleX = (static_cast<float>(textureSizes[tex].width) / textureSizes[tex].height) * scale;
    return _scene.CreateEntity(Button(action),
                               Transform(float3{0, 0, 0}, float3{rotX, 0, 180}, float3{scaleX, scale, 1}),
                               Anchor(screenPos),
                               Mesh(PLANE_MESH, Material(UI_SHADER, tex)));
}

void NeonScene::TogglePause() {
    if (_gameState == GameState::Paused) {
        SetGameState(GameState::Gameplay);
    }
    else if (_gameState == GameState::Gameplay) {
        SetGameState(GameState::Paused);
    }
}

void NeonScene::SetGameState(GameState state) {
    if (state == GameState::Menu) {
        ClearLevel();
    }
    
    _gameState = state;
    
    _clock.Paused(state != GameState::Gameplay);
    
    hpField.hidden = state != GameState::Gameplay;
    waveField.hidden = state != GameState::Gameplay;
    enemiesRemainingField.hidden = state != GameState::Gameplay;
    
    for (auto entity : _mainMenuUI) {
        _scene.Get<Mesh>(entity).hidden = true;
    }
    for (auto entity : _pauseMenuUI) {
        _scene.Get<Mesh>(entity).hidden = true;
    }
    for (auto entity : _gameplayUI) {
        _scene.Get<Mesh>(entity).hidden = true;
    }
    for (auto entity : _gameOverUI) {
        _scene.Get<Mesh>(entity).hidden = true;
    }
    for (auto entity : _levelClearedUI) {
        _scene.Get<Mesh>(entity).hidden = true;
    }
    
    if (state == GameState::Menu) {
        for (auto entity : _mainMenuUI) {
            _scene.Get<Mesh>(entity).hidden = false;
        }
    }
    else if (state == GameState::Paused) {
        for (auto entity : _pauseMenuUI) {
            _scene.Get<Mesh>(entity).hidden = false;
        }
    }
    else if (state == GameState::Gameplay){
        for (auto entity : _gameplayUI) {
            _scene.Get<Mesh>(entity).hidden = false;
        }
    }
    else if (state == GameState::GameOver) {
        for (auto entity : _gameOverUI) {
            _scene.Get<Mesh>(entity).hidden = false;
        }
    }
    else if (state == GameState::LevelCleared) {
        for (auto entity : _levelClearedUI) {
            _scene.Get<Mesh>(entity).hidden = false;
        }
    }
    
    SetUnlockLevel(_unlockLevel);
}

void NeonScene::ClearLevel() {
    _scene.GetGroup<Enemy>()->Update([this](auto entity, auto& enemy) {
        _scene.DestroyEntity(entity);
    });
    
    _scene.GetGroup<PlayerProjectile>()->Update([this](auto entity, auto& projectile) {
        _scene.DestroyEntity(entity);
    });
    
    _scene.GetGroup<Pickup>()->Update([this](auto entity, auto& projectile) {
        _scene.DestroyEntity(entity);
    });
    
    _scene.Get<Physics>(player).prevPosition = {0, 0, 0};
    _scene.Get<Physics>(player).position = {0, 0, 0};
    _scene.Get<Transform>(player).position = {0, 0, 0};
    _scene.Get<Camera>(cam).SetPosition(CameraPosition());
    
    auto& playerHP = _scene.Get<HP>(player);
    playerHP.Set(playerHP.Max());
    hpField.SetValue(playerHP.Get());
    waveField.SetValue(1);
    
    threeSixtyShots = false;
    threeSixtyShotsEndTime = 0;
    
    currentWave = 0;
    currentSubWave = 0;
    
    spreadMult = 1.0;
    prevSpreadMult = 1.0f;
}

void NeonScene::LoadLevel(int i) {
    levelIdx = i;
    
    SetGameState(GameState::Gameplay);
    
    ClearLevel();
    
    _scene.Get<Mesh>(ground).material.texture = CurrentLevel().groundTexture;
    _scene.Get<Transform>(ground).scale = {CurrentLevel().mapSize.x, CurrentLevel().mapSize.y, 1};
    
    enemiesRemainingField.SetValue(CurrentLevel().waves[currentWave].enemyCount);
    nextSubWaveStartTime = _clock.Time() + CurrentLevel().waves[currentWave].subWaves[currentSubWave].duration;
}

const Level& NeonScene::CurrentLevel() {
    return Level::GetLevel(levelIdx);
}

void NeonScene::UpdateLevelProgress(double time) {
    if (_gameState != GameState::Gameplay) {
        return;
    }
    
    if (time > nextSubWaveStartTime && currentSubWave < CurrentLevel().waves[currentWave].subWaves.size()) {
        SpawnSubWave(CurrentLevel().waves[currentWave].subWaves[currentSubWave]);
        nextSubWaveStartTime = time + CurrentLevel().waves[currentWave].subWaves[currentSubWave].duration;
        currentSubWave++;
    }
    
    bool waveDefeated = enemiesRemainingField.GetValue() <= 0;
    if (waveDefeated && currentSubWave >= CurrentLevel().waves[currentWave].subWaves.size()) {
        currentWave++;
        waveField.SetValue(currentWave + 1);
        currentSubWave = 0;
        
        if (currentWave >= CurrentLevel().waves.size()) {
            
            if (_unlockLevel <= levelIdx) {
                SetUnlockLevel(_unlockLevel + 1);
            }
            
            SetGameState(GameState::LevelCleared);
            _audios.push_back(LEVEL_CLEARED_AUDIO);
        }
        else
        {
            enemiesRemainingField.SetValue(CurrentLevel().waves[currentWave].enemyCount);
            nextSubWaveStartTime = time + CurrentLevel().waves[currentWave].subWaves[currentSubWave].duration;
        }
    }
}

void NeonScene::SpawnSubWave(const Wave::SubWave& subWave) {
    float2 mapSize = CurrentLevel().mapSize;
    
    float3 spawnPos = {0, 0, 0};
    
    for (size_t i = 0; i < subWave.count; i++) {
        float sign = RandomBetween(0.0f, 1.0f) > 0.5f ? -1 : 1;
        
        if (RandomBetween(0.0f, 1.0f) > 0.5f) {
            spawnPos.y = sign * (mapSize.y / 2 + RandomBetween(subWave.type.scale.y, subWave.type.scale.y * 4));
            spawnPos.x = RandomBetween(-mapSize.x / 2, mapSize.x / 2);
        }
        else {
            spawnPos.x = sign * (mapSize.x / 2 + RandomBetween(subWave.type.scale.x, subWave.type.scale.x * 4));
            spawnPos.y = RandomBetween(-mapSize.y / 2, mapSize.y / 2);
        }
        
        spawnPos.z = -subWave.type.scale.z / 2;
        
        auto mesh = subWave.type.mesh;
        mesh.tint = {0, 0, 0, 1};
        auto enemy = subWave.type.enemy;
        
        auto tf = Transform(spawnPos, float3{0, RandomBetween(0.0f, 360.0f), 0}, subWave.type.scale);
        _scene.CreateEntity(Physics(tf, float3{0, 0, 0}, float3{0, -90 * subWave.type.enemy.maxMovementSpeed, 0}, 0.6f),
                            std::move(tf),
                            std::move(mesh),
                            std::move(enemy),
                            HP(subWave.type.maxHP));
    }
}

float NeonScene::RandomBetween(float a, float b) {
    std::uniform_real_distribution<float> distribution(a, b);
    return distribution(randomEngine);
}

void NeonScene::SelectWeapon(int i) {
    if (i > -1 && i < weapons.size()) {
        weaponIdx = i;
        _scene.Get<Mesh>(spreadCircle).tint = CurrentWeapon().projectileMesh.material.color;
        _scene.Get<Mesh>(player).material.color = CurrentWeapon().projectileMesh.material.color;
    }
}

Weapon& NeonScene::CurrentWeapon() {
    return weapons[weaponIdx];
}

Entity NeonScene::CreatePickup(float3 pos) {
    pos.z = -0.5f;
    
    float3 scale = {1, 1, 1};
    
    auto type = Pickup::Health;
    auto mat = Material(LIT_SHADER, SPHERE_HEART_TEX, float4{1, 0, 0, 1});
    
    if (RandomBetween(0.0f, 1.0f) > 0.75) {
        type = Pickup::ThreeSixtyShots;
        mat.texture = SPHERE_360_SHOTS_TEX;
        mat.color = {1, 1, 1, 1};
        
        scale = {2, 2, 2};
        pos.z = -1.0f;
    }
    
    auto tf = Transform(pos, float3{90, 0, 0}, scale);
    return _scene.CreateEntity(Physics(tf, float3{0, 0, 0}, float3{0, 0, 90 * RandomBetween(-1.0f, 1.0f)}),
                               std::move(tf),
                               Pickup(type),
                               Mesh(SPHERE_MESH, mat));
}

void NeonScene::Update(float aspectRatio) {
    _scene.Get<Camera>(cam).SetAspectRatio(aspectRatio);
    
    _audios.clear();
    
    if (!_musicPlaying) {
        _audios.push_back(MUSIC_AUDIO);
        _musicPlaying = true;
    }
    
    double time = _clock.Time();
    double dt = time - _prevRenderTime;
    
    moveDir += {directionalInput.x, directionalInput.y, 0};
    moveDir = VecNormalize(moveDir);
    
    mouseClicked |= prevMouseState && !mouseDown;
    
    bool didTick = time >= _nextTickTime;
    while (time >= _nextTickTime) {
        Tick(time);
        _nextTickTime += _timestep;
    }
    
    Render(time, dt);
    
    _prevRenderTime = time;
    mouseDelta = {0, 0};
    
    if (didTick || _clock.Paused()) {
        moveDir = {0, 0, 0};
        prevMouseState = mouseDown;
        mouseClicked = false;
    }
}

void NeonScene::Tick(double time) {
    UpdateLevelProgress(time);
    
    if (threeSixtyShots && time > threeSixtyShotsEndTime) {
        threeSixtyShots = false;
    }
    
    _scene.Get<Physics>(player).velocity = moveDir * movementSpeed;
    
    {
        float3 targetPos = _scene.Get<Physics>(player).position;
        
        _scene.GetGroup<Physics, Enemy>()->UpdateParallel([targetPos, time](auto entity, auto& physics, auto& enemy) {
            float3 dir = targetPos - physics.position;
            dir.z = 0;
            dir = VecNormalize(dir);
            
            float3 newVel = physics.velocity + dir * enemy.acceleration;
            float len = VecLength(newVel);
            if (len > enemy.maxMovementSpeed) {
                newVel /= len;
                newVel *= enemy.maxMovementSpeed;
            }
            physics.velocity = newVel;
            physics.rotation.z = std::atan2f(dir.y, dir.x) * RadToDeg;
        });
    }
    
    {
        float3 pos = _scene.Get<Physics>(player).position;
        pos.x = std::clamp(pos.x, -CurrentLevel().mapSize.x / 2, CurrentLevel().mapSize.x / 2);
        pos.y = std::clamp(pos.y, -CurrentLevel().mapSize.y / 2, CurrentLevel().mapSize.y / 2);
        _scene.Get<Physics>(player).position = pos;
    }
    
    prevSpreadMult = spreadMult;
    if (mouseDown && CurrentWeapon().cooldownEndTime < time) {
        float3 playerWorldPos = _scene.Get<Physics>(player).position;
        float3 aimPos = _scene.Get<Transform>(crosshair).position;
        
        float2 aimDir = aimPos.xy - playerWorldPos.xy;
        aimDir = VecNormalize(aimDir);
        
        float3 spawnPos = playerWorldPos;
        
        if (!threeSixtyShots) {
            spawnPos += float3{aimDir.x, aimDir.y, 0} * 0.5f * CurrentWeapon().projectileSize;
        }
        
        int directionCount = threeSixtyShots ? 12 : 1;
        
        for (int n = 0; n < directionCount; n++) {
            for (int i = 0; i < CurrentWeapon().projectilesPerShot; i++) {
                float2 vel = aimDir + float2{-aimDir.y, aimDir.x} * CurrentWeapon().spread * spreadMult * RandomBetween(-1.0f, 1.0f);
                vel = VecNormalize(vel);
                
                float4 vel4 = float4{vel.x, vel.y, 0, 1} * RotationMatrix(zAxis, n * 30);
                
                vel = {vel4.x, vel4.y};
                
                float r = std::atan2f(vel.y, vel.x) * RadToDeg;
                
                PlayerProjectile projectile = CurrentWeapon().projectile;
                projectile.despawnTime = time + projectile.lifespan;
                vel *= projectile.speed;
                
                auto mesh = CurrentWeapon().projectileMesh;
                
                auto tf = Transform(spawnPos, float3{0, 0, r}, float3{1.0f, 0.5f, 0.5f} * CurrentWeapon().projectileSize);
                _scene.CreateEntity(Physics(tf, float3{vel.x, vel.y, 0}),
                                    std::move(tf),
                                    std::move(mesh),
                                    std::move(projectile));
                
                CurrentWeapon().cooldownEndTime = time + CurrentWeapon().cooldown;
            }
        }
        
        _audios.push_back(CurrentWeapon().audio);
        
        spreadMult = std::clamp(spreadMult += 4 * _timestep, 1.0f, 2.0f);
    }
    else if (CurrentWeapon().cooldownEndTime < time){
        spreadMult = std::clamp(spreadMult -= 0.5f * _timestep, 1.0f, 2.0f);
    }
    
    {
        auto& playerTf = _scene.Get<Transform>(player);
        auto& playerPhysics = _scene.Get<Physics>(player);
        
        std::atomic<int> totalDamage = 0;
        _scene.GetGroup<Transform, Physics, Enemy>()->UpdateParallel([&, t = time](auto entity,
                                                                                   auto& tf,
                                                                                   auto& physics,
                                                                                   auto& enemy) {
            if (Physics::Overlapping(physics, playerPhysics, tf, playerTf, 0.05f)) {
                if (enemy.cooldownEndTime < t) {
                    totalDamage += enemy.attackDamage;
                    enemy.cooldownEndTime = t + enemy.attackCooldown;
                }
            }
        });
        
        _scene.Get<HP>(player).Decrease(totalDamage);
        if (totalDamage > 0) {
            auto& playerMesh = _scene.Get<Mesh>(player);
            playerMesh.tint.x = std::clamp(playerMesh.tint.x - 0.25f * totalDamage, 0.0f, 1.0f);
            
            if (_scene.Get<HP>(player).Get() <= 0) {
                SetGameState(GameState::GameOver);
                _audios.push_back(GAME_OVER_AUDIO);
            }
            else {
                _audios.push_back(LOSE_HP_AUDIO);
            }
        }
    }
    
    hpField.SetValue(_scene.Get<HP>(player).Get());
    
    {
        auto& playerTf = _scene.Get<Transform>(player);
        auto& playerPhysics = _scene.Get<Physics>(player);
        
        std::atomic<int> heal = 0;
        std::atomic<bool> collected360 = false;
        
        _scene.GetGroup<Pickup, Physics, Transform>()->UpdateParallel([&, this](auto entity,
                                                                                auto& pickup,
                                                                                auto& physics,
                                                                                auto& tf) {
            if (Physics::Overlapping(playerPhysics, physics, playerTf, tf, 0.05f)) {
                
                if (pickup.type == Pickup::Health) {
                    heal += 10;
                }
                else if (pickup.type == Pickup::ThreeSixtyShots) {
                    collected360 = true;
                }
                
                _scene.DestroyEntity(entity);
            }
        });
        
        if (collected360) {
            threeSixtyShots = true;
            threeSixtyShotsEndTime = time + 7.5f;
            _audios.push_back(POWER_UP_AUDIO);
        }
        
        _scene.Get<HP>(player).Increase(heal);
        
        if (heal > 0) {
            _audios.push_back(COLLECT_HP_AUDIO);
        }
    }
    
    _scene.GetGroup<Transform, Physics, PlayerProjectile>()->Update([&, t = time](auto projectileEntity,
                                                                                  auto& projectileTf,
                                                                                  auto& projectilePhysics,
                                                                                  auto& projectile) {
        std::atomic<bool> didHit = false;
        _scene.GetGroup<Transform, Physics, Enemy, HP, Mesh>()->UpdateParallel([&](auto enemyEntity,
                                                                                   auto& enemyTf,
                                                                                   auto& enemyPhysics,
                                                                                   auto& enemy,
                                                                                   auto& enemyHP,
                                                                                   auto& enemyMesh) {
            if (Physics::Overlapping(projectilePhysics, enemyPhysics, projectileTf, enemyTf)) {
                
                if (enemyHP.Get() > 0 && !didHit.exchange(true) && enemyEntity != projectile.hit) {
                    enemyHP.Decrease(projectile.damage);
                    enemyMesh.tint.x = std::clamp(enemyMesh.tint.x - 0.25f * projectile.damage, 0.0f, 1.0f);
                    projectile.hit = enemyEntity;
                    if (enemy.blocksPiercing) {
                        projectile.destructsOnCollision = true;
                    }
                }
            }
        });
        
        if ((didHit && projectile.destructsOnCollision) || projectile.despawnTime < t) {
            _scene.DestroyEntity(projectileEntity);
        }
    });
    
    
    int entitiesDestroyed = 0;
    _scene.GetGroup<HP, Transform, Enemy>()->Update([&](auto entity,
                                                        auto& hp,
                                                        auto& tf,
                                                        auto& enemy) {
        if (hp.Get() < 1) {
            float3 pos = tf.position;
            _scene.DestroyEntity(entity);
            entitiesDestroyed++;
            
            if (RandomBetween(0.0f, 1.0f) > 0.95f) {
                CreatePickup(pos);
            }
        }
    });
    
    if (entitiesDestroyed > 0) {
        _audios.push_back(EXPLOSION_AUDIO);
    }
    
    enemiesRemainingField.SetValue(enemiesRemainingField.GetValue() - entitiesDestroyed);
    
    _scene.GetGroup<Transform, Physics>()->UpdateParallel([timestep = _timestep](auto entity, auto& tf, auto& physics) {
        Physics::Update(physics, tf, timestep);
    });
    
    
    auto physicsBodies = _scene.GetGroup<Transform, Physics>(GetComponentMask<PlayerProjectile>())->GetMembers();
    for (size_t i = 0; i < physicsBodies.size(); i++) {
        for (size_t j = i + 1; j < physicsBodies.size(); j++) {
            Transform& tfA = std::get<1>(physicsBodies[i]);
            Transform& tfB = std::get<1>(physicsBodies[j]);
            
            Physics& physicsA = std::get<2>(physicsBodies[i]);
            Physics& physicsB = std::get<2>(physicsBodies[j]);
            
            float3 aToB = physicsB.position - physicsA.position;
            aToB.z = 0;
            
            float dist = VecLength(aToB);
            float overlap = physicsA.GetScaledCollisionRadius(tfA) + physicsB.GetScaledCollisionRadius(tfB) - dist;
            
            if (overlap > 0) {
                physicsA.position -= aToB * overlap;
                physicsB.position += aToB * overlap;
            }
            
        }
    }
    
}

float3 NeonScene::CameraPosition() {
    auto& camera = _scene.Get<Camera>(cam);
    camera.SetPosition({0.0f, 0.0f, -camDistance});
    float3 topRightPos = camera.ScreenPointToWorld({1.0f, 1.0f}, 0);
    float hWidth = topRightPos.x;
    float vWidth = topRightPos.y;
    
    float2 mapSize = CurrentLevel().mapSize;
    
    float3 playerPos = _scene.Get<Transform>(player).position;
    playerPos.z = 0;
    
    float3 camPos = playerPos;
    camPos.x = std::clamp(camPos.x, std::min(-mapSize.x / 2.0f + hWidth, 0.0f), std::max(mapSize.x / 2.0f - hWidth, 0.0f));
    camPos.y = std::clamp(camPos.y, std::min(-mapSize.y / 2.0f + vWidth, 0.0f), std::max(mapSize.y / 2.0f - vWidth, 0.0f));
    camPos.z -= camDistance;
    
    return camPos;
}

void NeonScene::Render(double time, double dt) {
    const double interpolation = std::clamp((_timestep - (_nextTickTime - time)) / _timestep, 0.0, 1.0);
    
    float interpolatedSpreadMult = prevSpreadMult + (spreadMult - prevSpreadMult) * interpolation;
    float spreadScale = 0.5f + weapons[weaponIdx].spread * 10 * (interpolatedSpreadMult - 0.5f);
    _scene.Get<Transform>(spreadCircle).scale = float3{spreadScale, spreadScale, spreadScale};
    
    _scene.GetGroup<Transform, Physics>()->UpdateParallel([interpolation](auto entity,
                                                                          auto& tf,
                                                                          auto& physics) {
        if (!tf.teleported) {
            tf.position = physics.GetInterpolatedPosition(interpolation);
        }
        
        if (!tf.rotationSet) {
            tf.rotation = physics.GetInterpolatedRotation(interpolation);
        }
    });
    
    _scene.GetGroup<Transform, Enemy>()->Update([](auto entity,
                                                   auto& tf,
                                                   auto& enemy) {
        float angle = tf.rotation.y * DegToRad;
        float minY = std::abs(std::sin(angle)) + std::abs(std::cos(angle));
        tf.position.z *= minY;
    });
    
    _scene.Get<Camera>(cam).SetPosition(CameraPosition());
    
    {
        float3 crosshairPos = _scene.Get<Camera>(cam).ScreenPointToWorld(mousePos, 0);
        _scene.Get<Transform>(crosshair).position = crosshairPos;
        _scene.Get<Transform>(spreadCircle).position = crosshairPos;
    }
    
    RenderUI();
    
    _scene.GetGroup<Mesh, HP>()->UpdateParallel([dt](auto entity,
                                                     auto& mesh,
                                                     auto& hp) {
        float tint = std::min(mesh.tint.x + static_cast<float>(dt) * 0.5f, std::abs(static_cast<float>(hp.Get()) / hp.Max()));
        mesh.tint = float4{tint, tint, tint, 1};
    });
    
    _scene.GetGroup<Transform, Mesh>()->UpdateParallel([](auto entity,
                                                          auto& tf,
                                                          auto& mesh) {
        float4x4 S = ScaleMatrix(tf.scale);
        float4x4 T = TranslationMatrix(tf.position);
        
        float4x4 rX = RotationMatrix(xAxis, tf.rotation.x);
        float4x4 rY = RotationMatrix(yAxis, tf.rotation.y);
        float4x4 rZ = RotationMatrix(zAxis, tf.rotation.z);
        
        float4x4 R = rZ * rY * rX;
        
        mesh.modelMatrix =  T * R * S;
    });
}

void NeonScene::RenderUI() {
    auto& camera = _scene.Get<Camera>(cam);
    
    float2 hpPos = mousePos;
    hpField.screenPos = hpPos;
    
    float2 hpExtraMargin = {0, 0.5f + _scene.Get<Transform>(spreadCircle).scale.y};
    
    UpdateField(hpField, hpExtraMargin);
    UpdateField(waveField);
    UpdateField(enemiesRemainingField);
    
    _scene.GetGroup<Transform, Anchor>()->UpdateParallel([&camera](auto entity,
                                                                   auto& tf,
                                                                   auto& anchor) {
        tf.position = camera.ScreenPointToWorld(anchor.screenPosition, 0) + anchor.margin;
    });
    
    
    float3 mPos = camera.ScreenPointToWorld(mousePos, 0);
    _scene.GetGroup<Button, Transform, Mesh>()->UpdateParallel([mPos](auto entity,
                                                                      auto& bt,
                                                                      auto& tf,
                                                                      auto& mesh) {
        if (!mesh.hidden && !bt.disabled) {
            float leftBound = tf.position.x - tf.scale.x / 2;
            float rightBound = tf.position.x + tf.scale.x / 2;
            
            float lowerBound = tf.position.y - tf.scale.y / 2;
            float upperBound = tf.position.y + tf.scale.y / 2;
            
            bt.highlighted = mPos.x > leftBound && mPos.x < rightBound && mPos.y > lowerBound && mPos.y < upperBound;
        }
        else {
            bt.highlighted = false;
        }
        
        if (!bt.disabled) {
            mesh.tint = bt.highlighted ? float4{1, 1, 1, 1} : float4{0.6, 0.6, 0.6, 1};
        }
        else {
            mesh.tint = {0.3, 0.3, 0.3, 1};
        }
    });
    
    
    if (mouseClicked) {
        for (auto& bt : *_scene.GetPool<Button>()) {
            if (bt.highlighted) {
                bt.onClick();
                _audios.push_back(CLICK_AUDIO);
                break;
            }
        }
    }
}

void NeonScene::UpdateField(const NumberField& field, float2 extraMargin) {
    if (field.hidden) {
        for (auto entity : field.digitEntities) {
            _scene.Get<Mesh>(entity).hidden = true;
        }
        
        _scene.Get<Mesh>(field.tex).hidden = true;
        return;
    }
    
    _scene.Get<Mesh>(field.tex).hidden = false;
    
    for (size_t i = 0; i < field.digitEntities.size(); i++) {
        Mesh& mesh = _scene.Get<Mesh>(field.digitEntities[i]);
        
        if (i < field.digits.size()) {
            mesh.material.texture = static_cast<TextureType>(ZERO_TEX + field.digits[i]);
            mesh.hidden = false;
        }
        else {
            mesh.hidden = true;
        }
    }
    
    float numScaleX = _scene.Get<Transform>(field.digitEntities[0]).scale.x;
    float texScaleX = _scene.Get<Transform>(field.tex).scale.x;
    
    float overallWidth = texScaleX + numScaleX * field.digits.size();
    
    float3 left = {-overallWidth / 2 + extraMargin.x, extraMargin.y, 0};
    float3 texMargin = left;
    
    if (field.texFirst) {
        texMargin.x = left.x + texScaleX / 2;
        left.x += texScaleX;
    }
    else {
        texMargin.x = left.x + numScaleX * field.digits.size() + texScaleX / 2;
        left.x += numScaleX / 4;
    }
    
    _scene.Get<Anchor>(field.tex).margin = texMargin;
    _scene.Get<Anchor>(field.tex).screenPosition = field.screenPos;
    
    for (size_t i = 0; i < field.digitEntities.size(); i++) {
        float3 margin = left;
        margin.x += numScaleX * i;
        _scene.Get<Anchor>(field.digitEntities[i]).margin = margin;
        _scene.Get<Anchor>(field.digitEntities[i]).screenPosition = field.screenPos;
    }
}


FrameData NeonScene::GetFrameData() {
    GlobalUniforms uniforms;
    uniforms.projMatrix = _scene.Get<Camera>(cam).GetProjectionMatrix();
    uniforms.viewMatrix = _scene.Get<Camera>(cam).GetViewMatrix();
    
    FrameData frameData;
    frameData.globalUniforms = uniforms;
    frameData.clearColor = _scene.Get<Camera>(cam).clearColor;
    
    _groupSizes.clear();
    _groupMeshes.clear();
    _groupTextures.clear();
    _groupShaders.clear();
    _instances.clear();
    auto meshPool = _scene.GetPool<Mesh>();
    meshPool->Sort();
    
    if (meshPool->size() > 0) {
        const Mesh& firstMesh = (*meshPool)[0];
        auto prevMat = firstMesh.material;
        auto prevType = firstMesh.type;
        
        _groupSizes.push_back(0);
        _groupMeshes.push_back(prevType);
        _groupTextures.push_back(prevMat.texture);
        _groupShaders.push_back(prevMat.shader);
        size_t groupIdx = 0;
        for (auto& mesh : *meshPool) {
            if (mesh.hidden) {
                continue;;
            }
            
            Instance instance;
            instance.transform = mesh.modelMatrix;
            instance.color = mesh.material.color * mesh.tint;
            _instances.emplace_back(instance);
            
            auto type = mesh.type;
            auto mat = mesh.material;
            if (type != prevType || mat != prevMat) {
                _groupSizes.push_back(0);
                _groupMeshes.push_back(type);
                _groupTextures.push_back(mat.texture);
                _groupShaders.push_back(mat.shader);
                prevMat = mat;
                prevType = type;
                groupIdx++;
            }
            
            _groupSizes[groupIdx]++;
        }
    }
    
    assert(_instances.size() < MAX_INSTANCE_COUNT && "Number of instances must be less than MAX_ENTITY_COUNT");
    frameData.instanceCount = _instances.size();
    frameData.instances = _instances.data();
    
    frameData.groupCount = static_cast<uint32_t>(_groupSizes.size());
    frameData.groupSizes = _groupSizes.data();
    frameData.groupMeshes = _groupMeshes.data();
    frameData.groupTextures = _groupTextures.data();
    frameData.groupShaders = _groupShaders.data();
    
    frameData.audios = _audios.data();
    frameData.audioCount = _audios.size();
    
    return frameData;
}

double NeonScene::Timestep() const {
    return _timestep;
}

size_t NeonScene::MaxInstanceCount() const {
    return _maxInstanceCount;
}
