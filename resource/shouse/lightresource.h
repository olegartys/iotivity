#ifndef LIGHTRESOURCE_H
#define LIGHTRESOURCE_H

#include "Log.h"
#include "baseresource.h"

namespace PH = std::placeholders;

class BinaryLightTypeInterface : public BaseResourceTypeInterface {
protected:
    BinaryLightTypeInterface() :
        BaseResourceTypeInterface("type.binary_light", "iface.binary_light") {}

};

class BinaryLightModel {
public:
    BinaryLightModel(bool state) :
        mState(state) {}

    virtual ~BinaryLightModel() = default;

protected:
    bool mState;

};

#endif // LIGHTRESOURCE_H
