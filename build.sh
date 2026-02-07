#!/bin/bash
set -e

cd "$(dirname "$0")"

case "$1" in
    clean)
        rm -rf build
        echo "Cleaned build directory"
        ;;
    release)
        cmake -DCMAKE_BUILD_TYPE=Release -B build
        cmake --build build -j$(nproc)
        echo "Built: build/hyprlaunch.so + build/hyprlaunch-ui"
        ;;
    debug)
        cmake -DCMAKE_BUILD_TYPE=Debug -B build
        cmake --build build -j$(nproc)
        echo "Built (debug): build/hyprlaunch.so + build/hyprlaunch-ui"
        ;;
    install)
        $0 release
        sudo cp build/hyprlaunch.so /usr/lib/hyprland/plugins/
        sudo cp build/hyprlaunch-ui /usr/local/bin/
        echo "Installed plugin + UI binary"
        ;;
    *)
        cmake -DCMAKE_BUILD_TYPE=Release -B build
        cmake --build build -j$(nproc)
        echo "Built: build/hyprlaunch.so + build/hyprlaunch-ui"
        ;;
esac
