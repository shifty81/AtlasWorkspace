# Hosted Project Support

**Phase 5 of the Master Roadmap**

## Purpose

Formalize how projects like NovaForge are hosted, loaded, and extended through the workspace.

## Tasks

### Project Adapter Contract
- [ ] Define IGameProjectAdapter interface
- [ ] Define IProjectPanelFactory interface
- [ ] Define IGameplaySystemPanelProvider interface
- [ ] Define IGameplaySystemSchemaProvider interface
- [ ] Define GameplaySystemPanelDescriptor struct

### NovaForge Adapter
- [ ] Create NovaForge/Source/EditorAdapter/ directory
- [ ] Implement NovaForgeAdapter
- [ ] Implement NovaForgeGameplayPanelRegistry
- [ ] Convert first six gameplay editors to adapter panels

### Build Gating
- [ ] NovaForge not built by default from workspace root
- [ ] NF_STANDALONE for independent builds
- [ ] NF_HOSTED for workspace-integrated builds

### Future Project Support
- [ ] Project template system
- [ ] Plugin/project loading contracts
- [ ] Multi-project workspace support
