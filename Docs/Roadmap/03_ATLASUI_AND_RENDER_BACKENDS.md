# AtlasUI and Render Backends

**Phase 2 of the Master Roadmap**

## Purpose

Correct the backend strategy so GDI is fallback-only and D3D11/DirectWrite become the primary target path.

## Tasks

### Backend Selector
- [ ] Create `UIBackendSelector.h` / `UIBackendSelector.cpp`
- [ ] Define `UIBackendType` enum (D3D11, GDI, OpenGL)
- [ ] Centralize backend selection (remove hardcoding from main.cpp)

### D3D11 Backend
- [ ] Create `D3D11Backend.h` / `D3D11Backend.cpp`
- [ ] Implement UIBackend interface
- [ ] Handle device creation, swap chain, render target
- [ ] Integrate with AtlasUI draw list

### DirectWrite Text Backend
- [ ] Create `DirectWriteTextBackend.h` / `DirectWriteTextBackend.cpp`
- [ ] Font loading and caching
- [ ] Text measurement
- [ ] Glyph rendering

### GDI Isolation
- [ ] Mark GDI as fallback-only in headers and docs
- [ ] Stop implying GDI is "primary" or "default"
- [ ] Keep GDI functional for bootstrap/recovery scenarios

### OpenGL Isolation
- [ ] Mark OpenGL as compatibility-only
- [ ] Isolate from default build path
- [ ] Document as non-canonical
