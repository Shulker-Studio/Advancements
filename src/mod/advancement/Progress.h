#pragma once

#include <map>
#include <string>
#include <vector>

namespace my_mod::advancement {

inline constexpr int PlayerProgressDataVersion = 1;

struct AdvancementProgress {
    std::map<std::string, std::string> criteria;
    bool                              done{false};
};

struct PlayerProgress {
    std::map<std::string, AdvancementProgress> advancements;
    int                                        dataVersion{PlayerProgressDataVersion};
};

struct ProgressLoadResult {
    PlayerProgress           progress;
    std::vector<std::string> errors;

    [[nodiscard]] bool ok() const { return errors.empty(); }
};

struct ProgressMutationResult {
    bool                     changed{false};
    bool                     done{false};
    std::vector<std::string> errors;

    [[nodiscard]] bool ok() const { return errors.empty(); }
};

} // namespace my_mod::advancement
