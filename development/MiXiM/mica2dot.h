#ifndef __MICA2DOT_H__
#define __MICA2DOT_H__

#include "node.h"

/** Mica2dot node implementation. */
class Mica2dot : public Node {
	Module_Class_Members(Mica2dot, Node, 0);

	public:
		virtual void initialize();
		virtual void finish();
};

#endif

