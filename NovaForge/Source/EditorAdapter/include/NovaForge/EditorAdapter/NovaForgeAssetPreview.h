#pragma once
// NovaForge::NovaForgeAssetPreview — per-asset preview scene for AssetEditorTool.
//
// Implements NF::IViewportSceneProvider so AssetEditorTool can render a preview
// of the selected asset in the viewport without a full game-boot sequence.
//
// The asset is represented as a single PreviewEntity whose mesh, material, and
// attachment metadata are editable. Changes are tracked as dirty and can be
// applied (committed) or reverted.
//
// Phase D.3 — Asset Preview Viewport

#include "NF/Workspace/IViewportSceneProvider.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace NovaForge {

// ── AssetPreviewDescriptor ────────────────────────────────────────────────────

struct AssetPreviewDescriptor {
    std::string      assetPath;
    std::string      meshTag;
    std::string      materialTag;
    std::string      attachmentTag;
    PreviewTransform transform;
};

// ── NovaForgeAssetPreview ─────────────────────────────────────────────────────

class NovaForgeAssetPreview : public NF::IViewportSceneProvider {
public:
    NovaForgeAssetPreview()  = default;
    ~NovaForgeAssetPreview() override = default;

    // ── IViewportSceneProvider ────────────────────────────────────────────

    NF::ViewportSceneState provideScene(NF::ViewportHandle       /*handle*/,
                                        const NF::ViewportSlot&  /*slot*/) override {
        NF::ViewportSceneState st;
        st.hasContent  = !m_descriptor.assetPath.empty();
        st.entityCount = m_world.entityCount();
        st.clearColor  = 0x1A1A1AFFu;
        return st;
    }

    // ── Asset binding ─────────────────────────────────────────────────────

    void bindAsset(const AssetPreviewDescriptor& descriptor) {
        m_descriptor  = descriptor;
        m_lastApplied = descriptor;
        m_dirty       = false;
        rebuildPreview();
    }

    void clearAsset() {
        m_descriptor  = {};
        m_lastApplied = {};
        m_dirty       = false;
        m_world.clearEntities();
        m_previewEntity = kInvalidEntityId;
    }

    [[nodiscard]] bool hasAsset() const { return !m_descriptor.assetPath.empty(); }

    [[nodiscard]] const AssetPreviewDescriptor& descriptor() const { return m_descriptor; }

    // ── Editable fields ───────────────────────────────────────────────────

    bool setTransform(const PreviewTransform& t) {
        if (!hasAsset()) return false;
        m_descriptor.transform = t;
        m_world.setTransform(m_previewEntity, t);
        m_dirty = true;
        return true;
    }

    bool setMeshTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.meshTag = tag;
        m_world.setMesh(m_previewEntity, tag);
        m_dirty = true;
        return true;
    }

    bool setMaterialTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.materialTag = tag;
        m_world.setMaterial(m_previewEntity, tag);
        m_dirty = true;
        return true;
    }

    bool setAttachmentTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.attachmentTag = tag;
        m_dirty = true;
        return true;
    }

    // ── Dirty tracking ────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty()                  { m_dirty = false; }

    // ── Save/revert ───────────────────────────────────────────────────────

    /// Commit current changes as the new baseline.
    bool apply() {
        if (!hasAsset()) return false;
        m_lastApplied = m_descriptor;
        m_dirty       = false;
        return true;
    }

    /// Revert to the last applied state.
    bool revert() {
        if (!hasAsset()) return false;
        m_descriptor = m_lastApplied;
        m_dirty      = false;
        rebuildPreview();
        return true;
    }

    // ── Inspector data ────────────────────────────────────────────────────

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> properties() const {
        auto fmtF = [](float v) -> std::string {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.3f", static_cast<double>(v));
            return buf;
        };

        return {
            {"assetPath",     m_descriptor.assetPath},
            {"meshTag",       m_descriptor.meshTag},
            {"materialTag",   m_descriptor.materialTag},
            {"attachmentTag", m_descriptor.attachmentTag},
            {"position.x",   fmtF(m_descriptor.transform.position.x)},
            {"position.y",   fmtF(m_descriptor.transform.position.y)},
            {"position.z",   fmtF(m_descriptor.transform.position.z)},
            {"rotation.x",   fmtF(m_descriptor.transform.rotation.x)},
            {"rotation.y",   fmtF(m_descriptor.transform.rotation.y)},
            {"rotation.z",   fmtF(m_descriptor.transform.rotation.z)},
            {"scale.x",      fmtF(m_descriptor.transform.scale.x)},
            {"scale.y",      fmtF(m_descriptor.transform.scale.y)},
            {"scale.z",      fmtF(m_descriptor.transform.scale.z)},
        };
    }

    // ── Preview world access ──────────────────────────────────────────────

    [[nodiscard]] const NovaForgePreviewWorld& previewWorld() const { return m_world; }

private:
    AssetPreviewDescriptor m_descriptor;
    AssetPreviewDescriptor m_lastApplied;
    NovaForgePreviewWorld  m_world;
    EntityId               m_previewEntity = kInvalidEntityId;
    bool                   m_dirty         = false;

    void rebuildPreview() {
        m_world.clearEntities();
        if (!m_descriptor.assetPath.empty()) {
            m_previewEntity = m_world.createEntity("PreviewAsset");
            m_world.setTransform(m_previewEntity, m_descriptor.transform);
            m_world.setMesh(m_previewEntity, m_descriptor.meshTag);
            m_world.setMaterial(m_previewEntity, m_descriptor.materialTag);
            m_world.clearDirty();
        }
    }
};

} // namespace NovaForge
