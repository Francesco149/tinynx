/*
    $ gcc example.c
    $ ./a.out /path/to/Map.nx /
    [none] Back
    [none] Effect.img
    [none] Map
    [none] MapHelper.img
    [none] Obj
    [none] Physics.img
    [none] Tile
    [none] WorldMap
*/

#define NX_IMPLEMENTATION
#define NX_NOBITMAP
#include "nx.c"

int main(int argc, char* argv[])
{
    struct nx_file file;
    struct nx_node node, child;

    uint32_t i;
    char buf[0x10000];

    nx_map(&file, argv[1]);
    nx_get(&file, argv[2], &node);

    for (i = 0; i < node.nchildren; ++i)
    {
        nx_node_at(&file, node.first_child_id + i, &child);
        nx_string_at(&file, child.name_id, buf);

        printf("[%s] %s\n", nx_typestr(child.type), buf);
    }

    return 0;
}

