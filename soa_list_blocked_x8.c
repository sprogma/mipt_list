#include "string.h"
#include "assert.h"
#include "stdlib.h"
#include "inttypes.h"
#include "stdio.h"

#include "mylist.h"

#define NO_PRINF
#define NO_VERIFY

#define f_item 0
#define u_item 1
#define f_tail(lst) ((lst)->prev[0])
#define f_head(lst) ((lst)->next[0])
#define u_tail(lst) ((lst)->prev[1])
#define u_head(lst) ((lst)->next[1])
int is_correct(iterator_t it)
{
    return it > 1;
}

struct item
{
    int value, next, prev;
};

struct list_t
{
    int *value;
    int *next;
    int *prev;
    int alloc;
    int size;
};

#ifdef NO_PRINF
#define dump_to_image(lst)
#else
void dump_to_image(struct list_t *lst)
{
    printf("node size: %d [%d]\n", lst->size, lst->alloc);
    for (int i = 0; i < lst->alloc; ++i)
    {
        printf("node[%d] = {v:%d p:%d n:%d}\n", i, lst->value[i], lst->prev[i], lst->next[i]);
    }
}
#endif


#ifdef NO_VERIFY
#define list_is_wrong(lst) 0
#else
int32_t list_is_wrong(struct list_t *lst)
{    
    iterator_t it;
    it = list_head(lst);
    for (int i = 0; i < lst->alloc; ++i)
    {
        if (it <= 1 || it > lst->alloc)
        {
            printf("ERROR: AT i=%d/%d\n", i, lst->alloc);
            return 1;
        }
        // printf("%d = %d: <%d %d> <<%d %d>>\n", i + 2, lst->value[i + 2], lst->prev[i + 2], lst->next[i + 2], lst->next[lst->prev[i + 2]], lst->prev[lst->next[i + 2]]);
        assert(lst->next[TO_PREV(lst->prev[i + 2])] == i + 2);
        assert(TO_PREV(lst->prev[lst->next[i + 2]]) == i + 2);
        it = list_next(lst, it);
    }
    
    return 0;
}
#endif

/* standart initializators */
struct list_t *list_create(int32_t capacity)
{    
    struct list_t *lst = calloc(1, sizeof(*lst));
    
    lst->alloc = 2;
    lst->value = calloc(2, sizeof(*lst->prev));
    lst->next = calloc(2, sizeof(*lst->value));
    lst->prev = calloc(2, sizeof(*lst->next));
    lst->size = 0;

    lst->value[f_item] = 0xBEBEBEBE;
    lst->value[u_item] = 0xBEBEBEBE;
    f_tail(lst) = f_head(lst) = f_item;
    u_tail(lst) = u_head(lst) = u_item;

    list_reserve(lst, capacity);

    #ifndef NO_PRINF
    printf("List created\n");
    #endif

    return lst;
}

struct list_t *list_create_from_array(int32_t *array, int32_t array_len, int32_t capacity)
{
    struct list_t *lst = list_create(capacity);
    /* ! using fact that first node id is 2, second - 3 and so on. */
    for (int i = 0; i < array_len; ++i)
    {
        list_insert(lst, 2 + i, array[i]);
    }
    return lst;
}

result_t list_free(struct list_t *lst)
{
    free(lst->value);
    free(lst->prev);
    free(lst->next);
    free(lst);
    return 0;
}

result_t list_reserve(struct list_t *lst, int32_t capacity)
{
    #ifndef NO_PRINF
    printf("ASK FOR CAPACITY %d\n", capacity);
    #endif
    capacity += 2; // add u_item and f_item
    if (capacity > lst->alloc)
    {
        int prev_size = lst->alloc;
        lst->alloc = 64;
        while (lst->alloc < capacity)
        {
            lst->alloc *= 2;
        }
        int *new_value = realloc(lst->value, sizeof(*lst->value) * lst->alloc);
        iterator_t *new_prev = realloc(lst->prev, sizeof(*lst->prev) * lst->alloc);
        iterator_t *new_next = realloc(lst->next, sizeof(*lst->next) * lst->alloc);
        if (new_value == NULL || new_prev == NULL || new_next == NULL)
        {
            return 1;
        }
        lst->value = new_value;
        lst->prev = new_prev;
        lst->next = new_next;
        /* mark new nodes as free */
        // first element
        lst->prev[prev_size] = f_tail(lst); // | FREE_MASK; already with it
        lst->next[f_tail(lst)] = prev_size;
        // last element
        lst->next[lst->alloc - 1] = f_item;
        lst->prev[f_item] = lst->alloc - 1;
        // other links
        for (int i = prev_size; i < lst->alloc; ++i)
        {
            if (i != lst->alloc - 1)
            {
                lst->next[i] = i + 1;
            }
            if (i != prev_size)
            {
                lst->prev[i] = i - 1;
            }
            #ifndef NDEBUG
            lst->value[i] = -1;
            #endif
        }
    }
}


/* helping functions */
int32_t list_size(struct list_t *lst)
{
    return lst->size;
}


static void swap_items(struct list_t *lst, iterator_t a, iterator_t b)
{
    if (a == b)
    {
        return;
    }
    iterator_t pa, na, pb, nb;
    if (lst->next[b] == a)
    {
        iterator_t tmp = b;
        b = a;
        a = tmp;
    }
    pa = lst->prev[a];
    na = lst->next[a];
    pb = lst->prev[b];
    nb = lst->next[b];
    if (na == b)
    {
        // pa a b nb -> pa b a nb
        lst->next[pa] = b;
        lst->next[b] = a;
        lst->next[a] = nb;

        lst->prev[b] = pa;
        lst->prev[a] = b;
        lst->prev[nb] = a;
    }
    else
    {
        // pa a na   pb b nb    ->    pa b na   pb a nb
        lst->next[a] = nb;
        lst->prev[a] = pb;
        lst->next[b] = na;
        lst->prev[b] = pa;

        lst->next[pa] = b;
        lst->prev[na] = b;
        lst->next[pb] = a;
        lst->prev[nb] = a;
    }
    int tmp = lst->value[a];
    lst->value[a] = lst->value[b];
    lst->value[b] = tmp;

    if (list_is_wrong(lst)) { }
}


/* iterators */
iterator_t list_head(struct list_t *lst)
{
    return u_head(lst);
}

iterator_t list_tail(struct list_t *lst)
{
    return u_tail(lst);
}

iterator_t list_next(struct list_t *lst, iterator_t it)
{
    return lst->next[it];
}

iterator_t list_prev(struct list_t *lst, iterator_t it)
{
    return lst->prev[it];
}

iterator_t list_move(struct list_t *lst, iterator_t it, int32_t steps)
{    
    if (steps == 0)
    {
        return it;
    }
    if (steps < 0)
    {
        steps *= -1;
        for (int i = 0; i < steps; ++i)
        {
            it = list_prev(lst, it);
        }
        return it;
    }
    for (int i = 0; i < steps; ++i)
    {
        it = list_next(lst, it);
    }
    return it;
}

int32_t list_get(struct list_t *lst, iterator_t it)
{
    return lst->value[it];
}


/* insertion and deletion of elements */
iterator_t list_insert(struct list_t *lst, iterator_t it, int32_t value)
{
    list_reserve(lst, lst->size + 2);

    #ifndef NO_PRINF
    printf("insert %d at %d\n", value, it);
    #endif
    dump_to_image(lst);
    
    iterator_t new_item = f_head(lst);
    lst->prev[lst->next[f_head(lst)]] = f_item;
    f_head(lst) = lst->next[f_head(lst)];

    iterator_t before = lst->prev[it];

    // printf("%d %d %d\n", new_item, it, before);

    // x Y z: x next = Y
    lst->next[before] = new_item;
    // x Y z: z prev = Y
    lst->prev[it] = new_item;
    // x Y z: Y next = z
    lst->next[new_item] = it;
    // x Y z: Y prev = x
    lst->prev[new_item] = before;
    // set value
    lst->value[new_item] = value;
    
    #ifndef NO_PRINF
    printf("after insert:\n");
    #endif
    dump_to_image(lst);

    lst->size++;

    if (list_is_wrong(lst)) { }

    /* go to left 8 times, to find block for this element */
    for (int i = 0; before % 8 != 2 && i < 8; ++i)
    {
        before = lst->prev[before];
    }
    if (before % 8 != 2)
    {
        return new_item;
    }
    /* optimize: if there is empty element in IT's block - than swap it */
    int blk_start = ((before - 2) & ~0x7) + 2;
    int blk_end = blk_start + 8;
    if (blk_start < 0 || blk_end > lst->alloc || (blk_start <= new_item && new_item < blk_end))
    {
        return new_item;
    }
    // printf("BLOCK %d %d\n", blk_start, blk_end);
    /* find element not from this block */
    long long block_ll = 0;
    unsigned char *block = (void *)&block_ll;
    // printf("A %d insert %d it=%d bef=%d\n", lst->size, new_item, it, before);
    {
        int p = 0;
        p = before;
        while (blk_start <= p && p < blk_end)
        {
            block[p - blk_start] = 1;
            p = lst->next[p];
        }
    }

    // printf("B");
    // if (block_ll != 0)
    {        
        for (int i = 0; i < 8; ++i)
        {
            if (block[i] == 0)
            {
                // static int t = 0;
                // t++;
                // if (t % 1000000 == 0)
                // {
                //     fprintf(stderr, "optimizated %d\n", t);
                // }
                // assert(blk_start + i != 1);
                // assert(new_item != 1);
                swap_items(lst, blk_start + i, new_item);
                return blk_start + i;
            }
        }
    }

    if (list_is_wrong()) {}
    
    return new_item;
}

iterator_t list_remove(struct list_t *lst, iterator_t it)
{
    iterator_t after = lst->next[it];
    iterator_t before = lst->prev[it];
    // x Y z: x next = z
    lst->next[before] = after;
    // x Y z: z prev = x
    lst->prev[after] = before;
    
    // free node
    lst->prev[it] = f_tail(lst);
    lst->next[it] = f_item;
    lst->next[f_tail(lst)] = it;
    f_tail(lst) = it;

    lst->size--;
        
    return after;
}


/* call this function at free time, to optimizate structure */
result_t list_optimize(struct list_t *lst)
{
    if (list_is_wrong(lst))
    {
        printf("structure is bad.\n");
        abort();
    }
    iterator_t it = list_head(lst);
    for (int i = 0; i < lst->size; ++i)
    {
        swap_items(lst, it, i + 2);
        int new_it = list_next(lst, i + 2);
        assert(new_it > 1 || i == lst->size - 1);
        it = new_it;
    }
    if (list_is_wrong(lst))
    {
        printf("structure is bad.\n");
        abort();
    }
    #ifndef NO_VERIFY
    for (int i = 0; i + 1 < lst->size; ++i)
    {
        assert(lst->next[2 + i] == 2 + i + 1);
    }
    #endif
    return 0;
}


/* get element by index */
int32_t list_at(struct list_t *lst, int32_t index)
{
    iterator_t it = list_move(lst, u_head(lst), index);
    return lst->value[it];
}
