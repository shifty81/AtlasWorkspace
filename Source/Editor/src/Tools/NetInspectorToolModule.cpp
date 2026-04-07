#include "NetInspectorToolModule.h"

namespace atlas::editor {

std::string NetInspectorToolModule::Name() const { return "Net Inspector"; }
void NetInspectorToolModule::OnRegister() {}
void NetInspectorToolModule::OnUnregister() {}
void NetInspectorToolModule::RegisterPanels() {}
void NetInspectorToolModule::RegisterMenus() {}
void NetInspectorToolModule::RegisterModes() {}
bool NetInspectorToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void NetInspectorToolModule::Update(float /*deltaTime*/) {}
void NetInspectorToolModule::Render() {}

} // namespace atlas::editor
