#include "Scene.hpp"
#include "ThreadPool.hpp"
#include <span>

#include <iostream>

namespace {

template<typename Func>
void UpdateInParallel(std::vector<std::vector<Entity>>& groups, size_t totalCount, Func updateFunc) {
    auto& threadPool = ThreadPool::GetInstance();
    const size_t entitiesPerThread = totalCount / ThreadPool::ThreadCount;
    std::vector<std::future<void>> jobs;
    
    std::vector<std::span<Entity>> ranges;
    int rangesEntityCount = 0;
    
    size_t idx = 0;
    for (auto& group : groups) {
        for (size_t idxInGroup = 0; idxInGroup < group.size();) {
            size_t rangeSize = std::min(group.size() - idxInGroup, entitiesPerThread - rangesEntityCount);
            ranges.emplace_back(group.data() + idxInGroup, rangeSize);
            rangesEntityCount += rangeSize;
            
            if (rangesEntityCount == entitiesPerThread) {
                jobs.push_back(threadPool.SubmitJob(std::bind(updateFunc, ranges, idx)));
                ranges.clear();
                rangesEntityCount = 0;
            }
            
            idx += rangeSize;
            idxInGroup += rangeSize;
        }
    }
    
    if (rangesEntityCount > 0) {
        updateFunc(ranges, idx);
    }
    
    for (auto& job : jobs) {
        job.wait();
    }
}

void UpdateEntitiesInParallel(std::vector<std::vector<Entity>>& groups, size_t totalCount, double timestep) {
    using Ranges = std::vector<std::span<Entity>>;
    UpdateInParallel(groups, totalCount, [ts = timestep](Ranges ranges, size_t idx) {
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
    using Ranges = std::vector<std::span<Entity>>;
    UpdateInParallel(groups, totalCount, [t = timeSinceUpdate,
                                          instances = instances.data()](Ranges ranges, size_t idx) {
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

Scene::Scene(size_t maxEntityCount, double timestep, Camera cam)
: _maxInstanceCount{maxEntityCount}
, _timestep{timestep}
, _instances(maxEntityCount)
, _nextTickTime{_gameClock.Time()}
, camera(cam) { }

Entity& Scene::AddEntity(Entity entity) {
    auto idx = entity.meshIdx;
    _entityGroups.resize(idx + 1);
    _entityGroups[idx].push_back(entity);
    
    _groupSizes.resize(idx + 1, 0);
    _groupSizes[idx]++;
    _instanceCount++;
    
    return _entityGroups[idx].back();
}

void Scene::Update() {
    auto time = _gameClock.Time();
    
    bool useMultithreading = ThreadPool::ThreadCount > 0 && _instanceCount > 100;
    
    while (time >= _nextTickTime) {
        
        if (useMultithreading) {
            UpdateEntitiesInParallel(_entityGroups, _instanceCount, _timestep);
        }
        else {
            UpdateEntitiesInSeries(_entityGroups, _timestep);
        }
        
        _nextTickTime += _timestep;
    }
    
    double timeSinceUpdate = _timestep - (_nextTickTime - time);
    
    if (useMultithreading) {
        UpdateInstancesInParallel(_entityGroups, _instanceCount, _instances, timeSinceUpdate);
    }
    else {
        UpdateInstancesInSeries(_entityGroups, _instances, timeSinceUpdate);
    }
}

FrameData Scene::GetFrameData(float aspectRatio) {
    GlobalUniforms uniforms;
    uniforms.projMatrix = ProjectionMatrix(camera.verticalFoV, aspectRatio, camera.nearPlane, camera.farPlane);
    uniforms.viewMatrix = TranslationMatrix(-camera.position);
    
    FrameData frameData;
    frameData.globalUniforms = uniforms;
    
    frameData.instanceCount = _instanceCount;
    frameData.instances = _instances.data();
    
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
