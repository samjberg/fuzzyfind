from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "0.0.1"

ext_modules = [
    Pybind11Extension(
        "fuzzyfind_functions",      # The name of your module
        ["src/fzf_main.cpp"],    # The source file(s)
    ),
]



setup(
    name="fuzzyfind_functions",
    version=__version__,
    author="Sam Berg",
    url="https://github.com/samjberg/fuzzyfind.git",
    description="A fuzzy finder filesystem search project in python backed in C++ using pybind11",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    python_requires=">=3.9",
)
