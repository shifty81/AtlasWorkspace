// S159 editor tests: ConstraintEditorV1, RigidBodyEditorV1, ColliderEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ConstraintEditorV1 ────────────────────────────────────────────────────

TEST_CASE("ConstraintEditorV1 basic", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    REQUIRE(ce.constraintCount() == 0);
    REQUIRE(ce.enabledCount() == 0);
}

TEST_CASE("ConstraintEditorV1 add/remove", "[Editor][S159]") {
    ConstraintEditorV1 ce;
    CevConstraint c1(1, "joint1"); c1.setType(CevConstraintType::Hinge); c1.setBodyA(10); c1.setBodyB(11);
    CevConstraint c2(2, "spring1"); c2.setType(CevConstraintType::Spring); c2.setEnabled(false);
    REQUIRE(ce.addConstraint(c1));
    REQUIRE(ce.addConstraint(c2));
    REQUIRE_FALSE(ce.addConstraint(c1));
    REQUIRE(ce.constraintCount() == 2);
    REQUIRE(ce.enabledCount() == 1);
    REQUIRE(ce.findConstraint(1) != nullptr);
    REQUIRE(ce.removeConstraint(1));
    REQUIRE(ce.constraintCount() == 1);
}

TEST_CASE("CevConstraintType names", "[Editor][S159]") {
    REQUIRE(std::string(cevConstraintTypeName(CevConstraintType::Fixed))  == "Fixed");
    REQUIRE(std::string(cevConstraintTypeName(CevConstraintType::Hinge))  == "Hinge");
    REQUIRE(std::string(cevConstraintTypeName(CevConstraintType::Slider)) == "Slider");
    REQUIRE(std::string(cevConstraintTypeName(CevConstraintType::Ball))   == "Ball");
    REQUIRE(std::string(cevConstraintTypeName(CevConstraintType::Cone))   == "Cone");
    REQUIRE(std::string(cevConstraintTypeName(CevConstraintType::Spring)) == "Spring");
}

// ── RigidBodyEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("RigidBodyEditorV1 basic", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    REQUIRE(rbe.bodyCount() == 0);
    REQUIRE(rbe.dynamicCount() == 0);
}

TEST_CASE("RigidBodyEditorV1 bodies", "[Editor][S159]") {
    RigidBodyEditorV1 rbe;
    RbvBody b1(1, "dynamic"); b1.setBodyType(RbvBodyType::Dynamic); b1.setMass(2.0f);
    RbvBody b2(2, "static"); b2.setBodyType(RbvBodyType::Static);
    REQUIRE(rbe.addBody(b1));
    REQUIRE(rbe.addBody(b2));
    REQUIRE_FALSE(rbe.addBody(b1));
    REQUIRE(rbe.bodyCount() == 2);
    REQUIRE(rbe.dynamicCount() == 1);
    REQUIRE(rbe.findBody(1)->mass() == Catch::Approx(2.0f));
    REQUIRE(rbe.removeBody(2));
    REQUIRE(rbe.bodyCount() == 1);
}

TEST_CASE("RbvBodyType names", "[Editor][S159]") {
    REQUIRE(std::string(rbvBodyTypeName(RbvBodyType::Dynamic))   == "Dynamic");
    REQUIRE(std::string(rbvBodyTypeName(RbvBodyType::Kinematic)) == "Kinematic");
    REQUIRE(std::string(rbvBodyTypeName(RbvBodyType::Static))    == "Static");
}

// ── ColliderEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("ColliderEditorV1 basic", "[Editor][S159]") {
    ColliderEditorV1 ce;
    REQUIRE(ce.colliderCount() == 0);
    REQUIRE(ce.triggerCount() == 0);
}

TEST_CASE("ColliderEditorV1 colliders", "[Editor][S159]") {
    ColliderEditorV1 ce;
    CldCollider c1(1, "box"); c1.setShape(CldShape::Box);
    CldCollider c2(2, "trigger"); c2.setIsTrigger(true); c2.setShape(CldShape::Sphere);
    REQUIRE(ce.addCollider(c1));
    REQUIRE(ce.addCollider(c2));
    REQUIRE_FALSE(ce.addCollider(c1));
    REQUIRE(ce.colliderCount() == 2);
    REQUIRE(ce.triggerCount() == 1);
    REQUIRE(ce.removeCollider(2));
    REQUIRE(ce.colliderCount() == 1);
}

TEST_CASE("CldShape names", "[Editor][S159]") {
    REQUIRE(std::string(cldShapeName(CldShape::Box))     == "Box");
    REQUIRE(std::string(cldShapeName(CldShape::Sphere))  == "Sphere");
    REQUIRE(std::string(cldShapeName(CldShape::Capsule)) == "Capsule");
    REQUIRE(std::string(cldShapeName(CldShape::Mesh))    == "Mesh");
    REQUIRE(std::string(cldShapeName(CldShape::Convex))  == "Convex");
    REQUIRE(std::string(cldShapeName(CldShape::Plane))   == "Plane");
}
