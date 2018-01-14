#include <cstring>

#include <cJSON.h>

#include <dynamic_resource/resource_property.h>

static ResourceProperty::Type strToPropertyType(const char *str) {
	if (!strcmp(str, "string")) {
		return ResourceProperty::Type::T_STRING;
	} else if (!strcmp(str, "int")) {
		return ResourceProperty::Type::T_INT;		
	} else {
		return ResourceProperty::Type::T_UNKNOWN;
	}
}

bool ResourceProperty::fromJson(const std::string& jsonString) {
	const char *c_jsonString;
	cJSON *jsonObj;

	c_jsonString = jsonString.c_str();

	jsonObj = cJSON_Parse(c_jsonString);
	if (!jsonObj) {
		return false;
	}

	return fromJson(jsonObj);
}

bool ResourceProperty::fromJson(const cJSON* json) {
	const cJSON *name;
	const cJSON *type;
	const cJSON *defaultValue;

	name = cJSON_GetObjectItemCaseSensitive(json, "name");
	if (!name) {
		return false;
	}

	type = cJSON_GetObjectItemCaseSensitive(json, "type");
	if (!type) {
		return false;
	}

	defaultValue = cJSON_GetObjectItemCaseSensitive(json, "default_value");
	if (!defaultValue) {
		return false;
	}

	mName = name->valuestring;
	mType = strToPropertyType(type->valuestring);
	mDefaultValue = defaultValue->valuestring;

	return true;
}
