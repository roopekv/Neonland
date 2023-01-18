#pragma once

#include <utility>
#include <tuple>
#include <vector>
#include <concepts>

#include "IGroup.hpp"
#include "Pool.hpp"

#include <future>
#include <array>
#include "ThreadPool.hpp"

#include <iostream>

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
class Group : public IGroup {
public:
    Group<Args...>(ComponentMask exclude, std::shared_ptr<Pool<Args>>... pools);
    
    template<typename Func> requires std::invocable<Func, Entity, Args&...>
    void Update(Func func);
    
    template<typename Func> requires std::invocable<Func, Entity, Args&...>
    void UpdateParallel(Func func);
    
    auto GetMembers() -> std::vector<std::tuple<Entity, Args&...>>;
private:
    std::tuple<std::shared_ptr<Pool<Args>>...> pools;
    
    template<typename Func> requires std::invocable<Func, Entity, Args&...>
    auto DoUpdates(Func func, std::shared_ptr<Pool<Args>>... pools);
    
    template<typename Func> requires std::invocable<Func, Entity, Args&...>
    auto DoUpdatesParallel(Func func, std::shared_ptr<Pool<Args>>... pools);

    auto GetMembers(std::shared_ptr<Pool<Args>>... pools) -> std::vector<std::tuple<Entity, Args&...>>;
    
    template<Component Type>
    auto GetPool() -> std::shared_ptr<Pool<Type>>;
    
    template<Component Type, Component... Types>
    auto LockPools(std::array<bool, sizeof...(Args)> lockStatesBeforeLock = {}) -> std::array<bool, sizeof...(Args)>;
    
    template<Component Type, Component... Types>
    void UnlockPools(std::array<bool, sizeof...(Args)> lockStatesBeforeLock);
};

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
Group<Args...>::Group(ComponentMask exclude, std::shared_ptr<Pool<Args>>... pools)
: IGroup{GetComponentMask<Args...>(), exclude}
, pools{std::make_tuple(pools...)} { }

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
template<typename Func> requires std::invocable<Func, Entity, Args&...>
void Group<Args...>::Update(Func func) {
    DoUpdates(func, GetPool<Args>()...);
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
template<typename Func> requires std::invocable<Func, Entity, Args&...>
void Group<Args...>::UpdateParallel(Func func) {
    auto statesBeforeLock = LockPools<Args...>();
    DoUpdatesParallel(func, GetPool<Args>()...);
    UnlockPools<Args...>(statesBeforeLock);
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
auto Group<Args...>::GetMembers() -> std::vector<std::tuple<Entity, Args&...>> {
    return GetMembers(GetPool<Args>()...);
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
template<typename Func> requires std::invocable<Func, Entity, Args&...>
auto Group<Args...>::DoUpdates(Func func, std::shared_ptr<Pool<Args>>... pools) {
    auto entities = groupEntities;
    
    for (auto entity : entities) {
        func(entity, pools->GetComponent(entity.id)...);
    }
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
template<typename Func> requires std::invocable<Func, Entity, Args&...>
auto Group<Args...>::DoUpdatesParallel(Func func, std::shared_ptr<Pool<Args>>... pools) {
    auto entities = groupEntities;
    
    if (entities.empty()) {
        return;
    }
    
    const size_t entityCount = entities.size();
    const auto jobCount = entityCount < ThreadPool::ThreadCount ? entityCount : ThreadPool::ThreadCount;
    const auto entitiesPerJob = entityCount / jobCount;
    
    using iterator = std::vector<Entity>::iterator;
    
    auto UpdateEntitiesInRange = [func, &pools...] (iterator begin, iterator end) {
        for (auto it = begin; it != end; ++it) {
            auto entity = *it;
            func(entity, pools->GetComponent(entity.id)...);
        }
    };
    
    
    std::vector<std::future<void>> jobs;
    jobs.reserve(jobCount);
    
    auto& threadPool = ThreadPool::GetInstance();
    
    auto entityIt = entities.begin();
    for (int i = 0; i < jobCount - 1; i++) {
        std::function<void()> task = std::bind(UpdateEntitiesInRange, entityIt, entityIt + entitiesPerJob);
        jobs.push_back(threadPool.SubmitJob(task));
        entityIt += entitiesPerJob;
    }
    
    UpdateEntitiesInRange(entityIt, entities.end());
    
    for (auto& job : jobs) {
        job.wait();
    }
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
auto Group<Args...>::GetMembers(std::shared_ptr<Pool<Args>>... pools) -> std::vector<std::tuple<Entity, Args&...>> {
    std::vector<std::tuple<Entity, Args&...>> members;
    members.reserve(groupEntities.size());
    
    for (auto entity : groupEntities) {
        auto member = std::forward_as_tuple(entity, pools->GetComponent(entity.id)...);
        members.push_back(member);
    }
    
    return members;
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
template<Component Type>
auto Group<Args...>::GetPool() -> std::shared_ptr<Pool<Type>> {
    return std::get<std::shared_ptr<Pool<Type>>>(pools);
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
template<Component Type, Component... Types>
auto Group<Args...>::LockPools(std::array<bool, sizeof...(Args)> lockStatesBeforeLock) -> std::array<bool, sizeof...(Args)> {
    lockStatesBeforeLock[sizeof...(Args) - sizeof...(Types) - 1] = GetPool<Type>()->IsRemoveLocked();
    
    auto poolPtr = GetPool<Type>();
    
    if (!poolPtr->IsRemoveLocked()) {
        poolPtr->LockRemove();
    }
    
    if constexpr (sizeof...(Types) > 0) {
        return LockPools<Types...>(lockStatesBeforeLock);
    }
    
    return lockStatesBeforeLock;
}

template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
template<Component Type, Component... Types>
void Group<Args...>::UnlockPools(std::array<bool, sizeof...(Args)> lockStatesBeforeLock) {
    bool wasAlreadyLocked = lockStatesBeforeLock[sizeof...(Args) - sizeof...(Types) - 1];
    
    auto poolPtr = GetPool<Type>();
    
    if (!wasAlreadyLocked && poolPtr->IsRemoveLocked()) {
        GetPool<Type>()->UnlockRemove();
    }
    
    if constexpr (sizeof...(Types) > 0) {
        UnlockPools<Types...>(lockStatesBeforeLock);
    }
}
