from os.path import join
from os import environ
from conan import conan_version
from conan.tools.files import mkdir

def pre_build(conanfile):
    """Create a directory in the local recipe folder

    Create a directory in the local recipe folder (i.e. relative to the directory in which this file is located).
    The intention is that output from (gtest) unit test should be copied from the build folder to the created directory,
    which will facilitate parsing of the test results in CI.
    Further, an attribute is set on the conanfile to pass the location of the testoutput directory to the build funcion.

    """
    assert conanfile

    target = "dds-fmu"

    if target and conanfile.name != target:
        conanfile.output.info(f"Package {conanfile.name} is not target package {target}, skipping")

    if not (target and conanfile.name == target):
        return

    if environ.get("CI_PROJECT_DIR"):
        test_output_destination = join(environ.get("CI_PROJECT_DIR"), "testoutput")
    else:
        test_output_destination = join(conanfile.recipe_folder, "testoutput")

    conanfile.output.info(f"Test output files will be copied to '{test_output_destination}'")
    mkdir(conanfile, test_output_destination)
    conanfile.test_output_destination = test_output_destination
