#pragma once
#include <string>
#include <vector>

namespace vcs {

struct DiffHunkLine {
  char tag;          // '+', '-', ' ' (add, delete, context)
  std::string text;  // line text without newline
};

std::vector<DiffHunkLine> diffText(const std::string& a, const std::string& b);

} 
