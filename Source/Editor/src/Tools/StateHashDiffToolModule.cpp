#include "StateHashDiffToolModule.h"

namespace atlas::editor {

std::string StateHashDiffToolModule::Name() const { return "State Hash Diff"; }
void StateHashDiffToolModule::OnRegister() {}
void StateHashDiffToolModule::OnUnregister() {}
void StateHashDiffToolModule::RegisterPanels() {}
void StateHashDiffToolModule::RegisterMenus() {}
void StateHashDiffToolModule::RegisterModes() {}
bool StateHashDiffToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void StateHashDiffToolModule::Update(float /*deltaTime*/) {}
void StateHashDiffToolModule::Render() {}

} // namespace atlas::editor
