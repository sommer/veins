#pragma once

#include "veins/veins.h"

namespace Veins {

/**
 * @brief Provides method templates to find omnet modules.
 *
 * @ingroup baseUtils
 * @ingroup utils
 */
template <typename T = cModule* const>
class FindModule {
public:
    /**
     * @brief Returns a pointer to a sub module of the passed module with
     * the type of this template.
     *
     * Returns NULL if no matching submodule could be found.
     */
    static T findSubModule(const cModule* const top)
    {
        for (cModule::SubmoduleIterator i(top); !i.end(); i++) {
            cModule* const sub = *i;
            // this allows also a return type of read only pointer: const cModule *const
            T dCastRet = dynamic_cast<T>(sub);
            if (dCastRet != NULL) return dCastRet;
            // this allows also a return type of read only pointer: const cModule *const
            T recFnd = findSubModule(sub);
            if (recFnd != NULL) return recFnd;
        }
        return NULL;
    }

    /**
     * @brief Returns a pointer to the module with the type of this
     * template.
     *
     * Returns NULL if no module of this type could be found.
     */
    static T findGlobalModule()
    {
        return findSubModule(getSimulation()->getSystemModule());
    }

    /**
     * @brief Returns a pointer to the host module of the passed module.
     *
     * Assumes that every host module is a direct sub module of the
     * simulation.
     */
    static cModule* const findHost(cModule* const m)
    {
        cModule* parent = m != nullptr ? m->getParentModule() : nullptr;
        cModule* node = m;

        // all nodes should be a sub module of the simulation which has no parent module!!!
        while (parent != nullptr && parent->getParentModule() != nullptr) {
            node = parent;
            parent = node->getParentModule();
        }
        return node;
    }
    // the constness version
    static const cModule* const findHost(const cModule* const m)
    {
        const cModule* parent = m != nullptr ? m->getParentModule() : nullptr;
        const cModule* node = m;

        // all nodes should be a sub module of the simulation which has no parent module!!!
        while (parent != nullptr && parent->getParentModule() != nullptr) {
            node = parent;
            parent = node->getParentModule();
        }
        return node;
    }
};

/**
 * @brief Finds and returns the pointer to a module of type T.
 * Uses FindModule<>::findSubModule(), FindModule<>::findHost(). See usage e.g. at ChannelAccess.
 */
template <typename T = cModule>
class AccessModuleWrap {
public:
    using wrapType = T;

private:
    T* pModule;

public:
    AccessModuleWrap()
        : pModule(nullptr)
    {
    }

    T* const get(cModule* const from = nullptr)
    {
        if (!pModule) {
            pModule = FindModule<T*>::findSubModule(FindModule<>::findHost(from != nullptr ? from : getSimulation()->getContextModule()));
        }
        return pModule;
    }
};

/**
 * @brief Return a vector containing pointers to all submodules of parentModule of type T
 */
template <class T>
std::vector<T*> getSubmodulesOfType(cModule* parentModule, bool recurse = false)
{
    std::vector<T*> result;
    for (cModule::SubmoduleIterator iter(parentModule); !iter.end(); iter++) {
        auto mm = dynamic_cast<T*>(*iter);
        if (mm != nullptr) result.push_back(mm);
        if (recurse) {
            for (auto m : getSubmodulesOfType<T>(*iter, recurse)) {
                result.emplace_back(m);
            }
        }
    }
    return result;
}

} // namespace Veins
