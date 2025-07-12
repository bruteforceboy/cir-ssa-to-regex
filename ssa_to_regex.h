#include <regex>
#include <set>
#include <string>
#include <vector>

// Replace SSA assignment lines like "%0 = ..." with regex named group for the
// variable index
auto replaceSsaAssignments(const std::vector<std::string> &prog) {
    std::vector<std::string> output;
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

// Replace SSA uses "%n" with "%[[Vn]]" if defined, otherwise wildcard {{.*}}
// Scope‑safe SSA use??
auto replaceSsaVarUses(const std::vector<std::string> &prog) {
    std::vector<std::string> output;
    std::stack<std::set<std::string>> scopeDefs;
    scopeDefs.push({});  // global scope

    static const std::regex assignDef(R"(^\s*%\[\[V(\d+):)");
    static const std::regex usePattern(
        R"(%\[(\[?V)?(\d+)\]?\])");  // either [[Vn]] or %n
    static const std::regex rawUse(R"(%(\d+))");

    for (auto &line : prog) {
        if (line.find('{') != std::string::npos)
            scopeDefs.push(scopeDefs.top());
        if (line.find('}') != std::string::npos && scopeDefs.size() > 1)
            scopeDefs.pop();

        {
            std::smatch m;
            if (std::regex_search(line, m, assignDef))
                scopeDefs.top().insert(m[1].str());
        }

        std::string result;
        size_t last = 0;
        for (auto it = std::sregex_iterator(line.begin(), line.end(), rawUse);
             it != std::sregex_iterator(); ++it) {
            auto &m = *it;
            result.append(line, last, m.position() - last);
            std::string idx = m[1].str();
            if (scopeDefs.top().count(idx))
                result += "%[[V" + idx + "]]";
            else
                result += "{{.*}}";
            last = m.position() + m.length();
        }
        result.append(line, last, std::string::npos);

        output.push_back(std::move(result));
    }

    return output;
}

// Remove CIR location info
// Truncate at ` loc(` if present
auto removeLocationInfo(const std::vector<std::string> &prog) {
    std::vector<std::string> output;
    for (const auto &line : prog) {
        auto pos = line.find(" loc(");
        output.push_back(pos == std::string::npos ? line : line.substr(0, pos));
    }
    return output;
}

// Trim trailing spaces
auto removeTrailingSpaces(const std::vector<std::string> &prog) {
    std::vector<std::string> output;
    for (const auto &line : prog) {
        size_t end = line.find_last_not_of(' ');
        output.push_back(end == std::string::npos ? std::string()
                                                  : line.substr(0, end + 1));
    }
    return output;
}

void ssaToRegex(std::vector<std::string> &prog) {
    prog = replaceSsaAssignments(prog);
    prog = replaceSsaVarUses(prog);
    prog = removeLocationInfo(prog);  // optional?
    prog = removeTrailingSpaces(prog);
}
