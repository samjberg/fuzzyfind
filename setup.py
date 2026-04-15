from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "fuzzyfind_functions",      # The name of your module
        ["src/fzf_main.cpp"],    # The source file(s)
    ),
]

setup(
    name="fuzzyfind_functions",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
