#include "RuleGraphEditorToolModule.h"

namespace atlas::editor {

std::string RuleGraphEditorToolModule::Name() const { return "Rule Graph Editor"; }
void RuleGraphEditorToolModule::OnRegister() {}
void RuleGraphEditorToolModule::OnUnregister() {}
void RuleGraphEditorToolModule::RegisterPanels() {}
void RuleGraphEditorToolModule::RegisterMenus() {}
void RuleGraphEditorToolModule::RegisterModes() {}
bool RuleGraphEditorToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void RuleGraphEditorToolModule::Update(float /*deltaTime*/) {}
void RuleGraphEditorToolModule::Render() {}

} // namespace atlas::editor
