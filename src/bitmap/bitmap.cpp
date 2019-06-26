#include <Magick++.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/stat.h>
#else

#endif

#include "../eprintf.hpp"

int main(int argc, char *argv[]) {
    int opt;

    // Data directory
    const char *data = "data/";

    // Tags directory
    const char *tags = "tags/";

    // Go through each argument
    while((opt = getopt(argc, argv, "hd:t:c:")) != -1) {
        switch(opt) {
            case 'd':
                data = optarg;
                break;

            case 't':
                tags = optarg;
                break;

            default:
                eprintf("Usage: %s [options] <bitmap-tag>\n", *argv);
                eprintf("Options:\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --data,-d <path>           Set the data directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory.\n\n");
                eprintf("Bitmap options:\n");
                eprintf("    --format,-f <type>         Format used in tag. Can be: a8, y8, ay8, a8y8,\n");
                eprintf("                               a8r8g8b8, x8r8g8b8, a1r5g5b5, a4r4g4b4, r5g6b5,\n");
                eprintf("                               p8, dxt1, dxt3, dxt5. Default (new tag): a8r8g8b8\n");
                eprintf("    --input-type,-i <type>     Input format. Can be: bmp, png, tag, tiff, tga.\n");
                eprintf("                               Default: tiff\n");
                return EXIT_FAILURE;
        }
    }

    // Make sure we have the bitmap tag path
    if(optind != argc - 1) {
        eprintf("%s: Expected a bitmap tag path. Use -h for help.\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Get the data and tag paths to have a directory separator at the end
    std::string data_path = data;
    if(data_path[data_path.length()] != '/') {
        data_path += "/";
    }
    std::string tags_path = tags;
    if(tags_path[tags_path.length()] != '/') {
        tags_path += "/";
    }

    Magick::InitializeMagick(*argv);



    return 0;
}
