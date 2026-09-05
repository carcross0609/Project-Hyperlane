"""Authors Phase 1 ship and input content for Project Hyperlane.

Run headless from the repo root:

  "$UE/Engine/Binaries/Mac/UnrealEditor-Cmd" "$PWD/Hyperlane.uproject" \\
      -run=pythonscript -script="$PWD/Tools/Python/gen_phase1_content.py" \\
      -unattended -nopause -nosplash -NullRHI

Idempotent: every asset is loaded if it already exists and updated in place,
so re-running is safe and never destroys authored work.

The content this produces is *derived* from the design docs, not invented
here — size-class handling curves are Bible 05 §1, the Ghtroc 720 starter is
Bible 05 §4 / DR-16. Retune by editing this script and re-running, so the
numbers stay reviewable in a diff instead of living only inside binary assets.

Set HYPERLANE_GEN_REPORT to a file path to capture a machine-readable run
report; otherwise progress goes to the Unreal log only.
"""

import os

import unreal

OUT = os.environ.get("HYPERLANE_GEN_REPORT")

report = []
tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(msg):
    report.append(msg)
    unreal.log(f"HYPERLANE_GEN: {msg}")


def create(name, path, cls, factory):
    """Load-or-create. Never deletes.

    Delete-then-recreate fails on a second run: the outgoing package is still
    loaded and referenced, so the delete is refused and the create then has
    nowhere to go. Every property is reassigned below regardless, so updating
    in place is equivalent and cannot destroy authored work.
    """
    full = f"{path}/{name}"
    # does_asset_exist first: load_asset on a missing path logs an Error that
    # looks like a failure but is just the create path.
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        return unreal.EditorAssetLibrary.load_asset(full)
    asset = tools.create_asset(name, path, cls, factory)
    if asset is None:
        log(f"FAIL  {full}")
    return asset


def data_asset(name, path, cls):
    factory = unreal.DataAssetFactory()
    try:
        factory.set_editor_property("data_asset_class", cls)
    except Exception as exc:                      # noqa: BLE001 - reported, not swallowed
        log(f"NOTE  data_asset_class unsettable ({exc}); relying on asset_class arg")
    return create(name, path, cls, factory)


# ---------------------------------------------------------------- size classes
# Six curves, one per size class. These are the *shape* of how a hull that
# size handles; individual ships scale them by their ratings, which is what
# makes DR-13 (pilotable capitals) cost one curve per class, not one flight
# model per hull.
#
# fields: fwd, rev, strafe, linResp, pitch, yaw, roll, angResp, boost
SIZE_CLASSES = [
    ("SC_Strike",     "Strike",       0.0,   30.0, 45000,12000,18000, 5.00, 110.0, 90.0, 200.0, 9.00, 1.90),
    ("SC_Light",      "Light",       20.0,   60.0, 30000, 8000,10000, 2.50,  60.0, 50.0, 110.0, 5.00, 1.75),
    ("SC_Corvette",   "Corvette",   100.0,  250.0, 20000, 5000, 4500, 1.10,  22.0, 20.0,  35.0, 2.20, 1.50),
    ("SC_Frigate",    "Frigate",    300.0,  600.0, 15000, 3500, 2500, 0.70,  12.0, 11.0,  18.0, 1.30, 1.40),
    ("SC_Cruiser",    "Cruiser",    600.0, 1200.0, 11000, 2500, 1500, 0.45,   7.0,  6.5,   9.0, 0.80, 1.30),
    ("SC_Battleship", "Battleship",1200.0, 4000.0,  8000, 1500,  900, 0.25,   3.5,  3.2,   4.5, 0.45, 1.20),
]

SIZE_PATH = "/Game/Ships/SizeClasses"
size_assets = {}

for (name, display, min_m, max_m, fwd, rev, strafe, lin,
     pitch, yaw, roll, ang, boost) in SIZE_CLASSES:
    asset = data_asset(name, SIZE_PATH, unreal.ShipSizeClassDef)
    if not asset:
        continue

    asset.set_editor_property("display_name", unreal.Text(display))
    asset.set_editor_property("min_length_meters", min_m)
    asset.set_editor_property("max_length_meters", max_m)

    fm = asset.get_editor_property("base_flight_model")
    fm.set_editor_property("max_forward_speed", float(fwd))
    fm.set_editor_property("max_reverse_speed", float(rev))
    fm.set_editor_property("max_strafe_speed", float(strafe))
    fm.set_editor_property("linear_responsiveness", lin)
    fm.set_editor_property("max_pitch_rate", pitch)
    fm.set_editor_property("max_yaw_rate", yaw)
    fm.set_editor_property("max_roll_rate", roll)
    fm.set_editor_property("angular_responsiveness", ang)
    fm.set_editor_property("boost_speed_multiplier", boost)
    asset.set_editor_property("base_flight_model", fm)

    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    size_assets[name] = asset
    log(f"OK    {SIZE_PATH}/{name}")

# ---------------------------------------------------------------- ship classes
# The starter (DR-16 / Bible 05 §4): a battered Ghtroc 720. Slightly under
# baseline on every axis — decades old and mortgaged, exactly as designed.
CLASS_PATH = "/Game/Ships/Classes"

ghtroc = data_asset("SHIP_Ghtroc720", CLASS_PATH, unreal.ShipClassDef)
if ghtroc:
    ghtroc.set_editor_property("display_name", unreal.Text("Ghtroc 720"))
    ghtroc.set_editor_property("manufacturer", unreal.Text("Ghtroc Industries"))
    if "SC_Light" in size_assets:
        ghtroc.set_editor_property("size_class", size_assets["SC_Light"])
    ghtroc.set_editor_property("sublight_rating", 0.90)
    ghtroc.set_editor_property("handling_rating", 0.85)
    ghtroc.set_editor_property("hyperdrive_rating", 2.00)
    unreal.EditorAssetLibrary.save_loaded_asset(ghtroc)
    log(f"OK    {CLASS_PATH}/SHIP_Ghtroc720")

# --------------------------------------------------------------- input actions
ACTION_PATH = "/Game/Input/Actions"
V = unreal.InputActionValueType

ACTIONS = [
    ("IA_Throttle", V.AXIS1D),
    ("IA_Strafe",   V.AXIS2D),
    ("IA_Look",     V.AXIS2D),
    ("IA_Roll",     V.AXIS1D),
    ("IA_Boost",    V.BOOLEAN),
    ("IA_Dock",     V.BOOLEAN),
]

actions = {}
for name, value_type in ACTIONS:
    act = create(name, ACTION_PATH, unreal.InputAction, unreal.InputAction_Factory())
    if not act:
        continue
    act.set_editor_property("value_type", value_type)
    unreal.EditorAssetLibrary.save_loaded_asset(act)
    actions[name] = act
    log(f"OK    {ACTION_PATH}/{name}")

# -------------------------------------------------------------- mapping context
INPUT_PATH = "/Game/Input"
imc = create("IMC_ShipControls", INPUT_PATH, unreal.InputMappingContext,
             unreal.InputMappingContext_Factory())


def key(name):
    k = unreal.Key()
    k.set_editor_property("key_name", name)
    return k


def modifiers(imc_outer, *classes):
    return [unreal.new_object(c, imc_outer) for c in classes]


NEG = unreal.InputModifierNegate
SWZ = unreal.InputModifierSwizzleAxis

if imc:
    mappings = []

    def bind(action_name, key_name, mods=()):
        act = actions.get(action_name)
        if not act:
            return
        m = unreal.EnhancedActionKeyMapping()
        m.set_editor_property("action", act)
        m.set_editor_property("key", key(key_name))
        if mods:
            m.set_editor_property("modifiers", list(mods))
        mappings.append(m)

    # Throttle: W/S, plus gamepad triggers.
    bind("IA_Throttle", "W")
    bind("IA_Throttle", "S", modifiers(imc, NEG))
    bind("IA_Throttle", "Gamepad_RightTriggerAxis")
    bind("IA_Throttle", "Gamepad_LeftTriggerAxis", modifiers(imc, NEG))

    # Strafe X = right, Y = up. Single keys land on X by default, so the
    # vertical pair needs a swizzle to reach Y.
    bind("IA_Strafe", "D")
    bind("IA_Strafe", "A", modifiers(imc, NEG))
    bind("IA_Strafe", "SpaceBar", modifiers(imc, SWZ))
    bind("IA_Strafe", "LeftControl", modifiers(imc, SWZ, NEG))
    bind("IA_Strafe", "Gamepad_LeftX")
    bind("IA_Strafe", "Gamepad_LeftY", modifiers(imc, SWZ))

    # Look: mouse gives a native 2D axis. A stick axis is 1D and lands on X,
    # so the vertical one has to be swizzled onto Y or the pad can yaw but
    # never pitch.
    bind("IA_Look", "Mouse2D")
    bind("IA_Look", "Gamepad_RightX")
    bind("IA_Look", "Gamepad_RightY", modifiers(imc, SWZ))

    # Roll on Q/E and the shoulder buttons.
    bind("IA_Roll", "E")
    bind("IA_Roll", "Q", modifiers(imc, NEG))
    bind("IA_Roll", "Gamepad_RightShoulder")
    bind("IA_Roll", "Gamepad_LeftShoulder", modifiers(imc, NEG))

    # Boost, held.
    bind("IA_Boost", "LeftShift")
    bind("IA_Boost", "Gamepad_FaceButton_Bottom")

    # Dock/undock, one press. Bound to a face button rather than a trigger so
    # it can never be hit while manoeuvring onto a berth.
    bind("IA_Dock", "F")
    bind("IA_Dock", "Gamepad_FaceButton_Top")

    imc.set_editor_property("mappings", mappings)
    unreal.EditorAssetLibrary.save_loaded_asset(imc)
    log(f"OK    {INPUT_PATH}/IMC_ShipControls ({len(mappings)} mappings)")

# ----------------------------------------------------------------- input config
cfg = data_asset("DA_ShipInput", INPUT_PATH, unreal.ShipInputConfig)
if cfg:
    if imc:
        cfg.set_editor_property("mapping_context", imc)
    for prop, act_name in (("throttle_action", "IA_Throttle"),
                           ("strafe_action", "IA_Strafe"),
                           ("look_action", "IA_Look"),
                           ("roll_action", "IA_Roll"),
                           ("boost_action", "IA_Boost"),
                           ("dock_action", "IA_Dock")):
        if act_name in actions:
            cfg.set_editor_property(prop, actions[act_name])
    unreal.EditorAssetLibrary.save_loaded_asset(cfg)
    log(f"OK    {INPUT_PATH}/DA_ShipInput")

if OUT:
    with open(OUT, "w") as fh:
        fh.write("\n".join(report) + "\n")
