import os
import argparse
import sys

subdir_path = os.path.join(os.path.dirname(__file__), 'internal')
sys.path.append(subdir_path)

from prepare_vcpkg import prepare_vcpkg

parser = argparse.ArgumentParser(description='Provides nastro dependencies')
parser.add_argument('--no-systemd', action='store_true')  # optional
args = parser.parse_args()

print("[Preparing dependencies]")

external_dir = os.getcwd()

#######
# vcpkg
os.chdir(external_dir)
prepare_vcpkg(args)

