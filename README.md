# tinytorrent

TinyTorrent is a file sharing service that runs on top of simple-kademlia, a DHT also on my profile that is currently a work in progress.
After cloning, run `git submodule update --init --recursive`

From the repo, run `cmake --build cmake-build-debug --target tinytorrent -- -j 2`
`cmake-build-debug/tinytorrent` to start the bootstrap server, defaulted in `main.cpp` as `127.0.0.1:8000`

Use the `--ip` and `--port` flags for each new instance.
