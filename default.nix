{ pkgs ? import <nixpkgs> {} }:

let
  inherit (pkgs) stdenv kdePackages qt6 xorg;
in
stdenv.mkDerivation {
  pname = "latte-dock";
  version = "unstable-9999";

  src = ./.;

  buildInputs = with kdePackages; [
    plasma-activities plasma-sdk
    plasma-wayland-protocols qt6.full
    xorg.libpthreadstubs xorg.libXdmcp xorg.libSM
    wayland
    plasma-workspace plasma-workspace.dev
    plasma-desktop plasma-desktop.dev
    libplasma
  ];

  nativeBuildInputs = with pkgs; with kdePackages; [
    extra-cmake-modules cmake
    karchive kwindowsystem kcrash knewstuff
    qt6.wrapQtAppsHook
  ];

  postInstall = ''
    mkdir -p $out/etc/xdg/autostart
    cp $out/share/applications/org.kde.latte-dock.desktop $out/etc/xdg/autostart
  '';

  meta = with pkgs.lib; {
    description = "Dock-style app launcher based on Plasma frameworks";
    homepage = "https://invent.kde.org/plasma/latte-dock";
    license = licenses.gpl2;
    platforms = platforms.unix;
    maintainers = [ ];
  };
}
