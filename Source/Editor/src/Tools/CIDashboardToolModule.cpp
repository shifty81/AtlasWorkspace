#include "CIDashboardToolModule.h"

namespace atlas::editor {

std::string CIDashboardToolModule::Name() const { return "CI Dashboard"; }
void CIDashboardToolModule::OnRegister() {}
void CIDashboardToolModule::OnUnregister() {}
void CIDashboardToolModule::RegisterPanels() {}
void CIDashboardToolModule::RegisterMenus() {}
void CIDashboardToolModule::RegisterModes() {}
bool CIDashboardToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void CIDashboardToolModule::Update(float /*deltaTime*/) {}
void CIDashboardToolModule::Render() {}

} // namespace atlas::editor
