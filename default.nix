{ pkgs ? import <nixpkgs> {} }:

let
  inherit (pkgs) stdenv libsForQt5;
in
stdenv.mkDerivation {
  pname = "latte-dock";
  version = "unstable-9999";

  src = ./.;

  buildInputs = with pkgs; with libsForQt5; [ plasma-framework plasma-wayland-protocols qtwayland xorg.libpthreadstubs xorg.libXdmcp xorg.libSM wayland plasma-workspace plasma-desktop ];

  nativeBuildInputs = with pkgs; with libsForQt5; [ extra-cmake-modules cmake karchive kwindowsystem qtx11extras kcrash knewstuff wrapQtAppsHook ];

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
