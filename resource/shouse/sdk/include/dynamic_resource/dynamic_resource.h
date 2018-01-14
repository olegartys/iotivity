#ifndef DYNAMIC_RESOURCE_H
#define DYNAMIC_RESOURCE_H

#include <string>
#include <vector>

#include <shouse/base_resource.h>

#include <dynamic_resource/resource_property.h>

class DynamicDataResource : public BaseResource {
public:
    DynamicDataResource(const std::string& uri, const std::string& iface, const std::string& type) : 
        BaseResource(uri, iface, type) {}

    bool init(const std::string& resourcePropertiesJson);

    // template <typename IterableContainer>
    bool init(const std::vector<ResourceProperty>& resourceProperties) {
        for (const auto& resourceProperty: resourceProperties) {
            if (!fillRepr(resourceProperty)) {
                return false;
            }
        }

        return true;
    }

protected:
    bool fillRepr(const ResourceProperty& rp);

private:
    static constexpr const char* LOG_TAG = "DynamicDataResource";

};

// template <typename IterableContainer>
// bool DynamicDataResource::init(const IterableContainer& resourceProperties) {
    
// }

#endif // DYNAMIC_RESOURCE_H
