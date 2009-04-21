#ifndef FIND_MODULE_H
#define FIND_MODULE_H

#include <omnetpp.h>

/**
 * @brief Provides method templates to find omnet modules.
 *
 * @ingroup baseUtils
 * @ingrouo utils
 */
template<typename T>
class FindModule
{
	public:
		static T findSubModule(cModule *top)
		{
			T ret;
			for (cModule::SubmoduleIterator i(top); !i.end(); i++)
			{
				cModule *sub = i();
				ret = dynamic_cast<T>(sub);
				if (ret!=NULL)
					return ret;
				ret = findSubModule(sub);
				if (ret!=NULL)
					return ret;
			}
			return NULL;
		}

		static T findGlobalModule() {return findSubModule(simulation.getSystemModule());}
};

/**
 * @brief Finds and returns the pointer to a module of type T.
 *
 * Uses findModuleWherever(). See usage e.g. at RoutingTableAccess.
 *
 * @ingroup baseUtils
 * @ingroup utils
 */
template<typename T>
class ModuleAccess
{
     // Note: MSVC 6.0 doesn't like const char *N as template parameter,
     // so we have to pass it via the ctor...
  private:
    T *p;
  public:
    ModuleAccess():
		p(0)
	{}

    T *get()
    {
        if (!p)
        {
            p = FindModule<T*>::findGlobalModule();
            if (!p) opp_error("Module %s not found",opp_typename(typeid(T)));
        }
        return p;
    }
};

#endif
