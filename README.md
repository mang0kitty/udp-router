```bash
# Fetch your code
git clone git@github.com:mangokittty/udp-router.git

# Fetch the latest version of boost and its submodules
git submodule update --init --recursive

# Build your code
make

# Start a few routers
my-router run A 10000
my-router run B 10001
my-router run C 10002
my-router run D 10003
my-router run E 10004

# Configure the routers with an initial neighbourhood table
my-router configure 10000 A B 10001 4
my-router configure 10001 B C 10002 3
my-router configure 10001 B E 10004 8
my-router configure 10004 E D 10003 3
my-router configure 10002 C D 10003 1

# Send a payload to a specific router by injecting it at into the network
my-router send 10002 B D "Hey there D, B here saying hello"
```