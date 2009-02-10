//
// Generated file, do not edit! Created by opp_msgc 4.0 from messages/Mac80211Pkt.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "Mac80211Pkt_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




Register_Class(Mac80211Pkt);

Mac80211Pkt::Mac80211Pkt(const char *name, int kind) : MacPkt(name,kind)
{
    this->address3_var = 0;
    this->address4_var = 0;
    this->fragmentation_var = 0;
    this->informationDS_var = 0;
    this->sequenceControl_var = 0;
    this->retry_var = 0;
    this->duration_var = 0;
}

Mac80211Pkt::Mac80211Pkt(const Mac80211Pkt& other) : MacPkt()
{
    setName(other.getName());
    operator=(other);
}

Mac80211Pkt::~Mac80211Pkt()
{
}

Mac80211Pkt& Mac80211Pkt::operator=(const Mac80211Pkt& other)
{
    if (this==&other) return *this;
    MacPkt::operator=(other);
    this->address3_var = other.address3_var;
    this->address4_var = other.address4_var;
    this->fragmentation_var = other.fragmentation_var;
    this->informationDS_var = other.informationDS_var;
    this->sequenceControl_var = other.sequenceControl_var;
    this->retry_var = other.retry_var;
    this->duration_var = other.duration_var;
    return *this;
}

void Mac80211Pkt::parsimPack(cCommBuffer *b)
{
    MacPkt::parsimPack(b);
    doPacking(b,this->address3_var);
    doPacking(b,this->address4_var);
    doPacking(b,this->fragmentation_var);
    doPacking(b,this->informationDS_var);
    doPacking(b,this->sequenceControl_var);
    doPacking(b,this->retry_var);
    doPacking(b,this->duration_var);
}

void Mac80211Pkt::parsimUnpack(cCommBuffer *b)
{
    MacPkt::parsimUnpack(b);
    doUnpacking(b,this->address3_var);
    doUnpacking(b,this->address4_var);
    doUnpacking(b,this->fragmentation_var);
    doUnpacking(b,this->informationDS_var);
    doUnpacking(b,this->sequenceControl_var);
    doUnpacking(b,this->retry_var);
    doUnpacking(b,this->duration_var);
}

int Mac80211Pkt::getAddress3() const
{
    return address3_var;
}

void Mac80211Pkt::setAddress3(int address3_var)
{
    this->address3_var = address3_var;
}

int Mac80211Pkt::getAddress4() const
{
    return address4_var;
}

void Mac80211Pkt::setAddress4(int address4_var)
{
    this->address4_var = address4_var;
}

int Mac80211Pkt::getFragmentation() const
{
    return fragmentation_var;
}

void Mac80211Pkt::setFragmentation(int fragmentation_var)
{
    this->fragmentation_var = fragmentation_var;
}

int Mac80211Pkt::getInformationDS() const
{
    return informationDS_var;
}

void Mac80211Pkt::setInformationDS(int informationDS_var)
{
    this->informationDS_var = informationDS_var;
}

int Mac80211Pkt::getSequenceControl() const
{
    return sequenceControl_var;
}

void Mac80211Pkt::setSequenceControl(int sequenceControl_var)
{
    this->sequenceControl_var = sequenceControl_var;
}

bool Mac80211Pkt::getRetry() const
{
    return retry_var;
}

void Mac80211Pkt::setRetry(bool retry_var)
{
    this->retry_var = retry_var;
}

simtime_t Mac80211Pkt::getDuration() const
{
    return duration_var;
}

void Mac80211Pkt::setDuration(simtime_t duration_var)
{
    this->duration_var = duration_var;
}

class Mac80211PktDescriptor : public cClassDescriptor
{
  public:
    Mac80211PktDescriptor();
    virtual ~Mac80211PktDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual bool getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(Mac80211PktDescriptor);

Mac80211PktDescriptor::Mac80211PktDescriptor() : cClassDescriptor("Mac80211Pkt", "MacPkt")
{
}

Mac80211PktDescriptor::~Mac80211PktDescriptor()
{
}

bool Mac80211PktDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<Mac80211Pkt *>(obj)!=NULL;
}

const char *Mac80211PktDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int Mac80211PktDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount(object) : 7;
}

unsigned int Mac80211PktDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return FD_ISEDITABLE;
        case 1: return FD_ISEDITABLE;
        case 2: return FD_ISEDITABLE;
        case 3: return FD_ISEDITABLE;
        case 4: return FD_ISEDITABLE;
        case 5: return FD_ISEDITABLE;
        case 6: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *Mac80211PktDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "address3";
        case 1: return "address4";
        case 2: return "fragmentation";
        case 3: return "informationDS";
        case 4: return "sequenceControl";
        case 5: return "retry";
        case 6: return "duration";
        default: return NULL;
    }
}

const char *Mac80211PktDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "int";
        case 1: return "int";
        case 2: return "int";
        case 3: return "int";
        case 4: return "int";
        case 5: return "bool";
        case 6: return "simtime_t";
        default: return NULL;
    }
}

const char *Mac80211PktDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int Mac80211PktDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    Mac80211Pkt *pp = (Mac80211Pkt *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool Mac80211PktDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    Mac80211Pkt *pp = (Mac80211Pkt *)object; (void)pp;
    switch (field) {
        case 0: long2string(pp->getAddress3(),resultbuf,bufsize); return true;
        case 1: long2string(pp->getAddress4(),resultbuf,bufsize); return true;
        case 2: long2string(pp->getFragmentation(),resultbuf,bufsize); return true;
        case 3: long2string(pp->getInformationDS(),resultbuf,bufsize); return true;
        case 4: long2string(pp->getSequenceControl(),resultbuf,bufsize); return true;
        case 5: bool2string(pp->getRetry(),resultbuf,bufsize); return true;
        case 6: double2string(pp->getDuration(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool Mac80211PktDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    Mac80211Pkt *pp = (Mac80211Pkt *)object; (void)pp;
    switch (field) {
        case 0: pp->setAddress3(string2long(value)); return true;
        case 1: pp->setAddress4(string2long(value)); return true;
        case 2: pp->setFragmentation(string2long(value)); return true;
        case 3: pp->setInformationDS(string2long(value)); return true;
        case 4: pp->setSequenceControl(string2long(value)); return true;
        case 5: pp->setRetry(string2bool(value)); return true;
        case 6: pp->setDuration(string2double(value)); return true;
        default: return false;
    }
}

const char *Mac80211PktDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

void *Mac80211PktDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    Mac80211Pkt *pp = (Mac80211Pkt *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


