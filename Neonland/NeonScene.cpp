#include "NeonScene.hpp"

#include <iostream>
#include <atomic>
#include <map>
#include <bit>

#include "Material.hpp"

NeonScene::NeonScene(size_t maxInstanceCount, double timestep, GameClock clock)
: _maxInstanceCount{maxInstanceCount}
, _timestep{timestep}
, _instances(maxInstanceCount)
, clock(clock)
, _nextTickTime{clock.Time()}
, _prevRenderTime{clock.Time()} {
    std::random_device device;
    randomEngine = std::default_random_engine(device());
    
    enemiesRemainingField = CreateField({56.0f / 96.0f, 1.0f}, ENEMIES_REMAINING_TEX);
    _scene.Get<Transform>(enemiesRemainingField.text).scale = float3{832.0f / 96.0f, 1.0f, 1.0f};
    
    healthField = CreateField(float2{56.0f / 96.0f, 1.0f} * 0.75f, HP_TEX);
    _scene.Get<Transform>(healthField.text).scale = float3{112.0f / 96.0f, 1.0f, 1.0f} * 0.75f;
    
    waveField = CreateField(float2{56.0f / 96.0f, 1.0f}, WAVE_TEX);
    _scene.Get<Transform>(waveField.text).scale = float3{288.0f / 96.0f, 1.0f, 1.0f};
    waveField.textFirst = true;
    
    ground = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 0}, float3{0, 0, 1}),
                                 Mesh(PLANE_MESH, Material(LIT_SHADER, GROUND0_TEX)));
    
    auto playerTf = Transform(float3{0, 0, -0.5f}, float3{0, 0, 0}, float3{1, 1, 1});
    player = _scene.CreateEntity(Physics(playerTf),
                                 std::move(playerTf),
                                 Mesh(SPHERE_MESH, Material(LIT_SHADER)),
                                 HP(100));
    
    cam = _scene.CreateEntity<Camera>();
    _scene.Get<Camera>(cam).SetPosition({0, 0, -camDistance});
    _scene.Get<Camera>(cam).SetFarClipPlane(camDistance + 10);
    
    crosshair = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 0}, float3{0.3, 0.3, 0.3}),
                                    Mesh(CROSSHAIR_MESH, Material(UI_SHADER, NO_TEX, float4{0, 1, 1, 1}), false));
    
    spreadCircle = _scene.CreateEntity(Transform(),
                                       Mesh(SPREAD_MESH, Material(UI_SHADER, NO_TEX), false));
    
    SelectWeapon(weaponIdx);
    LoadLevel(levelIdx);
}

NumberField NeonScene::CreateField(float2 size, TextureType suffix, float4 color) {
    NumberField field;
    for (size_t i = 0; i < 10; i++) {
        field.valueUIEntities[i] = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 180}, float3{size.x, size.y, 1}),
                                                       Mesh(PLANE_MESH, Material(UI_SHADER, ZERO_TEX, color), true));
    }
    field.text = _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, 0, 180}, float3{1, 1, 1}),
                                       Mesh(PLANE_MESH, Material(UI_SHADER, suffix, color)));
    
    return field;
}

void NeonScene::LoadLevel(int i) {
    levelIdx = i;
    
    _scene.Get<Mesh>(ground).material.texture = CurrentLevel().groundTexture;
    _scene.Get<Transform>(ground).scale = {CurrentLevel().mapSize.x, CurrentLevel().mapSize.y, 1};
    
    gameOver = false;
    levelWon = false;
    
    auto& playerHP = _scene.Get<HP>(player);
    playerHP.Set(playerHP.Max());
    healthField.SetValue(playerHP.Get());
    waveField.SetValue(1);
    
    currentWave = 0;
    currentSubWave = 0;
    
    enemiesRemainingField.SetValue(CurrentLevel().waves[currentWave].enemyCount);
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
    
    bool waveDefeated = enemiesRemainingField.GetValue() <= 0;
    if (waveDefeated && currentSubWave >= CurrentLevel().waves[currentWave].subWaves.size()) {
        currentWave++;
        waveField.SetValue(currentWave + 1);
        currentSubWave = 0;
        
        if (currentWave >= CurrentLevel().waves.size()) {
            levelWon = true;
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
            spawnPos.y = sign * (mapSize.y / 2 + RandomBetween(subWave.type.scale.y, subWave.type.scale.y * 8));
            spawnPos.x = RandomBetween(-mapSize.x / 2, mapSize.x / 2);
        }
        else {
            spawnPos.x = sign * (mapSize.x / 2 + RandomBetween(subWave.type.scale.x, subWave.type.scale.x * 8));
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

void NeonScene::Update(float aspectRatio) {
    double time = clock.Time();
    double dt = time - _prevRenderTime;
    
    if (aspectRatio != _scene.Get<Camera>(cam).GetAspectRatio()) {
        _scene.Get<Camera>(cam).SetAspectRatio(aspectRatio);
    }
    
    moveDir += {directionalInput.x, directionalInput.y, 0};
    moveDir = VecNormalize(moveDir);
    
    mousePressed |= !prevMouseState && mouseDown;
    
    bool didTick = time >= _nextTickTime;
    while (time >= _nextTickTime) {
        Tick(time);
        _nextTickTime += _timestep;
    }
    
    Render(time, dt);
    
    _prevRenderTime = time;
    mouseDelta = {0, 0};
    
    if (didTick) {
        moveDir = {0, 0, 0};
        prevMouseState = mouseDown;
        mousePressed = false;
    }
}

void NeonScene::Tick(double time) {
    UpdateLevelProgress(time);
    
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
            physics.prevRotation.z = physics.rotation.z;
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
        float3 playerPos = _scene.Get<Transform>(player).position;
        float3 aimPos = _scene.Get<Transform>(crosshair).position;
        
        float3 aimDir = aimPos - playerPos;
        aimDir.z = 0;
        
        aimDir = VecNormalize(aimDir);
        
        float3 spawnPos = playerPos + aimDir * 0.5f * CurrentWeapon().projectileSize;
        
        for (int i = 0; i < CurrentWeapon().projectilesPerShot; i++) {
            
            float3 vel = aimDir + float3{-aimDir.y, aimDir.x, 0.0f} * CurrentWeapon().spread * spreadMult * RandomBetween(-1.0f, 1.0f);
            vel = VecNormalize(vel);
            
            float r = std::atan2f(vel.y, vel.x) * RadToDeg;
            
            PlayerProjectile projectile = CurrentWeapon().projectile;
            projectile.despawnTime = time + projectile.lifespan;
            vel *= projectile.speed;
            
            auto mesh = CurrentWeapon().projectileMesh;
            
            auto tf = Transform(spawnPos, float3{0, 0, r}, float3{1.0f, 0.5f, 0.5f} * CurrentWeapon().projectileSize);
            _scene.CreateEntity(Physics(tf, vel), std::move(tf), std::move(mesh), std::move(projectile));
            
            CurrentWeapon().cooldownEndTime = time + CurrentWeapon().cooldown;
        }
        
        spreadMult = std::clamp(spreadMult += 4 * _timestep, 1.0f, 2.0f);
    }
    else if (CurrentWeapon().cooldownEndTime < time){
        spreadMult = std::clamp(spreadMult -= 0.5f * _timestep, 1.0f, 2.0f);
    }
    
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
        if (hp.Get() <= 0) {
            
            if (entity == player) {
                gameOver = true;
            }
            else {
                _scene.DestroyEntity(entity);
                entitiesDestroyed++;
            }
        }
    });
    
    healthField.SetValue(_scene.Get<HP>(player).Get());
    
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

void NeonScene::Render(double time, double dt) {
    const double interpolation = std::clamp((_timestep - (_nextTickTime - time)) / _timestep, 0.0, 1.0);
    
    float interpolatedSpreadMult = prevSpreadMult + (spreadMult - prevSpreadMult) * interpolation;
    float spreadScale = 0.5f + weapons[weaponIdx].spread * 10 * (interpolatedSpreadMult - 1);
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
        _scene.Get<Camera>(cam).SetPosition({0.0f, 0.0f, -camDistance});
        float3 topRightPos = _scene.Get<Camera>(cam).ScreenPointToWorld({1.0f, 1.0f}, 0);
        float hWidth = topRightPos.x;
        float hHeight = topRightPos.y;
        
        float2 mapSize = CurrentLevel().mapSize;
        
        float3 playerPos = _scene.Get<Physics>(player).GetInterpolatedPosition(interpolation);
        playerPos.z = 0;
        
        float3 camPos = playerPos;
        camPos.x = std::clamp(camPos.x, std::min(-mapSize.x / 2 + hWidth, 0.0f), std::max(mapSize.x / 2 - hWidth, 0.0f));
        camPos.y = std::clamp(camPos.y, std::min(-mapSize.y / 2 + hHeight, 0.0f), std::max(mapSize.y  / 2 - hHeight, 0.0f));
        camPos.z -= camDistance;
        
        _scene.Get<Camera>(cam).SetPosition(camPos);
    }
    
    {
        float3 playerPos = _scene.Get<Physics>(player).position;
        float3 crosshairPos = _scene.Get<Camera>(cam).ScreenPointToWorld(mousePos, 0);
        _scene.Get<Transform>(crosshair).position = crosshairPos;
        _scene.Get<Transform>(spreadCircle).position = crosshairPos;
        
        float3 dir = crosshairPos - playerPos;
        dir.z = 0;
        dir = VecNormalize(dir);
        
        if (!clock.Paused()) {
            float r = std::atan2f(dir.y, dir.x) * RadToDeg;
            _scene.Get<Transform>(player).SetRotation({0, 0, r});
        }
    }
    
    RenderUI();
    
    _scene.GetGroup<Mesh, HP>()->UpdateParallel([dt](auto entity,
                                                     auto& mesh,
                                                     auto& hp) {
        float tint = std::min(mesh.tint.x + static_cast<float>(dt) * 0.5f, std::abs(static_cast<float>(hp.Get()) / hp.Max()));
        mesh.tint = float4{tint, tint, tint, 1};
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

void NeonScene::UpdateField(const NumberField& field, float3 center) {
    auto& textTf = _scene.Get<Transform>(field.text);
    float numWidth = _scene.Get<Transform>(field.valueUIEntities[0]).scale.x;
    
    float overallWidth = textTf.scale.x + numWidth * field.valueNums.size();
    
    float3 left = center;
    left.x -= overallWidth / 2;
    
    textTf.position.y = left.y;
    if (field.textFirst) {
        textTf.position.x = left.x + textTf.scale.x / 2;
        left.x += textTf.scale.x;
    }
    else {
        textTf.position.x = left.x + numWidth * field.valueNums.size() + textTf.scale.x / 2;
        left.x += numWidth / 4;
    }
    
    for (size_t i = 0; i < field.valueUIEntities.size(); i++) {
        Mesh& mesh = _scene.Get<Mesh>(field.valueUIEntities[i]);
        
        if (i < field.valueNums.size()) {
            mesh.material.texture = static_cast<TextureType>(ZERO_TEX + field.valueNums[i]);
            mesh.hidden = false;
        }
        else {
            mesh.hidden = true;
        }
    }
    
    for (size_t i = 0; i < field.valueNums.size(); i++) {
        Transform& tf = _scene.Get<Transform>(field.valueUIEntities[i]);
        float3 pos = left;
        pos.x += tf.scale.x * i;
        tf.position = pos;
    }
}

void NeonScene::RenderUI() {
    auto& camera = _scene.Get<Camera>(cam);
    float3 center = camera.ScreenPointToWorld({0, 1}, 0);
    center.y -= _scene.Get<Transform>(waveField.text).scale.y;
    UpdateField(waveField, center);
    
    center.y -= _scene.Get<Transform>(enemiesRemainingField.text).scale.y * 1.5f;
    UpdateField(enemiesRemainingField, center);
    
    center = _scene.Get<Transform>(spreadCircle).position;
    center.y += _scene.Get<Transform>(spreadCircle).scale.y + 0.5f;
    UpdateField(healthField, center);
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
    
    return frameData;
}

double NeonScene::Timestep() const {
    return _timestep;
}

size_t NeonScene::MaxInstanceCount() const {
    return _maxInstanceCount;
}
