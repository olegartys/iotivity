#ifndef BASE_RESOURCE_SERVER_H
#define BASE_RESOURCE_SERVER_H

#include <shouse/base_resource.h>

template <typename ResourceType>
class BaseResourceServer {
public:
    virtual ~BaseResourceServer() = default;

    inline std::string uri() const { return mResource->uri(); }
	inline std::string type() const { return mResource->type(); }
	inline std::string iface() const { return mResource->iface(); }

protected:
    template <typename... Args>
    BaseResourceServer(Args&&... args) {
        static_assert(std::is_base_of<BaseResource, ResourceType>::value,
            "ResourceType template argument should be child of BaseResource");

        mResource.reset(new ResourceType(std::forward<Args>(args)...));
    }

    virtual OCEntityHandlerResult entityHandler(
        std::shared_ptr<OC::OCResourceRequest> request) = 0;

    inline const OCResourceHandle& hndl() const { return mResourceHandle; }

protected:
    std::unique_ptr<ResourceType> mResource;
    OCResourceHandle mResourceHandle;

};

#endif // BASE_RESOURCE_SERVER_H
