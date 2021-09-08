{ lib
, fetchFromGitHub
, pythonPackages
}:

with pythonPackages;

buildPythonPackage rec {
  version = "v0.10.1";
  pname = "west";

  src = fetchFromGitHub {
    owner = "zephyrproject-rtos";
    repo = "west";
    rev = version;
    sha256 = "sha256-ocNMfrh2uvcKUtGbXxv1xmvcNVhJUBR7Wq9n8nxuhXk";
  };

  propagatedBuildInputs = [
    colorama
    configobj
    packaging
    pyyaml
    pykwalify
  ];

  # pypi package does not include tests (and for good reason):
  # tests run under 'tox' and have west try to git clone repos (not sandboxable)
  doCheck = false;
  pythonImportsCheck = [
    "west"
  ];
}
