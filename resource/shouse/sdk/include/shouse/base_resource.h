#ifndef BASE_RESOURCE_H
#define BASE_RESOURCE_H

#include <memory>

#include <OCPlatform.h>
#include <OCApi.h>

class BaseResource {
public:
    virtual ~BaseResource() = default;

    BaseResource(const std::string& uri, const std::string& iface, const std::string& type) :
        mUri(uri), mIface(iface), mType(type) {}

    inline const std::string& uri() const { return mUri; }
    inline const std::string& iface() const { return mIface; }
    inline const std::string& type() const { return mType; } 

    inline const OC::OCRepresentation& repr() const { return mResourceRepr; }
    inline OC::OCRepresentation& repr() { return mResourceRepr; }

    void setRepr(const OC::OCRepresentation& repr) { mResourceRepr = repr; }

protected:
    std::string mUri;
    std::string mIface;
    std::string mType;

    OC::OCRepresentation mResourceRepr;

};


#endif // BASE_RESOURCE_H
