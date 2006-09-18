#ifndef __TNODE_H__
#define __TNODE_H__

#include "node.h"

/** TNOde type node implementation. */
class TNOde: public Node{
	Module_Class_Members(TNOde, Node, 0);

	public:
		virtual void initialize();
		virtual void finish();
};

#endif

