#!/usr/bin/env python3
"""
Create agar/* directory structure in build directory with symlinks to source headers.

This script creates the directory structure that matches the #include <agar/*/...>
paths used in the source code, symlinking to the actual source and generated headers.
"""

import os
import sys

def create_symlink(src, dst):
    """Create a relative symlink, removing any existing file/link."""
    if os.path.exists(dst) or os.path.islink(dst):
        os.remove(dst)

    # Create relative symlink
    dst_dir = os.path.dirname(dst)
    rel_src = os.path.relpath(src, dst_dir)
    os.symlink(rel_src, dst)
    return rel_src

def main():
    if len(sys.argv) < 3:
        print("Usage: create_config_symlinks.py BUILD_DIR SOURCE_DIR", file=sys.stderr)
        sys.exit(1)

    build_dir = sys.argv[1]
    source_dir = sys.argv[2]

    # Library directories to symlink
    libraries = ['core', 'gui', 'math', 'net', 'vg', 'sg', 'sk', 'au', 'map']

    # Create agar directory structure
    agar_dir = os.path.join(build_dir, 'agar')
    os.makedirs(agar_dir, exist_ok=True)

    # Create symlinks for each library's headers
    for lib in libraries:
        src_lib_dir = os.path.join(source_dir, lib)

        # Skip if library directory doesn't exist
        if not os.path.isdir(src_lib_dir):
            continue

        dst_lib_dir = os.path.join(agar_dir, lib)

        # Create directory symlink to the source library directory
        if os.path.exists(dst_lib_dir) or os.path.islink(dst_lib_dir):
            os.remove(dst_lib_dir)

        rel_src = os.path.relpath(src_lib_dir, agar_dir)
        os.symlink(rel_src, dst_lib_dir)
        print(f"Created symlink: {dst_lib_dir} -> {rel_src}")

    # Create agar/config directory for generated config headers
    config_dir = os.path.join(agar_dir, 'config')
    os.makedirs(config_dir, exist_ok=True)

    # Create symlinks for all config header files
    # Include lowercase .h files and _mk_* marker files, but skip config.h
    for filename in os.listdir(build_dir):
        if (filename.endswith('.h') and
            filename != 'config.h' and
            filename.islower()):

            src = os.path.join(build_dir, filename)
            dst = os.path.join(config_dir, filename)

            # Skip if not a file
            if not os.path.isfile(src):
                continue

            rel_src = create_symlink(src, dst)
            print(f"Created config symlink: {dst} -> {rel_src}")

    print(f"\nAgar directory structure created at {agar_dir}")
    print(f"  - Library headers symlinked from source directories")
    print(f"  - Config headers symlinked from build directory")

if __name__ == '__main__':
    main()

