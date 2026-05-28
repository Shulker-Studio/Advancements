#pragma once

#include "mod/trigger/TriggerIndex.h"

#include <deque>
#include <string_view>

namespace advancements {

struct TriggerRegistration {
    std::string_view            id;
    TriggerDescriptor::CompileFn compile;
    TriggerDescriptor::MatchFn   match;
};

class TriggerRegistry {
public:
    void registerTrigger(TriggerRegistration registration);

    [[nodiscard]] TriggerDescriptor const* find(std::string_view triggerId) const;

private:
    std::deque<TriggerDescriptor> mDescriptors;
};

} // namespace advancements
