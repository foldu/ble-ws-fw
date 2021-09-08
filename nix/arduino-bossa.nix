{ lib
, stdenv
, fetchFromGitHub
, wxGTK30
, libX11
, readline
, git
}:

let
  # BOSSA needs a "bin2c" program to embed images.
  # Source taken from:
  # http://wiki.wxwidgets.org/Embedding_PNG_Images-Bin2c_In_C
  bin2c = stdenv.mkDerivation {
    name = "bossa-bin2c";
    src = ./bin2c.c;
    dontUnpack = true;
    buildPhase = "cc $src -o bin2c";
    installPhase = "mkdir -p $out/bin; cp bin2c $out/bin/";
  };

in
stdenv.mkDerivation {
  pname = "arduino-bossa";
  version = "1.9";

  src = fetchFromGitHub {
    #owner = "shumatech";
    #repo = "bossa";
    #rev = "3532de82efd28fadbabc2b258d84dddf14298107";
    #sha256 = "sha256-nbihNZAQdoATHH9oUdyWS4tuDeKaRe8WYMKlNEwaNlM=";
    owner = "arduino";
    repo = "BOSSA";
    rev = "89f3556a761833522cd93c199581265ad689310b";
    sha256 = "sha256-sBJ6QMd7cTClDnGCeOU0FT6IczEjqqRxCD7kef5GuY8=";
  };

  patches = [ ./bossa-no-applet-build.patch ];

  nativeBuildInputs = [ bin2c ];
  buildInputs = [ wxGTK30 libX11 readline git ];

  # Explicitly specify targets so they don't get stripped.
  makeFlags = [ "bin/bossac" "bin/bossash" "bin/bossa" ];
  NIX_CFLAGS_COMPILE = "-Wno-error=deprecated-declarations";

  installPhase = ''
    mkdir -p $out/bin
    cp bin/bossa{c,sh,} $out/bin/
  '';
}
