#include "DesyncVisualizerToolModule.h"

namespace atlas::editor {

std::string DesyncVisualizerToolModule::Name() const { return "Desync Visualizer"; }
void DesyncVisualizerToolModule::OnRegister() {}
void DesyncVisualizerToolModule::OnUnregister() {}
void DesyncVisualizerToolModule::RegisterPanels() {}
void DesyncVisualizerToolModule::RegisterMenus() {}
void DesyncVisualizerToolModule::RegisterModes() {}
bool DesyncVisualizerToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void DesyncVisualizerToolModule::Update(float /*deltaTime*/) {}
void DesyncVisualizerToolModule::Render() {}

} // namespace atlas::editor
