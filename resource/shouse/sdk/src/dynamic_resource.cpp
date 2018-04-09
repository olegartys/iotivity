#include <cJSON.h>

#include <dynamic_resource/dynamic_resource.h>

#include <shouse/Log.h>

bool DynamicDataResource::init(const std::string& resourcePropertiesJsonStr) {
    ResourceProperty resourceProperty;
    const char *c_jsonString;
    cJSON *jsonObj;

    cJSON *resourcePropertiesJson;
    cJSON *resourcePropertyJson;

    c_jsonString = resourcePropertiesJsonStr.c_str();

    jsonObj = cJSON_Parse(c_jsonString);
    if (!jsonObj) {
        Log::error(LOG_TAG, "Error parsing json");
        return false;
    }

    resourcePropertiesJson = cJSON_GetObjectItemCaseSensitive(jsonObj, "properties");
    if (!resourcePropertiesJson) {
        Log::error(LOG_TAG, "Error getting 'properties' array");
        return false;
    }

    cJSON_ArrayForEach(resourcePropertyJson, resourcePropertiesJson) {
        bool res = resourceProperty.fromJson(resourcePropertyJson);
        if (!res) {
            Log::error(LOG_TAG, "Error building 'ResourceProperty'");
            return false;
        }

        res = fillRepr(resourceProperty);
        if (!res) {
            Log::error(LOG_TAG, "Error filling resource representation");
            return false;
        }
    }

    return true;
}

bool DynamicDataResource::setProp(const ResourceProperty& prop, const std::string& newValue) {
    ResourceProperty newProp;

    // Create local copy of the given property and change its value to new

    newProp = prop;
    newProp.mValue = newValue;

    // Insert property with new value into the OCRepresentation

    return fillRepr(newProp);
}

bool DynamicDataResource::getProp(ResourceProperty& outProp, const std::string& propName) const {
    std::string val;
    ResourceProperty prop;

    if (!repr().getValue(propName, val)) {
        Log::error(LOG_TAG, "Error getting property {}", propName);
        return false;
    }

    if (!prop.fromJson(val)) {
        Log::error(LOG_TAG, "Error construction json property string into"
            "ResourceProperty");
        return false;
    }

    outProp = std::move(prop);

    return true;
}

bool DynamicDataResource::fillRepr(const ResourceProperty& resourceProperty) {
    std::string jsonProp;

    // Convert property to JSON string

    if (!resourceProperty.toJson(jsonProp)) {
        return false;
    }

    // Store it into the OCRepresentation object as [name]->jsonPropertyRepr
    // pair

    repr().setValue(resourceProperty.mName, jsonProp);

    return true;
}
