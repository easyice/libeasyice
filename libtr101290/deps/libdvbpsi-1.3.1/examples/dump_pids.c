#include "config.h"

#if defined(HAVE_INTTYPES_H)
#   include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#   include <stdint.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>

#ifdef WIN32
#   define O_NONBLOCK (0) /* O_NONBLOCK does not exist for Windows */
#endif

static inline uint32_t ts_getcc(uint8_t *packet)
{
    return (packet[3] & 0x0f);
}

static inline uint32_t ts_getpid(uint8_t *packet)
{
    assert(packet[0] == 0x47);
    return ((uint16_t)(packet[1] & 0x1f) << 8) + packet[2];
}

int main(int argc, char *argv[])
{
    int file = open(argv[1], O_RDONLY | O_NONBLOCK);
    if (file < 0) {
        perror(argv[1]);
        printf("error opening %s\n", argv[1]);
        return -1;
    }

    int32_t s = 188; /* COULD ALSO BE 192 */
    uint8_t p[188] = { 0 };
    int64_t n = 0;
    size_t  len = 0;
    for (;;) {

        /* slow read */
        len = read(file, &p[0], s);
        if (len == 0)
            break;
        uint32_t pid = ts_getpid(&p[0]);
        uint32_t cc  = ts_getcc(&p[0]);
        n++;
        printf("packet %"PRId64", pid %u (0x%x), cc %d\n", n, pid, pid, cc );
    }

    close(file);
    return 0;
}
