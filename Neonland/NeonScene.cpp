#include "NeonScene.hpp"

#include <iostream>
#include <random>

NeonScene::NeonScene(size_t maxInstanceCount, double timestep, GameClock clock)
: _maxInstanceCount{maxInstanceCount}
, _timestep{timestep}
, _instances(maxInstanceCount)
, clock(clock)
, _nextTickTime{clock.Time()}
, _prevRenderTime{clock.Time()} {
    
    playerEntity = _scene.CreateEntity<Transform, Physics, Mesh>();
    camEntity = _scene.CreateEntity<Camera>();
    crosshairEntity = _scene.CreateEntity<Transform, Mesh>();
    
    auto& cam = _scene.GetComponent<Camera>(camEntity);
    cam.SetPosition({0, 0, -20});
    cam.SetFarClipPlane(200);
    
    int columns = sqrt(MAX_ENEMY_COUNT), rows = sqrt(MAX_ENEMY_COUNT);
    
    float gap = 0.5f;
    
    float width = (rows - 1) * (1 + gap);
    float height = (columns - 1) * (1 + gap);
    
    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    
    auto enemies = _scene.CreateEntities(columns * rows, Transform(), Physics(), Mesh(1));
    
    int i = 0;
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            float3 pos = {x * (1 + gap) - width / 2, y * (1 + gap) - height / 2};
            auto& enemyTf = _scene.GetComponent<Transform>(enemies[i]);
            enemyTf.SetPosition(pos);
            auto& enemyPhysics = _scene.GetComponent<Physics>(enemies[i]);
            enemyPhysics.angularVelocity = 30 * distribution(engine);
            i++;
        }
    }
}

void NeonScene::Update(float aspectRatio) {
    auto& cam = _scene.GetComponent<Camera>(camEntity);
    if (aspectRatio != cam.GetAspectRatio()) {
        cam.SetAspectRatio(aspectRatio);
    }
    
    auto time = clock.Time();

    bool useMultithreading = ThreadPool::ThreadCount > 0;
    auto updateFunc = [timestep = _timestep](auto entity, auto& tf, auto& physics) {
        physics.Update(tf, timestep);
    };
    
    auto physicsEntities = _scene.CreateGroup<Transform, Physics>();
    
    while (time >= _nextTickTime) {
        OnUpdate();
        
        if (useMultithreading) {
            physicsEntities->UpdateParallel(updateFunc);
//            UpdateEntitiesInParallel(_entityGroups, _instanceCount, _timestep);
        }
        else {
            physicsEntities->Update(updateFunc);
//            UpdateEntitiesInSeries(_entityGroups, _timestep);
        }

        _nextTickTime += _timestep;
    }

    double timeSinceUpdate = _timestep - (_nextTickTime - time);

    OnRender(time - _prevRenderTime);
    _prevRenderTime = time;

    auto entitiesWithMesh = _scene.CreateGroup<Transform, Mesh>();
    
    auto matrixUpdateFunc = [t = timeSinceUpdate / _timestep](auto entity, auto& tf, auto& mesh) {
        mesh.modelMatrix = tf.GetMatrix(t);
    };
    
    if (useMultithreading) {
        entitiesWithMesh->UpdateParallel(matrixUpdateFunc);
//        UpdateInstancesInParallel(_entityGroups, _instanceCount, _instances, timeSinceUpdate);
    }
    else {
        entitiesWithMesh->Update(matrixUpdateFunc);
//        UpdateInstancesInSeries(_entityGroups, _instances, timeSinceUpdate);
    }
}

void NeonScene::OnUpdate() {
    auto& playerPhysics = _scene.GetComponent<Physics>(playerEntity);
    playerPhysics.velocity = moveDir * 10;
    
    moveDir = {0, 0, 0};
}

void NeonScene::OnRender(double dt) {
    auto& crosshairTf = _scene.GetComponent<Transform>(crosshairEntity);
    auto& cam = _scene.GetComponent<Camera>(camEntity);
    
    crosshairTf.SetPosition(cam.ScreenPointToWorld(mousePos, crosshairTf.GetPosition().z));
    mouseDelta = {0, 0};
}

FrameData NeonScene::GetFrameData() {
    GlobalUniforms uniforms;
    auto& cam = _scene.GetComponent<Camera>(camEntity);
    uniforms.projMatrix = cam.GetProjectionMatrix();
    uniforms.viewMatrix = cam.GetViewMatrix();
    
    FrameData frameData;
    frameData.globalUniforms = uniforms;
    
    _groupSizes.clear();
    auto meshPool = _scene.GetPool<Mesh>();
    if (meshPool->size() > 0) {
        _groupSizes.resize(meshPool->back().GetMeshIdx() + 1, 0);
        _instances.resize(meshPool->size());
        
        size_t instanceIdx = 0;
        for (auto& mesh : *meshPool) {
            _instances[instanceIdx].transform = mesh.modelMatrix;
            _groupSizes[mesh.GetMeshIdx()]++;
            instanceIdx++;
        }
    }
    
    frameData.instanceCount = meshPool->size();
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
