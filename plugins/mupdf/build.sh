export CFLAGS=-fPIC -shared -Os -pipe  -Os -flto -ffunction-sections -Wl,--gc-sections
export LDFLAGS=-fPIC -shared -Os -pipe  -Os -flto -ffunction-sections -Wl,--gc-sections
make nuke
make HAVE_X11=no HAVE_GLFW=no HAVE_CURL=no build=release
