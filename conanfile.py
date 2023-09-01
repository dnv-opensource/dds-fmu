from os import path
from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.scm import Git, Version
from conan.tools.files import copy, load, update_conandata

required_conan_version = ">=1.53.0"

class DdsFmuConan(ConanFile):
    name = "dds-fmu"
    author = "Joakim Haugen"
    description = "DDS-FMU mediator"
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
        self.requires("xtypes/cci.20230530")

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

    def build(self):
        cmake = CMake(self)
        cmake.configure()

        if self.options.with_doc:
            cmake.build(target="doc")
            cmake.install(component="doc")

        cmake.build()

        if self._with_tests:
            cmake.test()
