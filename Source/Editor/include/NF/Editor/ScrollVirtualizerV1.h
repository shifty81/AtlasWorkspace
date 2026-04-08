#pragma once
// NF::Editor — scroll list virtualization
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace NF {

enum class SvScrollDir : uint8_t { Vertical, Horizontal, Both };
inline const char* svScrollDirName(SvScrollDir v) {
    switch (v) {
        case SvScrollDir::Vertical:   return "Vertical";
        case SvScrollDir::Horizontal: return "Horizontal";
        case SvScrollDir::Both:       return "Both";
    }
    return "Unknown";
}

enum class SvOverflowMode : uint8_t { Clip, Scroll, Auto };
inline const char* svOverflowModeName(SvOverflowMode v) {
    switch (v) {
        case SvOverflowMode::Clip:   return "Clip";
        case SvOverflowMode::Scroll: return "Scroll";
        case SvOverflowMode::Auto:   return "Auto";
    }
    return "Unknown";
}

class SvVirtualItem {
public:
    explicit SvVirtualItem(int index, float top, float height)
        : m_index(index), m_top(top), m_height(height) {}

    void setVisible(bool v) { m_visible = v; }

    [[nodiscard]] int   index()   const { return m_index;   }
    [[nodiscard]] float top()     const { return m_top;     }
    [[nodiscard]] float height()  const { return m_height;  }
    [[nodiscard]] bool  visible() const { return m_visible; }

private:
    int   m_index;
    float m_top;
    float m_height;
    bool  m_visible = false;
};

class SvVisibleRange {
public:
    explicit SvVisibleRange(int firstIndex, int lastIndex)
        : m_firstIndex(firstIndex), m_lastIndex(lastIndex) {}

    [[nodiscard]] int firstIndex() const { return m_firstIndex; }
    [[nodiscard]] int lastIndex()  const { return m_lastIndex;  }
    [[nodiscard]] int count()      const { return m_lastIndex - m_firstIndex + 1; }

private:
    int m_firstIndex;
    int m_lastIndex;
};

class ScrollVirtualizerV1 {
public:
    void setDir(SvScrollDir v)          { m_dir            = v; }
    void setOverflow(SvOverflowMode v)  { m_overflow       = v; }
    void setViewportHeight(float v)     { m_viewportHeight = v; }
    void setItemHeight(float v)         { m_itemHeight     = v; }
    void setScrollOffset(float v)       { m_scrollOffset   = v; }

    bool addItem(const SvVirtualItem& item) {
        for (auto& x : m_items) if (x.index() == item.index()) return false;
        m_items.push_back(item); return true;
    }

    [[nodiscard]] SvScrollDir   dir()            const { return m_dir;            }
    [[nodiscard]] SvOverflowMode overflow()      const { return m_overflow;       }
    [[nodiscard]] float         viewportHeight() const { return m_viewportHeight; }
    [[nodiscard]] float         itemHeight()     const { return m_itemHeight;     }
    [[nodiscard]] float         scrollOffset()   const { return m_scrollOffset;   }

    void updateVisibility() {
        for (auto& item : m_items) {
            bool vis = (item.top() + item.height() > m_scrollOffset) &&
                       (item.top() < m_scrollOffset + m_viewportHeight);
            item.setVisible(vis);
        }
    }

    [[nodiscard]] size_t visibleItems() const {
        size_t n = 0;
        for (auto& item : m_items) if (item.visible()) ++n;
        return n;
    }

    [[nodiscard]] SvVisibleRange computeRange() const {
        int first = -1, last = -1;
        for (auto& item : m_items) {
            if (item.visible()) {
                if (first < 0) first = item.index();
                last = item.index();
            }
        }
        if (first < 0) return SvVisibleRange(0, -1);
        return SvVisibleRange(first, last);
    }

private:
    std::vector<SvVirtualItem> m_items;
    SvScrollDir    m_dir            = SvScrollDir::Vertical;
    SvOverflowMode m_overflow       = SvOverflowMode::Auto;
    float          m_viewportHeight = 400.0f;
    float          m_itemHeight     = 24.0f;
    float          m_scrollOffset   = 0.0f;
};

} // namespace NF
