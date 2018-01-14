#ifndef DYNAMIC_RESOURCE_H
#define DYNAMIC_RESOURCE_H

#include <string>

#include <shouse/baseresource.h>
#include <shouse/dynamic_resource/resource_property.h>

class DynamicDataResource : public BaseResource {
public:
    DynamicDataResource(const std::string& uri) : 
        BaseResource(uri) {}

    bool init(const std::string& resourcePropertiesJson);

    template <typename IterableContainer>
    bool init(const IterableContainer& resourceProperties);

protected:
    bool fillRepr(const ResourceProperty& rp);

private:
    static constexpr const char* LOG_TAG = "DynamicDataResource";

};

template <typename IterableContainer>
bool DynamicDataResource::init(const IterableContainer& resourceProperties) {
    for (const auto& resourceProperty: resourceProperties) {
        if (!fillRepr(resourceProperty)) {
            return false;
        }
    }

    return true;
}

#endif // DYNAMIC_RESOURCE_H
