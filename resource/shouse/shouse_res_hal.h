#ifndef SHOUSE_RES_HAL_H
#define SHOUSE_RES_HAL_H

#include "OCPlatform.h"
#include "OCApi.h"

class ShouseResHAL {
public:
	virtual int onGet(OC::OCRepresentation& curRepr, const OC::QueryParamsMap& params) noexcept = 0;

	virtual int onPut(OC::OCRepresentation& curRepr, const OC::QueryParamsMap& params) noexcept = 0;

};

class ShouseClientHAL {
public:
	virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) = 0;

    virtual void onPut(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) = 0;
};

#endif // SHOUSE_RES_HAL_H