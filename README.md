# esy-peasy

- Easiest way to make native Reason programs and libraries.
- Merlin and LSP integration out of the box.
- For a project named `my-project`, produces single sharable library named
  `my-project` and a single executable named `MyProject`.

## Build An Example:

```sh
npm install -g esy@next
git clone git@github.com:jordwalke/esy-peasy-starter.git
esy install
esy build
```

## Make It Your Own Package:

- Change the `name` field in the `package.json` and rebuild.

## How Are The Binary And Library Built?

In general, each package may contain many named "Libraries" which can be
consumed, but `esy-peasy`produces exactly one sharable library with the
same name as the package.

- The single `.re` file in `bin/` becomes the `MyProject` executable.
- The contents of `lib/` becomes the `my-project` named library.
- Your binary `.re` file can automatically see the `my-project` library as the
  `YourProjectLib` module.
- When other packages depend on your package, they can then use your
  `my-project` library, which will then allow them to see the `YourProjectLib` module.

## Testing Your Binary:

As with any `esy` project, use the `esy x any-command-here` command to build
and install your package for the duration of `any-command-here` so that
`YourPackage` binary will be visible. You can pass arguments to your binary
program.

## Customizations:
- Omit the `lib/` directory if everything fits into the single file in `bin/`.
- You may rename the `bin/Index.re` file to be
  `bin/YourProjectNameCamelCased.re`.
- Nothing else.

## Adding New Package Dependencies:
- `esy add @opam/dep-name@version` automatically builds and adds a new
  dependency to `package.json`.
- Find the name of the library inside of that new package by running `esy ls-libs`.
- Add that *library* name to `package.json` like this: `"peasyLibs": ["the-library-name"]`.
- Use that library in your code and run `esy build`.

> Note: After adding/building a new dependency you can use `esy ls-modules` to see
  which named modules become available to you by adding the `peasyLibs` entry.

## Publish Prebuilt binaries of your executables to `npm`.
Use `esy` to make prebuilt binary releases of your program that can be installed
from plain `npm`.

```
esy release bin
cd _release/bin-darwin && npm publish --tag darwin
```

## Tradeoffs:
`esy-peasy` is good for rapidly making new small executables/libraries. Once they
grow, you'll want to "eject out" of `esy-peasy` and begin customizing using a more
advanced build system.
