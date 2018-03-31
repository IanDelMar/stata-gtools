#define SQUARE(x) ( (x) * (x) )
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
#define MIN(a, b) ( (a) > (b) ? (b) : (a) )

#define MAX_MATCHES 1

#include "gtools_math.h"
#include "qselect.c"

/**
 * @brief Standard deviation entries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Standard deviation of the elements of @v from @start to @end
 */
ST_double gf_array_dsd_range (const ST_double v[], const GT_size start, const GT_size end) {
    GT_size i;
    ST_double vvar  = 0;
    ST_double vmean = gf_array_dmean_range(v, start, end);
    for (i = start; i < end; i++)
        vvar += SQUARE(v[i] - vmean);
    return (sqrt(vvar / (end - start - 1)));
}

/**
 * @brief Mean of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Mean of the elements of @v from @start to @end
 */
ST_double gf_array_dmean_range (const ST_double v[], const GT_size start, const GT_size end) {
    return (gf_array_dsum_range(v, start, end) / (end - start));
}

/**
 * @brief Sum of entries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Mean of the elements of @v from @start to @end
 */
ST_double gf_array_dsum_range (const ST_double v[], const GT_size start, const GT_size end)
{
    GT_size i;
    ST_double vsum = 0;
    for (i = start; i < end; i++)
        vsum += v[i];
    return (vsum);
}

/**
 * @brief Min of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Min of the elements of @v from @start to @end
 */
ST_double gf_array_dmin_range (const ST_double v[], const GT_size start, const GT_size end)
{
    GT_size i;
    ST_double min = v[start];
    for (i = start + 1; i < end; ++i) {
        if (min > v[i]) min = v[i];
    }
    return (min);
}

/**
 * @brief Max of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Max of the elements of @v from @start to @end
 */
ST_double gf_array_dmax_range (const ST_double v[], const GT_size start, const GT_size end)
{
    GT_size i;
    ST_double max = v[start];
    for (i = start + 1; i < end; ++i) {
        if (max < v[i]) max = v[i];
    }
    return (max);
}

/**
 * @brief Quantile of enries in range of array
 *
 * This computes the (quantile)th quantile using qsort. When
 * computing multiple quantiles, the data will already be sorted for the
 * next iteration, so it's faster than sorting every time, but it it
 * still a VERY inefficient implementation.
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Quantile of the elements of @v from @start to @end
 */
ST_double gf_array_dquantile_range (
    ST_double v[],
    const GT_size start,
    const GT_size end,
    const ST_double quantile)
{
    ST_double q, qdbl, qfoo, Ndbl;
    GT_size   Ndiv, qth;
    GT_bool   rfoo, Nmod;
    GT_size   N  = (end - start);

    // Special cases
    // -------------

    if ( N == 1 ) {
        // If only 1 entry, can't take quantile
        return (v[start]);
    }
    else if ( N == 2 ) {
        // If 2 entries, only 3 options
        if ( quantile > 50 ) {
            return (MAX(v[start], v[end - 1]));
        }
        else if ( quantile < 50 ) {
            return (MIN(v[start], v[end - 1]));
        }
        else {
            return ( (v[start] + v[end - 1]) / 2 );
        }
    }

    // Get position of quantile
    // ------------------------

    Nmod = N % 100;
    Ndbl = (ST_double) N;

    if ( Nmod ) { // Numerical precision foo
        qth  = floor(qdbl = quantile * Ndbl / 100);
        qfoo = round(qdbl);
        rfoo = ((qfoo * 100 / Ndbl) == quantile);
    }
    else {
        Ndiv = N / 100;
        qth  = floor(qdbl = quantile * Ndiv);
        qfoo = round(qdbl);
        rfoo = ((qfoo / Ndiv) == quantile);
    }

    // 0th quantile is not a thing, so we can just take the min
    if ( qth == 0 ) {
        return (gf_array_dmin_range(v, start, end));
    }

    // Full selection algorithm
    // ------------------------

    GT_size left = start, right = end;
    GT_bool dmax = (qth == (N - 1)) | (qfoo == (N - 1));

    if ( rfoo ) {
        if ( dmax ) {
            q = (
                gf_array_dmax_range(v, left, right) +
                gf_qselect_range (v, left, right, qfoo - 1)
            ) / 2;
        }
        else {
            q = (
                gf_qselect_range (v, left, right, qfoo) +
                gf_qselect_range (v, left, right, qfoo - 1)
            ) / 2;
        }
    }
    else {
        if ( dmax ) {
            q = gf_array_dmax_range(v, left, right);
        }
        else{
            q = gf_qselect_range (v, left, right, qth);
        }
    }

    return (q);
}

/**
 * @brief Median of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Median of the elements of @v from @start to @end
 */
ST_double gf_array_dmedian_range (ST_double v[], const GT_size start, const GT_size end)
{
    return (gf_array_dquantile_range(v, start, end, 50));
}

/**
 * @brief IRQ for enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return IRQ for the elements of @v from @start to @end
 */
ST_double gf_array_diqr_range (ST_double v[], const GT_size start, const GT_size end)
{
    return (gf_array_dquantile_range(v, start, end, 75) - gf_array_dquantile_range(v, start, end, 25));
}

/**
 * @brief SE of the mean, sd / sqrt(n)
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return SE of the mean for the elements of @v from @start to @end
 */
ST_double gf_array_dsemean_range (const ST_double v[], const GT_size start, const GT_size end)
{
    return (gf_array_dsd_range(v, start, end) / sqrt(end - start));
}

/**
 * @brief SE of the mean, binomial, sqrt(p * (1 - p) / n)
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return SE of the mean for the elements of @v from @start to @end
 */
ST_double gf_array_dsebinom_range (const ST_double v[], const GT_size start, const GT_size end)
{
    GT_size i;
    ST_double p;
    for (i = start; i < end; i++) {
        if ( (v[i] != ((ST_double) 0)) && (v[i] != ((ST_double) 1)) ) return (SV_missval);
    }
    p = gf_array_dmean_range(v, start, end);
    return (sqrt(p * (1 - p) / (end - start)));
}

/**
 * @brief SE of the mean, poisson, sqrt(mean)
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return SE of the mean for the elements of @v from @start to @end
 */
ST_double gf_array_dsepois_range (const ST_double v[], const GT_size start, const GT_size end)
{
    GT_size i;
    // GT_size vsum = 0;
    for (i = start; i < end; i++) {
        if ( v[i] < 0 ) return (SV_missval);
        // vsum += (GT_size) round(v[i]);
    }

    // ST_double rmean = (GT_int) gf_array_dsum_range(v, start, end);
    ST_double rmean = (GT_int) (gf_array_dsum_range(v, start, end) + 0.5);
    return (sqrt(rmean) / (end - start));
}

/**
 * @brief Skewness
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Skewness of @v from @start to @end
 */
ST_double gf_array_dskew_range (const ST_double v[], const GT_size start, const GT_size end)
{
    GT_size i;
    ST_double s1, s2, aux1, aux2;
    ST_double m2 = 0;
    ST_double m3 = 0;
    ST_double vmean = gf_array_dmean_range(v, start, end);

    for (i = start; i < end; i++) {
        s1  = (v[i] - vmean);
        s2  = s1 * s1;
        m2 += s2;
        m3 += s2 * s1;
    }

    m2 /= (end - start);
    m3 /= (end - start);

    aux1 = sqrt(m2);
    aux2 = aux1 * aux1 * aux1;

    return (m3 / aux2);
}

/**
 * @brief Kurtosis
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Kurtosis of @v from @start to @end
 */
ST_double gf_array_dkurt_range (const ST_double v[], const GT_size start, const GT_size end)
{
    GT_size i;
    ST_double s;
    ST_double m2 = 0;
    ST_double m4 = 0;
    ST_double vmean = gf_array_dmean_range(v, start, end);

    for (i = start; i < end; i++) {
        s   = SQUARE(v[i] - vmean);
        m2 += s;
        m4 += s * s;
    }

    m2 /= (end - start);
    m4 /= (end - start);
    return (m4 / (m2 * m2));
}

/**
 * @brief Wrapper to choose summary function using a string
 *
 * @param fname Character with name of funtion to apply to @v
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return @fname(@v[@start to @end])
 */
ST_double gf_switch_fun (char * fname, ST_double v[], const GT_size start, const GT_size end)
{
    if ( strcmp (fname, "sum")        == 0 ) return (gf_array_dsum_range     (v, start, end));
    if ( strcmp (fname, "mean")       == 0 ) return (gf_array_dmean_range    (v, start, end));
    if ( strcmp (fname, "sd")         == 0 ) return (gf_array_dsd_range      (v, start, end));
    if ( strcmp (fname, "max")        == 0 ) return (gf_array_dmax_range     (v, start, end));
    if ( strcmp (fname, "min")        == 0 ) return (gf_array_dmin_range     (v, start, end));
    if ( strcmp (fname, "median")     == 0 ) return (gf_array_dmedian_range  (v, start, end));
    if ( strcmp (fname, "iqr")        == 0 ) return (gf_array_diqr_range     (v, start, end));
    if ( strcmp (fname, "semean")     == 0 ) return (gf_array_dsemean_range  (v, start, end));
    if ( strcmp (fname, "sebinomial") == 0 ) return (gf_array_dsebinom_range (v, start, end));
    if ( strcmp (fname, "sepoisson ") == 0 ) return (gf_array_dsepois_range  (v, start, end));
    if ( strcmp (fname, "skewness")   == 0 ) return (gf_array_dskew_range    (v, start, end));
    if ( strcmp (fname, "kurtosis")   == 0 ) return (gf_array_dkurt_range    (v, start, end));
    ST_double q = (ST_double) atof(fname);
    return (q > 0? gf_array_dquantile_range(v, start, end, q): 0);
}

/**
 * @brief Wrapper to encode summary function using an integer
 *
 * We use negative numbers so that we can return quantiles as is.
 *
 * @param fname Character with name of funtion
 * @return internal code for summary function
 */
ST_double gf_code_fun (char * fname)
{
    if ( strcmp (fname, "sum")        == 0 ) return (-1);  // sum
    if ( strcmp (fname, "mean")       == 0 ) return (-2);  // mean
    if ( strcmp (fname, "sd")         == 0 ) return (-3);  // sd
    if ( strcmp (fname, "max")        == 0 ) return (-4);  // max
    if ( strcmp (fname, "min")        == 0 ) return (-5);  // min
    if ( strcmp (fname, "count")      == 0 ) return (-6);  // count
    if ( strcmp (fname, "percent")    == 0 ) return (-7);  // percent
    if ( strcmp (fname, "median")     == 0 ) return (50);  // median
    if ( strcmp (fname, "iqr")        == 0 ) return (-9);  // iqr
    if ( strcmp (fname, "first")      == 0 ) return (-10); // first
    if ( strcmp (fname, "firstnm")    == 0 ) return (-11); // firstnm
    if ( strcmp (fname, "last")       == 0 ) return (-12); // last
    if ( strcmp (fname, "lastnm")     == 0 ) return (-13); // lastnm
    if ( strcmp (fname, "semean")     == 0 ) return (-15); // semean
    if ( strcmp (fname, "sebinomial") == 0 ) return (-16); // sebinomial
    if ( strcmp (fname, "sepoisson ") == 0 ) return (-17); // sepoisson
    if ( strcmp (fname, "skewness")   == 0 ) return (-19); // skewness
    if ( strcmp (fname, "kurtosis")   == 0 ) return (-20); // kurtosis
    ST_double q = (ST_double) atof(fname);                 // quantile
    return (q > 0? q: 0);
}

/**
 * @brief Wrapper to choose summary function using internal code
 *
 * See gf_code_fun above
 *
 * @param fcode double with function code
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return @fname(@v[@start to @end])
 */
ST_double gf_switch_fun_code (ST_double fcode, ST_double v[], const GT_size start, const GT_size end)
{
    if ( fcode == -1  )  return (gf_array_dsum_range     (v, start, end)); // sum
    if ( fcode == -2  )  return (gf_array_dmean_range    (v, start, end)); // mean
    if ( fcode == -3  )  return (gf_array_dsd_range      (v, start, end)); // sd)
    if ( fcode == -4  )  return (gf_array_dmax_range     (v, start, end)); // max
    if ( fcode == -5  )  return (gf_array_dmin_range     (v, start, end)); // min
    if ( fcode == -9  )  return (gf_array_diqr_range     (v, start, end)); // iqr
    if ( fcode == -15 )  return (gf_array_dsemean_range  (v, start, end)); // semean
    if ( fcode == -16 )  return (gf_array_dsebinom_range (v, start, end)); // sebinomial
    if ( fcode == -17 )  return (gf_array_dsepois_range  (v, start, end)); // sepoisson
    if ( fcode == -19 )  return (gf_array_dskew_range    (v, start, end)); // skewness
    if ( fcode == -20 )  return (gf_array_dkurt_range    (v, start, end)); // kurtosis
    return (gf_array_dquantile_range(v, start, end, fcode));               // percentiles
}

/**
 * @brief Compare function for qsort
 *
 * @param a First element
 * @param b Second element
 * @return @a - @b
 */
int gf_qsort_compare (const void * a, const void * b)
{
    return ( (int) *(ST_double*)a - *(ST_double*)b );
}

/**
 * @brief Determine if of enries in range of array are sorted
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Whether the elements of @v are sorted from @start to @end
 */
GT_bool gf_array_dsorted_range (const ST_double v[], const GT_size start, const GT_size end) {
    GT_size i;
    for (i = start + 1; i < end; i++) {
        if (v[i - 1] > v[i]) return (0);
    }
    return (1);
}
