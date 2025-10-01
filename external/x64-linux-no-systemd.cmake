set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Disable systemd for Qt
set(VCPKG_FEATURE_FLAGS -systemd)
set(QT_FEATURE_systemd OFF)

