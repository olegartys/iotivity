#ifndef SHOUSE_RES_HAL_H
#define SHOUSE_RES_HAL_H

#include <memory>
#include <vector>

#include <OCPlatform.h>
#include <OCApi.h>

#include <dynamic_resource/resource_property.h>

class ShouseResourceServer;

enum class ShouseHALResult: int {
	OK = 0, ERR = 1
};


class ShouseServerHAL {
public:
	virtual ShouseHALResult onGet(const std::string& propName, std::string& resultValue, const OC::QueryParamsMap& params) = 0;

	virtual ShouseHALResult onPut(const std::string& propName, const std::string& newValue, const OC::QueryParamsMap& params) = 0;

	virtual std::vector<ResourceProperty> getProperties() const = 0;

};

class ShouseClientHAL {
public:
	virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) = 0;

    virtual void onPut(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) = 0;

	virtual std::vector<ResourceProperty> getProperties() const = 0;

};

#endif // SHOUSE_RES_HAL_H
