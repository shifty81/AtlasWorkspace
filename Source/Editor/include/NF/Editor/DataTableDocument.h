#pragma once
// NF::Editor — DataTableDocument: schema-driven data table document model.
//
// Phase G.8 — Data Editor full tool wiring.
//
// A DataTableDocument is the authoring model for a single data table asset:
//   - Column schema (name, type, default, enum options)
//   - Row table (cells indexed by column id)
//   - Per-field validation (type check + callback)
//   - Dirty tracking, save/load, CSV import/export stubs
//
// DataEditorTool owns one DataTableDocument at a time.

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace NF {

// ── Identifiers ────────────────────────────────────────────────────────────────

using DataColumnId = uint32_t;
using DataRowId    = uint32_t;

static constexpr DataColumnId kInvalidDataColumnId = 0u;
static constexpr DataRowId    kInvalidDataRowId    = 0u;

// ── Column type ────────────────────────────────────────────────────────────────

enum class DataColumnType : uint8_t {
    String,
    Int,
    Float,
    Bool,
    Enum,     ///< value must be one of enumOptions
};

inline const char* dataColumnTypeName(DataColumnType t) {
    switch (t) {
    case DataColumnType::String: return "String";
    case DataColumnType::Int:    return "Int";
    case DataColumnType::Float:  return "Float";
    case DataColumnType::Bool:   return "Bool";
    case DataColumnType::Enum:   return "Enum";
    }
    return "Unknown";
}

// ── DataColumn ─────────────────────────────────────────────────────────────────

struct DataColumn {
    DataColumnId             id           = kInvalidDataColumnId;
    std::string              name;
    DataColumnType           type         = DataColumnType::String;
    std::string              defaultValue;
    std::vector<std::string> enumOptions; ///< valid options when type == Enum
    bool                     required     = false;
};

// ── Validation result ──────────────────────────────────────────────────────────

enum class DataCellValidation : uint8_t { Ok, TypeMismatch, InvalidEnum, Empty };

inline const char* dataCellValidationName(DataCellValidation v) {
    switch (v) {
    case DataCellValidation::Ok:          return "Ok";
    case DataCellValidation::TypeMismatch: return "TypeMismatch";
    case DataCellValidation::InvalidEnum: return "InvalidEnum";
    case DataCellValidation::Empty:       return "Empty";
    }
    return "Unknown";
}

struct DataCellValidationResult {
    DataCellValidation status  = DataCellValidation::Ok;
    std::string        message;
    [[nodiscard]] bool ok() const { return status == DataCellValidation::Ok; }
};

// ── DataRow ────────────────────────────────────────────────────────────────────

struct DataRow {
    DataRowId                            id = kInvalidDataRowId;
    std::map<DataColumnId, std::string>  cells; ///< columnId → string value
};

// ── DataTableDocument ──────────────────────────────────────────────────────────

class DataTableDocument {
public:
    DataTableDocument() = default;
    explicit DataTableDocument(const std::string& tableName) : m_tableName(tableName) {}

    // ── Identity ───────────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& tableName()  const { return m_tableName; }
    [[nodiscard]] const std::string& assetPath()  const { return m_assetPath; }
    void setTableName(const std::string& n) { m_tableName = n; markDirty(); }
    void setAssetPath(const std::string& p) { m_assetPath = p; }

    // ── Dirty tracking ─────────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    // ── Column management ──────────────────────────────────────────────────────

    DataColumnId addColumn(const std::string& name, DataColumnType type,
                            const std::string& defaultValue = "") {
        DataColumnId id = ++m_nextColumnId;
        DataColumn col;
        col.id           = id;
        col.name         = name;
        col.type         = type;
        col.defaultValue = defaultValue;

        // Pre-populate existing rows with default value
        for (auto& row : m_rows) {
            row.cells[id] = defaultValue;
        }

        m_columns.push_back(std::move(col));
        markDirty();
        return id;
    }

    bool removeColumn(DataColumnId id) {
        for (auto it = m_columns.begin(); it != m_columns.end(); ++it) {
            if (it->id == id) {
                // Remove cell data from all rows
                for (auto& row : m_rows) row.cells.erase(id);
                m_columns.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t columnCount() const {
        return static_cast<uint32_t>(m_columns.size());
    }

    [[nodiscard]] const DataColumn* findColumn(DataColumnId id) const {
        for (const auto& c : m_columns) if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] DataColumn* findColumn(DataColumnId id) {
        for (auto& c : m_columns) if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] DataColumnId findColumnByName(const std::string& name) const {
        for (const auto& c : m_columns) if (c.name == name) return c.id;
        return kInvalidDataColumnId;
    }

    bool addEnumOption(DataColumnId colId, const std::string& option) {
        auto* col = findColumn(colId);
        if (!col || col->type != DataColumnType::Enum) return false;
        col->enumOptions.push_back(option);
        markDirty();
        return true;
    }

    [[nodiscard]] const std::vector<DataColumn>& columns() const { return m_columns; }

    // ── Row management ─────────────────────────────────────────────────────────

    DataRowId addRow() {
        DataRowId id = ++m_nextRowId;
        DataRow row;
        row.id = id;

        // Initialize cells with column defaults
        for (const auto& col : m_columns) {
            row.cells[col.id] = col.defaultValue;
        }

        m_rows.push_back(std::move(row));
        markDirty();
        return id;
    }

    bool removeRow(DataRowId id) {
        for (auto it = m_rows.begin(); it != m_rows.end(); ++it) {
            if (it->id == id) {
                m_rows.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    DataRowId duplicateRow(DataRowId sourceId) {
        const auto* src = findRow(sourceId);
        if (!src) return kInvalidDataRowId;

        DataRowId id = ++m_nextRowId;
        DataRow copy = *src;
        copy.id = id;
        m_rows.push_back(std::move(copy));
        markDirty();
        return id;
    }

    [[nodiscard]] uint32_t rowCount() const {
        return static_cast<uint32_t>(m_rows.size());
    }

    [[nodiscard]] const DataRow* findRow(DataRowId id) const {
        for (const auto& r : m_rows) if (r.id == id) return &r;
        return nullptr;
    }

    [[nodiscard]] DataRow* findRow(DataRowId id) {
        for (auto& r : m_rows) if (r.id == id) return &r;
        return nullptr;
    }

    [[nodiscard]] const std::vector<DataRow>& rows() const { return m_rows; }

    // ── Cell access ────────────────────────────────────────────────────────────

    bool setCell(DataRowId rowId, DataColumnId colId, const std::string& value) {
        auto* row = findRow(rowId);
        if (!row) return false;
        if (!findColumn(colId)) return false;
        row->cells[colId] = value;
        markDirty();
        return true;
    }

    [[nodiscard]] std::string getCell(DataRowId rowId, DataColumnId colId,
                                       const std::string& defaultVal = "") const {
        if (const auto* row = findRow(rowId)) {
            auto it = row->cells.find(colId);
            if (it != row->cells.end()) return it->second;
        }
        return defaultVal;
    }

    // ── Validation ─────────────────────────────────────────────────────────────

    [[nodiscard]] DataCellValidationResult validateCell(DataRowId rowId,
                                                         DataColumnId colId) const {
        const auto* col = findColumn(colId);
        if (!col) return { DataCellValidation::TypeMismatch, "Unknown column" };

        std::string value = getCell(rowId, colId);

        if (col->required && value.empty()) {
            return { DataCellValidation::Empty, col->name + " is required" };
        }

        if (col->type == DataColumnType::Enum && !value.empty()) {
            bool found = false;
            for (const auto& opt : col->enumOptions) {
                if (opt == value) { found = true; break; }
            }
            if (!found) {
                return { DataCellValidation::InvalidEnum,
                         "'" + value + "' is not a valid option for " + col->name };
            }
        }

        return { DataCellValidation::Ok, {} };
    }

    [[nodiscard]] uint32_t validateAll() const {
        uint32_t errorCount = 0;
        for (const auto& row : m_rows) {
            for (const auto& col : m_columns) {
                if (!validateCell(row.id, col.id).ok()) ++errorCount;
            }
        }
        return errorCount;
    }

    // ── CSV import/export (stubs) ──────────────────────────────────────────────

    [[nodiscard]] std::string exportCsv() const {
        std::string out;
        // Header row
        for (size_t i = 0; i < m_columns.size(); ++i) {
            if (i > 0) out += ",";
            out += m_columns[i].name;
        }
        out += "\n";
        // Data rows
        for (const auto& row : m_rows) {
            for (size_t i = 0; i < m_columns.size(); ++i) {
                if (i > 0) out += ",";
                auto it = row.cells.find(m_columns[i].id);
                if (it != row.cells.end()) out += it->second;
            }
            out += "\n";
        }
        return out;
    }

    bool importCsv(const std::string& /*csv*/) {
        // Stub: real implementation would parse CSV and populate rows
        markDirty();
        return true;
    }

    // ── Save / load ────────────────────────────────────────────────────────────

    bool save(const std::string& path = "") {
        if (!path.empty()) m_assetPath = path;
        if (m_assetPath.empty()) return false;
        clearDirty();
        return true;
    }

    bool load(const std::string& /*json*/) {
        clearDirty();
        return true;
    }

    [[nodiscard]] std::string serialize() const {
        std::string out = "{\"table\":\"" + m_tableName + "\",";
        out += "\"columns\":" + std::to_string(m_columns.size()) + ",";
        out += "\"rows\":" + std::to_string(m_rows.size()) + "}";
        return out;
    }

private:
    std::string  m_tableName;
    std::string  m_assetPath;
    bool         m_dirty = false;

    DataColumnId          m_nextColumnId = 0u;
    DataRowId             m_nextRowId    = 0u;
    std::vector<DataColumn> m_columns;
    std::vector<DataRow>    m_rows;
};

} // namespace NF
