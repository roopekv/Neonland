#include "NeonScene.hpp"

#include <iostream>
#include <random>
#include <atomic>

NeonScene::NeonScene(size_t maxInstanceCount, double timestep, GameClock clock)
: _maxInstanceCount{maxInstanceCount}
, _timestep{timestep}
, _instances(maxInstanceCount)
, clock(clock)
, _nextTickTime{clock.Time()}
, _prevRenderTime{clock.Time()} {
    
    player = _scene.CreateEntity(Transform(), Physics(), Mesh(0), HP());
    cam = _scene.CreateEntity<Camera>();
    crosshair = _scene.CreateEntity(Transform(), Mesh(0));
    
    _scene.Get<Camera>(cam).SetPosition({0, 0, -20});
    _scene.Get<Camera>(cam).SetFarClipPlane(200);
    
    size_t enemyCount = 100'000;
    int columns = sqrt(enemyCount), rows = sqrt(enemyCount);
    
    float gap = 2.0f;
    
    float width = (rows - 1) * (1 + gap);
    float height = (columns - 1) * (1 + gap);
    
    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            Entity enemy = _scene.CreateEntity(Transform(), Physics(), Mesh(1), Enemy(), HP());
            float3 pos = {x * (1 + gap) - width / 2, y * (1 + gap) - height / 2};
            _scene.Get<Transform>(enemy).SetPosition(pos);
            _scene.Get<Physics>(enemy).angularVelocity.z = 30 * distribution(engine);
        }
    }
}

void NeonScene::Update(float aspectRatio) {
    auto time = clock.Time();
    if (aspectRatio != _scene.Get<Camera>(cam).GetAspectRatio()) {
        _scene.Get<Camera>(cam).SetAspectRatio(aspectRatio);
    }
    
    moveDir += {directionalInput.x, directionalInput.y, 0};
    float length = simd_length(moveDir);
    if (length > 0.0f) {
        moveDir /= length;
    }
    
    mousePressed |= !prevMouseState && mouseDown;
    
    bool updated = time >= _nextTickTime;
    while (time >= _nextTickTime) {
        std::atomic<int> totalDamage = 0;
        _scene.GetGroup<Transform, Physics>()->UpdateParallel([timestep = _timestep]
                                                              (auto entity, auto& tf, auto& physics) {
            tf._prevPosition = tf._position;
            tf._prevRotation = tf._rotation;
            tf._position += physics.velocity * timestep;
            tf._rotation += physics.angularVelocity * timestep;
            tf._movedOutsideUpdate = false;
            tf._rotatedOutsideUpdate = false;
        });
        
        auto& playerTf = _scene.Get<Transform>(player);
        auto& playerPhysics = _scene.Get<Physics>(player);
        
        _scene.GetGroup<Transform, Physics, Enemy, HP>()->UpdateParallel([&, t = time]
                                                                     (auto entity, auto& tf, auto& physics, auto& enemy, auto& hp) {
            float dist = simd_distance(tf.GetPosition(), playerTf.GetPosition());
            
            if (dist < playerPhysics.collisionRadius + physics.collisionRadius) {
                if (enemy.cooldownEndTime < t) {
                    totalDamage += enemy.attackDamage;
                    enemy.cooldownEndTime = t + enemy.attackCooldown;
                }
            }
        });
        
        _scene.GetGroup<Transform, Physics, PlayerProjectile>()->Update([&, t = time](auto projectileEntity, auto& projectileTf, auto& projectilePhysics, auto& projectile) {
            std::atomic<bool> didHit = false;
            _scene.GetGroup<Transform, Physics, Enemy, HP>()->UpdateParallel([&](auto entity, auto& enemyTf, auto& enemyPhysics, auto& enemy, auto& hp) {
                float dist = simd_distance(projectileTf.GetPosition(), enemyTf.GetPosition());
                if (dist < enemyPhysics.collisionRadius + projectilePhysics.collisionRadius) {
                    hp.currentHP -= projectile.damage;
                    didHit = true;
                }
            });
            
            if (didHit || projectile.despawnTime < t) {
                _scene.DestroyEntity(projectileEntity);
            }
        });
        
        _scene.Get<HP>(player).currentHP -= totalDamage;
        
        _scene.GetGroup<HP>()->UpdateParallel([&](auto entity, auto& hp) {
            if (hp.currentHP < 0) {
                
                if (entity == player) {
                    gameOver = true;
                }
                else {
                    _scene.DestroyEntity(entity);
                }
            }
        });
        
        _nextTickTime += _timestep;
    }
    
    double timeSinceUpdate = _timestep - (_nextTickTime - time);

    OnRenderUpdate(time, time - _prevRenderTime);
    _prevRenderTime = time;

    auto entitiesWithMesh = _scene.GetGroup<Transform, Mesh>();
    
    _scene.GetGroup<Transform, Mesh>()->UpdateParallel([interpolation = timeSinceUpdate / _timestep]
                                                       (auto entity, auto& tf, auto& mesh) {
        constexpr static auto xAxis = float3{1, 0, 0};
        constexpr static auto yAxis = float3{0, 1, 0};
        constexpr static auto zAxis = float3{0, 0, 1};
        
        float4x4 T, R, S;
        
        S = ScaleMatrix(tf.scale);
        
        float4x4 rX;
        float4x4 rY;
        float4x4 rZ;
        
        if (tf._movedOutsideUpdate) {
            T = TranslationMatrix(tf._position);
            
            rX = RotationMatrix(xAxis, tf._rotation.x);
            rY = RotationMatrix(yAxis, tf._rotation.y);
            rZ = RotationMatrix(zAxis, tf._rotation.z);
        }
        else {
            T = TranslationMatrix(tf._prevPosition + (tf._position - tf._prevPosition) * interpolation);
            
            float3 r = tf._rotation + (tf._rotation - tf._prevRotation) * interpolation;
            rX = RotationMatrix(xAxis, r.x);
            rY = RotationMatrix(yAxis, r.y);
            rZ = RotationMatrix(zAxis, r.z);
        }
        
        R = rZ * rY * rX;
        
        mesh.modelMatrix =  T * R * S;
    });
    
    if (updated) {
        moveDir = {0, 0, 0};
        prevMouseState = mouseDown;
        mousePressed = false;
    }
}

void NeonScene::OnPhysicsUpdate(double time) {

}

void NeonScene::OnRenderUpdate(double time, double dt) {
    _scene.Get<Physics>(player).velocity = moveDir * 5;
    
    if (mouseDown) {
        float3 playerPos = _scene.Get<Transform>(player).GetPosition();
        float3 aimPos = _scene.Get<Camera>(cam).ScreenPointToWorld(mousePos, playerPos.z);
        
        float3 dir = aimPos - playerPos;
        
        float length = simd_length(dir);
        if (length > 0.0f) {
            dir /= length;
        }
        
        float3 spawnPos = playerPos + dir * 0.5f;
        
        auto projectile = _scene.CreateEntity(Transform(spawnPos), Physics(), Mesh(0), PlayerProjectile());
        _scene.Get<Physics>(projectile).velocity = dir * _scene.Get<PlayerProjectile>(projectile).speed;
        _scene.Get<PlayerProjectile>(projectile).despawnTime = time + _scene.Get<PlayerProjectile>(projectile).lifespan;
    }
    
    float depth = _scene.Get<Transform>(crosshair).GetPosition().z;
    float3 pos = _scene.Get<Camera>(cam).ScreenPointToWorld(mousePos, depth);
    _scene.Get<Transform>(crosshair).SetPosition(pos);
    
    mouseDelta = {0, 0};
}

FrameData NeonScene::GetFrameData() {
    GlobalUniforms uniforms;
    uniforms.projMatrix = _scene.Get<Camera>(cam).GetProjectionMatrix();
    uniforms.viewMatrix = _scene.Get<Camera>(cam).GetViewMatrix();
    
    FrameData frameData;
    frameData.globalUniforms = uniforms;
    
    _groupSizes.clear();
    _instances.clear();
    auto meshPool = _scene.GetPool<Mesh>();
    if (meshPool->size() > 0) {
        _groupSizes.resize(meshPool->back().GetMeshIdx() + 1, 0);
        
        for (auto& mesh : *meshPool) {
            Instance instance;
            instance.transform = mesh.modelMatrix;
            _instances.emplace_back(instance);
            _groupSizes[mesh.GetMeshIdx()]++;
        }
    }
    
    assert(_instances.size() < MAX_INSTANCE_COUNT && "Number of instances must be less than MAX_ENTITY_COUNT");
    frameData.instanceCount = _instances.size();
    frameData.instances = _instances.data();
    
    frameData.groupCount = static_cast<uint32_t>(_groupSizes.size());
    frameData.groupSizes = _groupSizes.data();
    
    return frameData;
}

double NeonScene::Timestep() const {
    return _timestep;
}

size_t NeonScene::MaxInstanceCount() const {
    return _maxInstanceCount;
}
