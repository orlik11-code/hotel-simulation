#pragma once

#include <string>
#include <string_view>
#include <utility>

namespace file_parse {

[[nodiscard]] std::string trim(std::string_view text);

[[nodiscard]] bool is_comment_or_empty(std::string_view line);

/** @throws std::invalid_argument if line is not key=value */
[[nodiscard]] std::pair<std::string, std::string>
split_key_value(const std::string &line, unsigned int line_no);

[[nodiscard]] unsigned int parse_positive(const std::string &value,
                                          const std::string &field);

[[nodiscard]] unsigned int parse_non_negative(const std::string &value,
                                              const std::string &field);

/** Returns inner name for lines like "[worker]" (without brackets), or empty. */
[[nodiscard]] std::string parse_section_header(std::string_view line);

} // namespace file_parse
