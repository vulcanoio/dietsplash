#include "pnmtologo.h"
#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *USAGE = "USAGE: genstaticlogo static_struct_name outfile.c file1.ppm [ file2.ppm ... ]";

static inline void write_header(FILE *out)
{
    fputs("/*\n"                                                \
          " *  DO NOT EDIT THIS FILE!\n"                        \
          " *\n"                                                \
          " *  It was automatically generated from image file\n"\
          " *\n"                                                \
          " *  Static dietsplash image\n"                        \
          " */\n\n"                                             \
          "#include \"pnmtologo.h\"\n\n"                        \
          "#ifndef __DIETSPLASH_STATICLOGO_C\n"                 \
          "#define __DIETSPLASH_STATICLOGO_C\n\n", out);
}

static inline void write_logo(FILE *out, struct image *logo, int imgidx,
                              const char *struct_name)
{
    long i;

    if (imgidx >= 0)
        fprintf(out, "static struct image %s%d = {\n", struct_name, imgidx);
    else
        fprintf(out, "static struct image %s = {\n", struct_name);

    fprintf(out, "    .width = %d,\n", logo->width);
    fprintf(out, "    .height = %d,\n", logo->height);

    fputs("    .pixels = {\n", out);
    for (i = 0; i < logo->width * logo->height; i++) {
        fprintf(out, "        { 0x%02x, 0x%02x, 0x%02x },\n",
                logo->pixels[i].red,
                logo->pixels[i].green,
                logo->pixels[i].blue);
    }
    fputs("    }\n};\n\n", out);
}

static inline void write_footer(FILE *out, int n_images,
                                const char *struct_name)
{
    int i;

    if (n_images > 1) {
        fprintf(out, "static struct image *%s[] = {\n", struct_name);
        for (i = 0; i < n_images; i++)
            fprintf(out, "    &%s%d,\n", struct_name, i);

        fputs("};\n\n", out);
    }

    fputs("#endif", out);
}

int main(int argc, char *argv[])
{
    const char *filename_out = NULL;
    int i, multiple_files = 0;
    const char *static_struct_name;
    static FILE *fp_out;

    if (argc < 4)
        die("%s", USAGE);

    if (argc > 4)
        multiple_files = 1;

    static_struct_name = argv[1];
    filename_out = argv[2];

    /* open logo file */
    if (strcmp(filename_out, "-")) {
	fp_out = fopen(filename_out, "w");
	if (!fp_out)
	    die("Cannot create file %s: %s\n", filename_out, strerror(errno));
    } else {
	fp_out = stdout;
    }

    write_header(fp_out);

    for (i = 3; i < argc; i++) {
        struct image *logo = ds_read_image(argv[i]);
        if (!logo)
            die("Cannot read file %s\n", argv[i]);

        write_logo(fp_out, logo, i - 4 + multiple_files, static_struct_name);
        free(logo);
    }

    write_footer(fp_out, argc - 3, static_struct_name);

    fclose(fp_out);

    return 0;
}
