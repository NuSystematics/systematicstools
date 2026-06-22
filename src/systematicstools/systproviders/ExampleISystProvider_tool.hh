#pragma once

#include "systematicstools/interface/ISystProviderTool.hh"

#include "yaml-cpp/yaml.h"

#include <memory>
#include <random>
#include <string>

class ExampleISystProvider : public systtools::ISystProviderTool {
public:
  explicit ExampleISystProvider(YAML::Node const &);

  systtools::SystMetaData BuildSystMetaData(YAML::Node const &,
                                          systtools::paramId_t) override;
  YAML::Node GetExtraToolOptions() override;
  bool SetupResponseCalculator(YAML::Node const &) override;

  std::string AsString() override;

private:
  bool applyToAll;
  std::unique_ptr<std::mt19937_64> RNgine;
  std::unique_ptr<std::normal_distribution<double>> RNJesus;
};
