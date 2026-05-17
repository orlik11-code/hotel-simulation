#include "config/file_parse.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>

namespace file_parse {

std::string trim(std::string_view text) {
  const auto start = text.find_first_not_of(" \t\r\n");
  if (start == std::string_view::npos) {
    return {};
  }
  const auto end = text.find_last_not_of(" \t\r\n");
  return std::string(text.substr(start, end - start + 1));
}

bool is_comment_or_empty(std::string_view line) {
  const std::string trimmed = trim(line);
  return trimmed.empty() || trimmed[0] == '#';
}

std::pair<std::string, std::string> split_key_value(const std::string &line,
                                                      unsigned int line_no) {
  const auto eq = line.find('=');
  if (eq == std::string::npos) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": expected key=value");
  }
  return {trim(line.substr(0, eq)), trim(line.substr(eq + 1))};
}

unsigned int parse_positive(const std::string &value, const std::string &field) {
  if (value.empty()) {
    throw std::invalid_argument("Empty value for " + field);
  }
  if (value[0] == '-') {
    throw std::invalid_argument("Values cannot be negative in " + field);
  }
  if (value == "0") {
    throw std::invalid_argument("Values cannot be 0 in " + field);
  }
  try {
    const unsigned long parsed = std::stoul(value);
    return static_cast<unsigned int>(parsed);
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid value for " + field + ": " + value);
  }
}

unsigned int parse_non_negative(const std::string &value,
                                const std::string &field) {
  if (value.empty()) {
    throw std::invalid_argument("Empty value for " + field);
  }
  if (value[0] == '-') {
    throw std::invalid_argument("Values cannot be negative in " + field);
  }
  try {
    return static_cast<unsigned int>(std::stoul(value));
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid value for " + field + ": " + value);
  }
}

unsigned int parse_percent(const std::string &value, const std::string &field) {
  if (value.empty()) {
    throw std::invalid_argument("Empty value for " + field);
  }
  try {
    const double parsed = std::stod(value);
    if (parsed < 0.0) {
      throw std::invalid_argument("Values cannot be negative in " + field);
    }
    if (parsed > 30.0) {
      throw std::invalid_argument(field + " cannot exceed 30");
    }
    return static_cast<unsigned int>(std::lround(parsed));
  } catch (const std::invalid_argument &) {
    throw;
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid value for " + field + ": " + value);
  }
}

std::string parse_section_header(std::string_view line) {
  const std::string trimmed = trim(line);
  if (trimmed.size() < 3 || trimmed.front() != '[' || trimmed.back() != ']') {
    return {};
  }
  return trim(trimmed.substr(1, trimmed.size() - 2));
}

} // namespace file_parse
