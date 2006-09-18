#ifndef __MICA2_H__
#define __MICA2_H__

#include "node.h"

/** Mica2 node implementation. */
class Mica2 : public Node {
	Module_Class_Members(Mica2, Node, 0);

	public:
		virtual void initialize();
		virtual void finish();
};

#endif

