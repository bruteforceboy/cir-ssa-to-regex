#include <deque>
#include <regex>
#include <set>
#include <string>

// Replace SSA assignment lines like "%0 = ..." with regex named group for the
// variable index
auto replaceSsaAssignments(const std::deque<std::string> &prog) {
    std::deque<std::string> output;
    static const std::regex assignPattern(R"(^\s*%(\d+)\s*=)");
    for (const auto &line : prog) {
        std::smatch match;
        if (std::regex_search(line, match, assignPattern)) {
            std::string fullMatch = match.str(0);
            size_t posPercent = fullMatch.find('%');
            std::string indent = (posPercent != std::string::npos)
                                     ? fullMatch.substr(0, posPercent)
                                     : "";
            std::string varIndex = match[1].str();
            std::string replacement = indent + "%\[[V" + varIndex + ":.*]] =";
            static const std::regex varReplacePattern(R"(^\s*%\d+\s*=)");
            std::string replacedLine =
                std::regex_replace(line, varReplacePattern, replacement);
            output.push_back(replacedLine);
        } else {
            output.push_back(line);
        }
    }
    return output;
}

// Collect all defined SSA variables from replaced definitions [[Vn:...]]
auto collectDefinedVars(const std::deque<std::string> &prog) {
    std::set<std::string> defs;
    static const std::regex repDef(R"(%\[\[V(\d+):)");
    for (const auto &line : prog) {
        std::smatch m;
        if (std::regex_search(line, m, repDef))
            defs.insert(m[1].str());
    }
    return defs;
}

// Replace SSA uses "%n" with "%[[Vn]]" if defined, otherwise wildcard {{.*}}
auto replaceSsaVarUses(const std::deque<std::string> &prog) {
    auto defs = collectDefinedVars(prog);
    std::deque<std::string> output;
    static const std::regex usePattern(R"(%(\d+))");
    for (const auto &line : prog) {
        std::string result;
        size_t last = 0;
        for (auto it =
                 std::sregex_iterator(line.begin(), line.end(), usePattern);
             it != std::sregex_iterator(); ++it) {
            auto &m = *it;
            result.append(line, last, m.position() - last);
            std::string idx = m[1].str();
            if (defs.count(idx))
                result.append("%[[V" + idx + "]]");
            else
                result.append("{{.*}}");
            last = m.position() + m.length();
        }
        result.append(line, last, std::string::npos);
        output.push_back(result);
    }
    return output;
}

auto replaceBlocks(const std::deque<std::string> &prog) { return prog; }

// Remove CIR location info
// Truncate at ' loc(' if present
auto removeLocationInfo(const std::deque<std::string> &prog) {
    std::deque<std::string> output;
    for (const auto &line : prog) {
        auto pos = line.find(" loc(");
        output.push_back(pos == std::string::npos ? line : line.substr(0, pos));
    }
    return output;
}

// Trim trailing spaces
auto removeTrailingSpaces(const std::deque<std::string> &prog) {
    std::deque<std::string> output;
    for (const auto &line : prog) {
        size_t end = line.find_last_not_of(' ');
        output.push_back(end == std::string::npos ? std::string()
                                                  : line.substr(0, end + 1));
    }
    return output;
}

void ssaToRegex(std::deque<std::string> &prog) {
    prog = replaceSsaAssignments(prog);
    prog = replaceSsaVarUses(prog);
    prog = replaceBlocks(prog);
    prog = removeLocationInfo(prog);  // optional?
    prog = removeTrailingSpaces(prog);
}
