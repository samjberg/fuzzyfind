import pybind11
from fuzzyfind_functions import smith_waterman, fuzzy_search
import os, sys


home_path = os.path.expanduser('~')
coding_path = os.path.join(home_path, 'Coding')
python_path = os.path.join(coding_path, 'Python')
cpp_path = os.path.join(coding_path, 'C++')
