# <PACKAGE_NAME>


[![CircleCI](https://circleci.com/gh/reasonml/reason-cli/tree/master.svg?style=svg)](https://circleci.com/gh/reasonml/reason-cli/tree/master)


**Contains the following libraries and executables:**

```
<PACKAGE_NAME>@<VERSION>
│
├─test/
│   name:    Test<PACKAGE_NAME_UPPER_CAMEL>.exe
│   main:    Test<PACKAGE_NAME_UPPER_CAMEL>
│   require: <PACKAGE_NAME>.lib
│
├─library/
│   library name: <PACKAGE_NAME>.lib
│   namespace:    <PACKAGE_NAME_UPPER_CAMEL>
│   require:
│
└─executable/
    name:    <PACKAGE_NAME_UPPER_CAMEL>App.exe
    main:    <PACKAGE_NAME_UPPER_CAMEL>App
    require: <PACKAGE_NAME>.lib
```

## Developing:

````
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

````
esy x <PACKAGE_NAME_UPPER_CAMEL>App.exe 
```

## Running Tests:

````
# Runs the "test" command in `package.json`.
esy test
```
