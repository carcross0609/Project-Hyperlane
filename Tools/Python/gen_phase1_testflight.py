"""Authors the Phase 1 flyable setup: ship Blueprint, game mode, test level.

Run headless from the repo root:

  "$UE/Engine/Binaries/Mac/UnrealEditor-Cmd" "$PWD/Hyperlane.uproject" \\
      -run=pythonscript -script="$PWD/Tools/Python/gen_phase1_testflight.py" \\
      -unattended -nopause -nosplash

Depends on gen_phase1_content.py having run first (ship classes + input).

Companion to that script: this one binds *content* to the C++ classes, which
is exactly the split TechnicalArchitecture §9 asks for — C++ owns logic, a
Blueprint subclass supplies the hull mesh and the data assets, and no art
path is ever hardcoded in C++.

Placeholder art is engine primitives on purpose. Phase 1's exit criterion is
that flying *feels* good; real hulls would only make bad handling look
prettier. The debris field exists for the same reason: velocity is invisible
without parallax, so an empty void makes any flight model feel identical.

Set HYPERLANE_GEN_REPORT to capture a machine-readable run report.
"""

import math
import os

import unreal

OUT = os.environ.get("HYPERLANE_GEN_REPORT")

report = []
tools = unreal.AssetToolsHelpers.get_asset_tools()

CUBE = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
CYLINDER = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cylinder")
BASIC_MAT = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/BasicShapeMaterial")


def log(msg):
    report.append(msg)
    unreal.log(f"HYPERLANE_GEN: {msg}")


def load_or_create(name, path, cls, factory):
    """Load-or-create; never deletes. See gen_phase1_content.py for why."""
    full = f"{path}/{name}"
    # does_asset_exist first: load_asset on a missing path logs an Error that
    # looks like a failure but is just the create path.
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        return unreal.EditorAssetLibrary.load_asset(full)
    asset = tools.create_asset(name, path, cls, factory)
    if asset is None:
        log(f"FAIL  {full}")
    return asset


def blueprint_of(name, path, parent):
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    return load_or_create(name, path, None, factory)


# ------------------------------------------------------------------ ship BP
BP_PATH = "/Game/Ships/Blueprints"

ghtroc_def = unreal.EditorAssetLibrary.load_asset("/Game/Ships/Classes/SHIP_Ghtroc720")
input_cfg = unreal.EditorAssetLibrary.load_asset("/Game/Input/DA_ShipInput")

ship_bp = blueprint_of("BP_Ghtroc720", BP_PATH, unreal.ShipPawn)
if ship_bp:
    cdo = unreal.get_default_object(ship_bp.generated_class())
    cdo.set_editor_property("ship_class", ghtroc_def)
    cdo.set_editor_property("input_config", input_cfg)

    # Placeholder hull: a boxy freighter silhouette. Wide and flat reads as
    # "cargo hauler" at a glance, which is all Phase 1 needs it to say.
    hull = cdo.get_editor_property("hull")
    hull.set_editor_property("static_mesh", CUBE)
    hull.set_editor_property("relative_scale3d", unreal.Vector(4.0, 2.5, 1.0))

    unreal.EditorAssetLibrary.save_loaded_asset(ship_bp)
    log(f"OK    {BP_PATH}/BP_Ghtroc720")

# -------------------------------------------------------------- game mode BP
GM_PATH = "/Game/Framework"

gm_bp = blueprint_of("BP_HyperlaneGameMode", GM_PATH, unreal.HyperlaneGameMode)
if gm_bp and ship_bp:
    gm_cdo = unreal.get_default_object(gm_bp.generated_class())
    # The C++ game mode defaults to the bare AShipPawn, which has no art and
    # no bindings. Pointing at the Blueprint is a content decision, so it
    # belongs here rather than in the constructor.
    gm_cdo.set_editor_property("default_pawn_class", ship_bp.generated_class())
    unreal.EditorAssetLibrary.save_loaded_asset(gm_bp)
    log(f"OK    {GM_PATH}/BP_HyperlaneGameMode")

# ------------------------------------------------------------------- level
MAP_PATH = "/Game/Maps/L_TestFlight"

level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

level_sub.new_level(MAP_PATH)


def spawn(cls, loc, rot=None):
    return actor_sub.spawn_actor_from_class(
        cls, unreal.Vector(*loc), unreal.Rotator(*(rot or (0.0, 0.0, 0.0))))


def spawn_mesh(mesh, loc, scale, label):
    a = spawn(unreal.StaticMeshActor, loc)
    comp = a.get_editor_property("static_mesh_component")
    comp.set_editor_property("static_mesh", mesh)
    comp.set_editor_property("relative_scale3d", unreal.Vector(*scale))
    if BASIC_MAT:
        comp.set_material(0, BASIC_MAT)
    a.set_actor_label(label)
    return a


# Key light + ambient. Without a skylight the unlit faces of every cube read
# as pure black and the debris field stops working as motion reference.
sun = spawn(unreal.DirectionalLight, (0.0, 0.0, 20000.0), (-40.0, 30.0, 0.0))
sun.set_actor_label("Sun")

sky = spawn(unreal.SkyLight, (0.0, 0.0, 15000.0))
sky.set_actor_label("Ambient")

spawn(unreal.PlayerStart, (0.0, 0.0, 0.0)).set_actor_label("PlayerStart")

# Debris field: deterministic pseudo-random placement so the level is stable
# across regenerations and two runs are visually comparable when tuning.
DEBRIS_COUNT = 260
SPREAD = 120000.0

for i in range(DEBRIS_COUNT):
    # Cheap deterministic hash -> three decorrelated unit values.
    fx = math.fmod(math.sin(i * 12.9898) * 43758.5453, 1.0)
    fy = math.fmod(math.sin(i * 78.2331) * 43758.5453, 1.0)
    fz = math.fmod(math.sin(i * 39.4251) * 43758.5453, 1.0)
    fs = math.fmod(math.sin(i * 93.7193) * 43758.5453, 1.0)

    loc = (fx * SPREAD, fy * SPREAD, fz * SPREAD * 0.35)

    # Skip anything that would spawn on top of the player's start position.
    if abs(loc[0]) < 6000.0 and abs(loc[1]) < 6000.0 and abs(loc[2]) < 6000.0:
        continue

    s = 3.0 + abs(fs) * 22.0
    spawn_mesh(CUBE, loc, (s, s * 0.8, s * 0.6), f"Debris_{i:03d}")

# A station-scale object, so there is one thing in the level big enough to
# give a sense of absolute scale rather than only relative motion.
spawn_mesh(CYLINDER, (45000.0, 12000.0, 0.0), (60.0, 60.0, 22.0), "StationPlaceholder")

level_sub.save_current_level()
log(f"OK    {MAP_PATH} ({DEBRIS_COUNT} debris + station)")

if OUT:
    with open(OUT, "w") as fh:
        fh.write("\n".join(report) + "\n")
