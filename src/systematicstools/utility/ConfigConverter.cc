#include "systematicstools/utility/ConfigConverter.hh"
#include "fhiclcpp/ParameterSetWalker.h"
#include <cctype>
#include <sstream>
#include <string>

namespace systtools {

namespace {

std::string quoteString(std::string const &value) {
  std::ostringstream out;
  out << '"';
  for (char c : value) {
    switch (c) {
      case '\\': out << "\\\\"; break;
      case '"': out << "\\\""; break;
      case '\b': out << "\\b"; break;
      case '\f': out << "\\f"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      default:
        out << c;
        break;
    }
  }
  out << '"';
  return out.str();
}

std::string FormatScalar(YAML::Node const &scalar) {
  if (!scalar.IsScalar()) {
    return "";
  }

  std::string token = scalar.Scalar();
  if (token.empty()) {
    return quoteString(token);
  }

  bool isNumber = true;
  bool isBoolean = (token == "true" || token == "false");
  bool isQuoted = ((token.front() == '"' && token.back() == '"') ||
                   (token.front() == '\'' && token.back() == '\''));

  if (!isQuoted) {
    for (char c : token) {
      if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '+')) {
        isNumber = false;
        break;
      }
    }
  }

  if (isBoolean) {
    return token;
  }

  if (isQuoted) {
    return token;
  }

  if (isNumber) {
    return token;
  }

  return quoteString(token);
}

std::string DecodeFHiCLStringToken(std::string const &value) {
  if (value.size() < 2) {
    return value;
  }

  char quote = value.front();
  if ((quote != '"' && quote != '\'') || value.back() != quote) {
    return value;
  }

  std::string out;
  out.reserve(value.size() - 2);

  for (std::size_t idx = 1; idx + 1 < value.size(); ++idx) {
    char c = value[idx];
    if (c == '\\' && idx + 1 < value.size() - 1) {
      char next = value[++idx];
      switch (next) {
        case '\\': out.push_back('\\'); break;
        case '"': out.push_back('"'); break;
        case '\'': out.push_back('\''); break;
        case 'b': out.push_back('\b'); break;
        case 'f': out.push_back('\f'); break;
        case 'n': out.push_back('\n'); break;
        case 'r': out.push_back('\r'); break;
        case 't': out.push_back('\t'); break;
        default:
          out.push_back(next);
          break;
      }
      continue;
    }
    out.push_back(c);
  }

  return out;
}

std::string BuildFHiCLText(YAML::Node const &node) {
  if (node.IsScalar()) {
    return FormatScalar(node);
  }

  if (node.IsSequence()) {
    std::string out = "[";
    for (std::size_t idx = 0; idx < node.size(); ++idx) {
      if (idx) out += ", ";
      out += BuildFHiCLText(node[idx]);
    }
    out += "]";
    return out;
  }

  if (node.IsMap()) {
    std::string out;
    bool first = true;
    for (auto it = node.begin(); it != node.end(); ++it) {
      if (!first) {
        out += ",\n";
      }
      first = false;
      out += it->first.as<std::string>();
      out += ": ";
      out += BuildFHiCLText(it->second);
    }
    return "{\n" + out + "\n}";
  }

  return "";
}

class FHiCLWalker final : public fhicl::ParameterSetWalker {
public:
  explicit FHiCLWalker(YAML::Node &root) {
    container_stack_.push_back(root);
  }

  void enter_table(key_t const &key, any_t const &) override {
    YAML::Node &container = container_stack_.back();
    if (container.IsSequence()) {
      container.push_back(YAML::Node(YAML::NodeType::Map));
      container_stack_.push_back(container[container.size() - 1]);
    } else {
      container[key] = YAML::Node(YAML::NodeType::Map);
      container_stack_.push_back(container[key]);
    }
  }

  void exit_table(key_t const &, any_t const &) override {
    if (container_stack_.size() > 1) {
      container_stack_.pop_back();
    }
  }

  void enter_sequence(key_t const &key, any_t const &) override {
    YAML::Node &container = container_stack_.back();
    if (container.IsSequence()) {
      container.push_back(YAML::Node(YAML::NodeType::Sequence));
      container[container.size() - 1].SetStyle(YAML::EmitterStyle::Flow);
      container_stack_.push_back(container[container.size() - 1]);
    } else {
      container[key] = YAML::Node(YAML::NodeType::Sequence);
      container[key].SetStyle(YAML::EmitterStyle::Flow);
      container_stack_.push_back(container[key]);
    }
  }

  void exit_sequence(key_t const &, any_t const &) override {
    if (container_stack_.size() > 1) {
      container_stack_.pop_back();
    }
  }

  void atom(key_t const &key, any_t const &value) override {
    YAML::Node &container = container_stack_.back();
    if (container.IsSequence()) {
      appendToSequence(container, value);
    } else {
      YAML::Node node = container[key];
      assignAtom(node, value);
    }
  }

private:
  void assignAtom(YAML::Node node, any_t const &value) {
    if (value.type() == typeid(std::string)) {
      node = DecodeFHiCLStringToken(std::any_cast<std::string>(value));
    } else if (value.type() == typeid(int)) {
      node = std::any_cast<int>(value);
    } else if (value.type() == typeid(unsigned int)) {
      node = std::any_cast<unsigned int>(value);
    } else if (value.type() == typeid(long)) {
      node = std::any_cast<long>(value);
    } else if (value.type() == typeid(unsigned long)) {
      node = std::any_cast<unsigned long>(value);
    } else if (value.type() == typeid(long long)) {
      node = std::any_cast<long long>(value);
    } else if (value.type() == typeid(unsigned long long)) {
      node = std::any_cast<unsigned long long>(value);
    } else if (value.type() == typeid(float)) {
      node = std::any_cast<float>(value);
    } else if (value.type() == typeid(double)) {
      node = std::any_cast<double>(value);
    } else if (value.type() == typeid(bool)) {
      node = std::any_cast<bool>(value);
    } else {
      node = std::string{};
    }
  }

  void appendToSequence(YAML::Node &sequence, any_t const &value) {
    if (value.type() == typeid(std::string)) {
      sequence.push_back(DecodeFHiCLStringToken(std::any_cast<std::string>(value)));
    } else if (value.type() == typeid(int)) {
      sequence.push_back(std::any_cast<int>(value));
    } else if (value.type() == typeid(unsigned int)) {
      sequence.push_back(std::any_cast<unsigned int>(value));
    } else if (value.type() == typeid(long)) {
      sequence.push_back(std::any_cast<long>(value));
    } else if (value.type() == typeid(unsigned long)) {
      sequence.push_back(std::any_cast<unsigned long>(value));
    } else if (value.type() == typeid(long long)) {
      sequence.push_back(std::any_cast<long long>(value));
    } else if (value.type() == typeid(unsigned long long)) {
      sequence.push_back(std::any_cast<unsigned long long>(value));
    } else if (value.type() == typeid(float)) {
      sequence.push_back(std::any_cast<float>(value));
    } else if (value.type() == typeid(double)) {
      sequence.push_back(std::any_cast<double>(value));
    } else if (value.type() == typeid(bool)) {
      sequence.push_back(std::any_cast<bool>(value));
    } else {
      sequence.push_back(std::string{});
    }
  }

  std::vector<YAML::Node> container_stack_;
};

} // namespace

YAML::Node FHiCLToYAML(fhicl::ParameterSet const &config) {
  YAML::Node root(YAML::NodeType::Map);
  FHiCLWalker walker(root);
  config.walk(walker);
  return root;
}

fhicl::ParameterSet YAMLToFHiCL(YAML::Node const &yamlnd) {
  std::string text = BuildFHiCLText(yamlnd);
  return fhicl::ParameterSet::make(text);
}

} // namespace systtools
