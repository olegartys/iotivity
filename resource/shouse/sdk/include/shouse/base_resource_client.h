#ifndef BASE_RESOURCE_CLIENT_H
#define BASE_RESOURCE_CLIENT_H

#include <functional>

#include <shouse/base_resource.h>

template <typename ResourceType>
class BaseResourceClient {
public:
    using onGetCb = std::function<void(const OC::HeaderOptions&,
        const std::map<std::string, ResourceProperty>& props, const int eCode)>;

    using onPutCb = std::function<void(const OC::HeaderOptions&,
        const std::map<std::string, ResourceProperty>& props, const int eCode)>;

public:
    virtual ~BaseResourceClient() = default;

    void setOCResource(const std::shared_ptr<OC::OCResource>& resource) {
        mOCResource = resource;
    }

    inline OC::OCRepresentation& repr() { return mResource->repr(); }

    virtual OCStackResult get(const OC::QueryParamsMap& queryParametersMap,
        onGetCb onGetHandler, bool async) = 0;

    virtual OCStackResult put(const OC::QueryParamsMap& queryParametersMap,
        onPutCb onPutHandler, bool async) = 0;

protected:
    template <typename... Args>
    BaseResourceClient(Args&&... args) {
        mResource.reset(new ResourceType(std::forward<Args>(args)...));
    }

protected:
    std::unique_ptr<ResourceType> mResource;
    std::shared_ptr<OC::OCResource> mOCResource;

};

#endif // BASE_RESOURCE_CLIENT_H