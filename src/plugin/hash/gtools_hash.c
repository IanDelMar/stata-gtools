#include "gtools_hash.h"
#include "gtools_sort.c"
#include "gtools_hash_fast.c"

ST_retcode gf_hash (
    uint64_t *h1,
    uint64_t *h2,
    struct StataInfo *st_info,
    GT_size *ix,
    clock_t stimer)
{

    ST_retcode rc = 0;

    GT_size i;
    uint64_t *h3;

    GT_bool sorted   = st_info->sorted;
    GT_size N        = st_info->N;
    GT_size rowbytes = st_info->rowbytes;
    GT_size kvars    = st_info->kvars_by;
    GT_size kstr     = st_info->kvars_by_str;

    // Hash the variables or biject
    // ----------------------------

    if ( st_info->biject ) {
        if ( (rc = gf_biject_varlist (h1, st_info)) ) goto exit;

        if ( st_info->benchmark > 2 )
            sf_running_timer (&stimer, "\t\tPlugin step 2.3: Bijected integers to natural numbers");

        // for (i = 1; i < N; i++) {
        //     if ( h1[i - 1] > h1[i] ) {
        //         sorted = 0;
        //         break;
        //     }
        // }

        // Sort hash with index
        // --------------------

        if ( !sorted ) {
            if ( (rc = gf_sort_hash (h1,
                                     ix,
                                     st_info->N,
                                     st_info->verbose,
                                     st_info->ctolerance)) ) goto exit;

            if ( st_info->benchmark > 2 )
                sf_running_timer (&stimer, "\t\tPlugin step 2.4: Sorted integer-only hash");
        }
        // else if ( st_info->verbose ) {
        //     sf_printf("(already sorted)\n");
        // }
    }
    else {

        h3 = calloc(N, sizeof *h3);
        if ( h3 == NULL ) sf_oom_error("sf_hash_byvars", "h3");
        GTOOLS_GC_ALLOCATED("h3")

        if ( kstr > 0 ) {
            // sorted = MultiSortCheckMC (st_info->st_charx,
            //                            st_info->N,
            //                            0,
            //                            st_info->kvars_by - 1,
            //                            st_info->rowbytes,
            //                            st_info->byvars_lens,
            //                            st_info->invert,
            //                            st_info->positions);

            for (i = 0; i < N; i++) {
                spookyhash_128(st_info->st_charx + (i * rowbytes),
                               rowbytes, h1 + i, h3 + i);
            }
        }
        else {
            // sorted = MultiSortCheckDbl(st_info->st_numx,
            //                            st_info->N,
            //                            0,
            //                            st_info->kvars_by - 1,
            //                            st_info->kvars_by * sizeof(ST_double),
            //                            st_info->invert);

            for (i = 0; i < N; i++) {
                spookyhash_128(st_info->st_numx + i * kvars,
                               sizeof(ST_double) * kvars, h1 + i, h3 + i);
            }
        }

        if ( st_info->benchmark > 2 )
            sf_running_timer (&stimer, "\t\tPlugin step 2.3: Hashed variables (128-bit)");

        // Sort hash with index
        // --------------------

        if ( !sorted ) {
            if ( (rc = gf_sort_hash (h1,
                                     ix,
                                     st_info->N,
                                     st_info->verbose,
                                     st_info->ctolerance)) ) goto exit;

            for (i = 0; i < st_info->N; i++) {
                h2[i] = h3[ix[i]];
            }

            if ( st_info->benchmark > 2 )
                sf_running_timer (&stimer, "\t\tPlugin step 2.4: Sorted integer-only hash");
        }
        else {
            // if ( st_info->verbose )
            //     sf_printf("(already sorted)\n");

            for (i = 0; i < st_info->N; i++) {
                h2[i] = h3[i];
            }
        }

        // Copy back second part of the hash in correct order
        // --------------------------------------------------

        free (h3);
        GTOOLS_GC_FREED("h3")
    }

exit:
    return (rc);
}


/**
 * @brief Use the grouping variables as a hassh
 *
 * The grouping variables are all integer and can be used as a hash.
 * With all-integer variables, we can construct a bijection to the whole
 * numbers. Generally speaking, we want a function f so that f: X^K
 * -> N, where X is a subset of Z. Though there are generic ways to
 * impement this function for X = Z, in this case we know the bounds for
 * X. The function we use is as follows (using 1-based indexing for ease
 * of exposition):
 *
 *     offset = 1
 *     hash   = z[, 1] - min(z[, 1]) + 1
 *     for k = 2 to K
 *         offset *= (zmax[k - 1] - zmin[k - 1] + 1)
 *         hash   += ( z[, k] - min(z[, k]) ) * offset
 *
 * What is happening is that we are fisrt mapping
 *
 *     var1 -> 1 to range of var1
 *
 * Call this vmap1. Then we are mapping
 *
 *     Smallest # of var2 -> vmap1
 *     2-smallest # of var2 -> vmap1 + 2 * range of vmap1
 *     ...
 *     ith-smallest # of var2 -> vmap1 + i * range of vmap1
 *
 * Call this vmap2. Then we do
 *
 *     Smallest # of vark -> vmap(k - 1)
 *     2-smallest # of vark -> vmap(k - 1) + 2 * range of vmap(k - 1)
 *     ...
 *     ith-smallest # of vark -> vmap(k - 1) + i * range of vmap(k - 1)
 *
 * @param h1 Where to store the map to the whole nubmers
 * @param st_info Meta structure with all the variables and data
 * @return Store map to whole numbers in @h1
 *
 */
ST_retcode gf_biject_varlist (uint64_t *h1, struct StataInfo *st_info)
{
    ST_double z;
    GT_size i, k, l;

    GT_size N        = st_info->N;
    GT_size kvars    = st_info->kvars_by;
    GT_size offset   = 1;
    GT_size *offsets = calloc(kvars, sizeof *offsets);
    if ( offsets == NULL ) sf_oom_error ("gf_biject_varlist", "offsets");

    offsets[0] = 0;
    for (k = 0; k < kvars - 1; k++) {
        l = kvars - (k + 1);
        offset *= (st_info->byvars_maxs[l] - st_info->byvars_mins[l] + 1);
        offsets[k + 1] = offset;
    }

    // Construct bijection to whole numbers (we index missing vaues to the
    // largest number plus 1 as a convention; note we set the maximum to
    // the actual max + 1 from Stata so the offsets are correct)
    //
    // NOTE(mauricio): Checking missing values by comparing to SV_missval is
    // not correct; it only works here because whenever there are extended
    // missing values, I use the spooky hash.

    for (i = 0; i < N; i++) {
        l = kvars - (0 + 1);
        z = *(st_info->st_numx + i * kvars + l);
        if ( z == SV_missval ) z = st_info->byvars_maxs[l];
        if ( st_info->invert[l] ) {
            h1[i] = (st_info->byvars_maxs[l] - z + 1);
        }
        else {
            h1[i] = (z - st_info->byvars_mins[l] + 1);
        }

        for (k = 1; k < kvars; k++) {
            l = kvars - (k + 1);
            z = *(st_info->st_numx + (i * kvars + l));
            if ( z == SV_missval ) z = st_info->byvars_maxs[l];
            if ( st_info->invert[l] ) {
                h1[i] += (st_info->byvars_maxs[l] - (GT_int) z) * offsets[k];
            }
            else {
                h1[i] += ((GT_int) z - st_info->byvars_mins[l]) * offsets[k];
            }
        }
        // sf_printf ("\tObs %9d = "GT_size_cfmt"\n", i, h1[i]);
    }

    free (offsets);
    return (0);
}

/**
 * @brief Set up variables for panel using 128-bit hashes
 *
 * Using sorted 128-bit hashes, generate info array with start and
 * ending positions of each group in the sorted hash.
 *
 * @param h1 Array of 64-bit integers containing first half of 128-bit hashes
 * @param h2 Array of 64-bit integers containing second half of 128-bit hashes
 * @param st_info Meta structure with all the variables and data
 * @param hash_level whether we used a bijection (0) or a 128-bit hash (1)
 * @return info arary with start and end positions of each group
 */
ST_retcode gf_panelsetup (
    uint64_t *h1,
    uint64_t *h2,
    struct StataInfo *st_info,
    GT_size *ix,
    const GT_bool hash_level)
{
    if (hash_level == 0) return (gf_panelsetup_bijection (h1, st_info));

    ST_retcode rc = 0;
    GT_size collision64 = 0;
    st_info->J = 1;
    GT_size i   = 0;
    GT_size i2  = 0;
    GT_size l   = 0;
    GT_size start_l;
    GT_size range_l;

    uint64_t el = h1[i++];
    uint64_t el2;

    GT_size   *ix_l;
    uint64_t *h2_l;

    GT_size *info_largest = calloc(st_info->N + 1, sizeof *info_largest);
    if ( info_largest == NULL ) return (sf_oom_error("gf_panelsetup", "info_largest"));

    info_largest[l++] = 0;
    if ( st_info->N > 1 ) {
        do {
            if (h1[i] != el) {

                // The 128-bit hash is stored in 2 64-bit parts; almost
                // surely sorting by one of them is sufficient, but in case
                // it is not, sort by the other, and that should be enough.
                //
                // Sorting by both keys all the time is time-consuming,
                // whereas sorting by only one key is fast. Since we only
                // expect about 1 collision every 4 billion groups, it
                // should be very rare to have to use both keys. (Stata caps
                // observations at 20 billion anyway, and there's one hash
                // per *group*, not row).
                //
                // Still, if the 64-bit hashes are not enough, use the full
                // 128-bit hashes, wehere we don't expect a collision until
                // we have 16 quintillion groups in our data.
                //
                // See burtleburtle.net/bob/hash/spooky.html for details.

                if ( !gf_check_allequal(h2, info_largest[l - 1], i) ) {
                    collision64++;
                    start_l = info_largest[l - 1];
                    range_l = i - start_l;

                    ix_l = ix + start_l;
                    h2_l = h2 + start_l;

                    if ( (rc = gf_radix_sort16 (h2_l, ix_l, range_l)) ) goto exit;

                    // Now that the hash and index are sorted, add to
                    // info_largest based on h2_l
                    el2 = h2_l[i2++];
                    while ( i2 < range_l ) {
                        if ( h2_l[i2] != el2 ) {
                            info_largest[l++] = start_l + i2;
                            el2 = h2_l[i2];
                        }
                        i2++;
                    }
                    i2 = 0;
                }

                info_largest[l++] = i;
                el = h1[i];
            }
            i++;
        } while( i < st_info->N );
    }
    info_largest[l] = st_info->N;

    st_info->J = l;
    st_info->info = calloc(l + 1, sizeof *st_info->info);
    if ( st_info->info == NULL ) return (sf_oom_error("gf_panelsetup", "st_info->info"));
    GTOOLS_GC_ALLOCATED("st_info->info")

    for (i = 0; i < l + 1; i++) {
        st_info->info[i] = info_largest[i];
    }

    if ( (collision64 > 0) & (st_info->verbose) )
        sf_printf("Found "
                  GT_size_cfmt" 64-bit hash collision(s). Fell back on 128-bit hash.\n",
                  collision64);

exit:
    free (info_largest);
    return (rc);
}

/**
 * @brief Short utility to check if segment of array is equal
 *
 * Check if elements from start to end of array @hash are equal
 * from @start to @end.
 *
 * @param hash Array of 64-bit integers to check are equal
 * @param start Start position of check
 * @param end End position of check
 * @return 1 if @hash is equal from @start to @end; 0 otherwise
 */
ST_retcode gf_check_allequal (uint64_t *hash, GT_size start, GT_size end)
{
    uint64_t first = hash[start]; GT_size i;
    for (i = start + 1; i < end; i++)
        if ( hash[i] != first ) return (0);
    return (1);
}

/**
 * @brief Set up variables for panel using 64-bit hashes
 *
 * Using sorted 64-bit hashes, generate info array with start and ending
 * positions of each group in the sorted hash. Gtools uses this only if
 * the inputs were all integers and was able to biject them into the
 * whole numbers.
 *
 * @param h1 Array of 64-bit integers containing first half of 128-bit hashes
 * @param st_info Meta structure with all the variables and data
 * @return info arary with start and end positions of each group
 */
ST_retcode gf_panelsetup_bijection (uint64_t *h1, struct StataInfo *st_info)
{
    st_info->J = 1;
    GT_size i  = 0;
    GT_size l  = 0;

    uint64_t el = h1[i++];
    GT_size *info_largest = calloc(st_info->N + 1, sizeof *info_largest);
    if ( info_largest == NULL ) return (sf_oom_error("gf_panelsetup_bijection", "info_largest"));

    info_largest[l++] = 0;
    if ( st_info->N > 1 ) {
        do {
            if (h1[i] != el) {
                info_largest[l++] = i;
                el  = h1[i];
            }
            i++;
        } while( i < st_info->N );
    }
    info_largest[l] = st_info->N;

    st_info->J = l;
    st_info->info = calloc(l + 1, sizeof *st_info->info);
    if ( st_info->info == NULL ) return (sf_oom_error("gf_panelsetup_bijection", "st_info->info"));
    GTOOLS_GC_ALLOCATED("st_info->info")

    for (i = 0; i < l + 1; i++)
        st_info->info[i] = info_largest[i];

    free (info_largest);

    return (0);
}

/**
 * @brief Check hash worked correctly
 *
 * Make sure all the elements within a hash group are the same
 *
 * @param st_info Stata structure with meta info and data
 * @param level whether to leave a copy of the by variables in memory
 * @return return code for whether check was successful
 */
int sf_check_hash (struct StataInfo *st_info, int level)
{
    return(sf_check_hash_fast(st_info, level));

    GT_size i, j, k;
    GT_size kvars   = st_info->kvars_by;
    GT_size kstr    = st_info->kvars_by_str;
    ST_retcode rc  = 0;
    clock_t timer  = clock();
    clock_t stimer = clock();

    GTOOLS_CHAR(buf1, 32);
    GTOOLS_CHAR(buf2, 32);
    GTOOLS_CHAR(buf3, 32);
    GTOOLS_CHAR(buf4, 32);

    // NOTE(mauricio): strbuffer will remain 0 if level is 22 and
    // multisort will be skipped. // 2017-11-21 08:02 EST

    st_info->strbuffer = 0;
    if ( st_info->biject ) {
        goto bycopy;
    }

    /*********************************************************************
     *                               Setup                               *
     *********************************************************************/

    GT_bool multisort, skipbycopy;
    GT_size start, end, sel, selx, numpos, strpos, rowbytes;
    GT_size l_str  = 0;
    GT_size k_num  = 0;
    GTOOLS_MAX (st_info->byvars_lens, kvars, kmax, k);

    // Figure out the number of numeric by variables and the combined string
    // length of string by variables.
    for (k = 0; k < kvars; k++) {
        if (st_info->byvars_lens[k] > 0) {
            l_str += st_info->byvars_lens[k];
        }
        else {
            k_num += 1;
        }
    }
    l_str = l_str > 0? (l_str + 1): 1;

    // Will compare string in st_strbase to st_strcomp and number as are being
    // read to numbers in st_numbase and st_nummiss
    ST_double z;

    GT_size klen = kmax > 0? (kmax + 1): 1;
    char *s  = malloc(klen * sizeof(char)); memset (s, '\0', klen);
    char *st_strbase = malloc(l_str * sizeof(char)); memset (st_strbase, '\0', l_str);
    char *st_strcomp = malloc(l_str * sizeof(char)); memset (st_strcomp, '\0', l_str);

    if ( st_strbase == NULL ) return(sf_oom_error("sf_check_hash_index", "st_strbase"));
    if ( st_strcomp == NULL ) return(sf_oom_error("sf_check_hash_index", "st_strcomp"));

    ST_double *st_numbase = calloc(k_num > 0? k_num: 1, sizeof *st_numbase);
    GT_bool   *st_nummiss = calloc(k_num > 0? k_num: 1, sizeof *st_nummiss);

    if ( st_numbase == NULL ) return(sf_oom_error("sf_check_hash_index", "st_numbase"));
    if ( st_nummiss == NULL ) return(sf_oom_error("sf_check_hash_index", "st_nummiss"));

    GT_size collisions_count = 0;
    GT_size collisions_row   = 0;

    /*********************************************************************
     *             Allocate memory to final collapsed array              *
     *********************************************************************/

    if ( kstr > 0 ) {
        for (j = 0; j < st_info->J; j++) {
            memset (st_strbase, '\0', l_str);
            start  = i = st_info->info[j];
            end    = st_info->info[j + 1];
            strpos = 0;
            numpos = 0;

            // The idea is to compare all group entries to the first group entry
            // -----------------------------------------------------------------

            for (k = 0; k < kvars; k++) {
                sel = st_info->ix[i] * st_info->rowbytes + st_info->positions[k];
                if ( st_info->byvars_lens[k] > 0 ) {
                    memcpy (st_strbase + strpos,
                            st_info->st_charx + sel,
                            strlen(st_info->st_charx + sel));
                    strpos = strlen(st_strbase);
                }
                else {
                    z = *((ST_double *) (st_info->st_charx + sel));
                    st_numbase[numpos] = z;
                    ++numpos;
                }
            }

            /***************
             *  debugging  *
             ***************
            printf ("Checking: strings = '");
            printf (st_strbase);
            printf ("' and numbers = ");
            for (k = 0; k < k_num; k++) {
                if ( st_nummiss[k] ) {
                    printf ("[missing], ");
                }
                else {
                    printf ("%.5f, ", st_numbase[k]);
                }
            }
            printf ("vs:\n");
             ***************
             *  debugging  *
             ***************/

            // Check 2nd entry of group onward
            // -------------------------------

            for (i = start + 1; i < end; i++) {
                memset (st_strcomp, '\0', l_str);
                collisions_row = 0;
                numpos = 0;
                strpos = 0;
                for (k = 0; k < kvars; k++) {
                    sel = st_info->ix[i] * st_info->rowbytes + st_info->positions[k];
                    if ( st_info->byvars_lens[k] > 0 ) {
                        // Concatenate string and compare result
                        memcpy (st_strcomp + strpos,
                                st_info->st_charx + sel,
                                strlen(st_info->st_charx + sel));
                        strpos = strlen(st_strcomp);
                    }
                    else {
                        // Compare each number individually
                        z = *((ST_double *) (st_info->st_charx + sel));
                        if ( st_numbase[numpos] != z ) ++collisions_row;
                        ++numpos;
                    }
                }

                if ( kmax > 0 ) {
                    if ( (strlen (st_strbase) != strlen (st_strcomp)) ) {
                        ++collisions_row;
                    }
                    else if ( strncmp(st_strbase, st_strcomp, strlen(st_strcomp)) != 0 ) {
                        ++collisions_row;
                        /***************
                         *  debugging  *
                         ***************{
                        printf ("\tstrings = '");
                        printf (st_strcomp);
                        printf ("\n");
                         ***************
                         *  debugging  *
                         ***************/
                    }
                }
            }

            if ( collisions_row > 0 ) ++collisions_count;
        }
    }
    else {
        for (j = 0; j < st_info->J; j++) {
            start = i = st_info->info[j];
            end   = st_info->info[j + 1];

            // The idea is to compare all group entries to the first group entry
            // -----------------------------------------------------------------

            for (k = 0; k < kvars; k++) {
                sel  = st_info->ix[i] * kvars + k;
                z    = *(st_info->st_numx + sel);
                st_numbase[k] = z;
            }

            // Check 2nd entry of group onward
            // -------------------------------

            for (i = start + 1; i < end; i++) {
                collisions_row = 0;
                for (k = 0; k < kvars; k++) {
                    sel = st_info->ix[i] * kvars + k;
                    z   = *(st_info->st_numx + sel);
                    if ( st_numbase[k] != z ) ++collisions_row;
                }
            }

            if ( collisions_row > 0 ) ++collisions_count;
        }
    }

    if ( st_info->benchmark > 2 )
        sf_running_timer (&stimer, "\t\tPlugin step 4.1: Checked for hash collisions");

    /*********************************************************************
     *                Prompt user if there are collisions                *
     *********************************************************************/

    if ( collisions_count > 0 ) {
        sf_format_size(collisions_count,  buf1);
        sf_format_size(st_info->kvars_by, buf2);
        sf_format_size(st_info->N,        buf3);
        sf_format_size(st_info->J,        buf4);
        sf_errprintf ("There may be "
                      "%s 128-bit hash collisions: "
                      "%s variables, "
                      "%s obs, "
                      "%s groups\n",
                      buf1, buf2, buf3, buf4);

        // sf_errprintf ("There may be "
        //               GT_size_cfmt" 128-bit hash collisions: "
        //               GT_size_cfmt" variables, "
        //               GT_size_cfmt" obs, "
        //               GT_size_cfmt" groups\n",
        //               collisions_count, st_info->kvars_by, st_info->N, st_info->J);
        sf_errprintf ("This is likely a bug; please file a bug report at github.com/mcaceresb/stata-gtools/issues\n");

        rc = 17000; level = 0;
    }
    else {
        if ( st_info->verbose ) {
            sf_format_size(st_info->kvars_by, buf1);
            sf_format_size(st_info->N,        buf2);
            sf_format_size(st_info->J,        buf3);
            sf_printf ("There were no hash collisions: "
                       "%s variables, "
                       "%s obs, "
                       "%s groups\n",
                       buf1, buf2, buf3);

            // sf_printf ("There were no hash collisions: "
            //            GT_size_cfmt" variables, "
            //            GT_size_cfmt" obs, "
            //            GT_size_cfmt" groups\n",
            //            st_info->kvars_by, st_info->N, st_info->J);
        }
    }

    free (s);
    free (st_strbase);
    free (st_strcomp);
    free (st_numbase);
    free (st_nummiss);

    /*********************************************************************
     *              Read in copy of variables, if requested              *
     *********************************************************************/

    // Create a de-duplicated copy of the by variables. In some cases it is
    // useful to keep the copy in memory, but most of the time you just want
    // the sort. Hence we skip the step if the data is already sorted and we
    // won't use the by copy later.

bycopy:

    multisort  = (st_info->biject == 0) & (st_info->unsorted == 0) & (st_info->sorted == 0);
    rowbytes   = st_info->rowbytes + sizeof(GT_size);
    skipbycopy = ( (multisort == 0) & (level == 22) ) | st_info->countonly;

    // debug
    // -----
    // printf("debug 1: multisort = %u, skipby = %u\n", multisort, skipbycopy);
    // printf("\tdebug 2: biject    = %lu\n", st_info->biject);
    // printf("\tdebug 2: unsorted  = %d\n", st_info->unsorted);
    // printf("\tdebug 2: sorted    = %d\n", st_info->sorted);
    // printf("\tdebug 3: multisort = %d\n", multisort);
    // printf("\tdebug 3: level     = %d\n", level);
    // printf("\tdebug 3: countonly = %d\n", st_info->countonly);

    if ( (level > 0) & (skipbycopy == 0) ) {
        if ( kstr > 0 ) {
            st_info->strL_bybytes = malloc(sizeof *st_info->strL_bybytes);;
            st_info->st_by_numx   = malloc(sizeof *st_info->st_by_numx);
            st_info->st_by_charx  = calloc(st_info->J, rowbytes);

            if ( st_info->strL_bybytes == NULL ) return (sf_oom_error("sf_read_byvars", "st_info->strL_bybytes"));
            if ( st_info->st_by_numx   == NULL ) return (sf_oom_error("sf_read_byvars", "st_info->st_by_numx"));
            if ( st_info->st_by_charx  == NULL ) return (sf_oom_error("sf_read_byvars", "st_info->st_by_charx"));

            GTOOLS_GC_ALLOCATED("st_info->strL_bybytes")
            GTOOLS_GC_ALLOCATED("st_info->st_by_numx")
            GTOOLS_GC_ALLOCATED("st_info->st_by_charx")

            for (j = 0; j < st_info->J; j++) {
                memset (st_info->st_by_charx + j * rowbytes, '\0', rowbytes);
                sel = j * rowbytes + st_info->positions[kvars];
                memcpy (st_info->st_by_charx + sel, &j, sizeof(GT_size));
            }

            for (j = 0; j < st_info->J; j++) {
                for (k = 0; k < kvars; k++) {
                    sel  = st_info->ix[st_info->info[j]] * st_info->rowbytes + st_info->positions[k];
                    selx = j * rowbytes + st_info->positions[k];
                    if ( st_info->byvars_lens[k] > 0 ) {
                        memcpy (st_info->st_by_charx + selx,
                                st_info->st_charx + sel,
                                strlen(st_info->st_charx + sel));
                        st_info->strbuffer += strlen(st_info->st_charx + sel);
                    }
                    else {
                        memcpy (st_info->st_by_charx + selx,
                                st_info->st_charx + sel,
                                sizeof(ST_double));
                    }
                }
            }
        }
        else {
            st_info->strL_bybytes = malloc(sizeof *st_info->strL_bybytes);;
            st_info->st_by_numx   = calloc(st_info->J * (kvars + 1), sizeof *st_info->st_by_numx);
            st_info->st_by_charx  = malloc(sizeof *st_info->st_by_charx);

            if ( st_info->strL_bybytes == NULL ) return (sf_oom_error("sf_read_byvars", "st_info->strL_bybytes"));
            if ( st_info->st_by_numx   == NULL ) return (sf_oom_error("sf_read_byvars", "st_info->st_by_numx"));
            if ( st_info->st_by_charx  == NULL ) return (sf_oom_error("sf_read_byvars", "st_info->st_by_charx"));

            GTOOLS_GC_ALLOCATED("st_info->strL_bybytes")
            GTOOLS_GC_ALLOCATED("st_info->st_by_numx")
            GTOOLS_GC_ALLOCATED("st_info->st_by_charx")

            for (j = 0; j < st_info->J; j++) {
                for (k = 0; k < kvars; k++) {
                    sel  = st_info->ix[st_info->info[j]] * kvars + k;
                    selx = j * (kvars + 1) + k;
                    st_info->st_by_numx[selx] = st_info->st_numx[sel];
                }
                st_info->st_by_numx[j * (kvars + 1) + kvars] = j;
            }
        }

        if ( st_info->benchmark > 2 )
            sf_running_timer (&stimer, "\t\tPlugin step 4.2: Keep only one row per group");

        st_info->free = 6;

        // Skip if the user specifies the results need not be sorted
        // (unsorted, countonly). Also skip with the bijection, where
        // you get the sorting for free, or if we determined the data
        // was already sorted.
        //
        // Note here unsorted refers to the Stata option that tells
        // the plugin to not sort the data, whereas sorted refers
        // to the plugin's internal check that determined the data
        // was already sorted.

        if ( (level > 1) &  multisort ) {
            if ( kstr > 0 ) {
                if ( st_info->mlast ) {
                    MultiQuicksortMCMlast (st_info->st_by_charx,
                                           st_info->J,
                                           0,
                                           kvars - 1,
                                           rowbytes,
                                           st_info->byvars_lens,
                                           st_info->invert,
                                           st_info->positions);
                }
                else {
                    MultiQuicksortMC (st_info->st_by_charx,
                                      st_info->J,
                                      0,
                                      kvars - 1,
                                      rowbytes,
                                      st_info->byvars_lens,
                                      st_info->invert,
                                      st_info->positions);
                }
            }
            else {
                if ( st_info->mlast ) {
                    MultiQuicksortDblMlast(st_info->st_by_numx,
                                           st_info->J,
                                           0,
                                           kvars - 1,
                                           (kvars + 1) * sizeof(ST_double),
                                           st_info->invert);
                }
                else {
                    MultiQuicksortDbl(st_info->st_by_numx,
                                      st_info->J,
                                      0,
                                      kvars - 1,
                                      (kvars + 1) * sizeof(ST_double),
                                      st_info->invert);
                }
            }

            if ( st_info->benchmark > 2 )
                sf_running_timer (&stimer, "\t\tPlugin step 4.3: Sorted groups in memory");
        }
    }
    else {

        st_info->st_by_charx  = malloc(sizeof *st_info->st_by_charx);
        st_info->st_by_numx   = malloc(sizeof *st_info->st_by_numx);
        st_info->strL_bybytes = malloc(sizeof *st_info->strL_bybytes);;

        st_info->free = 8;
    }

    free (st_info->st_numx);
    free (st_info->st_charx);

    GTOOLS_GC_FREED("st_info->st_numx")
    GTOOLS_GC_FREED("st_info->st_charx")

    if ( st_info->N < st_info->Nread ) {
        free (st_info->ix);
        GTOOLS_GC_FREED("st_info->ix")
    }

    st_info->ix = calloc(st_info->J, sizeof *st_info->ix);
    if ( st_info->ix == NULL ) sf_oom_error ("sf_check_hash", "st_info->ix");
    GTOOLS_GC_ALLOCATED("st_info->ix")

    if ( (level > 0) & (skipbycopy == 0) ) {
        st_info->free = 7;
        if ( kstr > 0 ) {
            for (j = 0; j < st_info->J; j++) {
                st_info->ix[j] = *((GT_size *) (st_info->st_by_charx + j * rowbytes + st_info->positions[kvars]));
            }
        }
        else {
            for (j = 0; j < st_info->J; j++) {
                st_info->ix[j] = (GT_size) st_info->st_by_numx[j * (kvars + 1) + kvars];
            }
        }
    }
    else {
        // This should also apply if the data is already sorted, bijection, etc.
        // else if ( st_info->kvars_by == 0 ) {
        for (j = 0; j < st_info->J; j++)
            st_info->ix[j] = j;
    }

    if ( st_info->benchmark > 1 )
        sf_running_timer (&timer, "\tPlugin step 4: Created indexed array with sorted by vars");

    free(buf1);
    free(buf2);
    free(buf3);
    free(buf4);

    return (rc);
}
