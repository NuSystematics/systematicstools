#include "systematicstools/utility/ParameterAndProviderConfigurationUtility.hh"

#include "systematicstools/interface/YAMLSystParamHeaderConverters.hh"

#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

namespace systtools {

param_header_map_t BuildParameterHeaders(YAML::Node const &yamlnd,
                                         std::string const &key) {

  param_header_map_t headers;

  // Foreach provider block
  YAML::Node provider_keys = yamlnd[key];
  for (auto const &provkeyNode : provider_keys) {
    std::string provkey = provkeyNode.as<std::string>();
    YAML::Node provider_cfg = yamlnd[provkey];

    std::string provname = provider_cfg["tool_type"].as<std::string>();
    if (provider_cfg["instance_name"]) {
      provname += "_" + provider_cfg["instance_name"].as<std::string>();
    }

    YAML::Node ParameterHeaderKeyNames = provider_cfg["parameter_headers"];

    // Foreach handled parameter block
    for (auto const &ParamHeaderKeyNode : ParameterHeaderKeyNames) {
      std::string ParamHeaderKey = ParamHeaderKeyNode.as<std::string>();
      SystParamHeader hdr = YAMLToSystParamHeader(provider_cfg[ParamHeaderKey]);

      // Check that this unique Id hasn't been used before.
      if (headers.find(hdr.systParamId) != headers.end()) {
        throw systParamId_collision()
            << "[ERROR]:\t Header describing parameter " << hdr.systParamId
            << " already exists (provider: "
            << std::quoted(headers[hdr.systParamId].ProviderFQName)
            << ", prettyName: "
            << std::quoted(headers[hdr.systParamId].Header.prettyName)
            << ")"
            << ", this parameter: { provider: " << std::quoted(provname)
            << ", prettyName: " << std::quoted(hdr.prettyName)
            << "} cannot be added. ";
      }

      headers.emplace(param_header_map_t::key_type{hdr.systParamId},
                      param_header_map_t::mapped_type{provname, hdr});
    }
  }

  return headers;
}
} // namespace systtools
