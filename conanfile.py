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
    url = "https://gitlab.sintef.no/seaops/dds-fmu"
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
            "Visual Studio": "15.7",
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
        self.requires("cppfmu/1.0")
        self.requires("fast-dds/2.11.1")
        self.requires("stduuid/1.2.3")
        self.requires("rapidxml/1.13")
        self.requires("xtypes/cci.20230615")

        if self.options.with_tools:
            self.requires("kuba-zip/0.2.6")
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
        if self._with_tests:
            self.tool_requires("fmu-compliance-checker/2.0.4")
            self.test_requires("gtest/1.13.0")
        if self.options.with_doc:
            self.tool_requires("doxygen/1.9.4")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["DDSFMU_WITH_TOOLS"] = self.options.with_tools
        tc.variables["DDSFMU_WITH_DOC"] = self.options.with_doc
        tc.generate()

        deps = CMakeDeps(self)
        if self._with_tests:
            deps.build_context_activated = ["fmu-compliance-checker"]
            deps.build_context_build_modules = ["fmu-compliance-checker"]
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
             "Licenses for dds-fmu and dependencies are listed below.\n")

        save(self, license_txt, f"{self.name}: {self.license}\n\n", append=True)
        copy(self, "LICENSE", self.recipe_folder, path.join(self.build_folder, "licenses"))

        for dep, lic in sorted(deplist):
            save(self, license_txt, f"{dep}: {lic}\n", append=True)

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
