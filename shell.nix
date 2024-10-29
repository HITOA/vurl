with import <nixpkgs> {
  config.allowUnfree = true;
};
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [ 
    cmake
    gdb
    pkg-config
    shaderc

    wayland
    wayland-scanner

    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    libxkbcommon
    
    libGL
    vulkan-headers
    vulkan-tools
    vulkan-loader
    vulkan-validation-layers

    renderdoc
    rgp
  ];

  NIX_LD_LIBRARY_PATH = lib.makeLibraryPath [
    stdenv.cc.cc
    vulkan-loader
    wayland
    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    libxkbcommon
  ];

  NIX_LD = lib.fileContents "${stdenv.cc}/nix-support/dynamic-linker";

  #Volk need vulkan-loader in LD_LIBRARY_PATH
  shellHook = ''
    export "LD_LIBRARY_PATH=$NIX_LD_LIBRARY_PATH"
    cmake -B"build" -DCMAKE_BUILD_TYPE=Debug -DGLFW_BUILD_WAYLAND=ON -DVURL_BUILD_WSI_WAYLAND=ON .
  '';
}