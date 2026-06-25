#pragma once

#include "systematicstools/utility/exceptions.hh"

#include "yaml-cpp/yaml.h"

#include <string>

namespace systtools {
struct SystParamHeader;
}

namespace systtools {

/// Exception thrown when an unexpected key is found in a YAML::Node
/// being parsed as a SystParamHeader
// TODO REVIDE
NEW_SYSTTOOLS_EXCEPT(invalid_SystParamHeader_key);

///\brief Deserializes a SystParamHeader instance from a passed YAML parameter
/// set.
SystParamHeader YAMLToSystParamHeader(YAML::Node const &yamlnd);

///\brief Serializes a SyhstParamHeader instance to a YAML node.
YAML::Node SystParamHeaderToYAML(SystParamHeader const &sph);
}
