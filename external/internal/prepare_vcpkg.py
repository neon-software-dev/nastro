import os
import shutil
import subprocess

VCPKG_BASELINE_VERSION = "2025.07.25"

def ensure_vcpkg():
    if os.path.isdir('vcpkg'):
        print("vcpkg already exists, skipping")
        return

    subprocess.run(['git', 'clone', "https://github.com/microsoft/vcpkg.git"])
    os.chdir('vcpkg')
    
    subprocess.run(['git', 'checkout', VCPKG_BASELINE_VERSION])

    if os.name == 'nt':
        os.system('bootstrap-vcpkg.bat -disableMetrics')
    else:
        os.system('./bootstrap-vcpkg.sh --disableMetrics')

    # Install/copy custom target triplets
    shutil.copy('../x64-linux-no-systemd.cmake', './triplets/')

    print("[Prepared vcpkg]")
    

def install_dep(dep, args, linuxDynamic):
    depStr = dep
    
    # If we're on linux and the dependency should be linux dynamic, use the custom triplet
    if os.name != 'nt' and linuxDynamic:
    	depStr = dep + ' --triplet x64-linux-dynamic'
    elif args.no_systemd:
        depStr = depStr + ' --triplet x64-linux-no-systemd'

    print('[Installing: %s]' % (depStr))
    
    if os.name == 'nt':
        os.system('vcpkg install %s' % (depStr))
    else:
        os.system('./vcpkg install %s' % (depStr))


def install_dependencies(args):
    # Switch to vcpkg dir
    os.chdir('vcpkg')
	
    #Install dependencies
    install_dep('qtbase[widgets,gui]', args, False)
    install_dep('qtcharts', args, False)
    install_dep('gtest', args, False)

    print("[Prepared vcpkg]")


def prepare_vcpkg(args):
    external_dir = os.getcwd()

    print("[Ensuring vcpkg]")
    ensure_vcpkg()
    
    print("[Installing dependencies]")
    os.chdir(external_dir)
    install_dependencies(args)
    

