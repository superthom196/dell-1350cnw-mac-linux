/* CUPS raster (8-bit RGB or gray) -> concatenated CMYK PAM / PGM pages on stdout.
 * Feeds foo2hbpl1 without needing Ghostscript. Each page is padded with
 * white out to the full paper size, since foo2hbpl1 picks the paper code
 * from the image dimensions.
 * Usage: rastertoppm [raster-file]   (reads stdin when no file given) */
#include <cups/raster.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd = 0;
    if (argc > 1 && (fd = open(argv[1], O_RDONLY)) < 0) {
        perror(argv[1]);
        return 1;
    }

    cups_raster_t *ras = cupsRasterOpen(fd, CUPS_RASTER_READ);
    cups_page_header2_t h;
    int pages = 0;

    while (cupsRasterReadHeader2(ras, &h)) {
        int gray = h.cupsNumColors == 1;
        int bpp = gray ? 1 : 3;
        if (h.cupsBitsPerColor != 8 || (h.cupsNumColors != 3 && !gray)) {
            fprintf(stderr, "ERROR: unsupported raster (%u colors, %u bits)\n",
                    h.cupsNumColors, h.cupsBitsPerColor);
            return 1;
        }

        /* Full page size in device pixels, and where the rendered area sits. */
        unsigned fw = (h.PageSize[0] * h.HWResolution[0] + 36) / 72;
        unsigned fh = (h.PageSize[1] * h.HWResolution[1] + 36) / 72;
        unsigned left = h.ImagingBoundingBox[0] * h.HWResolution[0] / 72;
        unsigned top = (h.PageSize[1] - h.ImagingBoundingBox[3]) * h.HWResolution[1] / 72;
        if (fw < h.cupsWidth) fw = h.cupsWidth;
        if (fh < h.cupsHeight) fh = h.cupsHeight;
        if (left + h.cupsWidth > fw) left = fw - h.cupsWidth;
        if (top + h.cupsHeight > fh) top = fh - h.cupsHeight;

        /* Colour pages go out as CMYK PAM. foo2hbpl1's own RGB conversion
         * prints black as C+M+Y+K, which blurs text; here neutral greys
         * and black use K only. White is all zeros in CMYK. */
        int obpp = gray ? 1 : 4;
        unsigned char *line = malloc(h.cupsBytesPerLine);
        unsigned char *out = malloc((size_t)fw * obpp);
        unsigned char blank = gray ? 255 : 0;
        memset(out, blank, (size_t)fw * obpp);

        if (gray)
            printf("P5\n%u %u\n255\n", fw, fh);
        else
            printf("P7\nWIDTH %u\nHEIGHT %u\nDEPTH 4\nMAXVAL 255\n"
                   "TUPLTYPE CMYK\nENDHDR\n", fw, fh);
        for (unsigned y = 0; y < top; y++)
            fwrite(out, obpp, fw, stdout);
        for (unsigned y = 0; y < h.cupsHeight; y++) {
            cupsRasterReadPixels(ras, line, h.cupsBytesPerLine);
            unsigned char *dp = out + (size_t)left * obpp;
            if (gray) {
                memcpy(dp, line, h.cupsWidth);
            } else {
                for (unsigned x = 0; x < h.cupsWidth; x++, dp += 4) {
                    int r = line[x * 3], g = line[x * 3 + 1], b = line[x * 3 + 2];
                    int m = r > g ? (r > b ? r : b) : (g > b ? g : b);
                    dp[3] = 255 - m;
                    dp[0] = m ? (m - r) * 255 / m : 0;
                    dp[1] = m ? (m - g) * 255 / m : 0;
                    dp[2] = m ? (m - b) * 255 / m : 0;
                }
            }
            fwrite(out, obpp, fw, stdout);
        }
        memset(out, blank, (size_t)fw * obpp);
        for (unsigned y = top + h.cupsHeight; y < fh; y++)
            fwrite(out, obpp, fw, stdout);

        free(line);
        free(out);
        fprintf(stderr, "PAGE: %d 1\n", ++pages);
    }

    cupsRasterClose(ras);
    return pages ? 0 : 1;
}
