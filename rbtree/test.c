// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <rbtree.h>
#include <string.h>

struct test_node {
    struct rb_node node;
    unsigned int value;
    unsigned int max;
};

static __always_inline unsigned int
test_max(struct test_node *node)
{
    return (node->value > node->max) ? node->value : node->max;
}

RB_DECLARE_CALLBACKS_MAX(static, test_rb_augment_cb,
                         struct test_node, node,
                         unsigned int, max, test_max)

static struct test_node *
_search(struct rb_root *root, unsigned int value)
{
    struct rb_node *node = root->rb_node;

    while (node) {
        struct test_node *this = rb_entry(node, struct test_node, node);

        if (value < this->value) {
            node = node->rb_left;
        } else if (value > this->value) {
            node = node->rb_right;
        } else {
            return this;
        }
    }

    return NULL;
}

static int
_insert(struct test_node *data,
        struct rb_root *root,
        const struct rb_augment_callbacks *cb)
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &(root->rb_node);

    while (*link) {
        struct test_node *this = rb_entry(*link, struct test_node, node);

        parent = *link;
        if (data->value < this->value)
            link = &((*link)->rb_left);
        else if (data->value > this->value)
            link = &((*link)->rb_right);
        else
            return -1;
    }

    rb_link_node(&data->node, parent, link);
    if (cb)
        rb_insert_augmented(&data->node, root, cb);
    else
        rb_insert_color(&data->node, root);

    return 0;
}

static int
test_insertion(struct rb_root *root,
               struct test_node *reserved_nodes,
               const struct rb_augment_callbacks *cb)
{
    int i;
    for (i = 0; i < 10; i++) {
        reserved_nodes[i].value = i;
        if (_insert(&reserved_nodes[i], root, cb))
            return -1;
    }
    return 0;
}

static int
test_search(struct rb_root *root)
{
    if (_search(root, 5) == NULL)
        return -1;

    if (_search(root, 6) == NULL)
        return -1;

    if (_search(root, 10) != NULL)
        return -1;

    return 0;
}

static int
test_deleting(struct rb_root *root,
              const struct rb_augment_callbacks *cb)
{
    struct test_node *node;
    node = _search(root, 3);
    if (node == NULL)
        return -1;

    if (cb)
        rb_erase_augmented(&(node->node), root, cb);
    else
        rb_erase(&(node->node), root);

    if (_search(root, 3) != NULL)
        return -1;

    return 0;
}

static void
_test_rbtree(struct rb_root *root,
             struct test_node *reserved_nodes,
             const struct rb_augment_callbacks *cb)
{
    if (RB_EMPTY_ROOT(root))
        printk(_GREEN("rbtree is empty!\n"));
    else
        printk(_RED("rbtree is NOT empty!\n"));

    if (test_insertion(root, reserved_nodes, cb))
        printk(_RED("rbtree insert failed!\n"));
    else
        printk(_GREEN("rbtree insert ok!\n"));

    if (!RB_EMPTY_ROOT(root))
        printk(_GREEN("rbtree has nodes!\n"));
    else
        printk(_RED("rbtree is empty!\n"));

    if (test_search(root))
        printk(_RED("rbtree search failed!\n"));
    else
        printk(_GREEN("rbtree search ok!\n"));

    if (test_deleting(root, cb))
        printk(_RED("rbtree delete node failed!\n"));
    else
        printk(_GREEN("rbtree delete node ok!\n"));
}

static void
test_classic_rbtree(void)
{
    struct rb_root rbtree = RB_ROOT;
    struct test_node nodes[10];

    memset(nodes, 0, sizeof(nodes));

    printk(_GREEN("Test classic rbtree ...\n"));
    _test_rbtree(&rbtree, nodes, NULL);
    printk(_GREEN("Test classic rbtree ok!\n"));
}

static void
test_argumented_rbtree(void)
{
    struct rb_root rbtree = RB_ROOT;
    struct test_node nodes[10];

    memset(nodes, 0, sizeof(nodes));

    printk(_GREEN("Test argumented rbtree ...\n"));
    _test_rbtree(&rbtree, nodes, &test_rb_augment_cb);
    printk(_GREEN("Test argumented rbtree ok!\n"));
}

static int
init_module(void)
{
    printk("module[test_rbtree]: init begin ...\n");

    test_classic_rbtree();

    printk("\n");

    test_argumented_rbtree();

    printk("module[test_rbtree]: init end!\n");

    return 0;
}
