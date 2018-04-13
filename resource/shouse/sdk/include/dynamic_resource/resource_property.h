#ifndef RESOURCE_PROPERTY_H
#define RESOURCE_PROPERTY_H

#include <string>

#include <cJSON.h>

struct ResourceProperty final {
	enum class Type {
    	T_STRING, T_INT, T_BOOL, T_DOUBLE, T_UNKNOWN
	};

	ResourceProperty() = default;

    ResourceProperty(const std::string& name, Type type, const std::string& value) noexcept :
    	mName(name), mType(type), mValue(value) { }

    bool fromJson(const std::string& jsonString);
	bool fromJson(const cJSON* json);

    bool toJson(std::string& out) const;

    friend std::string to_string(const ResourceProperty& resourceProperty);

    std::string mName;
    Type 		mType;
    std::string mValue;

};

const char* propertyTypeToStr(const ResourceProperty::Type type);

#endif // RESOURCE_PROPERTY_H