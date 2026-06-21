# Huggle Linux packaging

These scripts build packages from the current local checkout. Artifacts are
written to `packaging/output/`.

## One-command builds

Run the matching script on the target distribution:

```bash
./packaging/package-debian.sh
./packaging/package-ubuntu.sh
./packaging/package-fedora.sh
./packaging/package-rocky.sh
./packaging/package-appimage.sh
```

Each script accepts `--version x.y.z`. The default version is read from
`src/huggle_core/definitions_prod.hpp`; this option overrides the Huggle
application version, not the operating-system version.

The shared `package-deb.sh` and `package-rpm.sh` scripts can also be executed
directly with no arguments. They detect the current container distribution
from `/etc/os-release`.

The distro scripts check their build dependencies and print the appropriate
installation command when something is missing. Rocky Linux may require CRB
and EPEL for Qt 6 WebEngine.

AppImage packaging additionally requires `linuxdeploy`,
`linuxdeploy-plugin-qt`, and `linuxdeploy-plugin-appimage` in `PATH`. Use
`--qt /path/to/Qt` when Qt is installed outside the system prefix.

All package builds enable Qt 6, Qt WebEngine, audio, and Huggle extensions.
Before configuring CMake, they generate `src/huggle_core/version.txt` and
ensure `src/huggle_core/definitions.hpp` exists, matching `./configure`.
