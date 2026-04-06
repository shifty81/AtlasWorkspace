#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// Message level for console entries.
enum class MessageLevel : uint8_t { Info, Warning, Error };

/// AtlasUI ConsolePanel — displays log messages with level filtering.
/// Replaces the legacy NF::Editor::ConsolePanel for the AtlasUI framework.
class ConsolePanel final : public PanelBase {
public:
    ConsolePanel()
        : PanelBase("atlas.console", "Console") {}

    void paint(IPaintContext& context) override;

    struct Message {
        std::string text;
        MessageLevel level = MessageLevel::Info;
        float timestamp = 0.f;
    };

    void addMessage(const std::string& text, MessageLevel level, float timestamp = 0.f) {
        m_messages.push_back({text, level, timestamp});
        if (m_messages.size() > kMaxMessages) {
            m_messages.erase(m_messages.begin());
        }
    }

    void clearMessages() { m_messages.clear(); }
    [[nodiscard]] size_t messageCount() const { return m_messages.size(); }
    [[nodiscard]] const std::vector<Message>& messages() const { return m_messages; }

    static constexpr size_t kMaxMessages = 1000;

private:
    std::vector<Message> m_messages;
};

} // namespace NF::UI::AtlasUI
