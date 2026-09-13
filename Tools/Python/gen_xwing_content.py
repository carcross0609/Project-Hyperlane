"""Authors the X-wing flight-test hull: mesh, paint, ship class, Blueprint.

Run headless from the repo root, after gen_phase1_content.py:

  "$UE/Engine/Binaries/Mac/UnrealEditor-Cmd" "$PWD/Hyperlane.uproject" \\
      -run=pythonscript -script="$PWD/Tools/Python/gen_xwing_content.py" \\
      -unattended -nopause -nosplash

Then re-run gen_phase1_testflight.py, which owns which hull the harness flies.

Off-roster on purpose: the T-65B postdates the Republic-era setting, so it is
a flight-test hull, not a Bible 05 tick-zero class (see gen_xwing_mesh.py).

What this demonstrates is DR-13 doing its job. The X-wing gets fighter
handling by pointing at SC_Strike. No flight code knows the X-wing exists, and
it flies nothing like the Ghtroc despite running every line of the same
movement component.

Idempotent: every asset is load-or-create and fully reassigned, and the mesh
is reimported over itself.

Set HYPERLANE_GEN_REPORT to capture a machine-readable run report.
"""

import os
import sys

import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
sys.path.insert(0, os.path.join(PROJECT, "Tools", "Python"))

import gen_xwing_mesh  # noqa: E402 -- import path is set just above

OUT = os.environ.get("HYPERLANE_GEN_REPORT")

MESH_PATH = "/Game/Ships/Meshes"
MATERIAL_PATH = "/Game/Ships/Materials"
CLASS_PATH = "/Game/Ships/Classes"
BP_PATH = "/Game/Ships/Blueprints"

# Per slot: linear base colour, roughness, metallic, emissive. Keys must match
# gen_xwing_mesh.MATERIALS. Emissive is HDR so the exhausts read as lit in a
# dark level that has no bloom tuned for them.
PAINT = {
    "Hull":       ((0.78, 0.77, 0.74), 0.55, 0.00, (0.0, 0.0, 0.0)),
    "Stripe":     ((0.55, 0.05, 0.04), 0.50, 0.00, (0.0, 0.0, 0.0)),
    "Detail":     ((0.28, 0.28, 0.30), 0.45, 0.30, (0.0, 0.0, 0.0)),
    "Engine":     ((0.12, 0.12, 0.13), 0.50, 0.40, (0.0, 0.0, 0.0)),
    "EngineGlow": ((0.02, 0.01, 0.01), 0.90, 0.00, (8.0, 2.2, 1.2)),
    "Canopy":     ((0.02, 0.03, 0.05), 0.15, 0.10, (0.0, 0.0, 0.0)),
    "Astromech":  ((0.08, 0.20, 0.62), 0.40, 0.20, (0.0, 0.0, 0.0)),
}

# Framing and collision follow the hull's size, so they belong on the
# Blueprint that binds this hull rather than in C++ defaults sized for the
# Ghtroc. A sphere is a poor fit for an X: this covers fuselage and engines,
# and the wingtips overhang it.
COLLISION_RADIUS = 650.0
CAMERA_ARM_LENGTH = 2600.0
CAMERA_SOCKET_OFFSET = (0.0, 0.0, 260.0)

report = []
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def log(msg):
    report.append(msg)
    unreal.log("HYPERLANE_GEN: " + msg)


def flush_report():
    if OUT:
        with open(OUT, "w") as fh:
            fh.write("\n".join(report) + "\n")


def fail(msg):
    log("FAIL  " + msg)
    flush_report()
    raise SystemExit(1)


def load_or_create(name, path, cls, factory):
    """Load-or-create; never deletes. See gen_phase1_content.py for why."""
    full = path + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        return unreal.EditorAssetLibrary.load_asset(full)
    asset = tools.create_asset(name, path, cls, factory)
    if asset is None:
        fail("create " + full)
    return asset


def save(asset):
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        fail("save " + asset.get_path_name())


# ---------------------------------------------------------------------- mesh
obj_path = os.path.join(PROJECT, "Saved", "Generated", "SourceArt", "SM_XWing.obj")
stats = gen_xwing_mesh.write_obj(gen_xwing_mesh.build(), obj_path, "SM_XWing")
log("OK    wrote %s (%d tris)" % (obj_path, stats["triangles"]))

task = unreal.AssetImportTask()
for key, value in (("filename", obj_path), ("destination_path", MESH_PATH),
                   ("destination_name", "SM_XWing"), ("replace_existing", True),
                   ("automated", True), ("save", True)):
    task.set_editor_property(key, value)
tools.import_asset_tasks([task])

mesh = unreal.EditorAssetLibrary.load_asset(MESH_PATH + "/SM_XWing")
if not isinstance(mesh, unreal.StaticMesh):
    fail("import %s into %s" % (obj_path, MESH_PATH))

# Axis self-check. OBJ carries no axis metadata, so the map in
# gen_xwing_mesh.ue_to_obj is a measured fact about this engine's importer.
# Re-verifying it every run turns an importer change into a loud failure
# instead of a ship that quietly imports on its side.
box = mesh.get_bounding_box()
want = stats["bounds_min_ue"] + stats["bounds_max_ue"]
got = (box.min.x, box.min.y, box.min.z, box.max.x, box.max.y, box.max.z)
if any(abs(a - b) > 1.0 for a, b in zip(want, got)):
    fail("imported bounds %s != authored %s -- OBJ axis mapping is wrong for this engine"
         % (tuple(round(g, 1) for g in got), tuple(round(w, 1) for w in want)))
log("OK    %s/SM_XWing imported; bounds match authored geometry" % MESH_PATH)

# Nanite off. The importer enables it by default, but for a ~1k-triangle
# hand-built hull it buys nothing and costs a decimated fallback mesh: with
# Nanite on, LOD0 read back at 703 of 1012 triangles, and anything drawing the
# fallback -- a pass or platform without Nanite -- would see thin parts like
# the cannon barrels simplified away.
nanite = mesh.get_editor_property("nanite_settings")
if nanite.get_editor_property("enabled"):
    nanite.set_editor_property("enabled", False)
    mesh.set_editor_property("nanite_settings", nanite)
if mesh.get_editor_property("nanite_settings").get_editor_property("enabled"):
    fail("could not disable Nanite on SM_XWing")

lod0_triangles = mesh.get_num_triangles(0)
if lod0_triangles != stats["triangles"]:
    log("NOTE  SM_XWing LOD0 reports %d triangles, authored %d" % (lod0_triangles, stats["triangles"]))
log("OK    SM_XWing Nanite disabled")

# --------------------------------------------------------------------- paint
paint = load_or_create("M_ShipPaint", MATERIAL_PATH, unreal.Material, unreal.MaterialFactoryNew())

# Rebuilt from nothing each run so a re-run cannot stack duplicate nodes.
#
# Deliberately not MaterialEditingLibrary.delete_all_material_expressions: in
# 5.8 it deletes from the array it is iterating, so it skips every other node
# and leaves orphaned duplicate parameters behind (a re-run left 5 nodes where
# 4 were authored). Deleting from a snapshot is safe, and the count checks turn
# any recurrence into a loud failure instead of a quietly dirty graph.
for expression in list(mel.get_material_expressions(paint)):
    mel.delete_material_expression(paint, expression)
if mel.get_num_material_expressions(paint) != 0:
    fail("M_ShipPaint still has %d expressions after clearing" % mel.get_num_material_expressions(paint))


def parameter(cls, name, x, y, default):
    expr = mel.create_material_expression(paint, cls, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", default)
    return expr


base = parameter(unreal.MaterialExpressionVectorParameter, "BaseColor", -500, -300,
                 unreal.LinearColor(0.7, 0.7, 0.7, 1.0))
rough = parameter(unreal.MaterialExpressionScalarParameter, "Roughness", -500, -100, 0.5)
metal = parameter(unreal.MaterialExpressionScalarParameter, "Metallic", -500, 0, 0.0)
glow = parameter(unreal.MaterialExpressionVectorParameter, "Emissive", -500, 150,
                 unreal.LinearColor(0.0, 0.0, 0.0, 1.0))

wiring = ((base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR),
          (rough, "", unreal.MaterialProperty.MP_ROUGHNESS),
          (metal, "", unreal.MaterialProperty.MP_METALLIC),
          (glow, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR))

for expr, output, prop in wiring:
    if not mel.connect_material_property(expr, output, prop):
        fail("connect %s -> %s" % (expr.get_name(), prop))

# Every node in this graph is a wired parameter, so any extra node is an orphan.
node_count = mel.get_num_material_expressions(paint)
if node_count != len(wiring):
    fail("M_ShipPaint has %d expressions, expected exactly the %d wired parameters"
         % (node_count, len(wiring)))

mel.recompile_material(paint)
save(paint)
log("OK    %s/M_ShipPaint" % MATERIAL_PATH)

instances = {}
for slot, (colour, roughness, metallic, emissive) in PAINT.items():
    mi = load_or_create("MI_XWing_" + slot, MATERIAL_PATH, unreal.MaterialInstanceConstant,
                        unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, paint)
    mel.set_material_instance_vector_parameter_value(mi, "BaseColor", unreal.LinearColor(colour[0], colour[1], colour[2], 1.0))
    mel.set_material_instance_scalar_parameter_value(mi, "Roughness", roughness)
    mel.set_material_instance_scalar_parameter_value(mi, "Metallic", metallic)
    mel.set_material_instance_vector_parameter_value(mi, "Emissive", unreal.LinearColor(emissive[0], emissive[1], emissive[2], 1.0))
    mel.update_material_instance(mi)
    save(mi)
    instances[slot] = mi
log("OK    %d paint instances" % len(instances))

# Bind by slot *name*, never by index: slot order is whatever the importer
# decided, and a silent index mismatch paints the canopy red.
bound = set()
for index, slot in enumerate(mesh.get_editor_property("static_materials")):
    name = str(slot.get_editor_property("material_slot_name"))
    if name not in instances:
        fail("mesh slot %r has no paint; gen_xwing_mesh.MATERIALS and PAINT disagree" % name)
    mesh.set_material(index, instances[name])
    bound.add(name)

unpainted = set(stats["triangles_by_material"]) - bound
if unpainted:
    fail("authored materials missing from the imported mesh: %s" % sorted(unpainted))

save(mesh)
log("OK    bound %d material slots on SM_XWing" % len(bound))

# ---------------------------------------------------------------- ship class
strike = unreal.EditorAssetLibrary.load_asset("/Game/Ships/SizeClasses/SC_Strike")
input_cfg = unreal.EditorAssetLibrary.load_asset("/Game/Input/DA_ShipInput")
if not strike or not input_cfg:
    fail("SC_Strike or DA_ShipInput missing; run gen_phase1_content.py first")

class_factory = unreal.DataAssetFactory()
try:
    class_factory.set_editor_property("data_asset_class", unreal.ShipClassDef)
except Exception as exc:  # noqa: BLE001 -- reported, and asset_class below still decides
    log("NOTE  data_asset_class unsettable (%s)" % exc)

ship_class = load_or_create("SHIP_XWing", CLASS_PATH, unreal.ShipClassDef, class_factory)
ship_class.set_editor_property("display_name", unreal.Text("T-65B X-wing (flight test)"))
ship_class.set_editor_property("manufacturer", unreal.Text("Incom Corporation"))
# The Strike size class is the point: the X-wing inherits a fighter's handling
# curve instead of anyone writing fighter flight code.
ship_class.set_editor_property("size_class", strike)
ship_class.set_editor_property("sublight_rating", 1.00)
ship_class.set_editor_property("handling_rating", 0.95)
ship_class.set_editor_property("hyperdrive_rating", 1.00)
save(ship_class)
log("OK    %s/SHIP_XWing (Strike)" % CLASS_PATH)

# ----------------------------------------------------------------- Blueprint
bp_factory = unreal.BlueprintFactory()
bp_factory.set_editor_property("parent_class", unreal.ShipPawn)
ship_bp = load_or_create("BP_XWing", BP_PATH, None, bp_factory)

cdo = unreal.get_default_object(ship_bp.generated_class())
cdo.set_editor_property("ship_class", ship_class)
cdo.set_editor_property("input_config", input_cfg)

hull = cdo.get_editor_property("hull")
hull.set_editor_property("static_mesh", mesh)
hull.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))

cdo.get_editor_property("collision_root").set_editor_property("sphere_radius", COLLISION_RADIUS)

boom = cdo.get_editor_property("camera_boom")
boom.set_editor_property("target_arm_length", CAMERA_ARM_LENGTH)
boom.set_editor_property("socket_offset", unreal.Vector(*CAMERA_SOCKET_OFFSET))

save(ship_bp)
log("OK    %s/BP_XWing" % BP_PATH)

flush_report()
