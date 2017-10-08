#define NX_IMPLEMENTATION
#define NX_MONOLITHIC
#include "../nx.c"

#ifdef _WIN32
#include <io.h> /* _mktemp */

#define EXESUFFIX ".exe"
#define PATH_SEPARATOR "\\"
#else
#define EXESUFFIX
#define PATH_SEPARATOR "/"
#endif

#include <inttypes.h> /* PRI fmt specifiers */

#define global static

/* ------------------------------------------------------------- */

#ifdef _WIN32
global char binaryout_path[MAX_PATH + 1];
#endif

global FILE* binaryout = 0;

#ifndef NX_NOBITMAP
internalfn
void write2(uint16_t v)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)(v >> 8);
    fwrite(buf, 1, 2, binaryout);
}

internalfn
void write4(uint32_t v)
{
    uint8_t buf[4];

    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)((v >> 8) & 0xFF);
    buf[2] = (uint8_t)((v >> 16) & 0xFF);
    buf[3] = (uint8_t)((v >> 24) & 0xFF);

    fwrite(buf, 1, 4, binaryout);
}
#endif

/* ------------------------------------------------------------- */

global char buf[0x10000];

internalfn
void usage()
{
    info(
        "usage: nx command parameters path/to/nx/file.nx:"
        "/path/to/node\n"
        "\n"
        "-v\n"
        "    prints version information\n"
        "\n"
        "info path/to/file.nx:path/to/node\n"
        "    shows information about the node. also displays "
        "header data if\n    path points to the root node\n"
        "\n"
        "ls path/to/file.nx:path/to/node\n"
        "    lists children nodes with a short representation of "
        "the content\n"
        "\n"
        "cat path/to/file.nx:path/to/node\n"
        "    outputs the node's contents. bitmaps and audio are "
        "outputted as\n"
        "    raw binary data, so pipe those to a file or a "
        "player/viewer\n"
        "\n"
    );
}

internalfn
int32_t nodeinfo(struct nx_file* file, struct nx_node* node)
{
    if (!node->id)
    {
        printf(
            "nodes: %u\n"
            "nodes_offset: %" PRIu64 "x\n"
            "strings: %u\n"
            "strings_offset: %" PRIu64 "x\n"
            "bitmaps: %u\n"
            "bitmaps_offset: %" PRIu64 "x\n"
            "audio: %u\n"
            "audio_offset: %" PRIu64 "x\n",
            file->nnodes, file->nodes_offset, file->nstrings,
            file->strings_offset, file->nbitmaps,
            file->bitmaps_offset, file->naudio,
            file->audio_offset
        );
    }

    printf(
        "name: %s\n"
        "children_start_id: %u\n"
        "children: %hu\n"
        "type: %s\n",
        buf, node->first_child_id, node->nchildren,
        nx_typestr(node->type)
    );

    return 0;
}

#ifdef _WIN32
internalfn
void binary_start(char const* suffix)
{
    char tmppath[261];
    uint32_t n;

    n = GetTempPathA(261, tmppath);
    if (!n) {
        printgle("GetTempPathA");
        exit(1);
    }
    
    /* I tried everything and couldn't get it
    to output binary data to stdout on windows */
    if (!GetTempFileNameA(tmppath, "nx", 0, binaryout_path)) {
        printgle("GetTempFileNameA");
        exit(1);
    }

    strcpy(binaryout_path + strlen(binaryout_path), suffix);

    binaryout = fopen(binaryout_path, "wb");
    if (!binaryout) {
        perror("fopen");
        exit(1);
    }

    info("W: outputting to a tmp file instead because windows "
        "sucks\n");
}

internalfn
void binary_end()
{
    char cmdbuf[1024];
    fclose(binaryout);
    binaryout = stdout;
    sprintf(cmdbuf, "explorer %s", binaryout_path);
    system(cmdbuf);
}
#else
#define binary_start(x)
#define binary_end()
#endif

internalfn
int32_t cat(struct nx_file* file, struct nx_node* node)
{
    int32_t res = 0;

    switch (node->type)
    {
    case NX_NONE: break;

    case NX_INT64:
        printf("%" PRId64 "\n", (int64_t)read8(node->data));
        break;

    case NX_REAL:
        printf("%g\n", read_double(node->data));
        break;

    case NX_STRING:
        res = nx_string_at(file, read4(node->data), buf);
        if (res < 0) {
            break;
        }
        puts(buf);
        break;

    case NX_VECTOR:
        printf("(%d, %d)\n", (int32_t)read4(node->data),
            (int32_t)read4(node->data + 4));
        break;

#ifndef NX_NOBITMAP
    case NX_BITMAP:
    {
        uint8_t* pixels = 0;
        uint16_t width = read2(node->data + 4);
        uint16_t height = read2(node->data + 6);
        int32_t uncompressed_size = width * height * 4;

        pixels = (uint8_t*)malloc(uncompressed_size);
        if (!pixels) {
            res = NX_EOOM;
            break;
        }

        res = nx_bitmap_at(file, read4(node->data), pixels,
            uncompressed_size);
        if (res < 0) {
            break;
        }

        binary_start(".bmp");

        /* bfType = BM (windblows) */
        fprintf(binaryout, "BM");
        write4(14 + 40 + 4 * 4 + uncompressed_size); /* bfSize */
        write2(0);
        write2(0);
        write4(14 + 40); /* bfOffBits */

        write4(40); /* biSize */
        write4(width);
        write4((uint32_t)(-(int32_t)height)); /* top-down */
        write2(1); /* biPlanes */
        write2(32); /* biBitCount */
        write4(3); /* biCompression =  BI_BITFIELDS */
        write4(uncompressed_size); /* biSizeImage */
        write4(2835); /* biXPelsPerMeter */
        write4(2835); /* biYPelsPerMeter */
        write4(0); /* biClrUsed */
        write4(0); /* biClrImportant */

        /* BGRA8888 -> RGBA8888 color masks */
        fwrite(
            "\x00\x00\xFF\x00"
            "\x00\xFF\x00\x00"
            "\xFF\x00\x00\x00"
            "\x00\x00\x00\xFF",
            4 * 4, 1, binaryout
        );

        fwrite(pixels, uncompressed_size, 1, binaryout);

        binary_end();
        break;
    }
#endif

    case NX_AUDIO:
    {
        int32_t error;
        uint8_t const* raw_audio =
            nx_audio_at(file, read4(node->data), &error);
        if (!raw_audio || error < 0) {
            info("nx_audio_at failed\n");
        }

        binary_start(".mp3");
        fwrite(raw_audio, read4(node->data + 4), 1, binaryout);
        binary_end();
        break;
    }

    default:
    {
        uint8_t i;
        printf("unknown type %hu: ", node->type);
        for (i = 0; i < 8; ++i) printf("%02X ", node->data[i]);
        puts("");
    }
    }

    return res;
}

internalfn
int32_t ls(struct nx_file* file, struct nx_node* node)
{
    int32_t res;

    int32_t i;
    struct nx_node child;

    for (i = 0; i < node->nchildren; ++i)
    {
        res = nx_node_at(file, node->first_child_id + i, &child);
        if (res < 0) {
            return res;
        }

        res = nx_string_at(file, child.name_id, buf);
        if (res < 0) {
            return res;
        }

        printf("[%s] %s -> ", nx_typestr(child.type), buf);

        switch (child.type)
        {
        case NX_NONE:
            printf("(%u children)\n", child.nchildren);
            break;

        case NX_STRING:
            res = nx_string_at(file, read4(child.data), buf);
            if (res < 0) {
                return res;
            }

            printf("%.64s%s\n", buf,
                strlen(buf) > 64 ? "..." : "");
            break;

        case NX_BITMAP:
            printf("%hdx%hd pixels\n", read2(child.data + 4),
                read2(child.data + 6));
            break;

        case NX_AUDIO:
            printf("%u bytes\n", read4(child.data + 4));
            break;

        case NX_INT64:
        case NX_REAL:
        case NX_VECTOR:
        default:
            cat(file, &child);
            break;
        }
    }

    return 0;
}

#define mymax(a, b) ((a) > (b) ? (a) : (b))
#define has_suffix(str, suffix) \
    (!strcmp(mymax((str), (str) + strlen(str) - strlen(suffix)), \
        (suffix)))

internalfn
int32_t run(int argc, char* argv[])
{
    int32_t res;

    char* p;
    char const* file_path;
    char const* node_path;

    struct nx_file file;
    struct nx_node node;

    binaryout = stdout;

    if (argc > 1 && *argv[1] == '-' && argv[1][1] == 'v')
    {
        printf("%d.%d.%d\n",
            NX_VERSION_MAJOR, NX_VERSION_MINOR, NX_VERSION_PATCH);
        return 0;
    }

    /* if we're not being symlinked as a command, require
       command name and shift args by 1 */
    if (has_suffix(argv[0], PATH_SEPARATOR "nx" EXESUFFIX))
    {
        if (argc < 3) {
            usage();
            return NX_ESYNTAX;
        }

        --argc, ++argv;
    }

    if (argc < 2)
    {
        info("Usage: %s path/to/nx/file.nx:"
            "/path/to/node\n", argv[0]);
        return NX_ESYNTAX;
    }

    /* parse path */
    p = (char*)argv[argc - 1];
    file_path = p;
    for (; *p && *p != ':'; ++p);

    if (!*p) {
        node_path = "";
    } else {
        *p++ = 0;
        node_path = p;
    }

    /* load nx file */
    res = nx_map(&file, file_path);
    if (res < 0) {
        goto cleanup;
    }

    /* get node by path */
    res = nx_get(&file, node_path, &node);
    if (res < 0) {
        goto cleanup;
    }

    /* handle commands */
    /* todo: generate a perfect hash for this stuff? */
    if (has_suffix(argv[0], "info")) res = nodeinfo(&file, &node);
    else if (has_suffix(argv[0], "ls")) res = ls(&file, &node);
    else if (has_suffix(argv[0], "cat")) res = cat(&file, &node);

    else {
        usage();
        res = NX_ESYNTAX;
    }

cleanup:
    nx_unmap(&file);
    return res;
}

int main(int argc, char* argv[])
{
    int32_t err;

    err = run(argc, argv);
    if (err < 0) {
        info("%s\n", nx_errstr(err));
        return 1;
    }

    return 0;
}

