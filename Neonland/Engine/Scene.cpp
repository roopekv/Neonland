#include "Scene.hpp"
#include "ThreadPool.hpp"
#include <span>

#include <iostream>

namespace {

template<typename Func>
void UpdateInParallel(std::vector<std::vector<Entity>>& groups, size_t totalCount, Func updateFunc) {
    auto& threadPool = ThreadPool::GetInstance();
    const size_t entitiesPerBatch = totalCount / ThreadPool::ThreadCount;
    std::vector<std::future<void>> jobs;
    
    std::vector<std::span<Entity>> batch;
    int batchSize = 0;
    
    size_t idx = 0;
    for (auto& group : groups) {
        for (size_t idxInGroup = 0; idxInGroup < group.size();) {
            size_t subbatchSize = std::min(group.size() - idxInGroup, entitiesPerBatch - batchSize);
            batch.emplace_back(group.data() + idxInGroup, subbatchSize);
            batchSize += subbatchSize;
            
            if (batchSize == entitiesPerBatch) {
                jobs.push_back(threadPool.SubmitJob(std::bind(updateFunc, batch, idx)));
                idx += batchSize;
                batch.clear();
                batchSize = 0;
            }
            
            idxInGroup += subbatchSize;
        }
    }
    
    if (batchSize > 0) {
        updateFunc(batch, idx);
    }
    
    for (auto& job : jobs) {
        job.wait();
    }
}

void UpdateEntitiesInParallel(std::vector<std::vector<Entity>>& groups, size_t totalCount, double timestep) {
    using Batch = std::vector<std::span<Entity>>;
    UpdateInParallel(groups, totalCount, [ts = timestep](Batch ranges, size_t idx) {
        for (auto& range : ranges) {
            for (auto& entity : range) {
                entity.Update(ts);
            }
        }
    });
}

void UpdateInstancesInParallel(std::vector<std::vector<Entity>>& groups,
                               size_t totalCount,
                               std::vector<Instance>& instances,
                               double timeSinceUpdate) {
    using Batch = std::vector<std::span<Entity>>;
    UpdateInParallel(groups, totalCount, [t = timeSinceUpdate,
                                          instances = instances.data()](Batch ranges, size_t idx) {
        for (auto& range : ranges) {
            for (auto& entity : range) {
                instances[idx] = entity.GetInstance(t);
                idx++;
            }
        }
    });
}

void UpdateEntitiesInSeries(std::vector<std::vector<Entity>>& groups, double timestep) {
    for (auto& group : groups) {
        for (auto& enemy : group) {
            enemy.Update(timestep);
        }
    }
}

void UpdateInstancesInSeries(std::vector<std::vector<Entity>>& groups,
                             std::vector<Instance>& instances,
                             double timeSinceUpdate) {
    size_t instanceIdx = 0;
    for (auto& group : groups) {
        for (auto& enemy : group) {
            instances[instanceIdx] = enemy.GetInstance(timeSinceUpdate);
            instanceIdx++;
        }
    }
}

}

Scene::Scene(size_t maxEntityCount, double timestep, Camera cam, GameClock clock)
: _maxInstanceCount{maxEntityCount}
, _timestep{timestep}
, _instances(maxEntityCount)
, clock(clock)
, _nextTickTime{clock.Time()}
, _prevRenderTime{clock.Time()}
, camera(cam) { }

Entity& Scene::AddEntity(Entity&& entity) {
    uint32_t idx = entity.type;
    
    if (idx + 1 > _entityGroups.size()) {
        _entityGroups.resize(idx + 1);
        _groupSizes.resize(idx + 1, 0);
    }
    
    _entityGroups[idx].push_back(entity);
    _groupSizes[idx]++;
    _instanceCount++;
    
    if (_instanceCount > _maxInstanceCount) {
        throw std::length_error("Instance count exceeded the maximum value set.");
    }
    
    return _entityGroups[idx].back();
}

std::vector<Entity>& Scene::GetEntitiesOfType(uint32_t type) {
    return _entityGroups[type];
}

void Scene::Update() {
    auto time = clock.Time();
    
    bool useMultithreading = ThreadPool::ThreadCount > 0 && _instanceCount > ThreadPool::ThreadCount;
    
    while (time >= _nextTickTime) {
        
        OnUpdate();
        
        if (useMultithreading) {
            UpdateEntitiesInParallel(_entityGroups, _instanceCount, _timestep);
        }
        else {
            UpdateEntitiesInSeries(_entityGroups, _timestep);
        }
        
        _nextTickTime += _timestep;
    }
    
    double timeSinceUpdate = _timestep - (_nextTickTime - time);
    
    OnRender(time - _prevRenderTime);
    _prevRenderTime = time;
    
    if (useMultithreading) {
        UpdateInstancesInParallel(_entityGroups, _instanceCount, _instances, timeSinceUpdate);
    }
    else {
        UpdateInstancesInSeries(_entityGroups, _instances, timeSinceUpdate);
    }
}

FrameData Scene::GetFrameData() {
    GlobalUniforms uniforms;
    uniforms.projMatrix = camera.GetProjectionMatrix();
    uniforms.viewMatrix = camera.GetViewMatrix();
    
    FrameData frameData;
    frameData.globalUniforms = uniforms;
    
    frameData.instanceCount = _instanceCount;
    frameData.instances = _instances.data();
    
    frameData.groupCount = static_cast<uint32_t>(_groupSizes.size());
    frameData.groupSizes = _groupSizes.data();
    
    return frameData;
}

double Scene::Timestep() const {
    return _timestep;
}

size_t Scene::InstanceCount() const {
    return _instanceCount;
}

size_t Scene::MaxInstanceCount() const {
    return _maxInstanceCount;
}
