#include "systematicstools/utility/YAMLSystParamHeaderUtility.hh"

#include "systematicstools/utility/string_parsers.hh"

#include "systematicstools/interface/SystMetaData.hh"
#include "systematicstools/interface/types.hh"

#include "yaml-cpp/yaml.h"

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

namespace systtools {

bool ParseYAMLVariationDescriptor(YAML::Node const &yamlnd,
                                  std::string const &CV_key,
                                  std::string const &vardescriptor_key,
                                  SystParamHeader &hdr) {
  bool has_cv = static_cast<bool>(yamlnd[CV_key]);
  bool has_var = static_cast<bool>(yamlnd[vardescriptor_key]);

  if (!has_cv && !has_var) {
    return false;
  }

  if (has_cv) hdr.centralParamValue = yamlnd[CV_key].as<double>();
  std::string var_descriptor;
  YAML::Node var_node;
  if (has_var) {
    var_node = yamlnd[vardescriptor_key];
    if (var_node.IsScalar()) {
      var_descriptor = var_node.as<std::string>();
    } else if (var_node.IsSequence()) {
      hdr.paramVariations = var_node.as<std::vector<double>>();
      hdr.isRandomlyThrown = false;
      hdr.isSplineable = false;

      if (hdr.paramVariations.size() == 1) {
        hdr.centralParamValue = hdr.paramVariations.front();
        hdr.isCorrection = true;
      } else if (hdr.paramVariations.empty()) {
        throw invalid_YAML_variation_descriptor()
            << "[ERROR]: variation_descriptor sequence was provided, but it "
               "contained no parameter variations.";
      }
      return true;
    } else {
      throw invalid_YAML_variation_descriptor()
          << "[ERROR]: variation_descriptor must be either a scalar string "
             "descriptor or a numeric YAML sequence.";
    }
  }

  trim(var_descriptor);

  // 1) When "variation_descriptor" is provided
  if (var_descriptor.size()) {
    char fchar = var_descriptor.front();
    std::string var_descriptor_trimmed =
        var_descriptor.substr(1, var_descriptor.length() - 2);
    trim(var_descriptor_trimmed);
    if (fchar == '(') { // Spline knots
      std::vector<double> range_step_values =
          ParseToVect<double>(var_descriptor_trimmed, ",");
      if (range_step_values.size() != 3) {
        throw invalid_YAML_variation_descriptor()
            << "[ERROR]: When parsing spline knot descriptor found "
            << std::quoted(var_descriptor_trimmed)
            << ", but the descriptor must be in the format: "
               "(<start>,<end>,<step>).";
      }
      hdr.paramVariations.push_back(range_step_values[0]);
      while ((hdr.paramVariations.back() + range_step_values[2]) <=
             (range_step_values[1] + std::numeric_limits<double>::epsilon())) {
        hdr.paramVariations.push_back(hdr.paramVariations.back() +
                                      range_step_values[2]);
      }
      hdr.isSplineable = true;
    } else if (fchar == '[') { // Discrete tweaks
      hdr.paramVariations = ParseToVect<double>(var_descriptor_trimmed, ",");
    } else if (fchar == '{') { // OneSigmaShifts
      std::vector<double> sigShifts =
          ParseToVect<double>(var_descriptor_trimmed, ",");
      if (sigShifts.size() == 1) {
        hdr.oneSigmaShifts[0] = -sigShifts.front();
        hdr.oneSigmaShifts[1] = sigShifts.front();
      } else if (sigShifts.size() == 2) {
        hdr.oneSigmaShifts[0] = sigShifts.front();
        hdr.oneSigmaShifts[1] = sigShifts.back();
      } else {
        throw invalid_YAML_variation_descriptor()
            << "[ERROR]: When parsing sigma shifts found "
            << std::quoted(var_descriptor_trimmed)
            << ", but expected {sigma_both_natural_units}, or "
               "{sigma_low_natural_units, sigma_up_natural_units}.";
      }
      hdr.isRandomlyThrown = true;
      hdr.isSplineable = false;
    } else {
      throw invalid_YAML_variation_descriptor()
          << "[ERROR]: Found tweak definition " << std::quoted(var_descriptor)
          << ", but expected to find either, \"{sigma_low_natural_units, "
             "sigma_up_natural_units}\" or \"[spline knot 1, spline knot "
             "2, spline knot 3,...]\"";
    }

    // If there is only one variation, set isCorrection to true.
    // Also, if RW(CV) is not 1.0, it should be included in variation_descriptor,
    // so the systprovider can calculate that non-1.0 reweight.
    // So when RW(CV) is not 1.0 and we want a correction to another value (Alt),
    // we need variation_descriptor: [CV, Alt].
    // This means when we have only one variation, this means RW(CV) is 1.0, and no need to evaluate RW(CV) separately
    if (!hdr.isRandomlyThrown) {
      if (hdr.paramVariations.size() == 1) {
        // Because of the reason above, we can safely set central value to the given single variation value
        hdr.centralParamValue = hdr.paramVariations.front();
        hdr.isCorrection = true;
      } else if (!hdr.paramVariations.size()) {
        throw invalid_YAML_variation_descriptor()
            << "[ERROR]: When parsing " << var_descriptor
            << ", failed to determine any parameter variations.";
      }
    }
  }
  // 2) When "variation_descriptor" is NOT provided;
  //    E.g., only central_value is given
  else {
    if(!has_cv){
      // We already have 
      //   if (!has_cv && !has_var) {
      //    return false;
      //  }
      // , so this won't happen, but for safety..
      throw invalid_YAML_variation_descriptor()
            << "[ERROR]: Neither variation_descriptor nor central_value is provided";
    }

    // Set isCorrection to true
    hdr.isCorrection = true;
    // Let's still fill paramVariations with the central value
    hdr.paramVariations.clear();
    hdr.paramVariations.push_back( hdr.centralParamValue );
  }
  return true;
}

bool MakeYAMLDefinedRandomVariations(YAML::Node const &yamlnd,
                                    std::string const &nthrows_key,
                                    SystParamHeader &hdr,
                                    std::string const &distribution_key,
                                    uint64_t seed, size_t NThrows) {
  if (!hdr.isRandomlyThrown) {
    return false;
  }

  if (yamlnd[nthrows_key]) NThrows = yamlnd[nthrows_key].as<size_t>();

  if (!NThrows) {
    return false;
  }

  hdr.paramVariations.clear();

  std::mt19937_64 generator(
      seed == 0 ? std::chrono::steady_clock::now().time_since_epoch().count()
                : seed);
  std::function<double()> RNJesus;

  // Choose distribution
  if (distribution_key.size() && yamlnd[distribution_key]) {
    std::string dist_ident = yamlnd[distribution_key].as<std::string>();

    if ((dist_ident == "normal") || (dist_ident == "gaussian")) {
      std::normal_distribution<double> distribution(0, 1);
      RNJesus = std::bind(distribution, generator);
    } else if (dist_ident == "uniform") {
      std::uniform_real_distribution<double> distribution(-1, 1);
      RNJesus = std::bind(distribution, generator);
    }
  } else {
    std::normal_distribution<double> distribution(0, 1);
    RNJesus = std::bind(distribution, generator);
  }

  double cv =
      (hdr.centralParamValue == kDefaultDouble) ? 0 : hdr.centralParamValue;
  for (uint64_t t = 0; t < NThrows; ++t) {
    double thr = RNJesus();
    double shift =
        fabs(thr) * ((thr < 0) ? hdr.oneSigmaShifts[0] : hdr.oneSigmaShifts[1]);
    hdr.paramVariations.push_back(cv + shift);
  }

  return true;
}

bool YAMLToolConfigurationParameterExists(
    YAML::Node const &yamlnd, std::string const &parameter_name) {

  std::string CV_key = parameter_name + "_central_value";
  std::string Tweak_key = parameter_name + "_variation_descriptor";

  bool has_cv = static_cast<bool>(yamlnd[CV_key]);
  bool has_var = static_cast<bool>(yamlnd[Tweak_key]);

  return (has_cv || has_var);
}

bool ParseYAMLToolConfigurationParameter(
    YAML::Node const &yamlnd, std::string const &parameter_name,
    SystParamHeader &hdr, uint64_t seed, size_t NThrows) {

  std::string CV_key = parameter_name + "_central_value";
  std::string Tweak_key = parameter_name + "_variation_descriptor";

  if (!ParseYAMLVariationDescriptor(yamlnd, CV_key, Tweak_key, hdr)) {
    return false;
  }

  // override isSplineable if specified
  if( yamlnd[parameter_name + "_isSplineable"] ) {
    hdr.isSplineable = yamlnd[parameter_name + "_isSplineable"].as<bool>();
  }

  hdr.prettyName = parameter_name;

  std::string NThrows_key = parameter_name + "_nthrows";
  std::string RandDist_key = parameter_name + "_random_distribution";

  MakeYAMLDefinedRandomVariations(yamlnd, NThrows_key, hdr, RandDist_key,
                                   seed, NThrows);

  return true;
}

} // namespace systtools
