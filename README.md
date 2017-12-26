# esy-peasy

The easiest way to make native Reason programs and libraries.

- Low-to-no configuration.
- Merlin and LSP integration out of the box.
- Easily move to another build system when your project grows.

A package named `my-project` builds a single library matching the package name
`my-project`, and builds a single binary named `MyProject`.

## Build An Example:

```sh
npm install -g esy@next
git clone git@github.com:jordwalke/esy-peasy-starter.git
esy install
esy build
```

## Make It Your Own Package:

- Change the `name` field in the `package.json` (to `my-project` for example).

## How Are The Binary And Library Built?

In general, each package may contain many named "Libraries" which can be
consumed, but `esy-peasy` keeps it simple, producing one named library per
esy package.

For an `esy` package named `my-project`:

- The single `.re` file in `bin/` becomes the `MyProject` executable.
- The contents of `lib/` becomes the `my-project` named library.
- Your binary `.re` file can automatically see the `my-project` library as the
  `YourProjectLib` module.
- The `my-project` library is made available to packages that depend on you.
  Once they depend on your package and compile against the `my-project`
  library, they too will be able to use the `YourProject` module.

## Testing Your Binary:

As with any `esy` project, use the `esy x any-command-here` command to build
and install your package for the duration of `any-command-here` so that
`YourPackage` binary will be visible. You can pass arguments to your binary
program.

## Customizations:
- Omit the `lib/` directory if everything fits into the single file in `bin/`.
- You may rename the `bin/Index.re` file to be
  `bin/YourProjectNameCamelCased.re`. Nothing else.

## Adding New Package Dependencies:
- Run `esy add @opam/dep-name@version` to automatically build and add a new
  dependency to `package.json`.
- Each package conatains several "Libraries". Find the name of
  the library you wish to consume by running `esy ls-libs`.
- Add that library name to `package.json` under a section like: `"peasyLibs": ["the-library-name"]`.
- Use that library in your code and run `esy build`.


```sh
esy x YourPackage --arg-to-your-package
```

## Publish Prebuilt binaries of your executables to `npm`.


## Tradeoffs:
`esy-peasy` has almost no configuration, and so it regenerates all build
config. This is fast enough for small projects, but once your small app grows
larger, you'll want to switch to a more full-featured build system. You can
"eject" out of `esy-peasy` and begin customizing it from there.

