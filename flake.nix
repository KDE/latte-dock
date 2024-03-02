{
  description = "Latte Dock flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let system = "x86_64-linux";
        pkgs = import nixpkgs { inherit system; };
        latte = import ./default.nix { inherit pkgs; };
    in {

      packages."${system}".latte-dock = latte;
      devShells."${system}".default = latte;

    };
}
