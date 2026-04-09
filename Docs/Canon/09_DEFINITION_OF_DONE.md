# Definition of Done

## A system is DONE only if

- It compiles successfully
- It runs without crashes
- It integrates with surrounding systems
- It is documented (at minimum in inventory docs)
- It contains no stub logic or placeholder behavior
- Tests exist and pass (where applicable)

## Otherwise, use these labels

| Label | Meaning |
|-------|---------|
| **Partial** | Some functionality exists but system is incomplete |
| **Stub** | File/class exists but contains no real implementation |
| **Planned** | Described in design docs but no code exists |
| **In Progress** | Actively being worked on |
| **Archived** | Retired from active development |

## Minimum proof for "Done"

- [ ] Builds
- [ ] Launches (if applicable)
- [ ] Renders (if UI component)
- [ ] Loads data (if data-driven)
- [ ] Saves data (if persistence involved)
- [ ] Test coverage exists (if testable)
- [ ] Docs updated

## Key Principle

> **Presence is not completion.**
> A file, folder, class, editor, or stub existing in the repo does not mean the system is implemented.
