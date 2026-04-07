#include "DataBrowserToolModule.h"

namespace atlas::editor {

std::string DataBrowserToolModule::Name() const { return "Data Browser"; }
void DataBrowserToolModule::OnRegister() {}
void DataBrowserToolModule::OnUnregister() {}
void DataBrowserToolModule::RegisterPanels() {}
void DataBrowserToolModule::RegisterMenus() {}
void DataBrowserToolModule::RegisterModes() {}
bool DataBrowserToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void DataBrowserToolModule::Update(float /*deltaTime*/) {}
void DataBrowserToolModule::Render() {}

} // namespace atlas::editor
