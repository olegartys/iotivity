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

const char* propertyTypeToStr(const ResourceProperty::Type type) {
	switch(type) {
	case ResourceProperty::Type::T_STRING:
		return "string";
	case ResourceProperty::Type::T_INT:
		return "int";
	case ResourceProperty::Type::T_UNKNOWN:
	default:
		return nullptr;
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
	const cJSON *value;

	name = cJSON_GetObjectItemCaseSensitive(json, "name");
	if (!name) {
		return false;
	}

	type = cJSON_GetObjectItemCaseSensitive(json, "type");
	if (!type) {
		return false;
	}

	value = cJSON_GetObjectItemCaseSensitive(json, "value");
	if (!value) {
		return false;
	}

	mName = name->valuestring;
	mType = strToPropertyType(type->valuestring);
	mValue = value->valuestring;

	return true;
}

bool ResourceProperty::toJson(std::string& out) const {
	cJSON *res_prop;
	cJSON *name;
	cJSON *type;
	cJSON *value;

	char* str = nullptr;

	res_prop = cJSON_CreateObject();
	if (!res_prop) {
		return false;
	}

	name = cJSON_CreateString(mName.c_str());
	if (!name) {
		cJSON_Delete(res_prop);
		return false;
	}

	type = cJSON_CreateString(propertyTypeToStr(mType));
	if (!type) {
		cJSON_Delete(res_prop);
		return false;
	}

	value = cJSON_CreateString(mValue.c_str());
	if (!value) {
		cJSON_Delete(res_prop);
		return false;
	}

	cJSON_AddItemToObject(res_prop, "name", name);	
	cJSON_AddItemToObject(res_prop, "type", type);
	cJSON_AddItemToObject(res_prop, "value", value);

	str = cJSON_Print(res_prop);
	if (!str) {
		cJSON_Delete(res_prop);
		return false;
	}

	out = str;

	return true;
}

std::string to_string(const ResourceProperty& resourceProperty) {
	std::string jsonStr;

	if (!resourceProperty.toJson(jsonStr)) {
		return "";
	}

	return jsonStr;
}
