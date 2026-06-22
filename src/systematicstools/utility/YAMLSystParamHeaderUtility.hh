#pragma once

#include "systematicstools/utility/exceptions.hh"
#include "yaml-cpp/yaml.h"

#include <cstdint>
#include <string>

namespace systtools {
struct SystParamHeader;
}

namespace systtools {

NEW_SYSTTOOLS_EXCEPT(invalid_YAML_variation_descriptor);
NEW_SYSTTOOLS_EXCEPT(invalid_YAML_random_distribution_descriptor);

///\brief Set up SystParamHeader variation definitions from common YAML format
///
/// Returns whether any setup occured
///
/// Uses string value of key in `yamlnd` to initialize SystParamHeader variation
/// datamembers for a few common uses. YAML examples (two equivalent styles
/// are supported: flattened per-parameter keys, or nested parameter maps):
///
/// 1) New central value
///    # flattened
///    MyParam_central_value: 0.5
///
///    # nested
///    MyParam:
///      central_value: 0.5
///
///    Effects:
///    - `isCorrection` = true
///    - `centralParamValue` = 0.5
///
/// 2) One-sigma shifts (symmetric)
///    # flattened
///    MyParam_variation_descriptor: "{1.0}"
///
///    # nested
///    MyParam:
///      variation_descriptor: "{1.0}"
///
///    Effects:
///    - `isRandomlyThrown` = true
///    - `oneSigmaShifts` = {-1.0, 1.0}
///
/// 3) One-sigma shifts (asymmetric)
///    MyParam_variation_descriptor: "{0.8,1.2}"
///
///    Effects:
///    - `oneSigmaShifts` = {0.8, 1.2}
///
/// 4) Spline/list descriptor (range with step)
///    MyParam_variation_descriptor: "(start,end,step)"
///    Example: "(-3,3,1)" -> paramVariations = {-3,-2,-1,0,1,2,3}
///    Sets `isSplineable` = true
///
/// 5) Discrete variations
///    MyParam_variation_descriptor: "[5,3,1,4]"
///    Produces `paramVariations` = {5,3,1,4}
///
/// Notes: The `variation_descriptor` and central-value keys may be provided
/// as top-level flattened keys ("<name>_central_value", "<name>_variation_descriptor")
/// or as nested mappings under the parameter name. All descriptors are parsed
/// from strings and must follow the formats shown above.
///
/// throws invalid_YAML_variation_descriptor on error
bool ParseYAMLVariationDescriptor(YAML::Node const &yamlnd,
                                  std::string const &CV_key,
                                  std::string const &vardescriptor_key,
                                  SystParamHeader &hdr);

///\brief Throws random parameter variations
///
/// Returns whether any throws were made
///
/// If distribution_key is not found, a gaussian distribution will be used.
/// Currently handles "normal", "gaussian", and "uniform" distributions, other
/// values will cause a invalid_YAML_random_distribution_descriptor exception
/// to be thrown.
///
bool MakeYAMLDefinedRandomVariations(YAML::Node const &yamlnd,
                                    std::string const &nthrows_key,
                                    SystParamHeader &hdr,
                                    std::string const &distribution_key = "",
                                    uint64_t seed = 0, size_t NThrows = 0);

///\brief Checks if yamlnd appears to provide standardized Tool Configuration
/// for a named parameter
///
/// If either "<parameter_name>_central_value" or
/// "<parameter_name>_variation_descriptor" exist, the parameter named
/// <parameter_name> is considered to exist in the configuration.
bool YAMLToolConfigurationParameterExists(
    YAML::Node const &yamlnd, std::string const &parameter_name);

///\brief Builds SystParamHeader from standardized YAML that can be used to
/// write Tool Configuration files.
///
/// Returns if parameter configuation keys were found.
///
bool ParseYAMLToolConfigurationParameter(
    YAML::Node const &yamlnd, std::string const &parameter_name,
    SystParamHeader &hdr, uint64_t seed = 0, size_t NThrows = 0);

} // namespace systtools
