#pragma once

#include "mod/advancement/AdvancementLoader.h"

#include <nlohmann/json.hpp>

#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace my_mod::advancement {

struct CriterionBinding {
    std::string                   advancementId;
    std::string                   criterionName;
    std::string                   triggerId;
    std::optional<nlohmann::json> conditions;
};

class TriggerIndex {
public:
    void rebuild(LoadResult const& result);

    [[nodiscard]] size_t triggerCount() const;
    [[nodiscard]] size_t bindingCount() const;
    [[nodiscard]] std::span<CriterionBinding const> find(std::string_view triggerId) const;

private:
    std::map<std::string, std::vector<CriterionBinding>, std::less<>> mBindings;
    size_t                                                           mBindingCount{0};
};

} // namespace my_mod::advancement
