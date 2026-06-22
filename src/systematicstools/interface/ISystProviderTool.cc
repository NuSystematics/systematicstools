#include "systematicstools/interface/ISystProviderTool.hh"

namespace systtools {

ISystProviderTool::ISystProviderTool(YAML::Node const &yamlnd)
    : fToolType{yamlnd["tool_type"].as<std::string>()}, fSeedSuggestion{0},
      fIsFullyConfigured{false}, fHaveSystMetaData{false} {
  if (!yamlnd["instance_name"]) {
    fInstanceName = "";
    fFQName = fToolType;
  } else {
    fInstanceName = yamlnd["instance_name"].as<std::string>();
    fFQName = fToolType + "_" + fInstanceName;
  }
}

paramId_t ISystProviderTool::GetParameterId(std::string const &prettyName) {
  return GetParamId(fSystMetaData, prettyName);
}

void ISystProviderTool::SuggestSeed(uint64_t seed) {
  if (fHaveSystMetaData) {
    throw ISystProviderTool_seed_suggestion_post_configure()
        << "[ERROR]: Seed was suggested to ISystProvider: "
        << std::quoted(GetFullyQualifiedName()) << " after configuration.";
  }
  fSeedSuggestion = seed;
}

void ISystProviderTool::SuggestParameterThrows(parameter_throws_list_t &&,
                                               bool) {
  throw ISystProviderTool_method_unimplemented()
      << "[ERROR]: Attempted to suggest parameter throws to provider tool "
      << std::quoted(GetToolType())
      << ", but it doesn't handle suggested throws.";
}

void ISystProviderTool::ConfigureFromToolConfig(YAML::Node const &yamlnd,
                                                paramId_t firstId) {
  fSystMetaData = this->BuildSystMetaData(yamlnd, firstId);

  // The following check expects them to be ordered, but the provider isn't
  // under any obligation to order them.
  std::stable_sort(fSystMetaData.begin(), fSystMetaData.end(),
                   [](SystParamHeader const &l, SystParamHeader const &r) {
                     return l.systParamId < r.systParamId;
                   });

  for (auto &hdr : fSystMetaData) {
    if (hdr.systParamId != firstId) {
      throw ISystProviderTool_noncontiguous_parameter_Ids()
          << "[ERROR]: Provider " << std::quoted(GetFullyQualifiedName())
          << " failed to set parameter " << std::quoted(hdr.prettyName)
          << " to firstId " << firstId << " != " << hdr.systParamId;
    }
    firstId++;
  }
  fHaveSystMetaData = true;
}

SystMetaData const &ISystProviderTool::GetSystMetaData() const{
  CheckHaveMetaData();
  return fSystMetaData;
}

YAML::Node ISystProviderTool::GetParameterHeadersDocument() {

  CheckHaveMetaData();

  YAML::Node ParamHeadersDoc;
  std::vector<std::string> HeaderKeys;
  for (auto &hdr : GetSystMetaData()) {
    ParamHeadersDoc[hdr.prettyName] = SystParamHeaderToYAML(hdr);
    HeaderKeys.push_back(hdr.prettyName);
  }
  ParamHeadersDoc["parameter_headers"] = HeaderKeys;
  ParamHeadersDoc["tool_type"] = GetToolType();
  if (GetInstanceName().size()) {
    ParamHeadersDoc["instance_name"] = GetInstanceName();
  }

  YAML::Node ToolOptions = GetExtraToolOptions();
  if (ToolOptions && !ToolOptions.IsNull()) {
    ParamHeadersDoc["tool_options"] = ToolOptions;
  }

  return ParamHeadersDoc;
}

bool ISystProviderTool::ConfigureFromParameterHeaders(
    YAML::Node const &yamlnd) {
  if (!yamlnd["parameter_headers"]) {
    throw invalid_ToolConfigurationYAML()
        << "[ERROR]: Expected key 'parameter_headers' in YAML parameter headers.";
  }

  YAML::Node ph_names = yamlnd["parameter_headers"];
  if (!ph_names.IsSequence()) {
    throw invalid_ToolConfigurationYAML()
        << "[ERROR]: 'parameter_headers' must be a YAML sequence of names.";
  }

  for (auto const &nameNode : ph_names) {
    std::string paramName = nameNode.as<std::string>();
    if (!yamlnd[paramName]) {
      throw invalid_ToolConfigurationYAML()
          << "[ERROR]: Parameter header '" << std::quoted(paramName)
          << "' referenced in 'parameter_headers' but not found.";
    }
    fSystMetaData.emplace_back(YAMLToSystParamHeader(yamlnd[paramName]));
  }

  fHaveSystMetaData = true;

  YAML::Node ToolOptions;
  if (yamlnd["tool_options"]) ToolOptions = yamlnd["tool_options"];

  fIsFullyConfigured = this->SetupResponseCalculator(ToolOptions);

  std::cout << "[INFO]: Syst provider " << std::quoted(GetFullyQualifiedName())
            << " configured " << fSystMetaData.size() << " parameters."
            << std::endl;

  return fIsFullyConfigured;
}

systtools::event_unit_response_t
ISystProviderTool::GetDefaultEventResponse() const
{
  auto const& smd = this->GetSystMetaData();
  systtools::event_unit_response_t resp;
  resp.reserve(smd.size());
  for(auto const& sph : smd) {
    resp.push_back(responses_for(sph));
  }
  return resp;
}

void ISystProviderTool::CheckHaveMetaData(paramId_t i) const{
  if (!fHaveSystMetaData) {
    throw ISystProviderTool_metadata_not_generated()
        << "[ERROR]: Requested syst set configuration from syst provider "
        << GetFullyQualifiedName() << ", but it has not been generated yet.";
  }
  if (i != kParamUnhandled<paramId_t>) {
    if (!ParamIsHandled(i)) {
      throw ISystProviderTool_metadata_not_generated()
          << "[ERROR]: SuggestParameterThrows Check failed. Parameter "
             "with id \""
          << i << "\", is not handled by this systematic provider: \""
          << GetFullyQualifiedName() << "\".";
    }
  }
}

ParamResponses
responses_for(SystParamHeader const& sph)
{
  // Backward compatibility; isCorrection header used to have empty paramVariations
  if (sph.isCorrection) {
    return {sph.systParamId, std::vector<double>{1.}};
  }
  return {sph.systParamId, std::vector<double>(sph.paramVariations.size(), 1.)};
}

} // namespace systtools
