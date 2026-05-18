#include "config/file_parse.hpp"

#include <chrono>
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

std::chrono::year_month_day parse_date(const std::string &value,
                                       const std::string &field) {
  const auto first_dash = value.find('-');
  const auto second_dash =
      first_dash == std::string::npos ? std::string::npos
                                      : value.find('-', first_dash + 1U);
  if (first_dash == std::string::npos || second_dash == std::string::npos ||
      value.find('-', second_dash + 1U) != std::string::npos) {
    throw std::invalid_argument(field +
                                " must use YYYY-MM-DD format: " + value);
  }

  const std::string year_part = value.substr(0U, first_dash);
  const std::string month_part =
      value.substr(first_dash + 1U, second_dash - first_dash - 1U);
  const std::string day_part = value.substr(second_dash + 1U);

  if (year_part.size() != 4U) {
    throw std::invalid_argument(field + " year must have 4 digits: " + value);
  }

  int year = 0;
  try {
    year = std::stoi(year_part);
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid year in " + field + ": " + value);
  }

  const unsigned int month = parse_positive(month_part, field + " (month)");
  const unsigned int day = parse_positive(day_part, field + " (day)");

  if (month > 12U) {
    throw std::invalid_argument(field + " month must be 1–12: " + value);
  }

  const std::chrono::year_month_day parsed{
      std::chrono::year{year},
      std::chrono::month{month},
      std::chrono::day{day}};

  if (!parsed.ok()) {
    throw std::invalid_argument("Invalid calendar date for " + field + ": " +
                                value);
  }

  return parsed;
}

std::string parse_section_header(std::string_view line) {
  const std::string trimmed = trim(line);
  if (trimmed.size() < 3 || trimmed.front() != '[' || trimmed.back() != ']') {
    return {};
  }
  return trim(trimmed.substr(1, trimmed.size() - 2));
}

} // namespace file_parse
