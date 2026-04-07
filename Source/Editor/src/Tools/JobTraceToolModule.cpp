#include "JobTraceToolModule.h"

namespace atlas::editor {

std::string JobTraceToolModule::Name() const { return "Job Trace"; }
void JobTraceToolModule::OnRegister() {}
void JobTraceToolModule::OnUnregister() {}
void JobTraceToolModule::RegisterPanels() {}
void JobTraceToolModule::RegisterMenus() {}
void JobTraceToolModule::RegisterModes() {}
bool JobTraceToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void JobTraceToolModule::Update(float /*deltaTime*/) {}
void JobTraceToolModule::Render() {}

} // namespace atlas::editor
