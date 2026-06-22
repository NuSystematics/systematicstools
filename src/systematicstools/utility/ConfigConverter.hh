#pragma once

#include "yaml-cpp/yaml.h"
#include "fhiclcpp/ParameterSet.h"

namespace systtools {

/// Convert a fhicl::ParameterSet into a YAML::Node.
YAML::Node FHiCLToYAML(fhicl::ParameterSet const &config);

/// Convert a YAML::Node into a fhicl::ParameterSet.
///
/// The YAML node is first rendered into a FHiCL-compatible text representation
/// and then parsed using fhicl::ParameterSet::make.
fhicl::ParameterSet YAMLToFHiCL(YAML::Node const &yamlnd);

} // namespace systtools
