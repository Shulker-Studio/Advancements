#pragma once

namespace my_mod {
class MyMod;
}

namespace my_mod::advancement {

void registerRuntimeTriggerAdapters(MyMod& mod);
void unregisterRuntimeTriggerAdapters();

} // namespace my_mod::advancement
