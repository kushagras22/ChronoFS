#include "vcs/Diff.hpp"
#include <sstream>
#include <algorithm>

// Simple LCS-based line diff producing a minimal unified-like output.
namespace vcs
{

    static std::vector<std::string> splitLines(const std::string &s)
    {
        std::vector<std::string> out;
        std::istringstream iss(s);
        std::string line;
        while (std::getline(iss, line))
            out.push_back(line);
        return out;
    }

    std::vector<DiffHunkLine> diffText(const std::string &a, const std::string &b)
    {
        auto A = splitLines(a);
        auto B = splitLines(b);
        size_t n = A.size(), m = B.size();
        std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
        for (size_t i = 0; i < n; i++)
            for (size_t j = 0; j < m; j++)
                dp[i + 1][j + 1] = (A[i] == B[j]) ? dp[i][j] + 1 : std::max(dp[i + 1][j], dp[i][j + 1]);
        // reconstruct
        std::vector<DiffHunkLine> out;
        size_t i = n, j = m;
        while (i > 0 || j > 0)
        {
            if (i > 0 && j > 0 && A[i - 1] == B[j - 1])
            {
                out.push_back({' ', A[i - 1]});
                i--;
                j--;
            }
            else if (j > 0 && (i == 0 || dp[i][j - 1] >= dp[i - 1][j]))
            {
                out.push_back({'+', B[j - 1]});
                j--;
            }
            else
            {
                out.push_back({'-', A[i - 1]});
                i--;
            }
        }
        std::reverse(out.begin(), out.end());
        return out;
    }

} 
