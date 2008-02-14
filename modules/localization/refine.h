#ifndef REFINE_H
#define REFINE_H

#include "omnetpp.h"
#include <string.h>
#include <assert.h>

#define add_struct(m,s,v)						\
	do { (m)->addPar(s) = (void *) memmove( new char[sizeof(v)], (v), \
						sizeof(v));		\
		(m)->par(s).configPointer(NULL,NULL,sizeof(v));		\
	} while (0)
// This one actually works. I can't be bothered to replace the one above.
// The previous one fucks up because when v is a pointer to the struct, sizeof returns the size of a pointer, not the struct.
// When v is a struct, the 2nd parameter of memmove is obviously of the wrong type.
// I choose to pass pointers instead of structs, mainly because thats what we'll do in get_struct2 anyway.
// (20020826, N.Reijers)
#define add_struct2(m,s,v)						\
	do { (m)->addPar(s) = (void *) memmove( new char[sizeof(*(v))], (v), \
						sizeof(*(v)));		\
		(m)->par(s).configPointer(NULL,NULL,sizeof(*(v)));	\
	} while (0)
#define add_array(m,s,p,n)						\
	do { (m)->addPar(s) = (void *) memmove( new char[(n)*sizeof(*(p))], (p), \
						(n)*sizeof(*(p)));	\
		(m)->par(s).configPointer(NULL,NULL,(n)*sizeof(*(p)));	\
	} while (0)

#define get_struct(m,s,p)	memmove( (p), (m)->par(s), sizeof(p))
#define get_struct2(m,s,p)	memmove( (p), (m)->par(s), sizeof(*(p)))
#define get_array(m,s,p,n)	memmove( (p), (m)->par(s), (n)*sizeof(*(p)))
#define get_table(m,s1,p,n,size)	memmove( (p), (m)->par(s1), (n)*size )

// message kind values (packet types):
enum {
	MSG_START = 0,
	MSG_STOP,
	MSG_RETRY,
	MSG_TIME_OUT,
	MSG_DONE,
	MSG_NEIGHBOR,
	MSG_TYPE_BASE,
};

// MAX_MSG_TYPES is the maximum number of message
// types a subclass of Node can use. So the integer
// indicating the message type (msg->kind) must be
// between 0 and MAX_MSG_TYPES-1.
// The first MSG_TYPE_BASE message types are in use
// Bit of an ugly solution, but it will do.
#define MAX_MSG_TYPES (MSG_TYPE_BASE+32)

// Possible status of nodes:
typedef enum {
	STATUS_ANCHOR = 0 /* This node has a known position */ ,
	STATUS_UNKNOWN = 1 /* This node has not yet been able to estimate a position */ ,
	STATUS_POSITIONED = 2 /* This node has estimated its position */ ,
	STATUS_BAD = 3		/* This is a bad node (meaning will depend on the algorithm) */
} NodeState;

#endif
