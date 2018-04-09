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
	virtual ~ShouseServerHAL() = default;

	virtual int open() = 0;

	virtual ShouseHALResult close(int id) = 0;

	virtual ShouseHALResult get(int id, const std::string& propName,
		std::string& resultValue, const OC::QueryParamsMap& params) = 0;

	virtual ShouseHALResult put(int id, const std::string& propName,
		const std::string& newValue, const OC::QueryParamsMap& params) = 0;

	virtual std::vector<ResourceProperty> properties() const = 0;

public:
	using open_t = int(*)();
	using close_t = ShouseHALResult(*)(int);
	using get_t = ShouseHALResult(*)(int, const std::string&, std::string&,
		const OC::QueryParamsMap&);
	using put_t = ShouseHALResult(*)(int, const std::string&,
		const std::string&, const OC::QueryParamsMap&);
	using properties_t = std::vector<ResourceProperty>(*)();

};

#endif // SHOUSE_RES_HAL_H
