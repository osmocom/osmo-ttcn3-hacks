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

    # Implicitly add .cc files
    if "IPL4asp_PortType" in ret:
        ret += ["IPL4asp_PT"]
    if "RTP_CodecPort" in ret:
        ret += ["IPL4asp_discovery"]

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
    elif file.endswith(".cc") or file.endswith(".c") or file.endswith(".l"):
        ret = get_imports_cc(file)
    elif file.endswith(".hh") or file.endswith(".h"):
        # Header file from /usr/share/titan
        ret = []
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
            if import_path.endswith(".y"):
                # ignore
                continue
            result += [import_path]

        assert len(result) <= 1, f"unexpected result for {pattern}:\n{result}"

        if len(result) == 1:
            ret = result[0]
            break

    if not ret:
        patterns_header_only = patterns + [
            f"/usr/include/titan/{import_name}.h",
            f"/usr/include/titan/{import_name}.hh",
        ]
        for pattern in patterns_header_only:
            result = glob.glob(pattern)
            if len(result) == 1:
                ret = result[0]
                break
            elif len(result) > 1:
                raise RuntimeError("Unexpectedly found more than 1 result: {result}")

    assert ret, f"\n\nfind_import_file failed for input_name:\n  {import_name}\npatterns tried:\n  {patterns}\n"

    cache_import_files[import_name] = ret
    return ret


def find_all_lib_files(lib_file):
    ret = [lib_file]
    # Implicit rules
    rules = {
        "_Templates.ttcn": ["_EncDec.cc"],
        "RSPRO_Types.ttcn": ["RSPRO_EncDec.cc"],
        "SDP_Types.ttcn": [
            "SDP_EncDec.cc",
            "SDP_parse_.tab.c",
            "lex.SDP_parse_.c",
        ],
        "IP_Types.ttcn": ["IP_EncDec.cc"],
        "IuUP_Types.ttcn": ["IuUP_EncDec.cc"],
        "RTP_Types.ttcn": ["RTP_EncDec.cc"],
        # e.g. NativeFunctions.ttcn -> NativeFunctionDefs.cc
        "s.ttcn": ["Defs.cc"],
        # e.g. M3UA_CodecPort_CtrlFunct.ttcn -> M3UA_CodecPort_CtrlFunctDef.cc
        "Funct.ttcn": ["FunctDef.cc", "Functdef.cc"],
        # e.g. TCCConversion_Functions.ttcn -> TCCConversion.cc
        "_Functions.ttcn": [".cc"],
        # e.g. TELNETasp_PortType.ttcn -> TELNETasp_PT.cc
        "TELNETasp_PortType.ttcn": ["TELNETasp_PT.cc"],
    }

    for end, end_ccs in rules.items():
        if not lib_file.endswith(end):
            continue
        for end_cc in end_ccs:
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
                if dep not in done_names and dep != lib_name:
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

    ret = ""

    targets_done = []

    # Libraries
    for lib in libraries_list:
        for source in lib["sources"]:
            if source.startswith("/usr/include/titan"):
                continue

            source = os.path.relpath(source, ttcn3_hacks_dir)
            source_ext = os.path.splitext(source)[1]
            source_noext = os.path.splitext(source)[0]
            o_name = f"$(BUILDDIR)/{source_noext}.o"

            if o_name in targets_done:
                continue
            targets_done += [o_name]

            deps = []
            if source.endswith(".cc") or source.endswith(".c"):
                cc_name = source
            else:
                cc_name = f"$(BUILDDIR)/{source_noext}.cc"

            deps = []
            for dep in lib["link_deps"]:
                dep_lib = libraries[dep]
                lib_sources = copy.copy(dep_lib["sources"]) + dep_lib["sources_deps"]
                for lib_source in lib_sources:
                    lib_source = os.path.relpath(lib_source, ttcn3_hacks_dir)
                    lib_source_ext = os.path.splitext(lib_source)[1]
                    lib_source_noext = os.path.splitext(lib_source)[0]
                    if lib_source_ext in [".ttcnpp", ".ttcn", ".asn"]:
                        lib_source = f"$(BUILDDIR)/{lib_source_noext}.cc"  # FIXME: .hh?
                    if lib_source not in deps:
                        deps += [lib_source]

            srcdir = os.path.dirname(source)
            builddir = f"$(BUILDDIR)/{srcdir}"

            ret += f"{o_name}: \\\n"
            ret += f"\t\t{cc_name} \\\n"
            for dep in deps:
                ret += f"\t\t{dep} \\\n"
            ret += f"\t\t$(NULL)\n"
            ret += f"\t@echo '  CC      {cc_name.replace('$(BUILDDIR)/', '')}'\n"
            ret += "\t$(Q)$(CPP_COMPILER) -o $@ \\\n"
            ret += f"\t\t-I./{srcdir} \\\n"
            ret += "\t\t-I./deps/titan.TestPorts.SCTPasp/src/ \\\n"
            ret += f"\t\t-I{builddir} \\\n"
            ret += "\t\t$<\n"
            ret += "\n"

            # Below we generate rules for .ttcn -> .cc. If we already have a
            # .cc file, this must be skipped.
            if source_ext in [".cc", ".c", ".hh", ".h"]:
                continue

            # Generate additional rules for .ttcnpp -> .ttcn files (need to run the
            # pre-processor on those)
            for source_dep in lib["sources_deps"]:
                if not source_dep.endswith(".ttcnpp"):
                    continue
                source_dep = os.path.relpath(source_dep, ttcn3_hacks_dir)
                source_dep_noext = os.path.splitext(source_dep)[0]
                source_dep_ttcn = f"{source_dep}.ttcn"

                if source_dep_ttcn in targets_done:
                    continue
                targets_done += [source_dep_ttcn]

                builddir = f"$(BUILDDIR)/{os.path.dirname(source_dep)}"
                ret += f"$(BUILDDIR)/{source_dep_noext}.ttcn: {source_dep}\n"
                ret += f"\t@echo '  TTCNPP  {source_dep}'\n"
                ret += f"\t@mkdir -p {builddir}\n"
                ret += "\t$(Q)$(PREPROCESSOR) \\\n"
                ret += "\t\t$< \\\n"
                ret += "\t\t-o $@ \\\n"
                ret += "\t\t$(NULL)\n"
                ret += "\n"

            # Prepare source_deps to be used in the .ttcn -> .cc rule below
            source_deps = []
            for source_dep in lib["sources_deps"]:
                source_dep_noext = os.path.splitext(source_dep)[0]
                source_dep_ext = os.path.splitext(source_dep)[1]
                if source_dep_ext == ".ttcnpp":
                    source_dep = f"{source_dep_noext}.ttcn"
                    source_deps += [f"$(BUILDDIR)/{os.path.relpath(source_dep, ttcn3_hacks_dir)}"]
                elif source_dep_ext in [".cc", ".hh", ".c", ".h"]:
                    continue
                else:
                    source_dep_new = os.path.relpath(source_dep, ttcn3_hacks_dir)
                    if source_dep_new not in source_deps and source_dep_new != source:
                        source_deps += [source_dep_new]

            if source.endswith(".ttcnpp"):
                source = f"{os.path.splitext(source)[0]}.ttcn"
                source = f"$(BUILDDIR)/{os.path.relpath(source, ttcn3_hacks_dir)}"

            # Generate the .ttcn -> .cc rule
            ret += f"{cc_name}: \\\n"
            ret += f"\t\t{source} \\\n"
            for source_dep in source_deps:
                ret += f"\t\t{source_dep} \\\n"
            ret += "\t\t$(NULL)\n"
            ret += f"\t@echo '  TTCN    {source}'\n"
            ret += f"\t@mkdir -p {builddir}\n"
            ret += "\t$(Q)$(TTCN3_COMPILER) \\\n"
            ret += f"\t\t-o {builddir} \\\n"
            ret += f"\t\t{source} \\\n"
            for source_dep in source_deps:
                ret += f"\t\t{source_dep} \\\n"
            ret += "\t\t$(NULL)\n"
            ret += "\n"

    # Testsuites
    for testsuite_name, testsuite in testsuites.items():
        for source in testsuite["sources"]:
            source = os.path.relpath(source, ttcn3_hacks_dir)
            source_noext = os.path.splitext(source)[0]
            o_name = f"$(BUILDDIR)/{source_noext}.o"
            cc_name = f"$(BUILDDIR)/{source_noext}.cc"
            srcdir = os.path.dirname(source)
            builddir = f"$(BUILDDIR)/{srcdir}"

            if o_name in targets_done:
                continue
            targets_done += [o_name]

            ret += f"{o_name}: {cc_name}\n"
            ret += f"\t@echo '  CC      {source_noext}.cc'\n"
            ret += "\t$(Q)$(CPP_COMPILER) -o $@ \\\n"
            ret += f"\t\t-I./{srcdir} \\\n"
            ret += f"\t\t-I{builddir} \\\n"

            include_dirs_todo = []

            for dep in testsuite["link_deps"]:
                lib = libraries[dep]
                lib_sources = copy.copy(lib["sources"]) + lib["sources_deps"]
                for lib_source in lib_sources:
                    lib_source = os.path.relpath(lib_source, ttcn3_hacks_dir)
                    lib_source_dir = os.path.dirname(lib_source)
                    if lib_source_dir not in include_dirs_todo:
                        include_dirs_todo += [lib_source_dir]

            for include_dir in include_dirs_todo:
                ret += f"\t\t-I$(BUILDDIR)/{include_dir} \\\n"
                ret += f"\t\t-I./{include_dir} \\\n"

            ret += "\t\t$<\n"
            ret += "\n"

            ret += f"{cc_name}: \\\n"
            ret += f"\t\t{source} \\\n"
            for dep in testsuite["link_deps"]:
                lib = libraries[dep]
                source_noext = os.path.splitext(lib["sources"][0])[0]
                source_noext = os.path.relpath(source_noext, ttcn3_hacks_dir)
                o_name = f"$(BUILDDIR)/{source_noext}.o"
                ret += f"\t\t{o_name} \\\n"
            ret += "\t\t$(NULL)\n"
            ret += f"\t@echo '  TTCN    {source}'\n"
            ret += f"\t@mkdir -p {builddir}\n"
            ret += "\t$(Q)$(TTCN3_COMPILER) \\\n"
            ret += f"\t\t-o {builddir} \\\n"

            sources_todo = [source]

            for dep in testsuite["link_deps"]:
                lib = libraries[dep]
                lib_sources = copy.copy(lib["sources"]) + lib["sources_deps"]
                for lib_source in lib_sources:
                    lib_source = os.path.relpath(lib_source, ttcn3_hacks_dir)
                    lib_source_ext = os.path.splitext(lib_source)[1]
                    if lib_source_ext in [".cc", ".hh", ".c", ".h"]:
                        continue
                    if lib_source.endswith(".ttcnpp"):
                        lib_source_noext = os.path.splitext(lib_source)[0]
                        lib_source = f"$(BUILDDIR)/{lib_source_noext}.ttcn"
                    if lib_source not in sources_todo:
                        sources_todo += [lib_source]

            for source_todo in sources_todo:
                ret += f"\t\t{source_todo} \\\n"
            ret += "\t\t$(NULL)\n"
            ret += "\n"

        testsuite_bin = f"$(BUILDDIR)/{testsuite_name}/{testsuite_name}"

        ret += f"{testsuite_bin}: \\\n"

        sources = copy.copy(testsuite["sources"])

        link_deps = []
        for dep in testsuite["link_deps"]:
            link_deps += [dep]
            link_deps += libraries[dep]["link_deps"]
        for dep in link_deps:
            lib = libraries[dep]
            for source in lib["sources"]:
                if source not in sources:
                    sources += [source]

        for source in sources:
            source = os.path.relpath(source, ttcn3_hacks_dir)
            source_ext = os.path.splitext(source)[1]
            if source_ext in [".h", ".hh"]:
                continue
            source_noext = os.path.splitext(source)[0]
            o_name = f"$(BUILDDIR)/{source_noext}.o"
            ret += f"\t\t{o_name} \\\n"
        ret += "\t\t$(NULL)\n"

        ret += f"\t@echo '  LINK    {testsuite_name}'\n"
        ret += "\t$(Q)$(LINKER) \\\n"
        ret += "\t\t-o $@ \\\n"
        ret += "\t\t$^\n"
        ret += "\n"

        ret += f"{testsuite_name}: {testsuite_bin}\n"
        ret += "\n"

    return ret


def rewrite_makefile(new_str):
    mb_path = os.path.join(ttcn3_hacks_dir, "Makefile")
    start = "### start of section generated by contrib/update_makefile.py ###"
    end = "### end of section generated by contrib/update_makefile.py ###"

    with open(mb_path, "r") as f:
        mb_old = f.read()

    header, rest = mb_old.split(start, 1)
    _, footer = rest.split(end, 1)

    mb_new = f"{header}{start}\n{new_str}\n{end}{footer}"

    with open(mb_path, "w") as file:
        file.write(mb_new)


def main():
    new_str = new_str_generate()
    rewrite_makefile(new_str)
    print("Done")


main()
