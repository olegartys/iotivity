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
    virtual OC::OCRepresentation& repr() { return mResourceRepr; }

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

class BaseResourceServer : public BaseResource, public BaseResourceTypeInterface {
public:
    virtual const OCResourceHandle& hndl() const { return mResourceHandle; }

protected:
    BaseResourceServer(const std::string& uri, const std::string& type, const std::string& iface) :
        BaseResource(uri), BaseResourceTypeInterface(type, iface) {}

    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) = 0;

    OCResourceHandle mResourceHandle;

};

class BaseResourceClient : public BaseResource, public BaseResourceTypeInterface {
public:
    virtual void setResource(const std::shared_ptr<OC::OCResource>& resource) = 0;

    virtual OCStackResult get(const OC::QueryParamsMap& queryParametersMap) = 0;

    virtual OCStackResult put(const OC::QueryParamsMap& queryParametersMap) = 0;

protected:
    BaseResourceClient(const std::string& uri, const std::string& type, const std::string& iface) :
        BaseResource(uri), BaseResourceTypeInterface(type, iface) {}

};

#endif // BASERESOURCE_H
