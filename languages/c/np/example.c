// Examples of quick hash tables and dynamic arrays in C
// https://nullprogram.com/blog/2025/01/19/
// This is free and unencumbered software released into the public domain.
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define new(a, n, t)    (t *)alloc(a, n, sizeof(t), _Alignof(t))
#define countof(a)      ((ptrdiff_t)(sizeof(a) / sizeof(*(a))))
#define S(s)            (Str){s, sizeof(s)-1}

typedef struct {
    char *beg;
    char *end;
} Arena;

void *alloc(Arena *a, ptrdiff_t count, ptrdiff_t size, ptrdiff_t align)
{
    ptrdiff_t pad = -(uintptr_t)a->beg & (align - 1);
    assert(count < (a->end - a->beg - pad)/size);  // TODO: OOM policy
    void *r = a->beg + pad;
    a->beg += pad + count*size;
    return memset(r, 0, count*size);
}

typedef struct {
    char     *data;
    ptrdiff_t len;
} Str;

Str copy(Arena *a, Str s)
{
    Str r = s;
    r.data = new(a, s.len, char);
    if (r.len) memcpy(r.data, s.data, r.len);
    return r;
}

Str concat(Arena *a, Str head, Str tail)
{
    if (!head.data || head.data+head.len != a->beg) {
        head = copy(a, head);
    }
    head.len += copy(a, tail).len;
    return head;
}

_Bool equals(Str a, Str b)
{
    if (a.len != b.len) {
        return 0;
    }
    return !a.len || !memcmp(a.data, b.data, a.len);
}

uint64_t hash64(Str s)
{
    uint64_t h = 0x100;
    for (ptrdiff_t i = 0; i < s.len; i++) {
        h ^= s.data[i] & 255;
        h *= 1111111111111111111;
    }
    return h;
}

Str print(Arena *a, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ptrdiff_t cap = a->end - a->beg;
    ptrdiff_t len = vsnprintf(a->beg, cap, fmt, ap);
    va_end(ap);

    Str r = {0};
    if (len<0 || len>=cap) {
        return r;  // TODO: trigger OOM
    }
    r.data  = a->beg;
    r.len   = len;
    a->beg += r.len;
    return r;
}


// Slice (push)

// Evalutes S many times and A possibly zero times.
#define push(a, s) \
    ((s)->len == (s)->cap \
        ? (s)->data = push_((a), (s)->data, &(s)->cap, sizeof(*(s)->data)), \
          (s)->data + (s)->len++ \
        : (s)->data + (s)->len++)

void *push_(Arena *a, void *data, ptrdiff_t *pcap, ptrdiff_t size)
{
    ptrdiff_t cap   = *pcap;
    ptrdiff_t align = _Alignof(void *);

    if (!data || a->beg != (char *)data + cap*size) {
        void *copy = alloc(a, cap, size, align);  // copy to bump pointer
        if (data) memcpy(copy, data, cap*size);
        data = copy;
    }

    ptrdiff_t extend = cap ? cap : 4;
    alloc(a, extend, size, 1);  // grow the backing buffer
    *pcap = cap + extend;
    return data;
}

typedef struct {
    Str      *data;
    ptrdiff_t len;
    ptrdiff_t cap;
} StrSlice;

void push_demo(Arena scratch)
{
    StrSlice words = {0};
    for (int i = 0; i < 256; i++) {
        Str word = print(&scratch, "word%d", i);
        *push(&scratch, &words) = word;
    }

    Str element = words.data[100];
    printf("%.*s\n", (int)element.len, element.data);
}


// Slice (append)

StrSlice clone(Arena *a, StrSlice s)
{
    StrSlice r = {0};
    r.len = r.cap = s.len;
    r.data = new(a, s.len, Str);
    for (ptrdiff_t i = 0; i < s.len; i++) {
        r.data[i] = s.data[i];
    }
    return r;
}

StrSlice append(Arena *a, StrSlice s, Str v)
{
    if (s.len == s.cap) {
        if (!s.data || (void *)(s.data + s.len) != a->beg) {
            s = clone(a, s);  // copy to bump pointer
        }
        ptrdiff_t extend = s.cap ? s.cap : 4;
        new(a, extend, Str);  // grow the backing buffer
        s.cap += extend;
    }
    s.data[s.len++] = v;
    return s;
}

void append_demo(Arena scratch)
{
    StrSlice words = {0};
    for (int i = 0; i < 256; i++) {
        Str word = print(&scratch, "word%d", i);
        words = append(&scratch, words, word);
    }

    Str element = words.data[100];
    printf("%.*s\n", (int)element.len, element.data);
}


// MSI

enum { ENVEXP = 10 };  // support up to 1,000 unique keys
typedef struct {
    Str keys[1<<ENVEXP];
    Str vals[1<<ENVEXP];
} FlatEnv;

Str *flatlookup(FlatEnv *env, Str key)
{
    uint64_t hash = hash64(key);
    uint32_t mask = (1<<ENVEXP) - 1;
    uint32_t step = (uint32_t)(hash>>(64 - ENVEXP)) | 1;
    for (int32_t i = (int32_t)hash;;) {
        i = (i + step) & mask;
        if (!env->keys[i].data) {
            env->keys[i] = key;
            return env->vals + i;
        } else if (equals(env->keys[i], key)) {
            return env->vals + i;
        }
    }
}

char **flat_to_envp(FlatEnv *env, Arena *a)
{
    int    cap  = 1<<ENVEXP;
    char **envp = new(a, cap, char *);
    int len = 0;
    for (int i = 0; i < cap; i++) {
        if (env->vals[i].data) {
            Str pair = env->keys[i];
            pair = concat(a, pair, S("="));
            pair = concat(a, pair, env->vals[i]);
            pair = concat(a, pair, S("\0"));
            envp[len++] = pair.data;
        }
    }
    return envp;
}

void msi_demo(Arena scratch)
{
    FlatEnv *env = new(&scratch, 1, FlatEnv);

    for (int i = 0; i < 256; i++) {
        Str key   = print(&scratch, "key%d", i);
        Str value = print(&scratch, "value%d", i);
        *flatlookup(env, key) = value;
    }

    Str value = *flatlookup(env, S("key100"));
    printf("%.*s\n", (int)value.len, value.data);
}


// Hash Trie

typedef struct Env Env;
struct Env {
    Env *child[4];
    Str  key;
    Str  value;
};

Str *lookup(Env **env, Str key, Arena *a)
{
    for (uint64_t h = hash64(key); *env; h <<= 2) {
        if (equals(key, (*env)->key)) {
            return &(*env)->value;
        }
        env = &(*env)->child[h>>62];
    }
    if (!a) return 0;
    *env = new(a, 1, Env);
    (*env)->key = key;
    return &(*env)->value;
}

typedef struct {
    char    **data;
    ptrdiff_t len;
    ptrdiff_t cap;
} EnvpSlice;

EnvpSlice env_to_envp_(EnvpSlice r, Env *env, Arena *a)
{
    if (env) {
        Str pair = env->key;
        pair = concat(a, pair, S("="));
        pair = concat(a, pair, env->value);
        pair = concat(a, pair, S("\0"));
        *push(a, &r) = pair.data;
        for (int i = 0; i < countof(env->child); i++) {
            r = env_to_envp_(r, env->child[i], a);
        }
    }
    return r;
}

char **env_to_envp(Env *env, Arena *a)
{
    EnvpSlice r = {0};
    r = env_to_envp_(r, env, a);
    push(a, &r);
    return r.data;
}

char **env_to_envp_safe(Env *env, Arena *a)
{
    EnvpSlice r = {0};

    typedef struct {
        Env *env;
        int  index;
    } Frame;
    Frame init[16];  // small size optimization

    struct {
        Frame    *data;
        ptrdiff_t len;
        ptrdiff_t cap;
    } stack = {init, 0, countof(init)};

    *push(a, &stack) = (Frame){env, 0};
    while (stack.len) {
        Frame *top = stack.data + stack.len - 1;

        if (!top->env) {
            stack.len--;

        } else if (top->index == countof(top->env->child)) {
            Str pair = top->env->key;
            pair = concat(a, pair, S("="));
            pair = concat(a, pair, top->env->value);
            pair = concat(a, pair, S("\0"));
            *push(a, &r) = pair.data;
            stack.len--;

        } else {
            int i = top->index++;
            *push(a, &stack) = (Frame){top->env->child[i], 0};
        }
    }

    push(a, &r);
    return r.data;
}

void hashtrie_demo(Arena scratch)
{
    Env *env = 0;

    for (int i = 0; i < 256; i++) {
        Str key   = print(&scratch, "key%d", i);
        Str value = print(&scratch, "value%d", i);
        *lookup(&env, key, &scratch) = value;
    }

    Str value = *lookup(&env, S("key100"), 0);
    printf("%.*s\n", (int)value.len, value.data);
}


// Test

int main(void)
{
    int   cap = 1<<24;
    char *mem = malloc(cap);
    Arena a   = {mem, mem+(cap)};

    msi_demo(a);
    hashtrie_demo(a);
    push_demo(a);
    append_demo(a);
}
