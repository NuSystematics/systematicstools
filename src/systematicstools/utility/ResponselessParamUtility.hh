#pragma once

#include "systematicstools/interface/SystMetaData.hh"

#include "yaml-cpp/yaml.h"

#include <vector>
#include <string>

namespace systtools {
///\brief Checks that all responseless parameters in the set have the same
///number of configured variations and adds dummy variations for the response
///parameter.
///
/// The dependent parameters combine into a single weight held by the response
/// parameter, so only the tool-level WeightMin/WeightMax of yamlnd apply, to
/// the response parameter. Weight limits of the dependent parameters are
/// never used.
void FinalizeAndValidateDependentParameters(
    SystMetaData &, YAML::Node const &yamlnd,
    std::string const &response_parameter_name,
    std::vector<std::string> const &dependent_parameter_names);

} // namespace systtools
