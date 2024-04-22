from os import path
from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.scm import Git, Version
from conan.tools.env import Environment
from conan.tools.files import copy, load, save, update_conandata

required_conan_version = ">=2.0.0"


class DdsFmuConan(ConanFile):
    name = "dds-fmu"
    author = "Joakim Haugen"
    description = "DDS-FMU mediator"
    license = "MPL-2.0"
    url = "https://gitlab.sintef.no/co-simulation/dds-fmu"
    topics = ("Co-simulation", "FMU", "DDS", "OMG-DDS")
    package_type = "shared-library"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "fPIC": [True, False],
        "with_tools": [True, False],
        "with_doc": [True, False]
    }
    default_options = {
        "fPIC": True,
        "with_tools": True,
        "with_doc": False
    }

    @property
    def _min_cppstd(self):
        return 17

    @property
    def _compilers_minimum_version(self):
        return {
            "msvc": "14.1",
            "gcc": "8.1",
            "clang": "7",
            "apple-clang": "10",
        }

    @property
    def _with_tests(self):
        return not self.conf.get("tools.build:skip_test", default=True)

    def export(self):
        git = Git(self, self.recipe_folder)
        scm_url, scm_commit = git.get_url_and_commit()
        update_conandata(self, {"sources": {"commit": scm_commit, "url": scm_url}})

    def set_version(self):
        self.version = \
            load(self, path.join(self.recipe_folder, "version.txt").strip())

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("cppfmu/1.0.0@sintef/stable")
        self.requires("fast-dds/2.11.2")
        self.requires("stduuid/1.2.3")
        self.requires("rapidxml/1.13")
        self.requires("eprosima-xtypes/cci.20230615@sintef/stable")

        if self.options.with_tools:
            self.requires("kuba-zip/0.3.2")
            self.requires("taywee-args/6.4.6")

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, self._min_cppstd)
        minimum_version = self._compilers_minimum_version.get(str(self.settings.compiler), False)
        if minimum_version and Version(self.settings.compiler.version) < minimum_version:
            raise ConanInvalidConfiguration(
                f"{self.ref} requires C++{self._min_cppstd}, which your compiler does not support."
            )

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.18.0 <4]")
        self.tool_requires("fmu-build-helper/1.0.0@sintef/stable")
        if self._with_tests:
            self.tool_requires("fmu-compliance-checker/2.0.4@sintef/stable")
            self.test_requires("gtest/1.13.0")
        if self.options.with_doc:
            self.tool_requires("doxygen/1.9.4")
            if self.settings.os == "Windows":
                self.tool_requires("strawberryperl/5.32.1.1")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["DDSFMU_WITH_TOOLS"] = self.options.with_tools
        tc.variables["DDSFMU_WITH_DOC"] = self.options.with_doc
        tc.generate()

        deps = CMakeDeps(self)
        deps.build_context_activated = ["fmu-build-helper"]
        deps.build_context_build_modules = ["fmu-build-helper"]
        if self._with_tests:
            deps.build_context_activated.append("fmu-compliance-checker")
            deps.build_context_build_modules.append("fmu-compliance-checker")
        deps.generate()

        deplist = []
        for require, dep in self.dependencies.items():
            if require.build or require.test:
                continue
            deplist.append((dep.ref.name, dep.license))
            if dep.package_folder and path.exists(path.join(dep.package_folder, "licenses")):
                copy(self, "*", path.join(dep.package_folder, "licenses"),
                     path.join(self.build_folder, "licenses", dep.ref.name), keep_path=True)

            if dep.ref.name == "cppfmu" and len(dep.cpp_info.srcdirs) > 0:
                copy(self, "fmi_functions.cpp", dep.cpp_info.srcdirs[0],
                    path.join(self.build_folder, dep.ref.name), keep_path=False)

        license_txt = path.join(self.build_folder, "licenses", "licenses.txt")
        save(self, license_txt,
             "Licenses for `dds-fmu` and its dependencies are listed below.\n\n")

        cols1 = 20
        cols2 = 30
        save(self, license_txt, f"| Library{' '*(cols1-7)}"
             f"| License{' '*(cols2-7)}|\n|{'-'*(cols1+1)}|{'-'*(cols2+1)}|\n", append=True)
        save(self, license_txt, f"| {self.name}{' '*(max(cols1-len(self.name),0))}|"
             f" {self.license}{' '*(max(cols2-len(self.license),0))}|\n", append=True)
        copy(self, "LICENSE", self.recipe_folder, path.join(self.build_folder, "licenses"))

        for dep, lic in sorted(deplist):
            len_lic_str = len(lic) if type(lic) is str else cols2-2
            save(self, license_txt, f"| {dep}{' '*(max(cols1-len(dep),0))}|"
                 f" {lic}{' '*(max(cols2-len_lic_str,0))}|\n", append=True)

        license_md = path.join(self.build_folder, "gen_md", "licenses.md")
        save(self, license_md, f"# Licenses {{#sec_licenses}}\n{load(self, license_txt)}")

    def build(self):
        cmake = CMake(self)
        cmake.configure()

        if self.options.with_doc:
            cmake.build(target="doc")
            cmake.install(component="doc")

        cmake.build()

        if self._with_tests:
            env = Environment()
            env.define("CTEST_OUTPUT_ON_FAILURE", "ON")
            with env.vars(self).apply():
                cmake.test()
