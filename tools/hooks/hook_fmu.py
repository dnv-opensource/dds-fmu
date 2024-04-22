import shutil
from os import environ, path, scandir
from conan.tools.files import mkdir


def run_fast_scandir(dir, ext):    # dir: str, ext: list
    subfolders, files = [], []

    for f in scandir(dir):
        if f.is_dir():
            subfolders.append(f.path)
        if f.is_file():
            if path.splitext(f.name)[1].lower() in ext:
                files.append(f.path)

    for dir in list(subfolders):
        sf, f = run_fast_scandir(dir, ext)
        subfolders.extend(sf)
        files.extend(f)
    return subfolders, files


def pre_build(conanfile):
    """Create a directory in the local recipe folder for fmu

    That is, the directory will be created relative to the directory in which this file is
    located). The intention is that output fmu should be copied from the build folder to
    the created directory. Further, an attribute is set on the conanfile to pass the
    location of the directory to the build function.

    """
    assert conanfile

    if conanfile.name != "dds-fmu":
        return

    if environ.get("CI_PROJECT_DIR"):
        install_destination = path.join(environ.get("CI_PROJECT_DIR"), "fmus")
    else:
        install_destination = path.join(conanfile.recipe_folder, "fmus")

    conanfile.output.info(
        f"Setting up build so that fmus will be copied to '{install_destination}'")
    mkdir(conanfile, install_destination)
    conanfile.fmu_install_destination = install_destination


def post_build(conanfile):
    """ Copy fmu to project directory so CI can package it """

    if conanfile.name != "dds-fmu":
        return

    if conanfile.fmu_install_destination:
        subfolders, files = run_fast_scandir(conanfile.build_folder, [".fmu"])

        for a_file in files:
            shutil.copy(a_file, conanfile.fmu_install_destination)
            conanfile.output.info(
                f"Copied {a_file} to {conanfile.fmu_install_destination}")
