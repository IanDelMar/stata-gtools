* ----------------------------------------------------------------------------
* Project: gtools
* Program: gtools_tests.do
* Author:  Mauricio Caceres Bravo <mauricio.caceres.bravo@gmail.com>
* Created: Tue May 16 07:23:02 EDT 2017
* Updated: Fri Jul 20 18:16:33 EDT 2018
* Purpose: Unit tests for gtools
* Version: 0.14.1
* Manual:  help gtools

* Stata start-up options
* ----------------------

version 13
clear all
set more off
set varabbrev off
set seed 42
set linesize 255
set type double

* Main program wrapper
* --------------------

program main
    syntax, [NOIsily *]

    if ( inlist("`c(os)'", "MacOSX") | strpos("`c(machine_type)'", "Mac") ) {
        local c_os_ macosx
    }
    else {
        local c_os_: di lower("`c(os)'")
    }
    log using gtools_tests_`c_os_'.log, text replace name(gtools_tests)

    * Set up
    * ------

    local  progname tests
    local  start_time "$S_TIME $S_DATE"

    di _n(1)
    di "Start:        `start_time'"
    di "Options:      `options'"
    di "OS:           `c(os)'"
    di "Machine Type: `c(machine_type)'"

    * Run the things
    * --------------

    cap noi {
        * qui do test_gquantiles_by.do
        * qui do test_gquantiles.do
        * qui do test_gcollapse.do
        * qui do test_gcontract.do
        * qui do test_gegen.do
        * qui do test_gisid.do
        * qui do test_gduplicates.do
        * qui do test_glevelsof.do
        * qui do test_gtoplevelsof.do
        * qui do test_gunique.do
        * qui do test_hashsort.do

        if ( `:list posof "dependencies" in options' ) {
            cap ssc install ralpha
            cap ssc install ftools
            cap ssc install unique
            cap ssc install distinct
            cap ssc install moremata
            cap ssc install fastxtile
            cap ssc install egenmisc
            cap ssc install egenmore
            ftools, compile
        }

        if ( `:list posof "basic_checks" in options' ) {

            di ""
            di "-------------------------------------"
            di "Basic unit-tests $S_TIME $S_DATE"
            di "-------------------------------------"

            unit_test, `noisily' test(checks_gcontract,     `noisily' oncollision(error))
            unit_test, `noisily' test(checks_isid,          `noisily' oncollision(error))
            unit_test, `noisily' test(checks_duplicates,    `noisily' oncollision(error))
            unit_test, `noisily' test(checks_levelsof,      `noisily' oncollision(error))
            unit_test, `noisily' test(checks_toplevelsof,   `noisily' oncollision(error))
            unit_test, `noisily' test(checks_unique,        `noisily' oncollision(error))
            unit_test, `noisily' test(checks_hashsort,      `noisily' oncollision(error))

            unit_test, `noisily' test(checks_gquantiles_by, `noisily' oncollision(error))
            unit_test, `noisily' test(checks_gquantiles_by, `noisily' oncollision(error) wgt([fw = int1]))
            unit_test, `noisily' test(checks_gquantiles_by, `noisily' oncollision(error) wgt([pw = int1]))
            unit_test, `noisily' test(checks_gquantiles_by, `noisily' oncollision(error) wgt([aw = int1]))
            unit_test, `noisily' test(checks_gquantiles,    `noisily' oncollision(error))
            unit_test, `noisily' test(checks_gquantiles,    `noisily' oncollision(error) wgt([fw = int1]))
            unit_test, `noisily' test(checks_gquantiles,    `noisily' oncollision(error) wgt([pw = int1]))
            unit_test, `noisily' test(checks_gquantiles,    `noisily' oncollision(error) wgt([aw = int1]))

            unit_test, `noisily' test(checks_gegen,         `noisily' oncollision(error))
            unit_test, `noisily' test(checks_gegen,         `noisily' oncollision(error) wgt([fw = int1]))
            unit_test, `noisily' test(checks_gegen,         `noisily' oncollision(error) wgt([iw = int1]))
            unit_test, `noisily' test(checks_gegen,         `noisily' oncollision(error) wgt([pw = int1]))
            unit_test, `noisily' test(checks_gegen,         `noisily' oncollision(error) wgt([aw = int1]))

            unit_test, `noisily' test(checks_gcollapse,     `noisily' oncollision(error))
            unit_test, `noisily' test(checks_gcollapse,     `noisily' oncollision(error) wgt([fw = int1]))
            unit_test, `noisily' test(checks_gcollapse,     `noisily' oncollision(error) wgt([iw = int1]))
            unit_test, `noisily' test(checks_gcollapse,     `noisily' oncollision(error) wgt([pw = int1]))
            unit_test, `noisily' test(checks_gcollapse,     `noisily' oncollision(error) wgt([aw = int1]))

            di _n(1)

            unit_test, `noisily' test(checks_corners, `noisily' oncollision(error))
        }

        if ( `:list posof "comparisons" in options' ) {

            di ""
            di "-----------------------------------------------------------"
            di "Consistency checks (v native commands) $S_TIME $S_DATE"
            di "-----------------------------------------------------------"

            compare_isid,          `noisily' oncollision(error)
            compare_duplicates,    `noisily' oncollision(error)
            compare_levelsof,      `noisily' oncollision(error)
            compare_toplevelsof,   `noisily' oncollision(error) tol(1e-4)
            compare_unique,        `noisily' oncollision(error) distinct
            compare_hashsort,      `noisily' oncollision(error)
            compare_egen,          `noisily' oncollision(error)
            compare_gcontract,     `noisily' oncollision(error)

            compare_gquantiles_by, `noisily' oncollision(error)
            compare_gquantiles_by, `noisily' oncollision(error) noaltdef wgt(both mix)
            compare_gquantiles,    `noisily' oncollision(error) noaltdef
            compare_gquantiles,    `noisily' oncollision(error) noaltdef wgt(both mix)

            compare_gcollapse,     `noisily' oncollision(error)
            compare_gcollapse,     `noisily' oncollision(error) wgt(g [fw = 1])
            compare_gcollapse,     `noisily' oncollision(error) wgt(c [fw = 1])
            compare_gcollapse,     `noisily' oncollision(error) wgt(both mix) tol(1e-4)
        }

        if ( `:list posof "switches" in options' ) {
            gquantiles_switch_sanity v1
            gquantiles_switch_sanity v2
            gquantiles_switch_sanity v3
        }

        if ( `:list posof "bench_test" in options' ) {
            bench_gquantiles_by, n(100)  bench(100) `noisily' oncollision(error)
            bench_gquantiles,    n(1000) bench(1)   `noisily' oncollision(error)
            bench_contract,      n(1000) bench(1)   `noisily' oncollision(error)
            bench_egen,          n(1000) bench(1)   `noisily' oncollision(error)
            bench_isid,          n(1000) bench(1)   `noisily' oncollision(error)
            bench_duplicates,    n(1000) bench(1)   `noisily' oncollision(error)
            bench_levelsof,      n(100)  bench(1)   `noisily' oncollision(error)
            bench_toplevelsof,   n(1000) bench(1)   `noisily' oncollision(error)
            bench_unique,        n(1000) bench(1)   `noisily' oncollision(error)
            bench_unique,        n(1000) bench(1)   `noisily' oncollision(error) distinct
            bench_hashsort,      n(1000) bench(1)   `noisily' oncollision(error) benchmode

            bench_collapse, collapse fcollapse bench(10)  n(100)    style(sum)    vars(15) oncollision(error)
            bench_collapse, collapse fcollapse bench(10)  n(100)    style(ftools) vars(6)  oncollision(error)
            bench_collapse, collapse fcollapse bench(10)  n(100)    style(full)   vars(1)  oncollision(error)

            bench_collapse, collapse fcollapse bench(0.05) n(10000) style(sum)    vars(15) oncollision(error)
            bench_collapse, collapse fcollapse bench(0.05) n(10000) style(ftools) vars(6)  oncollision(error)
            bench_collapse, collapse fcollapse bench(0.05) n(10000) style(full)   vars(1)  oncollision(error)
        }

        if ( `:list posof "bench_full" in options' ) {
            bench_gquantiles_by, n(10000)   bench(1000) `noisily' oncollision(error)
            bench_gquantiles,    n(1000000) bench(10)   `noisily' oncollision(error)
            bench_contract,      n(10000)   bench(10)   `noisily' oncollision(error)
            bench_egen,          n(10000)   bench(10)   `noisily' oncollision(error)
            bench_isid,          n(10000)   bench(10)   `noisily' oncollision(error)
            bench_duplicates,    n(10000)   bench(10)   `noisily' oncollision(error)
            bench_levelsof,      n(100)     bench(100)  `noisily' oncollision(error)
            bench_toplevelsof,   n(10000)   bench(10)   `noisily' oncollision(error)
            bench_unique,        n(10000)   bench(10)   `noisily' oncollision(error)
            bench_unique,        n(10000)   bench(10)   `noisily' oncollision(error) distinct
            bench_hashsort,      n(10000)   bench(10)   `noisily' oncollision(error) benchmode

            bench_collapse, collapse fcollapse bench(1000) n(100)    style(sum)    vars(15) oncollision(error)
            bench_collapse, collapse fcollapse bench(1000) n(100)    style(ftools) vars(6)  oncollision(error)
            bench_collapse, collapse fcollapse bench(1000) n(100)    style(full)   vars(1)  oncollision(error)

            bench_collapse, collapse fcollapse bench(0.1)  n(1000000) style(sum)    vars(15) oncollision(error)
            bench_collapse, collapse fcollapse bench(0.1)  n(1000000) style(ftools) vars(6)  oncollision(error)
            bench_collapse, collapse fcollapse bench(0.1)  n(1000000) style(full)   vars(1)  oncollision(error)
        }
    }
    local rc = _rc

    exit_message, rc(`rc') progname(`progname') start_time(`start_time') `capture'
    log close gtools_tests
    exit `rc'
end

* ---------------------------------------------------------------------
* Aux programs

capture program drop exit_message
program exit_message
    syntax, rc(int) progname(str) start_time(str) [CAPture]
    local end_time "$S_TIME $S_DATE"
    local time     "Start: `start_time'" _n(1) "End: `end_time'"
    di ""
    if (`rc' == 0) {
        di "End: $S_TIME $S_DATE"
        local paux      ran
        local message "`progname' finished running" _n(2) "`time'"
        local subject "`progname' `paux'"
    }
    else if ("`capture'" == "") {
        di "WARNING: $S_TIME $S_DATE"
        local paux ran with non-0 exit status
        local message "`progname' ran but Stata gave error code r(`rc')" _n(2) "`time'"
        local subject "`progname' `paux'"
    }
    else {
        di "ERROR: $S_TIME $S_DATE"
        local paux ran with errors
        local message "`progname' stopped with error code r(`rc')" _n(2) "`time'"
        local subject "`progname' `paux'"
    }
    di "`subject'"
    di ""
    di "`message'"
end

* Wrapper for easy timer use
cap program drop mytimer
program mytimer, rclass
    * args number what step
    syntax anything, [minutes ts]

    tokenize `anything'
    local number `1'
    local what   `2'
    local step   `3'

    if ("`what'" == "end") {
        qui {
            timer clear `number'
            timer off   `number'
        }
        if ("`ts'" == "ts") mytimer_ts `step'
    }
    else if ("`what'" == "info") {
        qui {
            timer off `number'
            timer list `number'
        }
        local seconds = r(t`number')
        local prints  `:di trim("`:di %21.2gc `seconds''")' seconds
        if ("`minutes'" != "") {
            local minutes = `seconds' / 60
            local prints  `:di trim("`:di %21.3gc `minutes''")' minutes
        }
        mytimer_ts Step `step' took `prints'
        qui {
            timer clear `number'
            timer on    `number'
        }
    }
    else {
        qui {
            timer clear `number'
            timer on    `number'
            timer off   `number'
            timer list  `number'
            timer on    `number'
        }
        if ("`ts'" == "ts") mytimer_ts `step'
    }
end

capture program drop mytimer_ts
program mytimer_ts
    display _n(1) "{hline 79}"
    if ("`0'" != "") display `"`0'"'
    display `"        Base: $S_FN"'
    display  "        In memory: `:di trim("`:di %21.0gc _N'")' observations"
    display  "        Timestamp: $S_TIME $S_DATE"
    display  "{hline 79}" _n(1)
end

capture program drop unit_test
program unit_test
    syntax, test(str) [NOIsily tab(int 4)]
    local tabs `""'
    forvalues i = 1 / `tab' {
        local tabs "`tabs' "
    }
    cap `noisily' `test'
    if ( _rc ) {
        di as error `"`tabs'test(failed): `test'"'
        exit _rc
    }
    else di as txt `"`tabs'test(passed): `test'"'
end

capture program drop gen_data
program gen_data
    syntax, [n(int 100) skipstr]
    clear
    set obs `n'

    * Random strings
    * --------------

    if ( "`skipstr'" == "" ) {
        qui ralpha str_long,  l(5)
        qui ralpha str_mid,   l(3)
        qui ralpha str_short, l(1)
    }

    * Generate does-what-it-says-on-the-tin variables
    * -----------------------------------------------

    local chars char(40 + mod(_n, 50))
    forvalues i = 1 / 50 {
        local chars `chars' + char(40 + mod(_n + `i', 50))
    }

    forvalues i = 35 / 115 {
        disp `i', char(`i')
    }

    if ( "`skipstr'" == "" ) {
        if ( `c(stata_version)' >= 14 ) {
            gen strL strL1 = str_long  + `chars'
            gen strL strL2 = str_mid   + `chars'
            gen strL strL3 = str_short + `chars'
            forvalues i = 1 / 42 {
                replace strL1 = strL1 + `chars'
                replace strL2 = strL2 + `chars'
                replace strL3 = strL3 + `chars'
            }
        }

        gen str32 str_32   = str_long + "this is some string padding"
        gen str12 str_12   = str_mid  + "padding" + str_short + str_short
        gen str4  str_4    = str_mid  + str_short
    }

    gen long   int1  = floor(uniform() * 1000)
    gen long   int2  = floor(rnormal())
    gen double int3  = floor(rnormal() * 5 + 10)

    gen double double1 = uniform() * 1000
    gen double double2 = rnormal()
    gen double double3 = rnormal() * 5 + 10

    * Mix up string lengths
    * ---------------------

    if ( "`skipstr'" == "" ) {
        replace str_32 = str_mid + str_short if mod(_n, 4) == 0
        replace str_12 = str_short + str_mid if mod(_n, 4) == 2
    }

    * Insert some blanks
    * ------------------

    if ( "`skipstr'" == "" ) {
        replace str_32 = "            " in 1 / 10
        replace str_12 = "   "          in 1 / 10
        replace str_4  = " "            in 1 / 10

        replace str_32 = "            " if mod(_n, 21) == 0
        replace str_12 = "   "          if mod(_n, 34) == 0
        replace str_4  = " "            if mod(_n, 55) == 0

        if ( `c(stata_version)' >= 14 ) {
            replace strL1 = "            " in 1 / 10
            replace strL2 = "   "          in 1 / 10
            replace strL3 = " "            in 1 / 10

            replace strL1 = "            " if mod(_n, 21) == 0
            replace strL2 = "   "          if mod(_n, 34) == 0
            replace strL3 = " "            if mod(_n, 55) == 0
        }
    }

    * Missing values
    * --------------

    if ( "`skipstr'" == "" ) {
        replace str_32 = "" if mod(_n, 10) ==  0
        replace str_12 = "" if mod(_n, 20) ==  0
        replace str_4  = "" if mod(_n, 20) == 10

        if ( `c(stata_version)' >= 14 ) {
            replace strL1 = "" if mod(_n, 10) ==  0
            replace strL2 = "" if mod(_n, 20) ==  0
            replace strL3 = "" if mod(_n, 20) == 10
        }
    }

    replace int2  = .   if mod(_n, 10) ==  0
    replace int3  = .a  if mod(_n, 20) ==  0
    replace int3  = .f  if mod(_n, 20) == 10

    replace double2 = .   if mod(_n, 10) ==  0
    replace double3 = .h  if mod(_n, 20) ==  0
    replace double3 = .p  if mod(_n, 20) == 10

    * Singleton groups
    * ----------------

    if ( "`skipstr'" == "" ) {
        replace str_32 = "|singleton|" in `n'
        replace str_12 = "|singleton|" in `n'
        replace str_4  = "|singleton|" in `n'
    }

    replace int1    = 99999  in `n'
    replace double1 = 9999.9 in `n'

    replace int3 = .  in 1
    replace int3 = .a in 2
    replace int3 = .b in 3
    replace int3 = .c in 4
    replace int3 = .d in 5
    replace int3 = .e in 6
    replace int3 = .f in 7
    replace int3 = .g in 8
    replace int3 = .h in 9
    replace int3 = .i in 10
    replace int3 = .j in 11
    replace int3 = .k in 12
    replace int3 = .l in 13
    replace int3 = .m in 14
    replace int3 = .n in 15
    replace int3 = .o in 16
    replace int3 = .p in 17
    replace int3 = .q in 18
    replace int3 = .r in 19
    replace int3 = .s in 20
    replace int3 = .t in 21
    replace int3 = .u in 22
    replace int3 = .v in 23
    replace int3 = .w in 24
    replace int3 = .x in 25
    replace int3 = .y in 26
    replace int3 = .z in 27

    replace double3 = .  in 1
    replace double3 = .a in 2
    replace double3 = .b in 3
    replace double3 = .c in 4
    replace double3 = .d in 5
    replace double3 = .e in 6
    replace double3 = .f in 7
    replace double3 = .g in 8
    replace double3 = .h in 9
    replace double3 = .i in 10
    replace double3 = .j in 11
    replace double3 = .k in 12
    replace double3 = .l in 13
    replace double3 = .m in 14
    replace double3 = .n in 15
    replace double3 = .o in 16
    replace double3 = .p in 17
    replace double3 = .q in 18
    replace double3 = .r in 19
    replace double3 = .s in 20
    replace double3 = .t in 21
    replace double3 = .u in 22
    replace double3 = .v in 23
    replace double3 = .w in 24
    replace double3 = .x in 25
    replace double3 = .y in 26
    replace double3 = .z in 27
end

capture program drop random_draws
program random_draws
    syntax, random(int) [binary(int 0) float double]
    forvalues i = 1 / `random' {
        gen `float'`double' random`i' = rnormal() * `i' * 5
        replace random`i' = . if mod(_n, 20) == 0
        if ( `binary' > 0 ) {
            replace random`i' = floor(runiform() * 1.99) if _n <= `=_N / `binary''
        }
    }
end

* ---------------------------------------------------------------------
* Run the things

main, dependencies basic_checks comparisons switches bench_test
