// https://gist.githubusercontent.com/pervognsen/2d48ef9757ee3fd579179239febc817e/raw/b21b2fef87bea52077c21e7edb428ebed75a4c6f/BTree.cpp
// https://shachaf.net/w/b-trees
// https://news.ycombinator.com/item?id=23001831
enum { BMAX = 32, BMIN = BMAX / 2, BHEIGHT = 6 };

struct BNode {
    uint32_t length;
    Key keys[BMAX];
    union {
        BNode *children[BMAX];
        Value values[BMAX];
    };
};

static void BNode_Initialize(BNode *node, uint32_t length, Key *keys, void *children) {
    node->length = length;
    CopyMemory(node->keys, keys, length * sizeof(Key));
    CopyMemory(node->children, children, length * sizeof(BNode *));
}

static BNode *BNode_Create(uint32_t length, Key *keys, void *children) {
    BNode *node = (BNode *)Allocate(sizeof(BNode));
    BNode_Initialize(node, length, keys, children);
    return node;
}

static void BNode_Destroy(BNode *node, uint32_t height) {
    for (uint32_t index = 0; index < node->length; index++) {
        if (height > 1) {
            BNode_Destroy(node->children[index], height - 1);
        }
        Free(node->children[index]);
    }
}

static INLINE Key BNode_GetMaxKey(BNode *node) {
    Assert(node->length > 0);
    return node->keys[node->length - 1];
}

static BNode *BNodeLeaf_Insert(BNode *leaf, Key key, Value value) {
    uint32_t index = SearchKeys(leaf->keys, leaf->length, key);
    if (index < leaf->length && leaf->keys[index] == key) {
        leaf->values[index] = value;
        return 0;
    }
    BNode *new_sibling = 0;
    if (leaf->length == BMAX) {
        new_sibling = BNode_Create(BMIN, leaf->keys + BMIN, leaf->values + BMIN);
        leaf->length = BMIN;
        if (index >= BMIN) {
            leaf = new_sibling;
            index -= BMIN;
        }
    }
    Array_Insert(leaf->keys, leaf->length, index, key);
    Array_Insert(leaf->values, leaf->length, index, value);
    leaf->length++;
    return new_sibling;
}

static BNode *BNode_Insert(BNode *node, Key key, Value value, uint32_t height) {
    Assert(height > 0);
    Assert(node->length > 0);
    uint32_t index = SearchKeys(node->keys, node->length, key);
    if (index == node->length) {
        index--;
        node->keys[index] = key;
    }
    BNode *new_child;
    if (height == 1) {
        new_child = BNodeLeaf_Insert(node->children[index], key, value);
    } else {
        new_child = BNode_Insert(node->children[index], key, value, height - 1);
    }
    BNode *new_sibling = 0;
    if (new_child) {
        if (node->length == BMAX) {
            new_sibling = BNode_Create(BMIN, node->keys + BMIN, node->children + BMIN);
            node->length = BMIN;
            if (index >= BMIN) {
                node = new_sibling;
                index -= BMIN;
            }
        }
        node->keys[index] = BNode_GetMaxKey(node->children[index]);
        Array_Insert(node->keys, node->length, index + 1, BNode_GetMaxKey(new_child));
        Array_Insert(node->children, node->length, index + 1, new_child);
        node->length++;
    }
    return new_sibling;
}

static bool BNodeLeaf_Delete(BNode *leaf, Key key) {
    uint32_t index = SearchKeys(leaf->keys, leaf->length, key);
    if (index < leaf->length && leaf->keys[index] == key) {
        Array_Delete(leaf->keys, leaf->length, index);
        Array_Delete(leaf->values, leaf->length, index);
        leaf->length--;
        return leaf->length == 0;
    }
    return false;
}

static void BNode_Delete(BNode *node, Key key, uint32_t height) {
    Assert(height > 0);
    uint32_t index = SearchKeys(node->keys, node->length, key);
    if (index < node->length) {
        if (height == 1) {
            if (BNodeLeaf_Delete(node->children[index], key) && node->length > 1) {
                Free(node->children[index]);
                Array_Delete(node->keys, node->length, index);
                Array_Delete(node->children, node->length, index);
                node->length--;
            }
        } else {
            BNode_Delete(node->children[index], key, height - 1);
        }
    }
}

struct BTree {
    uint32_t height;
    BNode root;
};

void BTree_Initialize(BTree *tree) {
    Assert(BMAX == 2 * BMIN);
    Assert(sizeof(BNode *) == sizeof(Value));
    tree->height = 0;
    tree->root.length = 0;
}

void BTree_Destroy(BTree *tree) {
    if (tree->height > 0) {
        BNode_Destroy(&tree->root, tree->height);
    }
}

Value *BTree_Find(BTree *tree, Key key) {
    uint32_t height = tree->height;
    BNode *node = &tree->root;
    for (;;) {
        uint32_t index = SearchKeys(node->keys, node->length, key);
        if (index == node->length) {
            return 0;
        }
        if (height == 0) {
            return (node->keys[index] == key) ? (node->values + index) : 0;
        }
        height--;
        node = node->children[index];
    }
}

void BTree_Insert(BTree *tree, Key key, Value value) {
    BNode *root = &tree->root;
    BNode *new_root_sibling;
    if (tree->height == 0) {
        new_root_sibling = BNodeLeaf_Insert(root, key, value);
    } else {
        new_root_sibling = BNode_Insert(root, key, value, tree->height);
    }
    if (new_root_sibling) {
        BNode *old_root = BNode_Create(root->length, root->keys, root->children);
        Key keys[2] = {BNode_GetMaxKey(old_root), BNode_GetMaxKey(new_root_sibling)};
        BNode *children[2] = {old_root, new_root_sibling};
        BNode_Initialize(root, 2, keys, children);
        tree->height++;
    }
}

void BTree_Delete(BTree *tree, Key key) {
    if (tree->height == 0) {
        BNodeLeaf_Delete(&tree->root, key);
    } else {
        BNode_Delete(&tree->root, key, tree->height);
    }
}
