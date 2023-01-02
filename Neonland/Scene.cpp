#include "Scene.hpp"

Scene::Scene(size_t maxEntityCount, double timestep)
: _maxInstanceCount{maxEntityCount}
, _timestep{timestep}
, _instanceBuffer{new Instance[maxEntityCount]}
, camPos{0, 0, 0}
, _nextTickTime{_gameClock.Time()}
, _instanceCount{0} {}

Scene::~Scene() {
    delete[] _instanceBuffer;
}

Scene::Scene(const Scene& other)
: _maxInstanceCount{other._maxInstanceCount}
, _timestep{other._timestep}
, _instanceBuffer{new Instance[other._maxInstanceCount]}
, camPos{0, 0, 0}
, _nextTickTime{_gameClock.Time()}
, _instanceCount{0} {
    memcpy(_instanceBuffer, other._instanceBuffer, sizeof(_instanceBuffer) * _maxInstanceCount);
}

Scene& Scene::operator=(const Scene& rhs) {
    if (_maxInstanceCount != rhs._maxInstanceCount) {
        delete[] _instanceBuffer;
        _instanceBuffer = new Instance[_maxInstanceCount];
    }
    memcpy(_instanceBuffer, rhs._instanceBuffer, sizeof(_instanceBuffer) * _maxInstanceCount);
    return *this;
}

Entity& Scene::AddEntity(Entity entity) {
    auto idx = entity.meshIdx;
    entityGroups.resize(idx + 1);
    entityGroups[idx].push_back(entity);
    
    _groupSizes.resize(idx + 1, 0);
    _groupSizes[idx]++;
    _instanceCount++;
    
    return entityGroups[idx].back();
}

void Scene::Update() {
    auto time = _gameClock.Time();
    
    while (time >= _nextTickTime) {
        for (auto& group : entityGroups) {
            for (auto& enemy : group) {
                enemy.Update(_timestep);
            }
        }
        _nextTickTime += _timestep;
    }
    
    double timeSinceUpdate = _timestep - (_nextTickTime - time);

    size_t instanceIdx = 0;
    for (auto& group : entityGroups) {
        for (auto& enemy : group) {
            _instanceBuffer[instanceIdx] = enemy.GetInstance(timeSinceUpdate);
            instanceIdx++;
        }
    }
}

FrameData Scene::GetFrameData(float aspectRatio) {
    GlobalUniforms uniforms;
    uniforms.projMatrix = ProjectionMatrix(60, aspectRatio, 0.1, 100);
    uniforms.viewMatrix = TranslationMatrix(-camPos);
    
    FrameData frameData;
    frameData.globalUniforms = uniforms;
    
    frameData.instanceCount = _instanceCount;
    frameData.instances = _instanceBuffer;
    
    frameData.groupCount = _groupSizes.size();
    frameData.groupSizes = _groupSizes.data();
    
    return frameData;
}

size_t Scene::InstanceCount() const {
    return _instanceCount;
}

size_t Scene::MaxInstanceCount() const {
    return _maxInstanceCount;
}
