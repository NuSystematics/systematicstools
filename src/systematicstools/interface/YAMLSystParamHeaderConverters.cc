#include "systematicstools/interface/YAMLSystParamHeaderConverters.hh"

#include "systematicstools/utility/string_parsers.hh"

#include "systematicstools/interface/types.hh"

#include <algorithm>
#include <vector>
#include <iomanip>

namespace systtools {

SystParamHeader YAMLToSystParamHeader(YAML::Node const &yamlnd) {

  // Member variables of SystParamHeader should be listed below
  static std::vector<std::string> allowed_keys = {
    "prettyName",
    "systParamId",
    "isWeightSystematicVariation",
    "unitsAreNatural",
    "differsEventByEvent",
    "centralParamValue",
    "isCorrection",
    "oneSigmaShifts",
    "paramValidityRange",
    "isSplineable",
    "isRandomlyThrown",
    "paramVariations",
    "isResponselessParam",
    "responseParamId",
    "responses",
    "opts"
  };

  if (!yamlnd.IsMap()) {
    throw invalid_SystParamHeader_key()
        << "[ERROR]: Expected YAML map for SystParamHeader, but got non-map node.";
  }

  for (auto const &kv : yamlnd) {
    std::string key = kv.first.as<std::string>();
    if (std::find(allowed_keys.begin(), allowed_keys.end(), key) ==
        allowed_keys.end()) {
      throw invalid_SystParamHeader_key()
          << "[ERROR]: When parsing YAML::Node as a "
             "systtools::SystParamHeader, encountered key "
          << std::quoted(key) << " which was not expected.";
    }
  }

  SystParamHeader sph;
  sph.prettyName = yamlnd["prettyName"].as<std::string>();
  sph.systParamId = yamlnd["systParamId"].as<paramId_t>();
  if (yamlnd["isWeightSystematicVariation"])
    sph.isWeightSystematicVariation =
        yamlnd["isWeightSystematicVariation"].as<bool>();
  if (yamlnd["unitsAreNatural"])
    sph.unitsAreNatural = yamlnd["unitsAreNatural"].as<bool>();
  if (yamlnd["differsEventByEvent"])
    sph.differsEventByEvent = yamlnd["differsEventByEvent"].as<bool>();
  if (yamlnd["centralParamValue"])
    sph.centralParamValue = yamlnd["centralParamValue"].as<double>();
  if (yamlnd["isCorrection"])
    sph.isCorrection = yamlnd["isCorrection"].as<bool>();
  if (yamlnd["oneSigmaShifts"])
    sph.oneSigmaShifts = yamlnd["oneSigmaShifts"].as<std::array<double, 2>>();
  if (yamlnd["paramValidityRange"])
    sph.paramValidityRange =
        yamlnd["paramValidityRange"].as<std::array<double, 2>>();
  if (yamlnd["isSplineable"])
    sph.isSplineable = yamlnd["isSplineable"].as<bool>();
  if (yamlnd["isRandomlyThrown"])
    sph.isRandomlyThrown = yamlnd["isRandomlyThrown"].as<bool>();
  if (yamlnd["paramVariations"])
    sph.paramVariations = yamlnd["paramVariations"].as<std::vector<double>>();
  if (yamlnd["isResponselessParam"])
    sph.isResponselessParam = yamlnd["isResponselessParam"].as<bool>();
  if (yamlnd["responseParamId"])
    sph.responseParamId = yamlnd["responseParamId"].as<paramId_t>();
  if (yamlnd["responses"])
    sph.responses = yamlnd["responses"].as<std::vector<double>>();
  if (yamlnd["opts"])
    sph.opts = yamlnd["opts"].as<std::vector<std::string>>();

  return sph;
}

YAML::Node SystParamHeaderToYAML(SystParamHeader const &sph) {
  if (!Validate(sph)) {
    (void)Validate(sph, false);
    throw invalid_SystParamHeader()
        << "[ERROR]: Parameter set (" << sph.systParamId << ":"
        << std::quoted(sph.prettyName) << ") failed validation.";
  }

  YAML::Node yaml;
  yaml["prettyName"] = sph.prettyName;
  yaml["systParamId"] = sph.systParamId;
  if (!sph.isWeightSystematicVariation)
    yaml["isWeightSystematicVariation"] = sph.isWeightSystematicVariation;
  if (sph.unitsAreNatural)
    yaml["unitsAreNatural"] = sph.unitsAreNatural;
  if (!sph.differsEventByEvent)
    yaml["differsEventByEvent"] = sph.differsEventByEvent;
  if (sph.centralParamValue != kDefaultDouble)
    yaml["centralParamValue"] = sph.centralParamValue;
  if (sph.isCorrection)
    yaml["isCorrection"] = sph.isCorrection;
  if (sph.oneSigmaShifts[0] != kDefaultDouble)
    yaml["oneSigmaShifts"] = std::vector<double>{
        sph.oneSigmaShifts[0], sph.oneSigmaShifts[1]};
  if ((sph.paramValidityRange[0] != kDefaultDouble) ||
      (sph.paramValidityRange[1] != kDefaultDouble))
    yaml["paramValidityRange"] = std::vector<double>{
        sph.paramValidityRange[0], sph.paramValidityRange[1]};
  if (sph.isSplineable)
    yaml["isSplineable"] = sph.isSplineable;
  if (sph.isRandomlyThrown)
    yaml["isRandomlyThrown"] = sph.isRandomlyThrown;
  if (!sph.paramVariations.empty()) {
    yaml["paramVariations"] = sph.paramVariations;
    yaml["paramVariations"].SetStyle(YAML::EmitterStyle::Flow);
  }
  if (sph.isResponselessParam)
    yaml["isResponselessParam"] = sph.isResponselessParam;
  if (sph.responseParamId != kParamUnhandled<paramId_t>)
    yaml["responseParamId"] = sph.responseParamId;
  if (!sph.responses.empty())
    yaml["responses"] = sph.responses;
  if (!sph.opts.empty())
    yaml["opts"] = sph.opts;

  return yaml;
}

} // namespace systtools
