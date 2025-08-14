#pragma once

#include <algorithm>
#include <cctype>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

namespace systtools {

template <typename T> inline T str2T(std::string const &str) {
  std::istringstream stream(str);
  T d;
  stream >> d;

  if (stream.fail()) {
    std::cerr << "[WARN]: Failed to parse string: " << str
              << " as requested type." << std::endl;
    return T();
  }

  return d;
}

template <> inline bool str2T<bool>(std::string const &str) {
  if ((str == "true") || (str == "True") || (str == "TRUE") || (str == "1")) {
    return true;
  }

  if ((str == "false") || (str == "False") || (str == "FALSE") ||
      (str == "0")) {
    return false;
  }

  std::istringstream stream(str);
  bool d;
  stream >> d;

  if (stream.fail()) {
    std::cerr << "[WARN]: Failed to parse string: " << str
              << " as requested type." << std::endl;
    return false;
  }

  return d;
}
template <typename T>
inline void AppendVect(std::vector<T> &target, std::vector<T> const &toApp) {
  for (size_t i = 0; i < toApp.size(); ++i) {
    target.push_back(toApp[i]);
  }
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

template <typename T>
inline std::vector<T>
ParseToVect(std::string const &inp, std::string const &delim,
            bool PushEmpty = false, bool trimInput = true) {
  std::string inpCpy = inp;
  if (trimInput) {
    trim(inpCpy);
  }
  size_t nextOccurence = 0;
  size_t prevOccurence = 0;
  std::vector<T> outV;
  bool AtEnd = false;
  while (!AtEnd) {
    nextOccurence = inpCpy.find(delim, prevOccurence);
    if (nextOccurence == std::string::npos) {
      if (prevOccurence == inpCpy.length()) {
        break;
      }
      AtEnd = true;
    }
    if (PushEmpty || (nextOccurence != prevOccurence)) {
      outV.push_back(str2T<T>(
          inpCpy.substr(prevOccurence, (nextOccurence - prevOccurence))));
    }
    prevOccurence = nextOccurence + delim.size();
  }
  return outV;
}

inline std::string str_replace(std::string const &inp, std::string const &from,
                           std::string const &to) {
  std::stringstream ss("");

  size_t nextOccurence = 0;
  size_t prevOccurence = 0;
  bool AtEnd = false;
  while (!AtEnd) {
    nextOccurence = inp.find(from, prevOccurence);
    if (nextOccurence == std::string::npos) {
      if (prevOccurence == inp.length()) {
        break;
      }
      AtEnd = true;
    }
    if ((nextOccurence != prevOccurence) || (nextOccurence == 0)) {
      ss << inp.substr(prevOccurence, (nextOccurence - prevOccurence));
      if (!AtEnd) {
        ss << to;
      }
    }
    prevOccurence = nextOccurence + from.size();
  }
  return ss.str();
}

// Expand $VAR or ${VAR} in the string
inline std::string expand_env_vars(const std::string& input) {
  static const std::regex env_pattern(R"(\$([A-Za-z_][A-Za-z0-9_]*)|\$\{([^}]+)\})");
  std::string result;
  result.reserve(input.size());

  std::sregex_iterator it(input.begin(), input.end(), env_pattern);
  std::sregex_iterator end;
  size_t last_pos = 0;
  for (; it != end; ++it) {
    size_t match_pos = it->position();
    size_t match_len = it->length();

    // Append text before the match
    result.append(input, last_pos, match_pos - last_pos);

    // Extract variable name (either group 1 or group 2 matched)
    std::string var_name = (*it)[1].matched ? (*it)[1].str() : (*it)[2].str();

    const char* val = std::getenv(var_name.c_str());
    if (!val) {
      throw std::runtime_error("Environment variable " + var_name + " not set");
    }
    result.append(val);

    last_pos = match_pos + match_len;
  }

  // Append any remaining text after the last match
  result.append(input, last_pos, std::string::npos);

  return result;
}

} // namespace systtools
