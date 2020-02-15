# aardvark::jsi

`aardvark::jsi` is a JavaScript interface library.

- Provides abstraction over different JS-engines (currently supports JavaScriptCore and quickjs).
- Automatic memory management.
- Convenient error handling using `expected`.

In addition to that it includes automatic bindings generator.
It allows to describe API in YAML format and the generate code for mapping types
between native and JS and expose your API.
The format supports structs, enums, callbacks, functions, and classes with 
properties and methods.

## Overview

- `jsi.h` &ndash; Common interface definition
- `jsc.h` &ndash; Implementation for JavaScriptCore
- `qjs.h` &ndash; Implementation for quickjs

- `idl` &ndash; Node.js module for automatically generating API bindings

## Config

Use flags `ADV_JSI_QJS` or `ADV_JSI_JSC` to select implementation.

