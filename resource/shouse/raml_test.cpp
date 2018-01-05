#include <Raml.h>
#include <RamlParser.h>
#include <memory>
#include <iostream>

int main() {
    RAML::RamlParser rp("/home/olegartys/src/iotivity/switch.raml");
    std::shared_ptr<RAML::Raml> raml = rp.getRamlPtr();

    std::cout << raml->getBaseUri() << "\n";
    std::cout << raml->getMediaType() << "\n";
    std::cout << raml->getTitle() << "\n";

    auto resources = raml->getResources();

    for (auto& res: resources) {
        std::cout << "=== ";
        std::cout << res.first << " " << res.second->getDisplayName() << "\n";
    }

    std::cout << "\n";

    auto schema = raml->getResource("/BinarySwitchResURI")->getAction(RAML::ActionType::GET)->getResponse("200")->getResponseBody("application/json")->getSchema();

    std::cout << schema->getSchema();

    // OC::OCRepresentation repr;
    // repr.set

//    RAML::Raml r;
//    r.get


    return 0;
}
