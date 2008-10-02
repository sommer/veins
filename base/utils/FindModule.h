#ifndef FIND_MODULE_H
#define FIND_MODULE_H

#include <omnetpp.h>

template<typename T>
class FindModule
{
	public:
		static T findSubModule(cModule *top) 
		{
			T ret;
			for (cSubModIterator i(*top); !i.end(); i++)
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

#endif
