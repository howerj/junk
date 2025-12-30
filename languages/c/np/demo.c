// Requires GNU C23: GCC >= 15 or Clang >= 21
#include <stddef.h>
#include <string.h>

#define affirm(c)       while (!(c)) unreachable()
#define lenof(a)        (iz)(sizeof(a) / sizeof(*(a)))
#define new(a, n, t)    (t *)alloc(a, n, sizeof(t), _Alignof(t))
#define maxof(t)        ((t)-1<1 ? (((t)1<<(sizeof(t)*8-2))-1)*2+1 : (t)-1)
#define S(s)            (Str){(u8 *)s, lenof(s)-1}

typedef unsigned char           u8;
typedef int                     b32;
typedef int                     i32;
typedef long long               i64;
typedef typeof((u8*)0-(u8*)0)   iz;
typedef typeof(sizeof(0))       uz;

typedef struct {
    u8 *beg;
    u8 *end;
} Arena;

static void *alloc(Arena *a, iz count, iz size, iz align)
{
    iz pad = -(uz)a->beg & (align - 1);
    affirm(count < (a->end - a->beg - pad)/size);
    u8 *r = a->beg + pad;
    a->beg += pad + count*size;
    return memset(r, 0, count*size);
}

typedef struct {
    u8 *data;
    iz  len;
} Str;

static Str span(u8 *beg, u8 *end)
{
    affirm(beg <= end);
    return (Str){beg, end-beg};
}

static Str slice(Str s, iz beg, iz end)
{
    affirm(beg >= 0 && end <= s.len);
    return span(s.data+beg, s.data+end);
}

#define Slice(T)        \
    struct Slice##T {   \
        T *data;        \
        iz len;         \
        iz cap;         \
    }

#define push(a, s)                          \
  ((s)->len == (s)->cap                     \
    ? (s)->data = push_(                    \
        (a),                                \
        (s)->data,                          \
        &(s)->cap,                          \
        sizeof(*(s)->data),                 \
        _Alignof(typeof(*(s)->data))        \
      ),                                    \
      (s)->data + (s)->len++                \
    : (s)->data + (s)->len++)

static void *push_(Arena *a, void *data, iz *pcap, iz size, iz align)
{
    iz cap = *pcap;

    if (a->beg != (u8 *)data + cap*size) {
        void *copy = alloc(a, cap, size, align);
        memcpy(copy, data, cap*size);
        data = copy;
    }

    iz extend = cap ? cap : 4;
    alloc(a, extend, size, align);
    *pcap = cap + extend;
    return data;
}

static Slice(i32) range(i32 n, Arena *a)
{
    Slice(i32) r = {};
    affirm(n < maxof(i32));
    for (i32 i = 0; i < n; i++) {
        *push(a, &r) = i;
    }
    return r;
}

static Slice(Str) split(Str s, u8 delim, Arena *a)
{
    Slice(Str) r = {};
    iz beg = 0;
    for (iz i = 0; i < s.len; i++) {
        if (s.data[i] == delim) {
            *push(a, &r) = slice(s, beg, i);
            beg = i + 1;
        }
    }
    *push(a, &r) = slice(s, beg, s.len);
    return r;
}

static Slice(u8) append(Arena *a, Slice(u8) buf, Str s)
{
    while (buf.cap-buf.len < s.len) {
        iz oldlen = buf.len;
        buf.len = buf.cap;
        push(a, &buf);
        buf.len = oldlen;
    }
    memcpy(buf.data+buf.len, s.data, s.len);
    buf.len += s.len;
    return buf;
}

static Str join(Slice(Str) strings, Str delim, Arena *a)
{
    Slice(u8) buf = {};
    for (iz i = 0; i < strings.len; i++) {
        if (i) buf = append(a, buf, delim);
        buf = append(a, buf, strings.data[i]);
    }
    return (Str){buf.data, buf.len};
}

static Slice(i64) generate_primes(i64 limit, Arena *a)
{
    Slice(i64) primes = {};

    if (limit > 2) {
        *push(a, &primes) = 2;
    }

    for (i64 n = 3; n < limit; n += 2) {
        b32 valid = true;
        for (iz i = 0; valid && i<primes.len; i++) {
            valid = n % primes.data[i];
        }
        if (valid) {
            *push(a, &primes) = n;
        }
    }

    return primes;
}


// Demo

#include <stdio.h>

static void print(Str s)
{
    fwrite(s.data, 1, s.len, stdout);
    putchar('\n');
}

int main()
{
    static u8 mem[1<<21];
    Arena a = {mem, mem+lenof(mem)};

    Slice(i32) nums = range(10, &a);
    for (iz i = 0; i < nums.len; i++) {
        printf("%d%c", (int)nums.data[i], i==nums.len-1?'\n':' ');
    }

    Str input = S("foo bar baz");
    Slice(Str) words = split(input, ' ', &a);
    print(input);
    for (iz i = 0; i < words.len; i++) {
        print(words.data[i]);
    }

    Str sentence = join(words, S(","), &a);
    print(sentence);

    Slice(i64) primes = generate_primes(100, &a);
    printf("primes:");
    for (iz i = 0; i < primes.len; i++) {
        printf(" %lld", (long long)primes.data[i]);
    }
    putchar('\n');

    return 0;
}
