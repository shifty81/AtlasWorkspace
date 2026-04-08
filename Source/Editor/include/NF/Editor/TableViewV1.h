#pragma once
// NF::Editor — table view widget
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

enum class TblSortDir : uint8_t { None, Asc, Desc };
inline const char* tblSortDirName(TblSortDir v) {
    switch (v) {
        case TblSortDir::None: return "None";
        case TblSortDir::Asc:  return "Asc";
        case TblSortDir::Desc: return "Desc";
    }
    return "Unknown";
}

enum class TblColType : uint8_t { Text, Number, Bool, Icon, Custom };
inline const char* tblColTypeName(TblColType v) {
    switch (v) {
        case TblColType::Text:   return "Text";
        case TblColType::Number: return "Number";
        case TblColType::Bool:   return "Bool";
        case TblColType::Icon:   return "Icon";
        case TblColType::Custom: return "Custom";
    }
    return "Unknown";
}

class TblColumn {
public:
    explicit TblColumn(uint32_t id, const std::string& header)
        : m_id(id), m_header(header) {}

    void setType(TblColType v)    { m_type    = v; }
    void setSortDir(TblSortDir v) { m_sortDir = v; }
    void setWidth(int v)          { m_width   = v; }
    void setVisible(bool v)       { m_visible = v; }

    [[nodiscard]] uint32_t           id()      const { return m_id;      }
    [[nodiscard]] const std::string& header()  const { return m_header;  }
    [[nodiscard]] TblColType         type()    const { return m_type;    }
    [[nodiscard]] TblSortDir         sortDir() const { return m_sortDir; }
    [[nodiscard]] int                width()   const { return m_width;   }
    [[nodiscard]] bool               visible() const { return m_visible; }

private:
    uint32_t    m_id;
    std::string m_header;
    TblColType  m_type    = TblColType::Text;
    TblSortDir  m_sortDir = TblSortDir::None;
    int         m_width   = 100;
    bool        m_visible = true;
};

class TblRow {
public:
    explicit TblRow(uint32_t id) : m_id(id) {}

    void setSelected(bool v)             { m_selected = v; }
    void setEnabled(bool v)              { m_enabled  = v; }
    void addCell(const std::string& v)   { m_cells.push_back(v); }

    [[nodiscard]] uint32_t                        id()        const { return m_id;           }
    [[nodiscard]] const std::vector<std::string>& cells()     const { return m_cells;        }
    [[nodiscard]] bool                            selected()  const { return m_selected;     }
    [[nodiscard]] bool                            enabled()   const { return m_enabled;      }
    [[nodiscard]] size_t                          cellCount() const { return m_cells.size(); }

private:
    uint32_t                 m_id;
    std::vector<std::string> m_cells;
    bool                     m_selected = false;
    bool                     m_enabled  = true;
};

class TableViewV1 {
public:
    bool addColumn(const TblColumn& c) {
        for (auto& x : m_columns) if (x.id() == c.id()) return false;
        m_columns.push_back(c); return true;
    }
    bool removeColumn(uint32_t id) {
        auto it = std::find_if(m_columns.begin(), m_columns.end(),
            [&](const TblColumn& c){ return c.id() == id; });
        if (it == m_columns.end()) return false;
        m_columns.erase(it); return true;
    }
    bool addRow(const TblRow& r) {
        for (auto& x : m_rows) if (x.id() == r.id()) return false;
        m_rows.push_back(r); return true;
    }
    bool removeRow(uint32_t id) {
        auto it = std::find_if(m_rows.begin(), m_rows.end(),
            [&](const TblRow& r){ return r.id() == id; });
        if (it == m_rows.end()) return false;
        m_rows.erase(it); return true;
    }
    [[nodiscard]] size_t columnCount() const { return m_columns.size(); }
    [[nodiscard]] size_t rowCount()    const { return m_rows.size();    }
    [[nodiscard]] size_t selectedRowCount() const {
        size_t n = 0;
        for (auto& r : m_rows) if (r.selected()) ++n;
        return n;
    }
    bool sortBy(uint32_t colId, TblSortDir dir) {
        for (auto& c : m_columns) {
            if (c.id() == colId) { c.setSortDir(dir); return true; }
        }
        return false;
    }

private:
    std::vector<TblColumn> m_columns;
    std::vector<TblRow>    m_rows;
};

} // namespace NF
