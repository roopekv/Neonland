#include "NeonScene.hpp"

#include <iostream>
#include <atomic>
#include <map>
#include <bit>

#include "UINumber.hpp"

NeonScene::NeonScene(size_t maxInstanceCount, double timestep, GameClock clock)
: _maxInstanceCount{maxInstanceCount}
, _timestep{timestep}
, _instances(maxInstanceCount)
, clock(clock)
, _nextTickTime{clock.Time()}
, _prevRenderTime{clock.Time()} {
    std::random_device device;
    randomEngine = std::default_random_engine(device());
    
    scoreField = CreateField({0.0025f, 0.005f});
    scoreField.screenOrigin = {0, 1};
    
    healthField = CreateField({0.0025f, 0.005f}, float4{1, 0, 0, 1});
    healthField.screenOrigin = {0, -1};
    
    ground = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 180}, float3{0, 0, 1}), Mesh(PLANE_MESH, GROUND_TEX));
    player = _scene.CreateEntity(Physics(Transform({0, -0.5f, 0})), Transform(), Mesh(SPHERE_MESH), HP(100));
    
    cam = _scene.CreateEntity<Camera>();
    _scene.Get<Camera>(cam).SetPosition({0, 0, -camDistance});
    _scene.Get<Camera>(cam).SetFarClipPlane(camDistance + 10);
    
    crosshair = _scene.CreateEntity(Transform(), Mesh(CROSSHAIR_MESH, NO_TEX, float4{0, 1, 1, 1}));
    _scene.Get<Transform>(crosshair).scale = {0.3, 0.3, 0.3};
    
    spreadCircle = _scene.CreateEntity(Transform(), Mesh(SPREAD_MESH, NO_TEX, float4{0, 1, 1, 1}));
    
    LoadLevel(levelIdx);
}

NumberField NeonScene::CreateField(float2 size, float4 color) {
    NumberField field;
    for (size_t i = 0; i < 10; i++) {
        field.valueUIEntities[i] = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 180}, float3{size.x, size.y, 1.0f}),
                                                       Mesh(PLANE_MESH, ZERO_TEX, color), UINumber(10));
        field.maxValueUIEntities[i] = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 180}, float3{size.x, size.y, 1.0f}),
                                                          Mesh(PLANE_MESH, ZERO_TEX, color), UINumber(10));
    }
    field.slash = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 180}, float3{size.x, size.y, 1.0f}),
                                      Mesh(PLANE_MESH, SLASH_TEX, color));
    field.SetValue(0);
    field.SetMaxValue(0);
    
    return field;
}

void NeonScene::LoadLevel(int i) {
    levelIdx = i;
    
    _scene.Get<Mesh>(ground).texture = CurrentLevel().groundTexture;
    _scene.Get<Transform>(ground).scale = {CurrentLevel().mapSize.x, CurrentLevel().mapSize.y, 1};
    
    gameOver = false;
    levelWon = false;
    
    auto& playerHP = _scene.Get<HP>(player);
    playerHP.currentHP = playerHP.maxHP;
    healthField.SetValue(playerHP.currentHP);
    healthField.SetMaxValue(playerHP.maxHP);
    
    currentWave = 0;
    currentSubWave = 0;
    
    scoreField.SetValue(0);
    scoreField.SetMaxValue(CurrentLevel().waves[currentWave].enemyCount);
    nextSubWaveStartTime = clock.Time() + CurrentLevel().waves[currentWave].subWaves[currentSubWave].duration;
}

const Level& NeonScene::CurrentLevel() {
    return Level::GetLevel(levelIdx);
}

void NeonScene::UpdateLevelProgress(double time) {
    if (levelWon || gameOver) {
        return;
    }
    
    if (time > nextSubWaveStartTime && currentSubWave < CurrentLevel().waves[currentWave].subWaves.size()) {
        SpawnSubWave(CurrentLevel().waves[currentWave].subWaves[currentSubWave]);
        nextSubWaveStartTime = time + CurrentLevel().waves[currentWave].subWaves[currentSubWave].duration;
        currentSubWave++;
    }
    
    bool waveDefeated = scoreField.GetValue() >= scoreField.GetMaxValue();
    if (waveDefeated && currentSubWave >= CurrentLevel().waves[currentWave].subWaves.size()) {
        currentWave++;
        currentSubWave = 0;
        scoreField.SetValue(0);
        
        if (currentWave >= CurrentLevel().waves.size()) {
            levelWon = true;
            currentWave = 0;
            scoreField.SetMaxValue(0);
        }
        else
        {
            scoreField.SetMaxValue(CurrentLevel().waves[currentWave].enemyCount);
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
        mesh.colorMult = 0.0f;
        auto enemy = subWave.type.enemy;
        
        float3 angularVel = {RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f)};
        
        float len = simd_length(angularVel);
        if (len > 1.0f) {
            angularVel /= len;
        }
        
        angularVel *= RandomBetween(45.0f, 90.0f);
        
        Transform tf = Transform(spawnPos, float3{0, 0, 0}, subWave.type.scale);
        _scene.CreateEntity(Physics(tf, float3{0, 0, 0}, angularVel),
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
        currentWeaponIdx = i;
    }
}

void NeonScene::Update(float aspectRatio) {
    double time = clock.Time();
    double dt = time - _prevRenderTime;
    
    if (aspectRatio != _scene.Get<Camera>(cam).GetAspectRatio()) {
        _scene.Get<Camera>(cam).SetAspectRatio(aspectRatio);
    }
    
    moveDir += {directionalInput.x, directionalInput.y, 0};
    float length = simd_length(moveDir);
    if (length > 0.0f) {
        moveDir /= length;
    }
    
    mousePressed |= !prevMouseState && mouseDown;
    
    EarlyRender(time, dt);
    
    bool didTick = time >= _nextTickTime;
    while (time >= _nextTickTime) {
        Tick(time);
        _nextTickTime += _timestep;
    }
    
    LateRender(time, dt);
    
    _prevRenderTime = time;
    mouseDelta = {0, 0};
    
    if (didTick) {
        moveDir = {0, 0, 0};
        prevMouseState = mouseDown;
        mousePressed = false;
    }
}

void NeonScene::EarlyRender(double time, double dt) {
    _scene.Get<Physics>(player).velocity = moveDir * movementSpeed;
}

void NeonScene::Tick(double time) {
    UpdateLevelProgress(time);
    {
        float3 targetPos = _scene.Get<Physics>(player).position;
        _scene.GetGroup<Physics, Enemy>()->UpdateParallel([targetPos](auto entity, auto& physics, auto& enemy) {
            float3 dir = targetPos - physics.position;
            
            float length = simd_length(dir);
            if (length > 0.0f) {
                dir /= length;
            }
            
            physics.velocity = dir * enemy.movementSpeed;
        });
    }
    
    {
        float3 pos = _scene.Get<Physics>(player).position;
        pos.x = std::clamp(pos.x, -CurrentLevel().mapSize.x / 2, CurrentLevel().mapSize.x / 2);
        pos.y = std::clamp(pos.y, -CurrentLevel().mapSize.y / 2, CurrentLevel().mapSize.y / 2);
        _scene.Get<Physics>(player).position = pos;
    }
    
    prevSpreadMult = spreadMult;
    Weapon& weapon = weapons[currentWeaponIdx];
    if (mouseDown && weapon.cooldownEndTime < time) {
        float3 playerPos = _scene.Get<Transform>(player).position;
        float3 aimPos = _scene.Get<Transform>(crosshair).position;
        
        float3 aimDir = aimPos - playerPos;
        aimDir.z = 0;
        
        float length = simd_length(aimDir);
        if (length > 0.0f) {
            aimDir /= length;
        }
        
        float3 spawnPos = playerPos + aimDir * 0.5f * weapon.bulletSize;
        
        for (int i = 0; i < weapon.bulletsPerShot; i++) {
            
            float3 vel = aimDir + float3{-aimDir.y, aimDir.x, 0.0f} * weapon.spread * spreadMult * RandomBetween(-1.0f, 1.0f);
            float length = simd_length(aimDir);
            if (length > 0.0f) {
                aimDir /= length;
            }
            
            float r = std::atan2f(vel.y, vel.x) * RadToDeg;
            
            PlayerProjectile projectile = weapon.bullet;
            projectile.despawnTime = time + projectile.lifespan;
            vel *= projectile.speed;
            
            auto tf = Transform(spawnPos, float3{0, 0, r}, float3{1.0f, 0.25f, 0.25f} * weapon.bulletSize);
            _scene.CreateEntity(Physics(tf, vel), std::move(tf), Mesh(SPHERE_MESH), std::move(projectile));
            
            weapon.cooldownEndTime = time + weapon.cooldown;
        }
        
        spreadMult = std::clamp(spreadMult += 2 * _timestep, 1.0f, 2.0f);
    }
    else if (weapon.cooldownEndTime < time){
        spreadMult = std::clamp(spreadMult -= _timestep, 1.0f, 2.0f);
    }
    
    auto& playerTf = _scene.Get<Transform>(player);
    auto& playerPhysics = _scene.Get<Physics>(player);
    
    std::atomic<int> totalDamage = 0;
    _scene.GetGroup<Transform, Physics, Enemy>()->UpdateParallel([&, t = time](auto entity,
                                                                               auto& tf,
                                                                               auto& physics,
                                                                               auto& enemy) {
        if (Physics::Overlapping(physics, playerPhysics, tf, playerTf)) {
            if (enemy.cooldownEndTime < t) {
                totalDamage += enemy.attackDamage;
                enemy.cooldownEndTime = t + enemy.attackCooldown;
            }
        }
    });
    _scene.Get<HP>(player).currentHP -= totalDamage;
    
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
                
                if (enemyHP.currentHP > 0) {
                    enemyHP.currentHP -= projectile.damage;
                    didHit = true;
                }
            }
        });
        
        if ((didHit && projectile.destructsOnCollision) || projectile.despawnTime < t) {
            _scene.DestroyEntity(projectileEntity);
        }
    });
    
    
    std::atomic<int> entitiesDestroyed = 0;
    _scene.GetGroup<HP>()->UpdateParallel([&](auto entity,
                                              auto& hp) {
        if (hp.currentHP <= 0) {
            
            if (entity == player) {
                gameOver = true;
            }
            else {
                _scene.DestroyEntity(entity);
                entitiesDestroyed++;
            }
        }
    });
    
    healthField.SetValue(_scene.Get<HP>(player).currentHP);
    
    scoreField.SetValue(scoreField.GetValue() + entitiesDestroyed);
    
    _scene.GetGroup<Transform, Physics>()->UpdateParallel([timestep = _timestep](auto entity, auto& tf, auto& physics) {
        Physics::Update(physics, tf, timestep);
    });
}

void NeonScene::LateRender(double time, double dt) {
    const double interpolation = std::clamp((_timestep - (_nextTickTime - time)) / _timestep, 0.0, 1.0);
    
    float interpolatedSpreadMult = prevSpreadMult + (spreadMult - prevSpreadMult) * interpolation;
    float spreadScale = 0.5f + weapons[currentWeaponIdx].spread * 4.0f * interpolatedSpreadMult;
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
    
    {
        float3 playerPos = _scene.Get<Physics>(player).GetInterpolatedPosition(interpolation);
        _scene.Get<Camera>(cam).SetPosition({0.0f, 0.0f, playerPos.z - camDistance});
        float3 topRightPos = _scene.Get<Camera>(cam).ScreenPointToWorld({1.0f, 1.0f}, playerPos.z);
        float hWidth = topRightPos.x;
        float hHeight = topRightPos.y;
        
        float2 mapSize = CurrentLevel().mapSize;
        
        float3 camPos = playerPos;
        camPos.x = std::clamp(camPos.x, std::min(-mapSize.x / 2 + hWidth, 0.0f), std::max(mapSize.x / 2 - hWidth, 0.0f));
        camPos.y = std::clamp(camPos.y, std::min(-mapSize.y / 2 + hHeight, 0.0f), std::max(mapSize.y  / 2 - hHeight, 0.0f));
        camPos.z -= camDistance;
        
        _scene.Get<Camera>(cam).SetPosition(camPos);
    }
    
    {
        float3 playerPos = _scene.Get<Physics>(player).position;
        float depth = playerPos.z - 1.0f;
        float3 crosshairPos = _scene.Get<Camera>(cam).ScreenPointToWorld(mousePos, depth);
        _scene.Get<Transform>(crosshair).position = crosshairPos;
        _scene.Get<Transform>(spreadCircle).position = crosshairPos;
        
        float3 dir = crosshairPos - playerPos;
        dir.z = 0;
        
        float length = simd_length(dir);
        if (length > 0.0f) {
            dir /= length;
        }
        
        float r = std::atan2f(dir.y, dir.x) * RadToDeg;
        _scene.Get<Transform>(player).SetRotation({0, 0, r});
    }
    
    RenderUI();
    
    _scene.GetGroup<Mesh, HP>()->UpdateParallel([dt](auto entity,
                                                     auto& mesh,
                                                     auto& hp) {
        mesh.colorMult = std::min(mesh.colorMult + static_cast<float>(dt) * 0.25f, std::abs(static_cast<float>(hp.currentHP) / hp.maxHP));
    });
    
    _scene.GetGroup<Transform, Mesh>()->UpdateParallel([interpolation](auto entity,
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

void NeonScene::UpdateField(const NumberField& field) {
    auto& camera = _scene.Get<Camera>(cam);
    {
        float3 origin = camera.ScreenPointToWorld(field.screenOrigin, camera.GetPosition().z + camera.GetNearClipPlane() + 0.01f);
        auto updateFieldElementPos = [&origin, this, field](int i, Transform& tf) {
            float ySign = field.screenOrigin.y < 0 ? 1 : -1;
            
            float3 pos = origin;
            pos.x += tf.scale.x * i;
            pos.y += ySign * tf.scale.y / 2 + ySign * 0.0025f;
            tf.position = pos;
        };
        
        updateFieldElementPos(0, _scene.Get<Transform>(field.slash));
        
        for (size_t i = 0; i < field.valueNums.size(); i++) {
            Transform& tf = _scene.Get<Transform>(field.valueUIEntities[i]);
            _scene.Get<UINumber>(field.valueUIEntities[i]).value = field.valueNums[i];
            
            updateFieldElementPos(-static_cast<int>(i) - 1, tf);
        }
        
        for (size_t i = 0; i < field.maxValueNums.size(); i++) {
            Transform& tf = _scene.Get<Transform>(field.maxValueUIEntities[i]);
            _scene.Get<UINumber>(field.maxValueUIEntities[i]).value = field.maxValueNums[i];
            
            updateFieldElementPos(static_cast<int>(i) + 1, tf);
        }
    }
}

void NeonScene::RenderUI() {
    UpdateField(scoreField);
    UpdateField(healthField);
    
    _scene.GetGroup<Mesh, UINumber>()->UpdateParallel([](auto entity, auto& mesh, auto& num) {
        if (num.value < 10) {
            mesh.texture = static_cast<TextureType>(ZERO_TEX + num.value);
            mesh.hidden = false;
        }
        else {
            mesh.hidden = true;
        }
    });
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
    _instances.clear();
    auto meshPool = _scene.GetPool<Mesh>();
    meshPool->Sort();
    
    if (meshPool->size() > 0) {
        const Mesh& firstMesh = (*meshPool)[0];
        auto prevTexture = firstMesh.texture;
        auto prevType = firstMesh.type;
        
        _groupSizes.push_back(0);
        _groupMeshes.push_back(prevType);
        _groupTextures.push_back(prevTexture);
        size_t groupIdx = 0;
        for (auto& mesh : *meshPool) {
            if (mesh.hidden) {
                continue;;
            }
            
            Instance instance;
            instance.transform = mesh.modelMatrix;
            instance.color = mesh.color;
            instance.color.x *= mesh.colorMult;
            instance.color.y *= mesh.colorMult;
            instance.color.z *= mesh.colorMult;
            _instances.emplace_back(instance);
            
            auto type = mesh.type;
            auto texture = mesh.texture;
            if (type != prevType || texture != prevTexture) {
                _groupSizes.push_back(0);
                _groupMeshes.push_back(type);
                _groupTextures.push_back(texture);
                prevTexture = texture;
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
    
    return frameData;
}

double NeonScene::Timestep() const {
    return _timestep;
}

size_t NeonScene::MaxInstanceCount() const {
    return _maxInstanceCount;
}
