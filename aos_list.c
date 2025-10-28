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
#define f_tail(lst) ((lst)->items[0].prev)
#define f_head(lst) ((lst)->items[0].next)
#define u_tail(lst) ((lst)->items[1].prev)
#define u_head(lst) ((lst)->items[1].next)

struct item
{
    int value, next, prev;
};

struct list_t
{
    struct item *items;
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
        printf("node[%d] = {v:%d p:%d n:%d}\n", i, lst->items[i].value, lst->items[i].prev, lst->items[i].next);
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
    for (int i = 0; i < lst->size; ++i)
    {
        if (it <= 1)
        {
            printf("ERROR: AT i=%d/%d\n", i, lst->size);
            return 1;
        }
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
    lst->items = calloc(2, sizeof(*lst->items));
    lst->size = 0;

    lst->items[f_item].value = 0xBEBEBEBE;
    lst->items[u_item].value = 0xBEBEBEBE;
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
    free(lst->items);
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
        lst->alloc = 1;
        while (lst->alloc < capacity)
        {
            lst->alloc *= 2;
        }
        struct item *new_ptr = realloc(lst->items, sizeof(*lst->items) * lst->alloc);
        if (new_ptr == NULL)
        {
            return 1;
        }
        lst->items = new_ptr;
        /* mark new nodes as free */
        // first element
        lst->items[prev_size].prev = f_tail(lst);
        lst->items[f_tail(lst)].next = prev_size;
        // last element
        lst->items[lst->alloc - 1].next = f_item;
        lst->items[f_item].prev = lst->alloc - 1;
        // other links
        for (int i = prev_size; i < lst->alloc; ++i)
        {
            if (i != lst->alloc - 1)
            {
                lst->items[i].next = i + 1;
            }
            if (i != prev_size)
            {
                lst->items[i].prev = i - 1;
            }
            #ifndef NDEBUG
            lst->items[i].value = -1;
            #endif
        }
    }
}


/* helping functions */
int32_t list_size(struct list_t *lst)
{
    return lst->size;
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
    return lst->items[it].next;
}

iterator_t list_prev(struct list_t *lst, iterator_t it)
{
    return lst->items[it].prev;
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
    return lst->items[it].value;
}


/* insertion and deletion of elements */
iterator_t list_insert(struct list_t *lst, iterator_t it, int32_t value)
{
    list_reserve(lst, lst->size + 1);

    #ifndef NO_PRINF
    printf("insert %d at %d\n", value, it);
    #endif
    dump_to_image(lst);
    
    int new_item = f_head(lst);
    lst->items[lst->items[f_head(lst)].next].prev = f_item;
    f_head(lst) = lst->items[f_head(lst)].next;

    int before = lst->items[it].prev;

    // x Y z: x next = Y
    lst->items[before].next = new_item;
    // x Y z: z prev = Y
    lst->items[it].prev = new_item;
    // x Y z: Y next = z
    lst->items[new_item].next = it;
    // x Y z: Y prev = x
    lst->items[new_item].prev = before;
    // set value
    lst->items[new_item].value = value;
    
    #ifndef NO_PRINF
    printf("after insert:\n");
    #endif
    dump_to_image(lst);

    lst->size++;

    return new_item;
}

result_t list_remove(struct list_t *lst, iterator_t it)
{
    int after = lst->items[it].next;
    int before = lst->items[it].prev;
    // x Y z: x next = z
    lst->items[before].next = after;
    // x Y z: z prev = x
    lst->items[after].prev = before;
    
    // free node
    lst->items[it].prev = f_tail(lst);
    lst->items[it].next = f_item;
    lst->items[f_tail(lst)].next = it;
    f_tail(lst) = it;

    lst->size--;
        
    return 0;
}

/* call this function at free time, to optimizate structure */

static void swap_items(struct list_t *lst, int a, int b)
{
    if (a == b)
    {
        return;
    }
    iterator_t pa = lst->items[a].prev, na = lst->items[a].next;
    iterator_t pb = lst->items[b].prev, nb = lst->items[b].next;
    if (na == b)
    {
        // pa a b nb -> pa b a nb
        lst->items[pa].next = b;
        lst->items[b].next = a;
        lst->items[a].next = nb;

        lst->items[b].prev = pa;
        lst->items[a].prev = b;
        lst->items[nb].prev = a;
    }
    else if (nb == a)
    {
        // pb b a na -> pb a b na
        lst->items[pb].next = a;
        lst->items[a].next = b;
        lst->items[b].next = na;

        lst->items[a].prev = pb;
        lst->items[b].prev = a;
        lst->items[na].prev = b;
    }
    else
    {
        /* swap a and b */
        lst->items[a].next = nb;
        lst->items[a].prev = pb;
        lst->items[b].next = na;
        lst->items[b].prev = pa;

        lst->items[pa].next = b;
        lst->items[na].prev = b;
        lst->items[pb].next = a;
        lst->items[nb].prev = a;
    }
    
    int tmp = lst->items[a].value;
    lst->items[a].value = lst->items[b].value;
    lst->items[b].value = tmp;
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
        int new_it = list_next(lst, it);
        swap_items(lst, it, i + 2);
        assert(new_it > 1);
        it = new_it;
    }
    return 0;
}



/* get element by index */
int32_t list_at(struct list_t *lst, int32_t index)
{
    iterator_t it = list_move(lst, u_head(lst), index);
    return lst->items[it].value;
}
