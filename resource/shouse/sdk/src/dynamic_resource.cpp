#include <cJSON.h>

#include <shouse/dynamic_resource/dynamic_resource.h>

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

bool DynamicDataResource::fillRepr(const ResourceProperty& resourceProperty) {
    switch (resourceProperty.mType) {
        case ResourceProperty::Type::T_STRING:
            repr().setValue(resourceProperty.mName, resourceProperty.mDefaultValue);
            break;

        case ResourceProperty::Type::T_INT: {
            int val;

            try {
                val = stoi(resourceProperty.mDefaultValue);
            } catch (std::exception&) {
                return false;
            }

            repr().setValue(resourceProperty.mName, val);
            break;
        }

        default:
            break;
    }

    return true;
}
