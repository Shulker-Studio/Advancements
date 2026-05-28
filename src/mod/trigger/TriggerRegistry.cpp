#include "mod/trigger/TriggerRegistry.h"

#include <algorithm>

namespace advancements {

void TriggerRegistry::registerTrigger(TriggerRegistration registration) {
    mDescriptors.push_back(TriggerDescriptor{registration.id, registration.compile, registration.match});
}

TriggerDescriptor const* TriggerRegistry::find(std::string_view triggerId) const {
    auto const found = std::ranges::find_if(mDescriptors, [triggerId](TriggerDescriptor const& descriptor) {
        return descriptor.id == triggerId;
    });
    if (found == mDescriptors.end()) {
        return nullptr;
    }
    return &*found;
}

} // namespace advancements
