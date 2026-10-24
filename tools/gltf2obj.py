#!/usr/bin/env python3
"""Asset prep tool (not part of the program): convert a Sketchfab glTF to OBJ.

Keeps only the shape: positions, normals, triangles, plus the material NAME of
each piece as "usemtl" so the program can tell body / glass / tires apart.
Colors and textures are dropped on purpose: appearance is implemented in our code.
No .mtl file is written.

usage: python3 gltf2obj.py scene.gltf out.obj [--skip NAME_PART ...]
  --skip  drop primitives whose material name contains one of these words (e.g. interior)
Only the Python standard library is used.
"""
import argparse
import json
import math
import os
import struct

COMPONENT = {5121: 'B', 5123: 'H', 5125: 'I', 5126: 'f'}  # unsigned byte/short/int, float
COUNT = {'SCALAR': 1, 'VEC3': 3}


def read_accessor(gltf, buffers, index):
    """Returns a list of tuples (VEC3) or ints (SCALAR)."""
    acc = gltf['accessors'][index]
    view = gltf['bufferViews'][acc['bufferView']]
    fmt = COMPONENT[acc['componentType']]
    n = COUNT[acc['type']]
    size = struct.calcsize(fmt) * n
    stride = view.get('byteStride', size)
    start = view.get('byteOffset', 0) + acc.get('byteOffset', 0)
    data = buffers[view['buffer']]
    out = []
    for i in range(acc['count']):
        v = struct.unpack_from('<' + fmt * n, data, start + i * stride)
        out.append(v if n > 1 else v[0])
    return out


def mat_mul(a, b):  # 4x4, column-major lists like glTF
    return [sum(a[k * 4 + r] * b[c * 4 + k] for k in range(4)) for c in range(4) for r in range(4)]


def local_matrix(node):
    if 'matrix' in node:
        return node['matrix']
    tx, ty, tz = node.get('translation', [0, 0, 0])
    x, y, z, w = node.get('rotation', [0, 0, 0, 1])
    sx, sy, sz = node.get('scale', [1, 1, 1])
    # T * R * S, column-major
    r = [1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w),
         2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w),
         2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y)]
    return [r[0] * sx, r[1] * sx, r[2] * sx, 0,
            r[3] * sy, r[4] * sy, r[5] * sy, 0,
            r[6] * sz, r[7] * sz, r[8] * sz, 0,
            tx, ty, tz, 1]


def transform_point(m, p):
    return tuple(m[r] * p[0] + m[4 + r] * p[1] + m[8 + r] * p[2] + m[12 + r] for r in range(3))


def transform_normal(m, n):
    # inverse-transpose of the 3x3 part keeps normals perpendicular under non-uniform scale
    a, b, c = m[0], m[4], m[8]
    d, e, f = m[1], m[5], m[9]
    g, h, i = m[2], m[6], m[10]
    co = [e * i - f * h, f * g - d * i, d * h - e * g,
          c * h - b * i, a * i - c * g, b * g - a * h,
          b * f - c * e, c * d - a * f, a * e - b * d]  # cofactor matrix = det * inverse-transpose
    x = co[0] * n[0] + co[3] * n[1] + co[6] * n[2]
    y = co[1] * n[0] + co[4] * n[1] + co[7] * n[2]
    z = co[2] * n[0] + co[5] * n[1] + co[8] * n[2]
    det = a * co[0] + b * co[1] + c * co[2]
    length = math.copysign(math.sqrt(x * x + y * y + z * z) or 1.0, det)  # a mirroring matrix (det < 0) would flip it
    return (x / length, y / length, z / length)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('gltf')
    ap.add_argument('obj')
    ap.add_argument('--skip', nargs='*', default=[])
    args = ap.parse_args()

    folder = os.path.dirname(os.path.abspath(args.gltf))
    gltf = json.load(open(args.gltf))
    buffers = [open(os.path.join(folder, b['uri']), 'rb').read() for b in gltf['buffers']]
    materials = [m.get('name', 'material%d' % i) for i, m in enumerate(gltf.get('materials', []))]

    out = open(args.obj, 'w')
    out.write('# converted from %s by tools/gltf2obj.py (shape only)\n' % os.path.basename(args.gltf))
    base = 1  # OBJ indices start at 1 and count across the whole file
    triangles = 0

    def visit(node_index, parent):
        nonlocal base, triangles
        node = gltf['nodes'][node_index]
        world = mat_mul(parent, local_matrix(node))
        if 'mesh' in node:
            for prim in gltf['meshes'][node['mesh']]['primitives']:
                if prim.get('mode', 4) != 4:  # triangles only
                    continue
                material = materials[prim['material']] if 'material' in prim else 'default'
                if any(word.lower() in material.lower() for word in args.skip):
                    continue
                pos = read_accessor(gltf, buffers, prim['attributes']['POSITION'])
                nor = read_accessor(gltf, buffers, prim['attributes']['NORMAL'])
                idx = read_accessor(gltf, buffers, prim['indices']) if 'indices' in prim else list(range(len(pos)))

                out.write('o %s\nusemtl %s\n' % (node.get('name', 'node%d' % node_index).replace(' ', '_'),
                                                 material.replace(' ', '_')))
                for p in pos:
                    out.write('v %.6g %.6g %.6g\n' % transform_point(world, p))
                for n in nor:
                    out.write('vn %.3g %.3g %.3g\n' % transform_normal(world, n))
                for t in range(0, len(idx) - 2, 3):
                    a, b, c = idx[t] + base, idx[t + 1] + base, idx[t + 2] + base
                    out.write('f %d//%d %d//%d %d//%d\n' % (a, a, b, b, c, c))
                base += len(pos)
                triangles += len(idx) // 3
        for child in node.get('children', []):
            visit(child, world)

    identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
    for root in gltf['scenes'][gltf.get('scene', 0)]['nodes']:
        visit(root, identity)
    out.close()
    print('%s: %d triangles, materials: %s' % (args.obj, triangles, ', '.join(materials)))


if __name__ == '__main__':
    main()
