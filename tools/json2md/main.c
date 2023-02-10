// SPDX-License-Identifier: GPL-2.0-only

#include <stdio.h>
#include <string.h>
#include <json.h>
#include <assert.h>

static FILE *
make_markdown_header(const char *json_file)
{
    char md_file[256];
    char *p = strrchr(json_file, '.');
    int len = p - json_file;
    memcpy(md_file, json_file, len);
    memcpy(md_file + len, ".md\0", 4);

    FILE *fp = fopen(md_file, "wb");
    fprintf(fp, "### Profile: %s\n", json_file);
    fprintf(fp, "```mermaid\n");
    fprintf(fp, "graph TD\n");
    return fp;
}

static void
complete_markdown(FILE *fp)
{
    fprintf(fp, "```\n");
    fclose(fp);
}

static void
mark_candidates_group(struct json_object *fixups, FILE *fp)
{
    if (fixups == NULL)
        return;

    for (int i = 0; i < json_object_array_length(fixups); i++) {
        json_object *fixup = json_object_array_get_idx(fixups, i);
        if (fixup == NULL)
            continue;

        json_object *candidates = NULL;
        json_object_object_get_ex(fixup, "candidates", &candidates);
        if (candidates == NULL)
            continue;

        json_object *select = NULL;
        json_object_object_get_ex(fixup, "select", &select);
        assert(select);
        const char *str_select = json_object_get_string(select);

        fprintf(fp, "subgraph %d\n", i);
        fprintf(fp, "direction LR\n");
        for (int i = 0; i < json_object_array_length(candidates); i++) {
            json_object *candidate = json_object_array_get_idx(candidates, i);
            const char *n = json_object_get_string(candidate);
            if (str_select && strncmp(str_select, n, strlen(n)) == 0)
                fprintf(fp, "%s[[%s]]\n", n, n);
            else
                fprintf(fp, "%s\n", n);

        }
        fprintf(fp, "end\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s profile\n", argv[0]);
        exit(-1);
    }

    json_object *root = json_object_from_file(argv[1]);
    if (root == NULL) {
        printf("cannot open profile '%s'\n", argv[1]);
        exit(-1);
    }

    FILE *fp = make_markdown_header(argv[1]);
    assert(fp != NULL);

    struct json_object *fixups= NULL;
    json_object_object_get_ex(root, "fixups", &fixups);
    mark_candidates_group(fixups, fp);

    struct json_object *depend= NULL;
    json_object_object_get_ex(root, "dependencies", &depend);
    if (depend == NULL) {
        printf("no dependencies\n");
        exit(-1);
    }

    json_object_object_foreach(depend, key, val) {
        int num = json_object_array_length(val);
        if (num == 0)
            continue;

        fprintf(fp, "%s --> ", key);
        for (int i = 0; i < num; i++) {
            json_object *obj = json_object_array_get_idx(val, i);
            const char *n = json_object_get_string(obj);
            if (i == (num - 1))
                fprintf(fp, "%s\n", n);
            else
                fprintf(fp, "%s & ", n);
        }
    }

    complete_markdown(fp);
    return 0;
}
