# Workspace Core Stabilization

**Phase 1 of the Master Roadmap**

## Purpose

Make the workspace host bootstrap clean, deterministic, and generic.

## Tasks

### Entrypoint
- [ ] Verify AtlasWorkspace.exe is the single primary entry
- [ ] Clean stale naming in window titles, app IDs, command IDs
- [ ] Remove stale WPF or MasterRepo assumptions
- [ ] Ensure NovaForge is not hard-wired as workspace core boot logic

### App/Tool Registry
- [ ] Verify child tools register through clean registry
- [ ] Remove stale registrations
- [ ] Document tool registration model

### Workspace/Project Boundary
- [ ] Verify no NovaForge-specific logic in workspace core
- [ ] Define project adapter contract interfaces
- [ ] Wire project adapter manager into workspace shell

### Shell Subsystems
- [ ] WorkspaceShell
- [ ] ToolRegistry
- [ ] PanelRegistry
- [ ] LayoutManager
- [ ] CommandManager
- [ ] SelectionManager
- [ ] NotificationCenter
- [ ] ProjectManager
- [ ] ProjectAdapterManager
- [ ] AtlasAIHost
- [ ] AssetIntakeService
- [ ] LogConsoleService
