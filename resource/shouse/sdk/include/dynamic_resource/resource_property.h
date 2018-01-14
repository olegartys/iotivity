#ifndef RESOURCE_PROPERTY_H
#define RESOURCE_PROPERTY_H

#include <string>

#include <cJSON.h>

struct ResourceProperty final {
	enum class Type {
    	T_STRING, T_INT, T_BOOL, T_DOUBLE, T_UNKNOWN
	};

	ResourceProperty() = default;

    ResourceProperty(const std::string& name, Type type, const std::string& defaultValue) noexcept :
    	mName(name), mType(type), mDefaultValue(defaultValue) { }

    bool fromJson(const std::string& jsonString);
	bool fromJson(const cJSON* json);

    std::string mName;
    Type 		mType;
    std::string mDefaultValue;

};

#endif // RESOURCE_PROPERTY_H