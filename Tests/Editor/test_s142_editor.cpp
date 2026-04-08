// S142 editor tests: PropertyGridV1, TreeViewV1, TableViewV1
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── PropertyGridV1 ────────────────────────────────────────────────────────────

TEST_CASE("PgPropType names", "[Editor][S142]") {
    REQUIRE(std::string(pgPropTypeName(PgPropType::Bool))   == "Bool");
    REQUIRE(std::string(pgPropTypeName(PgPropType::Int))    == "Int");
    REQUIRE(std::string(pgPropTypeName(PgPropType::Float))  == "Float");
    REQUIRE(std::string(pgPropTypeName(PgPropType::String)) == "String");
    REQUIRE(std::string(pgPropTypeName(PgPropType::Color))  == "Color");
    REQUIRE(std::string(pgPropTypeName(PgPropType::Vector)) == "Vector");
    REQUIRE(std::string(pgPropTypeName(PgPropType::Enum))   == "Enum");
    REQUIRE(std::string(pgPropTypeName(PgPropType::Object)) == "Object");
}

TEST_CASE("PgEditMode names", "[Editor][S142]") {
    REQUIRE(std::string(pgEditModeName(PgEditMode::ReadOnly)) == "ReadOnly");
    REQUIRE(std::string(pgEditModeName(PgEditMode::Inline))   == "Inline");
    REQUIRE(std::string(pgEditModeName(PgEditMode::Popup))    == "Popup");
    REQUIRE(std::string(pgEditModeName(PgEditMode::Custom))   == "Custom");
}

TEST_CASE("PgProperty defaults", "[Editor][S142]") {
    PgProperty p(1, "mass", PgPropType::Float);
    REQUIRE(p.id()       == 1u);
    REQUIRE(p.name()     == "mass");
    REQUIRE(p.type()     == PgPropType::Float);
    REQUIRE(p.editMode() == PgEditMode::Inline);
    REQUIRE(p.value()    == "");
    REQUIRE(p.tooltip()  == "");
    REQUIRE(p.visible()  == true);
    REQUIRE(p.readOnly() == false);
}

TEST_CASE("PgProperty setters", "[Editor][S142]") {
    PgProperty p(2, "tag", PgPropType::String);
    p.setEditMode(PgEditMode::Popup);
    p.setValue("player");
    p.setTooltip("Entity tag");
    p.setVisible(false);
    p.setReadOnly(true);
    REQUIRE(p.editMode() == PgEditMode::Popup);
    REQUIRE(p.value()    == "player");
    REQUIRE(p.tooltip()  == "Entity tag");
    REQUIRE(p.visible()  == false);
    REQUIRE(p.readOnly() == true);
}

TEST_CASE("PropertyGridV1 add and visibleCount", "[Editor][S142]") {
    PropertyGridV1 grid;
    grid.addProperty(PgProperty(1, "p1", PgPropType::Bool));
    grid.addProperty(PgProperty(2, "p2", PgPropType::Int));
    REQUIRE(grid.propertyCount() == 2u);
    REQUIRE(grid.visibleCount()  == 2u);
    grid.findProperty(1)->setVisible(false);
    REQUIRE(grid.visibleCount() == 1u);
}

TEST_CASE("PropertyGridV1 duplicate prevention and remove", "[Editor][S142]") {
    PropertyGridV1 grid;
    REQUIRE(grid.addProperty(PgProperty(5, "x", PgPropType::Float)) == true);
    REQUIRE(grid.addProperty(PgProperty(5, "x", PgPropType::Float)) == false);
    REQUIRE(grid.removeProperty(5) == true);
    REQUIRE(grid.propertyCount() == 0u);
}

TEST_CASE("PropertyGridV1 setReadOnly", "[Editor][S142]") {
    PropertyGridV1 grid;
    grid.addProperty(PgProperty(1, "speed", PgPropType::Float));
    REQUIRE(grid.setReadOnly(1, true) == true);
    REQUIRE(grid.findProperty(1)->readOnly() == true);
    REQUIRE(grid.setReadOnly(99, true) == false);
}

// ── TreeViewV1 ────────────────────────────────────────────────────────────────

TEST_CASE("TvNodeState names", "[Editor][S142]") {
    REQUIRE(std::string(tvNodeStateName(TvNodeState::Collapsed)) == "Collapsed");
    REQUIRE(std::string(tvNodeStateName(TvNodeState::Expanded))  == "Expanded");
    REQUIRE(std::string(tvNodeStateName(TvNodeState::Leaf))      == "Leaf");
}

TEST_CASE("TvSelectMode names", "[Editor][S142]") {
    REQUIRE(std::string(tvSelectModeName(TvSelectMode::Single)) == "Single");
    REQUIRE(std::string(tvSelectModeName(TvSelectMode::Multi))  == "Multi");
    REQUIRE(std::string(tvSelectModeName(TvSelectMode::None))   == "None");
}

TEST_CASE("TvNode defaults", "[Editor][S142]") {
    TvNode n(1, "Root");
    REQUIRE(n.id()       == 1u);
    REQUIRE(n.label()    == "Root");
    REQUIRE(n.state()    == TvNodeState::Leaf);
    REQUIRE(n.parentId() == 0u);
    REQUIRE(n.selected() == false);
    REQUIRE(n.enabled()  == true);
}

TEST_CASE("TreeViewV1 selectMode default", "[Editor][S142]") {
    TreeViewV1 tree;
    REQUIRE(tree.selectMode() == TvSelectMode::Single);
    tree.setSelectMode(TvSelectMode::Multi);
    REQUIRE(tree.selectMode() == TvSelectMode::Multi);
}

TEST_CASE("TreeViewV1 expand and collapse", "[Editor][S142]") {
    TreeViewV1 tree;
    TvNode n(1, "folder");
    n.setState(TvNodeState::Collapsed);
    tree.addNode(n);
    REQUIRE(tree.expand(1)  == true);
    REQUIRE(tree.findNode(1)->state() == TvNodeState::Expanded);
    REQUIRE(tree.collapse(1) == true);
    REQUIRE(tree.findNode(1)->state() == TvNodeState::Collapsed);
}

TEST_CASE("TreeViewV1 expand on Leaf returns false", "[Editor][S142]") {
    TreeViewV1 tree;
    tree.addNode(TvNode(1, "leaf"));
    REQUIRE(tree.expand(1)  == false);
    REQUIRE(tree.collapse(1) == false);
}

TEST_CASE("TreeViewV1 childrenOf", "[Editor][S142]") {
    TreeViewV1 tree;
    tree.addNode(TvNode(1, "root"));
    TvNode c1(2, "child1"); c1.setParentId(1);
    TvNode c2(3, "child2"); c2.setParentId(1);
    TvNode c3(4, "other");  c3.setParentId(0);
    tree.addNode(c1); tree.addNode(c2); tree.addNode(c3);
    auto children = tree.childrenOf(1);
    REQUIRE(children.size() == 2u);
}

TEST_CASE("TreeViewV1 selectedCount", "[Editor][S142]") {
    TreeViewV1 tree;
    tree.addNode(TvNode(1, "a"));
    tree.addNode(TvNode(2, "b"));
    tree.findNode(1)->setSelected(true);
    REQUIRE(tree.selectedCount() == 1u);
}

// ── TableViewV1 ──────────────────────────────────────────────────────────────

TEST_CASE("TblSortDir names", "[Editor][S142]") {
    REQUIRE(std::string(tblSortDirName(TblSortDir::None)) == "None");
    REQUIRE(std::string(tblSortDirName(TblSortDir::Asc))  == "Asc");
    REQUIRE(std::string(tblSortDirName(TblSortDir::Desc)) == "Desc");
}

TEST_CASE("TblColType names", "[Editor][S142]") {
    REQUIRE(std::string(tblColTypeName(TblColType::Text))   == "Text");
    REQUIRE(std::string(tblColTypeName(TblColType::Number)) == "Number");
    REQUIRE(std::string(tblColTypeName(TblColType::Bool))   == "Bool");
    REQUIRE(std::string(tblColTypeName(TblColType::Icon))   == "Icon");
    REQUIRE(std::string(tblColTypeName(TblColType::Custom)) == "Custom");
}

TEST_CASE("TblColumn defaults", "[Editor][S142]") {
    TblColumn c(1, "Name");
    REQUIRE(c.id()      == 1u);
    REQUIRE(c.header()  == "Name");
    REQUIRE(c.type()    == TblColType::Text);
    REQUIRE(c.sortDir() == TblSortDir::None);
    REQUIRE(c.width()   == 100);
    REQUIRE(c.visible() == true);
}

TEST_CASE("TblRow cells", "[Editor][S142]") {
    TblRow r(1);
    r.addCell("Alice");
    r.addCell("30");
    REQUIRE(r.cellCount() == 2u);
    REQUIRE(r.cells()[0]  == "Alice");
    REQUIRE(r.cells()[1]  == "30");
    REQUIRE(r.selected()  == false);
}

TEST_CASE("TableViewV1 columns and rows", "[Editor][S142]") {
    TableViewV1 table;
    table.addColumn(TblColumn(1, "Name"));
    table.addColumn(TblColumn(2, "Age"));
    table.addRow(TblRow(1));
    table.addRow(TblRow(2));
    REQUIRE(table.columnCount() == 2u);
    REQUIRE(table.rowCount()    == 2u);
}

TEST_CASE("TableViewV1 selectedRowCount and sortBy", "[Editor][S142]") {
    TableViewV1 table;
    table.addColumn(TblColumn(1, "Score"));
    TblRow r1(1); r1.setSelected(true);
    TblRow r2(2);
    table.addRow(r1);
    table.addRow(r2);
    REQUIRE(table.selectedRowCount() == 1u);
    REQUIRE(table.sortBy(1, TblSortDir::Asc) == true);
    REQUIRE(table.sortBy(99, TblSortDir::Asc) == false);
}
