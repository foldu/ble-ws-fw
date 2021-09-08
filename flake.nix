{
  description = "A very basic flake";

  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }: {
    overlay = final: prev: {
      bossa = prev.callPackage ./nix/arduino-bossa.nix { };
    };
  } //
  flake-utils.lib.eachDefaultSystem (system: {
    devShell =
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ self.overlay ];
        };
        pythonEnv = pkgs.python38.withPackages (p: [
          (pkgs.callPackage ./nix/west.nix { pythonPackages = p; })
          p.pyelftools
        ]);
      in
      pkgs.mkShell {
        nativeBuildInputs = [
          pythonEnv
          pkgs.cmake
          pkgs.gcc-arm-embedded-10
          pkgs.ninja
          pkgs.bear
          pkgs.bossa
        ];
        ZEPHYR_TOOLCHAIN_VARIANT = "gnuarmemb";
        GNUARMEMB_TOOLCHAIN_PATH = "${pkgs.gcc-arm-embedded-10}";
        BOARD = "arduino_nano_33_ble";
      };
  });
}
