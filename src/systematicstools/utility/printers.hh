#pragma once

#include "systematicstools/interface/EventResponse_product.hh"
#include "systematicstools/interface/YAMLSystParamHeaderConverters.hh"
#include "systematicstools/interface/SystParamHeader.hh"
#include "yaml-cpp/yaml.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace systtools {
inline std::string to_str(SystParamHeader const &sph, bool /*indent*/ = true) {
  YAML::Node yaml = SystParamHeaderToYAML(sph);
  return YAML::Dump(yaml);
}

inline std::string to_str(EventResponse const &er) {
  std::stringstream ss("");
  ss << "Event response contains " << er.size() << std::endl;
  for (size_t eur_it = 0; eur_it < er.size(); ++eur_it) {
    auto &eur = er[eur_it];
    ss << "\t\tFound " << eur.size() << " responses to event unit " << eur_it
       << ":" << std::endl;
    for (auto &pr : eur) {
      ss << "\t\t\tParam " << pr.pid << ": {" << std::flush;
      for (auto &r : pr.responses) {
        ss << r << ", " << std::flush;
      }
      ss << "}" << std::endl;
    }
  }
  return ss.str();
}

} // namespace systtools
