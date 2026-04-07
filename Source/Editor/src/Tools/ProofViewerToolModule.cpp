#include "ProofViewerToolModule.h"

namespace atlas::editor {

std::string ProofViewerToolModule::Name() const { return "Proof Viewer"; }
void ProofViewerToolModule::OnRegister() {}
void ProofViewerToolModule::OnUnregister() {}
void ProofViewerToolModule::RegisterPanels() {}
void ProofViewerToolModule::RegisterMenus() {}
void ProofViewerToolModule::RegisterModes() {}
bool ProofViewerToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void ProofViewerToolModule::Update(float /*deltaTime*/) {}
void ProofViewerToolModule::Render() {}

} // namespace atlas::editor
