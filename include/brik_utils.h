/**
 * @file brik_utils.h
 * @author Ben
 * @date 4 Tue 2019
 * @brief
 *
 * @bug sw decoder not working maybe sdl option.
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 *
 */
#ifndef __BRIK_UTILS_H_
#define __BRIK_UTILS_H_

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif  /* MAX */


#ifndef bool

typedef int bool;
#define true 1
#define false 0
#endif/* bool */

#ifndef UNUSED
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

extern void sps_parse(const unsigned char * pStart, unsigned short nLen, int32_t *frame_width, int32_t *frame_height);

#endif
