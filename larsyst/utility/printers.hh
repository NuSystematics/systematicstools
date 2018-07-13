#ifndef LARSYST_UTILITY_PRINTERS_SEEN
#define LARSYST_UTILITY_PRINTERS_SEEN

#include "larsyst/interface/EventResponse_product.hh"
#include "larsyst/interface/SystMetaData.hh"

#include "build_parameter_set_from_header.hh"

#include "fhiclcpp/ParameterSet.h"

#include <iomanip>
#include <string>

namespace larsyst {
inline std::string to_str(SystParamHeader const &sph, bool indent = true) {
  fhicl::ParameterSet ps = build_parameter_set_from_header(sph);
  return indent ? ps.to_indented_string() : ps.to_string();
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
} // namespace larsyst

namespace std {
template <typename T> inline auto quoted(T t) {
  std::stringstream ss("");
  ss << t;
  return std::quoted(ss.str());
}
} // namespace std

#endif
