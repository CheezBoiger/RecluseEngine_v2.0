//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/RGUID.hpp"

#include <vector>
#include <map>

namespace Recluse {
namespace ECS {

typedef Hash64  ComponentUUID;
typedef U32     ComponentUpdateFlags;
typedef Hash64  RegistryUUID;

enum ComponentUpdateFlag
{
    ComponentUpdateFlag_None = 0
};

// Declaration semantics used for editor.
#define REDITOR(attribute, ...)

// Attribute semantics for the editor.
#define RATTRIBUTE(varName, varValue)

// Call this macro when declareing a component. This will be used by the engine to determine 
// the proper calls to be made to the GameObject.
#define R_COMPONENT_DECLARE(_class) \
    public: \
    static Recluse::ECS::ComponentUUID classGUID() { return recluseHashFast(#_class, sizeof(#_class)); } \
    virtual Recluse::ECS::ComponentUUID getClassGUID() const override { return _class::classGUID(); }

#define R_COMPONENT_REGISTRY_DECLARE(_class) \
    public: \
    static Recluse::ECS::RegistryUUID classGUID() { return recluseHashFast(#_class, sizeof(#_class)); } 

class AbstractComponent : public Serializable
{
public:
    AbstractComponent(RGUID guid, const RGUID& ownerGuid = RGUID())
        : m_ownerGuid(ownerGuid)
        , m_cuuid(guid)
    { }

    virtual ~AbstractComponent() { }

    void initialize() { onInitialize(); }
    void release() { onRelease(); }
    void cleanUp() { onCleanUp(); }

    virtual ECS::ComponentUUID  getClassGUID() const { return 0; }

    // Set the component owner.
    void                        setOwner(RGUID owner) { m_ownerGuid = owner; }
    // Check if the component has an owner.
    Bool                        hasOwner() const { return m_ownerGuid.isValid(); }

    // Get the component owner.
    RGUID                       getOwner() const { return m_ownerGuid; }

    // Get the component uuid.
    RGUID                       getUUID() const { return m_cuuid; }

    virtual void                onRelease() = 0;
    virtual void                onCleanUp() = 0;
    virtual void                onInitialize() = 0;

private:
    // Game object owner of this component.
    RGUID                   m_ownerGuid;

    // The unique id of the component.
    RGUID                   m_cuuid;
};

// Component abstraction class. This is mainly a container holding 
// data that is to be processed or read by GameSystems. 
class R_PUBLIC_API Component : public AbstractComponent
{
public:
    typedef Component Super;
    static const U64 unknownComponent = ~0u;

    // Default component construction.
    Component(RGUID guid = generateRGUID(), const RGUID& ownerGuid = RGUID())
        : AbstractComponent(guid, ownerGuid)
        , m_enable(false)
    { }

    virtual ~Component() { }

    virtual ResultCode serialize(Archive* pArchive) const override { return RecluseResult_NoImpl; }

    virtual ResultCode deserialize(Archive* pArchive) override { return RecluseResult_NoImpl; }

    // Check if the component is enabled.
    REDITOR(RATTRIBUTE("visible", "public"))
    Bool            isEnabled() const { return m_enable; }

    // Enable the component.
    REDITOR(RATTRIBUTE("visible", "public"))
    void            setEnable(Bool enable) { m_enable = enable; if (enable) onEnable(); }

    // Initialize the component.
    void            onInitialize() override
    {
        m_enable = true;
    }

    // clean up the component. Does not release it!
    void            onCleanUp() override
    {
        m_enable = false;
    }

    // An actual release of the component. This will require that this object be destroyed completely,
    // and freed back to the system.
    void onRelease() override
    {
        cleanUp();
    }

    // Set update flags for the given component. The values are mainly specific to the system itself.
    // 0 is always a clear value.
    void                        setUpdateFlags(ComponentUpdateFlags updateFlags) { m_updateFlags = updateFlags; }

    // Get the given update flags.
    ComponentUpdateFlags        getUpdateFlags() const { return m_updateFlags; }

    // Clear all update flags from the component.
    void                        clearUpdateFlags() { m_updateFlags = ComponentUpdateFlag_None; }

private:

    // Check on when this component is enabled.
    virtual void    onEnable() { }

    // Flag whether this component is enabled or not.
    Bool                    m_enable;

    // Component update flags used by the system itself.
    ComponentUpdateFlags    m_updateFlags;
};


class AbstractRegistry : public Serializable
{
public:
    virtual ~AbstractRegistry() { }

    template<typename Registry>
    static AbstractRegistry* allocate() 
    {
        return new Registry();
    }

    static RecluseResult free(AbstractRegistry* registry)
    {
        if (registry)
            delete registry;
        return RecluseResult_Ok;
    }

    // Clear all components in the registry.
    void                    clearAll()   { onClearAll(); }
    
    ResultCode              initialize()
    {
        return onInitialize();
    }

    ResultCode              cleanUp()
    {
        return onCleanUp();
    }


    // Available functions to query from the given system.
    U32     getTotalComponents() const { return m_numberOfComponentsAllocated; }

protected:

    // Allows initializing the system before on intialize().
    virtual ResultCode      onInitialize()                  { return RecluseResult_NoImpl; }

    // Allows cleaning up the system before releasing.
    virtual ResultCode      onCleanUp()                     { return RecluseResult_NoImpl; }

    // Intended to clear all components from the game world.
    virtual void            onClearAll()                       { }

    U32             m_numberOfComponentsAllocated;
};


// Component Registry handles the management of components. 
// This helps let the programmer manage component allocations and access.
//! Registry is the required definition for the given component management, which is to 
//! define how to allocate, free, and update all components when the application interacts 
//! with. Do not inherit directly from components, instead inherit from this!
template<typename TypeComponent>
class ComponentRegistry : public AbstractRegistry
{
public:
    virtual ~ComponentRegistry() { }

    static ComponentUUID componentGUID()
    {
        return TypeComponent::classGUID();
    }

    // Allocates a component from the system pool.
    // Returns R_RESULT_OK if the system successfully allocated the component instance.
    ResultCode allocateComponent(const RGUID& owner)  
    {
        ResultCode err = onAllocateComponent(owner);
        if (err == RecluseResult_Ok) 
        {
            m_numberOfComponentsAllocated += 1;
        }
        return err;
    }

    // Frees up a component from the system pool.
    // Returns R_RESULT_OK if the system successfully freed the component instance.
    ResultCode freeComponent(const RGUID& owner)
    {
        ResultCode err = onFreeComponent(owner);
        if (err == RecluseResult_Ok) m_numberOfComponentsAllocated -= 1;
        return err;
    }

    // Get all components handled by the system. This is required, as systems must use this to 
    // iterate for all of their components.
    virtual std::vector<TypeComponent*> getAllComponents() { return { }; }

    // Get a component from the system. Return nullptr, if the component doesn't match the given 
    // game entity key.
    virtual TypeComponent*              getComponent(const RGUID& entityKey) { return nullptr; }

    virtual ResultCode serialize(Archive* pArchive) const override { return RecluseResult_NoImpl; }

    virtual ResultCode deserialize(Archive* pArchive) override { return RecluseResult_NoImpl; }

protected:
    // Allocation calls. These must be overridden, as they will be called by external systems,
    // when required. 
    virtual ResultCode onAllocateComponent(const RGUID& owner) = 0;

    // Free calls. These must be overridden, as they will be called by external systems when
    // required.
    virtual ResultCode onFreeComponent(const RGUID& owner) = 0;
};


// Global registry that holds all given component registries.
class Registry
{
public:
    template<typename ComponentType>
    ComponentRegistry<ComponentType>* getComponentRegistry() const
    {
        ECS::ComponentUUID uuid = ComponentType::classGUID();
        auto it = m_records.find(uuid);
        if (it != m_records.end())
        {
            return static_cast<ECS::ComponentRegistry<ComponentType>*>(it->second);
        }
        return nullptr;
    }

    template<typename RegistryType>
    ResultCode addComponentRegistry()
    {
        ECS::ComponentUUID uuid = RegistryType::componentGUID();
        auto it = m_records.find(uuid);
        if (it == m_records.end())
        {
            ECS::AbstractRegistry* registry = ECS::AbstractRegistry::allocate<RegistryType>();
            m_records.insert(std::make_pair(uuid, registry));
            return RecluseResult_Ok;
        }
        return RecluseResult_Failed;
    }

    template<typename ComponentType>
    ResultCode makeComponent(const RGUID& entityId, Bool enableByDefault = false)
    {
        ECS::ComponentUUID uuid = ComponentType::classGUID();
        auto it = m_records.find(uuid);
        if (it != m_records.end())
        {
            ECS::ComponentRegistry<ComponentType>* registry = static_cast<ECS::ComponentRegistry<ComponentType>*>(it->second);
            registry->allocateComponent(entityId);
            // Get the component and set the default for it.
            ComponentType* component = registry->getComponent(entityId);
            component->setEnable(enableByDefault);
            return RecluseResult_Ok;
        }
        return RecluseResult_Failed;
    }

    template<typename ComponentType>
    ResultCode removeComponent(const RGUID& entityId)
    {
        ECS::ComponentUUID uuid = ComponentType::classGUID();
        auto it = m_records.find(uuid);
        if (it != m_records.end())
        {
            ECS::ComponentRegistry<ComponentType>* registry = static_cast<ECS::ComponentRegistry<ComponentType>*>(it->second);
            registry->freeComponent(entityId);
            return RecluseResult_Ok;
        }
        return RecluseResult_Failed;
    }

    template<typename ComponentType>
    ComponentType* getComponent(const RGUID& guid) const
    {
        ECS::ComponentUUID uuid = ComponentType::classGUID();
        auto it = m_records.find(uuid);
        if (it != m_records.end())
        {
            ECS::ComponentRegistry<ComponentType>* registry = static_cast<ECS::ComponentRegistry<ComponentType>*>(it->second);
            return registry->getComponent(guid);
        }
        return nullptr;
    }

    void cleanUp()
    {
        for (auto registry : m_records)
        {
            registry.second->cleanUp();
            ECS::AbstractRegistry::free(registry.second);
        }
        m_records.clear();
    }
private:
    // Records kept, that hold Component registries.
    std::map<ComponentUUID, ECS::AbstractRegistry*> m_records;
};
} // ECS
} // Recluse