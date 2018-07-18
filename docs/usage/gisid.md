gisid 
=====

Efficiently check for unique identifiers using C plugins.  This is a fast
option to Stata's isid. It checks whether a set of variables uniquely
identifies observations in a dataset. It can additionally take `if` and
`in` but it cannot check an external data set or sort the data.

_Note for Windows users:_ It may be necessary to run `gtools, dependencies` at
the start of your Stata session.

Syntax
------

```stata
gisid varlist [if] [in] [, missok]
```

Options
-------

        missok indicates that missing values are permitted in varlist.

### Gtools options

(Note: These are common to every gtools command.)

- `compress` Try to compress strL to str#. The Stata Plugin Interface has
            only limited support for strL variables. In Stata 13 and
            earlier (version 2.0) there is no support, and in Stata 14
            and later (version 3.0) there is read-only support. The user
            can try to compress strL variables using this option.

- `verbose` prints some useful debugging info to the console.

- `benchmark` or `bench(level)` prints how long in seconds various parts of the
            program take to execute. Level 1 is the same as `benchmark`. Level 2
            additionally prints benchmarks for internal plugin steps.

- `hashlib(str)` On earlier versions of gtools Windows users had a problem
            because Stata was unable to find spookyhash.dll, which is bundled
            with gtools and required for the plugin to run correctly. The best
            thing a Windows user can do is run gtools, dependencies at the start
            of their Stata session, but if Stata cannot find the plugin the user
            can specify a path manually here.

- `hashmethod(str)` Hash method to use. `default` automagically chooses the
            algorithm. `biject` tries to biject the inputs into the
            natural numbers. `spooky` hashes the data and then uses the
            hash.

- `oncollision(str)` How to handle collisions. A collision should never happen
            but just in case it does `gtools` will try to use native
            commands. The user can specify it throw an error instead by
            passing `oncollision(error)`.

Examples
--------

You can download the raw code for the examples below
[here  <img src="https://upload.wikimedia.org/wikipedia/commons/6/64/Icon_External_Link.png" width="13px"/>](https://raw.githubusercontent.com/mcaceresb/stata-gtools/master/docs/examples/gisid.do)

```stata
. sysuse auto, clear
(1978 Automobile Data)

. gisid mpg
variable mpg does not uniquely identify the observations
r(459);

. gisid make

. replace make = "" in 1
(1 real change made)

. gisid make
variable make should never be missing
r(459);

. gisid make, missok
```

gisid can also take a range, that is
```
. gisid mpg in 1
. gisid mpg if _n == 1
```
