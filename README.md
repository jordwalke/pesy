# pesy: Native Reason Project from Json.

- Use `package.json` to declare libraries and executables.
- Generates `dune` config per directory.


![screenshot](./images/screenshot.png "Demo")

## `pesy` modes:

- **Create New Project:** Running `pesy` globally creates a new project.
- **Update `pesy` project:** Running `esy pesy` in an existing `pesy` project
  will update build config according to `package.json` contents.
- **Build `pesy` project:** Setting your `package.json` `build` command to
  `pesy` will verify build config is up to date before building your project.

### Create New Project:

If installed globally, `pesy` can be used to create `esy` projects instantly
inside of any directory. The project name is determined by the current
directory, and the generated projects will use invoke each build with `pesy` to
keep the `dune` build config in sync.

```
npm install -g pesy

mkdir my-project
cd my-project
pesy      # Hit enter to accept default name
```

This creates:

- `package.json` with useful dependencies/compilers.
- `.gitignore` and `README.md` with instructions for new contributors.
- `.circleci` continuous integration with cache configured for ultra-fast pull
  requests.
- `library/`, `executable/` and `test/` directory with starter modules.

The created project uses `pesy` in its build step. As always, run `esy pesy`
any time you update the build config in the `package.json`.

## Update `pesy` Project:

Once you've created a project, you normally only ever run `esy build` on the
command line. If you update your `package.json` `buildDirs` field, you will
need to run `esy pesy` which will udpate all the project build config based on
your `package.json` file changes. Then, you just run `esy build` as usual. You
only need to run `esy pesy` if you change your `package.json` file.

(Hopefully this could be automatically done in the future so you only ever run
`esy build` as usual).


## Build `pesy` Project:

If you created your project by invoking the globally installed `pesy`, then
your project is already setup to use `pesy` at build time. Its `esy.build`
field is set to `pesy`, which will run `pesy` to verify that all your build
config is up to date before invoking the Dune build. It will help walk you
through updating the Dune build config from your `package.json` if anything is
out of date.

You probably don't need `pesy` if you have an existing project that is working
well, but to add `pesy` to an existing project, follow these steps:

**1. Add a dependency on `pesy`, and configure `buildDirs`:**

    "name": "my-package",
    "dependencies": {
      "pesy": "*"
    },
    "buildDirs": {
      "exampleLib": {
        "namespace": "Examples",
        "name": "my-package.example-lib",
        "require": [ "bos.top" ]
      },
      "bin": {
        "name": "my-package.exe",
        "require": [
          "my-package.lib"
        ]
      }
    }

**2.Install and Build:**

```sh
esy install
esy pesy  # Generate the project build config from json
esy build
```

## Example Project:

The following example project already has an example config. You can base your
project off of this one.

```sh
npm install -g esy@next
git clone git@github.com:jordwalke/esy-peasy-starter.git

esy install
esy pesy    # Use pesy to configure build from package.json
esy build

```

- Change the `name` of the package, and names of libraries in `buildDirs`
  accordingly.
- Then rerun:

```
esy pesy
esy build
```

## Testing Binaries:

Use the standard `esy x any-command-here` command to run `any-command-here` as
if you had installed the package. For example `esy x YourPackage.exe --args`
builds and runs your built `YourPackage.exe` executable with arguments.


## Consuming New Library Dependencies:

- Add dependencies to `dependencies` in `package.json`.
- Add the name of that new dependencies *library*  to `package.json`'s
  `buildDirs` section that you want to use the library within. For example, if
  your project builds a library in the `exampleLib/` directory, and you want it
  to depend on a library named `bos.top` from an opam package named `bos`,
  change the `package.json` to look like this:
   
    "name": "my-package",
    "dependencies": {
      "@opam/bos": "*"
    },
    "buildDirs": {
      "exampleLib": {
        "namespace": "Examples",
        "name": "my-package.example-lib",
        "require": [ "bos.top" ]
      }
    }

- Then run:
```
esy install  # Fetch dependency sources
esy pesy     # Configure the build based on package.json
esy build    # Do the build
```
 
> Note: After adding/building a new dependency you can use `esy ls-libs` to see
> which named libraries become available to you by adding the package
> dependency.

> Note: You can also use `esy ls-modules` to see which named _modules_ become
> available to you from those libraries.


## Tradeoffs:
`esy-peasy` is good for rapidly making new small executables/libraries. Once they
grow, you'll want to "eject out" of `esy-peasy` and begin customizing using a more
advanced build system.


### Supported Config
Not all config is supported. This is just a proof of concept. If you'd like to
add support for more config fields, PRs are welcomed.

**Binaries**
- `name`: The name of the binary
- `require`: Libraries required

**Libraries**
- `name`: The name of the library
- `namespace`: The name that other modules will _see_ your module as, within
  their source code, if they `require` your library.
- `require`: Libraries required
