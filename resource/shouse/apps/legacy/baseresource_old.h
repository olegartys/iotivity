#ifndef BASERESOURCE_H
#define BASERESOURCE_H

#include <memory>

#include "OCPlatform.h"
#include "OCApi.h"

class BaseResource {
public:
    virtual ~BaseResource() = default;

    virtual const std::string& uri() const { return mUri; }
    virtual const OC::OCRepresentation& repr() const { return mResourceRepr; }

protected:
    BaseResource() = default;

    BaseResource(const std::string& uri) :
        mUri(uri) {}

protected:
    std::string mUri;
    OC::OCRepresentation mResourceRepr;

};

class BaseResourceTypeInterface {
public:
    virtual ~BaseResourceTypeInterface() = default;

    virtual const std::string& type() const { return mType; }
    virtual const std::string& iface() const { return mIface; }

protected:
    BaseResourceTypeInterface(const std::string& type, const std::string& iface) :
        mType(type), mIface(iface) {}

    std::string mType;
    std::string mIface;

};

class BaseResourceServer : public BaseResource {
public:
    virtual const OCResourceHandle& hndl() const { return mResourceHandle; }

protected:
    BaseResourceServer(const std::string& uri) :
        BaseResource(uri) {}

    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) = 0;

    OCResourceHandle mResourceHandle;

};

class BaseResourceClient : public BaseResource {
public:
    virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) = 0;

};

#endif // BASERESOURCE_H
