{
  description = "Latte Dock flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/38eff76eca85ed41f0630fe4f50cd92ba78310ff";
  };

  outputs = { self, nixpkgs }:
    let system = "x86_64-linux";
        pkgs = import nixpkgs { inherit system; };
        latte = import ./default.nix { inherit pkgs; };
    in {

      packages."${system}".default = latte;
      devShells."${system}".default = latte;

    };
}
