# Plugins

What a backend plugin exports and what it has to agree on with the core.

A plugin is a shared object exporting `rexlib_get_plugin`, returning a
`const plugin*`. `plugin_loader` resolves that symbol by name, so the contract
is the symbol and the `plugin` interface, not the C++ ABI of the loader.

It is still a C++ ABI boundary for everything the plugin touches: the plugin
and the core have to agree on compiler, standard library, and on Windows the
runtime and whether it is the debug one.
