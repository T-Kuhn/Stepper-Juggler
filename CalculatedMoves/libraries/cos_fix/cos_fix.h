/*
 * cos_fix.h: Fixed-point cos() and sin() functions, based on a sixth
 * degree polynomial approximation.
 *
 * argument is in units of 2*M_PI/2^16.
 * result is in units of 1/2^14 (range = [-2^14 : 2^14]).
 *
 * The cosine function uses an even-polynomial approximation of
 * cos(M_PI/2*x) for x in [0:1], and symmetries when x is outside [0:1].
 * Sine is defined as sin(x) = cos(3*M_PI/2+x).
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
 * Sixth degree polynomial:
 *      cos(M_PI/2*x) ~ (1 - x^2)*(1 - x^2*(0.23352 - 0.019531*x^2))
 * for x in [0:1]. Max error = 9.53e-5
 */
    int16_t cos_fix(uint16_t x);

    /*
 * Fixed point sin().
 */
    static inline int16_t sin_fix(uint16_t x)
    {
        return cos_fix(0xc000 + x);
    }

#ifdef __cplusplus
}
#endif