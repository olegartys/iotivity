#ifndef BASE_RESOURCE_SERVER_H
#define BASE_RESOURCE_SERVER_H

#include <shouse/base_resource.h>

template <typename ResourceType>
class BaseResourceServer {
public:
    virtual ~BaseResourceServer() = default;

    inline const OCResourceHandle& hndl() const { return mResourceHandle; }

    inline std::string uri() const { return mResource->uri(); }
	inline std::string type() const { return mResource->type(); }
	inline std::string iface() const { return mResource->iface(); }

	inline OC::OCRepresentation& repr() { return mResource->repr(); }
	inline const OC::OCRepresentation& repr() const { return mResource->repr(); }

protected:
    template <typename... Args>
    BaseResourceServer(Args&&... args) {
        mResource.reset(new ResourceType(std::forward<Args>(args)...));
    }

    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) = 0;

protected:
    std::unique_ptr<ResourceType> mResource;
    OCResourceHandle mResourceHandle;

};

#endif // BASE_RESOURCE_SERVER_H
