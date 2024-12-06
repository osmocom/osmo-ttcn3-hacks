#!/usr/bin/env python3
import os.path
import sys
import glob
import copy
import fnmatch
import pathlib

ttcn3_hacks_dir = os.path.realpath(f"{__file__}/../..")

cache_import_files = {}
cache_imports = {}

dirs_without_testsuites = [
    "COMMON",
    "ROHC_CNL113426_LATEST",
    "_*",
    "asn-test",
    "contrib",
    "deps",
    "doc",
    "library",
]

# Commented out imports
imports_ignore = [
    # deps/titan.ProtocolModules.ROSE/src/Remote_Operations_Information_Objects.asn
    "Remote_Operations_Useful_Definitions",
    # mme/snow-3g.c
    "f8",
]


def get_imports_ttcn(file):
    ret = []

    # errors=ignore: some ttcn files can't be parsed as utf-8, this can be
    # ignored since we only care about the imports here
    with open(os.path.join(ttcn3_hacks_dir, file), "r", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line.startswith("import from "):
                continue
            line_split = line.split(" ")
            assert len(line_split) >= 4, f"{file}: can't parse line: {line}"
            import_name = line_split[2]
            ret += [import_name]

    return ret


def get_imports_asn(file):
    ret = []

    asn_next_line_is_import = False

    with open(os.path.join(ttcn3_hacks_dir, file), "r", errors="ignore") as f:
        for line in f:
            line = line.strip()

            # Comment
            if line.startswith("-- "):
                continue
            # IMPORT on previous line
            elif asn_next_line_is_import:
                asn_next_line_is_import = False
                import_name = line.split(" ")[0].rstrip(";")
                import_name = import_name.replace("-", "_")
            # Unrelated line that looks similar to import
            elif fnmatch.fnmatch(line, "*(*FROM*"):
                continue
            # Imports like:
            #   FROM PKIX1Explicit88 { iso(1) identified-organization(3)
            #   IMPORTS Certificate, CertificateList, Time FROM PKIX1Explicit88 {...
            elif fnmatch.fnmatch(line, "*FROM *"):
                line_split = line.split("FROM ")
                assert len(line_split) == 2, f"{file}: can't parse line: {line}"

                import_name = line_split[1].split(" ")[0].rstrip(";")
                import_name = import_name.replace("-", "_")
            # Imports like:
            #   FROM\nModule-Name
            elif fnmatch.fnmatch(line, "*FROM"):
                asn_next_line_is_import = True
                continue
            else:
                continue
            ret += [import_name]
    return ret


def get_imports_cc(file):
    ret = []
    stem = pathlib.Path(file).stem

    with open(os.path.join(ttcn3_hacks_dir, file), "r") as f:
        for line in f:
            line = line.strip()
            if line.startswith('#include "'):
                line_split = line.split('"')
                assert len(line_split) >= 3, f"{file}: can't parse line: {line}"
                import_name = line_split[1]
                import_name = import_name.replace(".hh", "").replace(".h", "")
            else:
                continue
            # Don't count cc files including their own h/hh as dependency
            if stem == import_name:
                continue
            ret += [import_name]

    return ret


def get_imports(file):
    global cache_imports
    if file in cache_imports:
        return cache_imports[file]

    print(f"  {os.path.relpath(file, ttcn3_hacks_dir)}")

    ret = []
    if file.endswith(".ttcn") or file.endswith(".ttcnpp"):
        ret = get_imports_ttcn(file)
    elif file.endswith(".asn"):
        ret = get_imports_asn(file)
    elif file.endswith(".cc") or file.endswith(".c"):
        ret = get_imports_cc(file)
    else:
        print("ERROR: can't parse imports for this file type")
        sys.exit(1)

    ret_final = []
    for import_name in ret:
        if import_name in imports_ignore:
            continue
        if import_name in ret_final:
            continue
        ret_final += [import_name]
    ret = ret_final

    cache_imports[file] = ret
    return ret


def parse_testsuites():
    ret = {}
    for testsuite_path in sorted(glob.glob(f"{ttcn3_hacks_dir}/*/")):
        testsuite_name = os.path.relpath(testsuite_path, ttcn3_hacks_dir).rstrip("/")
        is_testsuite_dir = True
        for pattern in dirs_without_testsuites:
            if fnmatch.fnmatch(testsuite_name, pattern):
                is_testsuite_dir = False
        if not is_testsuite_dir:
            continue

        print(f"Parsing testsuite {testsuite_name}")

        sources = []
        sources_pattern = os.path.join(ttcn3_hacks_dir, testsuite_name, "*.ttcn")
        for source_path in sorted(glob.glob(sources_pattern)):
            if os.path.islink(source_path):
                # Symlink created by Makefile
                continue
            sources += [source_path]

        if not sources:
            print(f"ERROR: No ttcn source files found! Consider adding {testsuite_name} to dirs_without_testsuites.")
            sys.exit(1)

        link_deps = []
        for source in sources:
            for link_dep in get_imports(source):
                if link_dep not in link_deps:
                    link_deps += [link_dep]

        ret[testsuite_name] = {
            "sources": sources,
            "link_deps": link_deps,
        }
    return ret


def find_import_file(import_name, testsuite=None):
    global cache_import_files
    if import_name in cache_import_files:
        return cache_import_files[import_name]

    patterns = [
        os.path.join(ttcn3_hacks_dir, f"library/{import_name}.*"),
        os.path.join(ttcn3_hacks_dir, f"library/*/{import_name}.*"),
        os.path.join(ttcn3_hacks_dir, f"deps/*/src/{import_name}.*"),
        os.path.join(ttcn3_hacks_dir, f"deps/*/ttcn3/{import_name}.*"),
        # RSPRO_Types from remsim gets used in stp
        os.path.join(ttcn3_hacks_dir, f"remsim/{import_name}.ttcn"),
        os.path.join(ttcn3_hacks_dir, f"remsim/{import_name}.asn"),
    ]

    if testsuite:
        patterns += [
            os.path.join(ttcn3_hacks_dir, f"{testsuite}/{import_name}.ttcn"),
            os.path.join(ttcn3_hacks_dir, f"{testsuite}/{import_name}.c"),
        ]

    ret = None
    for pattern in patterns:
        result = []
        for import_path in glob.glob(pattern):
            if os.path.islink(import_path):
                # Symlink created by Makefile
                continue
            if import_path.endswith(".hh"):
                # Take the .cc file instead
                continue
            if import_path.endswith(".h"):
                # Take the .c file instead
                continue
            result += [import_path]

        assert len(result) <= 1, f"unexpected result for {pattern}:\n{result}"

        if len(result) == 1:
            ret = result[0]
            break

    assert ret, f"\n\nfind_import_file failed for input_name:\n  {import_name}\npatterns tried:\n  {patterns}\n"

    cache_import_files[import_name] = ret
    return ret


def find_all_lib_files(lib_file):
    ret = [lib_file]
    rules = {
        # e.g. NativeFunctions.ttcn -> NativeFunctionDefs.cc
        "s.ttcn": "Defs.cc",
        # e.g. M3UA_CodecPort_CtrlFunct.ttcn -> M3UA_CodecPort_CtrlFunctDef.cc
        "Funct.ttcn": "FunctDef.cc",
    }

    for end, end_cc in rules.items():
        if not lib_file.endswith(end):
            continue
        extra = lib_file.replace(end, end_cc)
        if os.path.exists(extra):
            ret += [extra]

    return ret


def get_sources_deps(lib_imports, testsuite):
    queue = copy.copy(lib_imports)
    done = []
    sources_deps = []
    while queue:
        import_name = queue.pop()
        done += [import_name]

        import_file = find_import_file(import_name, testsuite)
        lib_imports = get_imports(import_file)
        for i in lib_imports:
            if i not in done:
                queue += [i]

        if import_file not in sources_deps:
            sources_deps += [import_file]

    return sources_deps, done


def parse_libraries(testsuites):
    queue = {}
    ret = {}
    for testsuite_name, testsuite in testsuites.items():
        for lib in testsuite["link_deps"]:
            queue[lib] = testsuite_name

    while queue:
        lib, testsuite_name = queue.popitem()
        print(f"Parsing testsuite {testsuite_name} import {lib}")
        lib_file = find_import_file(lib, testsuite_name)
        print(f"  {os.path.relpath(lib_file, ttcn3_hacks_dir)}")

        sources = find_all_lib_files(lib_file)
        lib_imports = []
        for source in sources:
            for import_name in get_imports(source):
                if import_name not in lib_imports:
                    lib_imports += [import_name]
        sources_deps, lib_imports = get_sources_deps(lib_imports, testsuite_name)

        lib_imports_unique = []
        for import_name in lib_imports:
            if import_name not in lib_imports_unique:
                lib_imports_unique += [import_name]
        lib_imports = lib_imports_unique

        for dep in lib_imports:
            if dep not in queue and dep not in ret:
                queue[dep] = testsuite_name

        if not sources:
            print("ERROR: no sources found")
            sys.exit(1)

        ret[lib] = {"sources": sources, "link_deps": lib_imports, "sources_deps": sources_deps}

    return ret


def libraries_to_list(libraries_dict):
    print("Resolving library order...")
    ret = []
    done_names = []
    queue = dict(sorted(libraries_dict.items()))
    while queue:
        stuck = True
        for lib_name in queue.keys():
            lib = queue[lib_name]
            deps_found = True
            for dep in lib["link_deps"]:
                if dep not in done_names:
                    deps_found = False
                    break
            if not deps_found:
                continue
            stuck = False

            print(f"  {lib_name}")
            lib["name"] = lib_name
            ret += [lib]
            done_names += [lib_name]
            queue.pop(lib_name)
            break

        if not stuck:
            continue

        print("ERROR: can't resolve remaining library order, fix the circular import:")
        for lib_name in queue.keys():
            todo = []
            for dep in queue[lib_name]["link_deps"]:
                if dep not in done_names:
                    todo += [dep]
            print(f" - {lib_name}: {todo}")
        sys.exit(1)
    return ret


def new_str_generate():
    testsuites = parse_testsuites()
    libraries = parse_libraries(testsuites)
    libraries_list = libraries_to_list(libraries)

    # Testsuites
    ret = "testsuites = {\n"
    for testsuite_name, testsuite in testsuites.items():
        ret += f"    '{testsuite_name}': {{\n"
        for key in ["sources", "link_deps"]:
            if not testsuite[key]:
                continue
            ret += f"      '{key}': [\n"
            for element in sorted(testsuite[key]):
                if element.startswith("/"):
                    element = os.path.relpath(element, ttcn3_hacks_dir)
                ret += f"        '{element}',\n"
            ret += "        ],\n"
        ret += "      },\n"
    ret += "    }\n"

    # Libraries
    ret += "libs = [\n"
    for lib in libraries_list:
        ret += "  {\n"
        ret += f"    'name': '{lib['name']}',\n"

        for key in ["sources", "sources_deps", "link_deps"]:
            if not lib[key]:
                continue
            ret += f"    '{key}': [\n"
            for element in sorted(lib[key]):
                if element.startswith("/"):
                    element = os.path.relpath(element, ttcn3_hacks_dir)
                ret += f"      '{element}',\n"
            ret += "      ],\n"
        ret += "    },\n"
    ret += "]"
    return ret


def rewrite_meson_build(new_str):
    mb_path = os.path.join(ttcn3_hacks_dir, "meson.build")
    start = "### start of section generated by contrib/update_meson_build.py ###"
    end = "### end of section generated by contrib/update_meson_build.py ###"

    with open(mb_path, "r") as f:
        mb_old = f.read()

    header, rest = mb_old.split(start, 1)
    _, footer = rest.split(end, 1)

    mb_new = f"{header}{start}\n{new_str}\n{end}{footer}"

    with open(mb_path, "w") as file:
        file.write(mb_new)


def main():
    new_str = new_str_generate()
    rewrite_meson_build(new_str)
    print("Done")


main()
