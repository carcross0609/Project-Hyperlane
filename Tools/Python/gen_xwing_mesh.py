#!/usr/bin/env python3
"""Generates a low-poly X-wing flight-test hull as a Wavefront OBJ.

Run standalone to inspect the geometry without Unreal:

  python3 Tools/Python/gen_xwing_mesh.py [output.obj]

gen_xwing_content.py imports this module and writes the same file before
importing it into the project. The OBJ is therefore a build product: change
the numbers here, never the .obj. It is written under Saved/ (git-ignored) for
that reason -- committing it would give the mesh two sources of truth.

Why generated geometry rather than a downloaded model: this is a placeholder
for tuning flight, and a script keeps every dimension reviewable in a diff and
licence-clean. A real hull arrives later through the same import path.

Off-roster on purpose: the T-65B is a Rebellion-era design, roughly twenty
years after the game's Republic-era setting, so it is not one of Bible 05's
tick-zero classes. It exists only as a flight-test hull. The in-era equivalent
is its predecessor, the ARC-170.

Conventions
-----------
Geometry is authored in Unreal space -- X forward, Y right, Z up, centimetres
-- because that is the frame every dimension below is reasoned in. It is
converted to the frame Unreal's OBJ importer expects only when written
(ue_to_obj), and every face is re-wound counter-clockwise-outward in that
frame by testing it against an interior point of its part. Winding mistakes
in the part builders therefore cannot reach the file.
"""

import math
import os
import sys

# Material slot names, in slot order. Unreal keeps `usemtl` names as slot
# names, and gen_xwing_content.py binds a material instance to each by name,
# so renaming one here means renaming it there.
MATERIALS = ("Hull", "Stripe", "Detail", "Engine", "EngineGlow", "Canopy", "Astromech")

# One UV unit per metre. The paint materials are flat colour and never sample
# a texture, but the mesh still needs real UVs: Unreal's OBJ translator trips
# an ensure on a file without them, and tangents built from degenerate UVs are
# garbage the moment anyone adds a normal map.
UV_SCALE = 0.01

# ---------------------------------------------------------------- dimensions
# Centimetres, Unreal space. Roughly T-65B proportions (12.5 m long, 11.8 m
# span with S-foils open), slightly stylised so the silhouette reads at the
# chase camera's distance.

# Fuselage cross-sections: (x, half width, half height, z centre).
FUSELAGE_SECTIONS = (
    (-560.0, 62.0, 58.0,   0.0),   # tail
    (-360.0, 66.0, 62.0,   0.0),   # engine block, widest point
    (-150.0, 58.0, 58.0,   4.0),   # behind the cockpit
    (  60.0, 46.0, 48.0,   0.0),   # nose begins
    ( 400.0, 32.0, 30.0,  -9.0),
    ( 460.0, 30.0, 28.0, -10.0),   # squadron band starts
    ( 520.0, 27.0, 25.0, -11.0),   # squadron band ends
    ( 660.0, 16.0, 14.0, -15.0),
    ( 720.0,  6.0,  6.0, -16.0),   # nose tip
)
FUSELAGE_SEGMENT_MATERIALS = ("Hull", "Hull", "Hull", "Hull", "Hull", "Stripe", "Hull", "Hull")

# Canopy loft ends: (x, bottom half width, top half width, bottom z, top z).
# Bottoms sit a few cm inside the fuselage so no seam shows.
CANOPY_REAR = (-140.0, 44.0, 30.0, 52.0, 100.0)
CANOPY_FRONT = (120.0, 28.0, 16.0, 38.0, 56.0)

# Astromech dome: (x, y, base z, radius).
ASTROMECH = (-235.0, 0.0, 56.0, 23.0)

# S-foils, attack position. Wing roots start inside the fuselage side.
WING_ROOT_Y = 50.0
WING_ROOT_Z = 20.0
WING_DIHEDRAL_DEG = 13.0
WING_SPAN = 525.0
WING_LE_ROOT, WING_TE_ROOT = -345.0, -595.0
WING_LE_TIP, WING_TE_TIP = -405.0, -575.0
WING_THICK_ROOT, WING_THICK_TIP = 16.0, 8.0
# Span fractions: plain hull either side of a squadron-colour band.
WING_PANELS = ((0.00, 0.46, "Hull"), (0.46, 0.64, "Stripe"), (0.64, 1.00, "Hull"))

# Engines sit at the wing roots, outboard of each wing, forming the X of four
# exhausts seen from behind.
ENGINE_SPAN_OFFSET = 40.0
ENGINE_STANDOFF = 38.0
ENGINE_SEGMENTS = 12
ENGINE_STACK = (
    # x rear,   x front, r rear, r front, side,     rear cap,     front cap
    (-640.0, -612.0, 28.0, 32.0, "Detail", "EngineGlow", None),      # nozzle
    (-612.0, -330.0, 34.0, 34.0, "Engine", "Engine", None),          # body
    (-330.0, -300.0, 34.0, 30.0, "Detail", None, "Detail"),          # intake lip
)

# Laser cannons at each wing tip.
CANNON_TIP_OFFSET = 4.0
CANNON_SEGMENTS = 8
CANNON_STACK = (
    (-565.0, -300.0, 13.0, 11.0, "Detail", "Detail", "Detail"),      # mount
    (-300.0, 160.0, 5.0, 5.0, "Detail", None, None),                 # barrel
    (160.0, 215.0, 8.0, 8.0, "Detail", "Detail", "Detail"),          # flash suppressor
)


# ------------------------------------------------------------------ vectors

def _sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def _dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def _normalize(v):
    length = math.sqrt(_dot(v, v))
    if length < 1e-9:
        return None
    return (v[0] / length, v[1] / length, v[2] / length)


def _lerp(a, b, t):
    return a + (b - a) * t


def _average(points):
    count = float(len(points))
    return (sum(p[0] for p in points) / count,
            sum(p[1] for p in points) / count,
            sum(p[2] for p in points) / count)


def _newell(points):
    """Polygon normal by Newell's method: robust for slightly non-planar quads
    (the tapered wing skins), and counter-clockwise gives +normal in a
    right-handed frame."""
    nx = ny = nz = 0.0
    count = len(points)
    for i in range(count):
        cx, cy, cz = points[i]
        px, py, pz = points[(i + 1) % count]
        nx += (cy - py) * (cz + pz)
        ny += (cz - pz) * (cx + px)
        nz += (cx - px) * (cy + py)
    return _normalize((nx, ny, nz))


# ------------------------------------------------------------------- builder

class MeshBuilder:
    """Positions plus convex polygons, each tagged with an interior point of
    the part it belongs to. That point is what lets the writer orient every
    face outward, so the part builders never have to get winding right."""

    def __init__(self):
        self.positions = []
        self.faces = []

    def vertex(self, p):
        self.positions.append((float(p[0]), float(p[1]), float(p[2])))
        return len(self.positions) - 1

    def face(self, indices, material, interior, normals=None):
        if material not in MATERIALS:
            raise ValueError("unknown material %r" % (material,))
        self.faces.append((list(indices), material, interior, normals))


def _loft(mb, sections, segment_materials, cap_first=None, cap_last=None):
    """Skins consecutive rings that share a vertex count and angular order."""
    if len(segment_materials) != len(sections) - 1:
        raise ValueError("need one material per segment")
    rings = [[mb.vertex(p) for p in section] for section in sections]
    count = len(sections[0])
    for s, material in enumerate(segment_materials):
        interior = _average(sections[s] + sections[s + 1])
        a, b = rings[s], rings[s + 1]
        for i in range(count):
            j = (i + 1) % count
            mb.face([a[i], a[j], b[j], b[i]], material, interior)
    if cap_first:
        mb.face(rings[0], cap_first, _average(sections[0] + sections[1]))
    if cap_last:
        mb.face(rings[-1], cap_last, _average(sections[-1] + sections[-2]))


def _octagon(x, half_width, half_height, z_centre, chamfer=0.35):
    """A rectangle with cut corners -- reads as machined hull plating, where a
    plain box reads as a placeholder."""
    c = chamfer * min(half_width, half_height)
    ring = (
        (half_width, -half_height + c),
        (half_width, half_height - c),
        (half_width - c, half_height),
        (-half_width + c, half_height),
        (-half_width, half_height - c),
        (-half_width, -half_height + c),
        (-half_width + c, -half_height),
        (half_width - c, -half_height),
    )
    return [(x, y, z_centre + z) for y, z in ring]


def _canopy_section(x, bottom_half_width, top_half_width, bottom_z, top_z):
    return [(x, bottom_half_width, bottom_z),
            (x, top_half_width, top_z),
            (x, -top_half_width, top_z),
            (x, -bottom_half_width, bottom_z)]


def _cylinder_x(mb, x_rear, x_front, r_rear, r_front, cy, cz, segments,
                side, cap_rear=None, cap_front=None):
    """A frustum along +X with smooth side normals and flat caps."""
    # Half-segment offset puts a flat facet on top, which reads tidier than a
    # ridge along the crest of a low-segment cylinder.
    offset = math.pi / segments
    rear, front, radial = [], [], []
    for i in range(segments):
        a = offset + 2.0 * math.pi * i / segments
        ca, sa = math.cos(a), math.sin(a)
        rear.append(mb.vertex((x_rear, cy + r_rear * ca, cz + r_rear * sa)))
        front.append(mb.vertex((x_front, cy + r_front * ca, cz + r_front * sa)))
        radial.append((0.0, ca, sa))
    interior = ((x_rear + x_front) * 0.5, cy, cz)
    for i in range(segments):
        j = (i + 1) % segments
        mb.face([rear[i], rear[j], front[j], front[i]], side, interior,
                normals=[radial[i], radial[j], radial[j], radial[i]])
    if cap_rear:
        mb.face(rear, cap_rear, interior)
    if cap_front:
        mb.face(front, cap_front, interior)


def _stack(mb, stack, cy, cz, segments):
    for x_rear, x_front, r_rear, r_front, side, cap_rear, cap_front in stack:
        _cylinder_x(mb, x_rear, x_front, r_rear, r_front, cy, cz, segments,
                    side, cap_rear, cap_front)


def _dome(mb, cx, cy, base_z, radius, segments, band_material, cap_material):
    """Low-poly hemisphere, open at the base (the base is buried in the hull)."""
    centre = (cx, cy, base_z)
    rings = []
    for elevation in (0.0, 30.0, 60.0):
        e = math.radians(elevation)
        rings.append([(cx + radius * math.cos(e) * math.cos(2.0 * math.pi * i / segments),
                       cy + radius * math.cos(e) * math.sin(2.0 * math.pi * i / segments),
                       base_z + radius * math.sin(e))
                      for i in range(segments)])
    ring_ids = [[mb.vertex(p) for p in ring] for ring in rings]
    top = mb.vertex((cx, cy, base_z + radius))
    interior = (cx, cy, base_z + radius * 0.35)

    def normal(p):
        return _normalize(_sub(p, centre))

    for k in range(len(rings) - 1):
        material = band_material if k == 0 else cap_material
        for i in range(segments):
            j = (i + 1) % segments
            points = [rings[k][i], rings[k][j], rings[k + 1][j], rings[k + 1][i]]
            mb.face([ring_ids[k][i], ring_ids[k][j], ring_ids[k + 1][j], ring_ids[k + 1][i]],
                    material, interior, normals=[normal(p) for p in points])
    last = len(rings) - 1
    for i in range(segments):
        j = (i + 1) % segments
        mb.face([ring_ids[last][i], ring_ids[last][j], top], cap_material, interior,
                normals=[normal(rings[last][i]), normal(rings[last][j]), (0.0, 0.0, 1.0)])


def _wing_point(side, upper, span, thickness):
    """Wing-local (span, thickness) to world (y, z), rotated by the S-foil
    dihedral about the root. side is +1 right / -1 left."""
    angle = math.radians(WING_DIHEDRAL_DEG) * (1.0 if upper else -1.0)
    root_z = WING_ROOT_Z if upper else -WING_ROOT_Z
    y = WING_ROOT_Y + span * math.cos(angle) - thickness * math.sin(angle)
    z = root_z + span * math.sin(angle) + thickness * math.cos(angle)
    return side * y, z


def _wing(mb, side, upper):
    for f0, f1, material in WING_PANELS:
        corner = {}
        points = []
        for end, f in (("root", f0), ("tip", f1)):
            span = f * WING_SPAN
            half_thick = _lerp(WING_THICK_ROOT, WING_THICK_TIP, f) * 0.5
            for edge, x in (("le", _lerp(WING_LE_ROOT, WING_LE_TIP, f)),
                            ("te", _lerp(WING_TE_ROOT, WING_TE_TIP, f))):
                for skin, t in (("a", half_thick), ("b", -half_thick)):
                    y, z = _wing_point(side, upper, span, t)
                    corner[(end, edge, skin)] = mb.vertex((x, y, z))
                    points.append((x, y, z))
        interior = _average(points)
        c = corner
        # Skins, then leading and trailing edges. Panels share their internal
        # boundary, so only the outermost panel gets a tip cap, and the root
        # is buried in the fuselage.
        mb.face([c["root", "le", "a"], c["tip", "le", "a"], c["tip", "te", "a"], c["root", "te", "a"]], material, interior)
        mb.face([c["root", "le", "b"], c["root", "te", "b"], c["tip", "te", "b"], c["tip", "le", "b"]], material, interior)
        mb.face([c["root", "le", "a"], c["root", "le", "b"], c["tip", "le", "b"], c["tip", "le", "a"]], material, interior)
        mb.face([c["root", "te", "a"], c["tip", "te", "a"], c["tip", "te", "b"], c["root", "te", "b"]], material, interior)
        if f1 >= 1.0:
            mb.face([c["tip", "le", "a"], c["tip", "le", "b"], c["tip", "te", "b"], c["tip", "te", "a"]], material, interior)


def build():
    """Assembles the hull. Returns a MeshBuilder in Unreal space."""
    mb = MeshBuilder()

    _loft(mb, [_octagon(*section) for section in FUSELAGE_SECTIONS],
          FUSELAGE_SEGMENT_MATERIALS, cap_first="Detail", cap_last="Hull")
    _loft(mb, [_canopy_section(*CANOPY_REAR), _canopy_section(*CANOPY_FRONT)],
          ["Canopy"], cap_first="Canopy", cap_last="Canopy")
    _dome(mb, *ASTROMECH, segments=12, band_material="Hull", cap_material="Astromech")

    for side in (1.0, -1.0):
        for upper in (True, False):
            _wing(mb, side, upper)

            standoff = ENGINE_STANDOFF if upper else -ENGINE_STANDOFF
            cy, cz = _wing_point(side, upper, ENGINE_SPAN_OFFSET, standoff)
            _stack(mb, ENGINE_STACK, cy, cz, ENGINE_SEGMENTS)

            cy, cz = _wing_point(side, upper, WING_SPAN + CANNON_TIP_OFFSET, 0.0)
            _stack(mb, CANNON_STACK, cy, cz, CANNON_SEGMENTS)

    return mb


# -------------------------------------------------------------------- writer

def ue_to_obj(v):
    """Unreal space to the OBJ frame Unreal's own importer expects.

    Settled by measurement, not by the usual "OBJ is Y-up" convention: an
    axis-aligned box, asymmetric on every axis, was imported and its bounds
    read back. Unreal 5.8's Interchange OBJ translator treats the file as
    Z-up and converts handedness by negating Y, so the inverse is negating Y
    on the way out -- which is all this does. gen_xwing_content.py re-checks
    imported bounds against authored bounds on every run, so an importer
    change fails loudly instead of producing a sideways ship.

    A reflection, so the file is right-handed and the writer winds faces
    counter-clockwise-outward in it. Also applied to normals, which is valid
    because the map is orthogonal.
    """
    return (v[0], -v[1], v[2])


def write_obj(mb, path, object_name):
    """Writes mb as a single-object OBJ and returns stats for verification.

    Deliberately no `g` groups: several importers split groups into separate
    meshes, and the hull must import as one asset with one slot per material.
    """
    positions = [ue_to_obj(p) for p in mb.positions]
    normals = []
    normal_ids = {}

    def normal_id(n):
        key = (round(n[0], 5), round(n[1], 5), round(n[2], 5))
        if key not in normal_ids:
            normals.append(key)
            normal_ids[key] = len(normals)
        return normal_ids[key]

    uvs = []
    uv_ids = {}

    def uv_id(uv):
        key = (round(uv[0], 5), round(uv[1], 5))
        if key not in uv_ids:
            uvs.append(key)
            uv_ids[key] = len(uvs)
        return uv_ids[key]

    triangles = dict((m, []) for m in MATERIALS)
    flipped = 0
    skipped = 0

    for indices, material, interior, smooth in mb.faces:
        points = [positions[i] for i in indices]
        n = _newell(points)
        if n is None:
            skipped += 1
            continue

        if _dot(n, _sub(_average(points), ue_to_obj(interior))) < 0.0:
            indices = indices[::-1]
            n = (-n[0], -n[1], -n[2])
            if smooth:
                smooth = smooth[::-1]
            flipped += 1

        if smooth:
            ids = []
            for s in smooth:
                o = _normalize(ue_to_obj(s)) or n
                if _dot(o, n) < 0.0:
                    o = (-o[0], -o[1], -o[2])
                ids.append(normal_id(o))
        else:
            ids = [normal_id(n)] * len(indices)

        # Box projection onto the axis plane the face most nearly lies in.
        axis = max(range(3), key=lambda i: abs(n[i]))
        u_axis, v_axis = ((1, 2), (0, 2), (0, 1))[axis]
        face_uvs = [uv_id((positions[i][u_axis] * UV_SCALE, positions[i][v_axis] * UV_SCALE))
                    for i in indices]

        corners = [(indices[k] + 1, face_uvs[k], ids[k]) for k in range(len(indices))]
        for k in range(1, len(corners) - 1):
            triangles[material].append((corners[0], corners[k], corners[k + 1]))

    directory = os.path.dirname(path)
    if directory:
        os.makedirs(directory, exist_ok=True)

    with open(path, "w") as fh:
        fh.write("# Generated by Tools/Python/gen_xwing_mesh.py -- edit the script, not this file.\n")
        fh.write("o %s\n" % object_name)
        for p in positions:
            fh.write("v %.4f %.4f %.4f\n" % p)
        for uv in uvs:
            fh.write("vt %.5f %.5f\n" % uv)
        for n in normals:
            fh.write("vn %.5f %.5f %.5f\n" % n)
        for material in MATERIALS:
            if not triangles[material]:
                continue
            fh.write("usemtl %s\n" % material)
            for a, b, c in triangles[material]:
                fh.write("f %d/%d/%d %d/%d/%d %d/%d/%d\n" % (a + b + c))

    xs = [p[0] for p in mb.positions]
    ys = [p[1] for p in mb.positions]
    zs = [p[2] for p in mb.positions]
    return {
        "path": path,
        "vertices": len(mb.positions),
        "triangles": sum(len(t) for t in triangles.values()),
        "triangles_by_material": dict((m, len(triangles[m])) for m in MATERIALS if triangles[m]),
        "faces_rewound": flipped,
        "faces_skipped": skipped,
        "bounds_min_ue": (min(xs), min(ys), min(zs)),
        "bounds_max_ue": (max(xs), max(ys), max(zs)),
    }


def format_stats(stats):
    lo, hi = stats["bounds_min_ue"], stats["bounds_max_ue"]
    lines = [
        "wrote      %s" % stats["path"],
        "vertices   %d" % stats["vertices"],
        "triangles  %d" % stats["triangles"],
        "rewound    %d faces   skipped %d degenerate" % (stats["faces_rewound"], stats["faces_skipped"]),
        "bounds UE  min (%.1f, %.1f, %.1f)  max (%.1f, %.1f, %.1f)" % (lo + hi),
        "extent UE  X %.1f  Y %.1f  Z %.1f" % (hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2]),
    ]
    for material, count in stats["triangles_by_material"].items():
        lines.append("  %-11s %d tris" % (material, count))
    return "\n".join(lines)


def default_output_path():
    repo = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    return os.path.join(repo, "Saved", "Generated", "SourceArt", "SM_XWing.obj")


def main(argv):
    path = argv[1] if len(argv) > 1 else default_output_path()
    print(format_stats(write_obj(build(), path, "SM_XWing")))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
