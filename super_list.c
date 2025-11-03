#include "string.h"
#include "assert.h"
#include "stdlib.h"
#include "inttypes.h"
#include "stdio.h"
#include "immintrin.h"

#include "mylist.h"




#define used_item(lst) 0
#define free_item(lst) 1

#define used_head(lst) ((lst)->items[used_item(lst)].next)
#define used_tail(lst) ((lst)->items[used_item(lst)].prev)

#define free_head(lst) ((lst)->items[free_item(lst)].next)
#define free_tail(lst) ((lst)->items[free_item(lst)].prev)


#define DPRINTF(...)
// #define DPRINTF printf


#define ITEM_SIZE 16
#define ITEM_VALUES_COUNT (ITEM_SIZE - 3)
#define ITEM_MIN_VALUES_COUNT (ITEM_VALUES_COUNT / 2)

#define ITERATOR_SHIFT 4

struct expanded_iterator_t
{
    int block, index;
};

__attribute__((always_inline)) __inline__ 
void deconstruct_iterator(iterator_t it, int *block, int *item)
{
    *block = (it >> ITERATOR_SHIFT);
    *item = (it & ((1 << ITERATOR_SHIFT) - 1));
}

__attribute__((always_inline)) __inline__ 
iterator_t construct_iterator(int block, int item)
{
    return item | (block << ITERATOR_SHIFT);
}


int is_correct(iterator_t it)
{
    return it != 0;
}


struct item_header
{
    int size, next, prev;
};


struct item
{
    struct item_header;
    int value[ITEM_VALUES_COUNT];
};


struct list_t
{
    struct item *items;
    int alloc;
    int size;
    int block_size;
};


/* standart initializators */
struct list_t *list_create(int32_t capacity)
{    
    struct list_t *lst = calloc(1, sizeof(*lst));
    
    lst->alloc = 2;
    lst->items = calloc(1, 2 * sizeof(*lst->items));
    lst->size = 0;
    lst->block_size = 2;

    lst->items[free_item(lst)].next = free_item(lst);
    lst->items[free_item(lst)].prev = free_item(lst);
    lst->items[free_item(lst)].size = 1;
    lst->items[used_item(lst)].next = used_item(lst);
    lst->items[used_item(lst)].prev = used_item(lst);
    lst->items[used_item(lst)].size = 1;

    list_reserve(lst, 2 * capacity / ITEM_VALUES_COUNT + 100);

    return lst;
}

struct list_t *list_create_from_array(int32_t *array, int32_t array_len, int32_t capacity)
{
    struct list_t *lst = list_create(capacity);
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
        lst->items[prev_size].prev = free_tail(lst);
        lst->items[free_tail(lst)].next = prev_size;
        // last element
        lst->items[lst->alloc - 1].next = free_item(lst);
        lst->items[free_item(lst)].prev = lst->alloc - 1;
        // other links
        /* TODO: optimize this loop, becouse compilers doen't */
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
            lst->items[i].size = 0;
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
    iterator_t res = construct_iterator(used_head(lst), 0);
    DPRINTF("ret head=%d\n", res);
    DPRINTF(": head_next=%d\n", used_head(lst));
    DPRINTF(": alloc=%d\n", lst->alloc);
    return res;
}

iterator_t list_tail(struct list_t *lst)
{
    iterator_t res = construct_iterator(used_tail(lst), lst->items[used_tail(lst)].size - 1);
    DPRINTF("ret tail=%d\n", res);
    return res;
}

iterator_t list_next(struct list_t *lst, iterator_t it)
{
    int block, index;
    deconstruct_iterator(it, &block, &index);
    index++;
    assert(index <= lst->items[block].size);
    if (index == lst->items[block].size)
    {
        return construct_iterator(lst->items[block].next, 0);
    }
    return construct_iterator(block, index);
}

iterator_t list_prev(struct list_t *lst, iterator_t it)
{
    int block, index;
    deconstruct_iterator(it, &block, &index);
    assert(index <= lst->items[block].size);
    if (index == 0)
    {
        return construct_iterator(lst->items[block].prev, lst->items[lst->items[block].prev].size - 1);
    }
    return construct_iterator(block, index - 1);
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
    int block, index;
    deconstruct_iterator(it, &block, &index);
    return lst->items[block].value[index];
}


/* duplicates block, and return index of first one + it is 100% not full */
struct expanded_iterator_t duplicate_block(struct list_t *lst, int block, int index)
{
    list_reserve(lst, lst->block_size + 2);
    lst->block_size++;
    
    // int prev = lst->items[block].prev;
    int next = lst->items[block].next;
    int node = free_head(lst);

    /* update free list */
    lst->items[free_item(lst)].next = lst->items[node].next;
    lst->items[lst->items[node].next].prev = free_item(lst);

    /* array pointers */
    lst->items[block].next = node;
    lst->items[next].prev = node;
    
    /* update new node */
    lst->items[node].prev = block;
    lst->items[node].next = next;
    
    /* copy 1/2 of block to new block */
    /* MEGA OPTIMIZATION: !!block is 1 if block != 0 -> than we shouldn't */
    /* remove one fictive element from fitst node - else we need to remove one more */
    /* to right insert to leftmost node */
    int size_to_copy = ((lst->items[block].size + (!!block)) >> 1);
    int remain_size = lst->items[block].size - size_to_copy;
    memcpy(lst->items[node].value, lst->items[block].value + remain_size, sizeof(*lst->items[node].value) * size_to_copy);

    /* update node sizes */
    lst->items[node].size = size_to_copy;
    lst->items[block].size = remain_size;

    assert(lst->items[used_item(lst)].size == 1);
    assert(lst->items[free_item(lst)].size == 1);

    /* size_to_copy == block size / 2 < block size <= ITEM_VALUES_COUNT */
    assert(lst->items[block].size < ITEM_VALUES_COUNT || 
          (block == used_item(lst) && lst->items[next].size == ITEM_VALUES_COUNT));

    if (index < remain_size)
    {
        return (struct expanded_iterator_t){block, index};
    }
    return (struct expanded_iterator_t){node, index - remain_size};
}

/* remove block, return pointer on next block */
int remove_block(struct list_t *lst, int block)
{
    lst->block_size--;

    int next = lst->items[block].next;
    int prev = lst->items[block].prev;
    
    DPRINTF("RM {%d} %d {%d}\n", prev, block, next);
    
    /* update free list */
    lst->items[block].next = free_head(lst);
    lst->items[block].prev = free_item(lst);
    lst->items[free_head(lst)].prev = block; // not swap this string with next one
    lst->items[free_item(lst)].next = block;

    /* update next + prev */
    lst->items[prev].next = next;
    lst->items[next].prev = prev;

    DPRINTF("RETURN %d {%d}\n", next, prev);
    
    return next;
}


/* insertion and deletion of elements */
iterator_t list_insert(struct list_t *lst, iterator_t it, int32_t value)
{
    DPRINTF("insert %d before IT=%d\n", value, it);
    /* insert to block by iterator */
    int block, index;
    deconstruct_iterator(it, &block, &index);

    if (block == 0)
    {
        if (used_tail(lst) == 0)
        {
            struct expanded_iterator_t exp = duplicate_block(lst, block, 1);
            block = exp.block;
            index = exp.index;
        }
        else
        {
            block = used_tail(lst);
        }
        assert(block == used_tail(lst));
        assert(index == 0);
    }
    else if (lst->items[block].size == ITEM_VALUES_COUNT)
    {
        /* split node */
        /* no clever things: simply split node */
        /* TODO: optimize */
        struct expanded_iterator_t exp = duplicate_block(lst, block, index);
        block = exp.block;
        index = exp.index;
    }


    int size = lst->items[block].size;

    assert(size < ITEM_VALUES_COUNT);
    assert(index <= size);

    /* now - insert element to array */
    DPRINTF("insert at: %d of %d, block %d\n", index, ITEM_VALUES_COUNT, block);
    memmove(lst->items[block].value + index + 1, lst->items[block].value + index, sizeof(*lst->items[block].value) * (size - index));
    lst->items[block].value[index] = value;

    lst->size++;
    lst->items[block].size++;

    return construct_iterator(block, index);
}

iterator_t list_remove(struct list_t *lst, iterator_t it)
{
    int block, index;
    deconstruct_iterator(it, &block, &index);
    DPRINTF("rm iterator: %d\n", it);
    DPRINTF("removing... from %d [%d]\n", block, index);
    
    /* no clever: if node is empty: remove it */
    /* TODO: strongly optimize */
    if (lst->items[block].size == 1)
    {
        lst->size--;
        return construct_iterator(remove_block(lst, block), 0);
    }

    /* remove element from node */
    int size = lst->items[block].size;

    assert(size > 1);
    assert(index < size);
    
    /* update sizes */
    lst->size--;
    lst->items[block].size--;
    
    /* return next iterator */
    if (index + 1 == size)
    {
        return construct_iterator(lst->items[block].next, 0);
    }
    
    memmove(lst->items[block].value + index, lst->items[block].value + index + 1, sizeof(*lst->items[block].value) * (size - index));

    DPRINTF("new size of %d is %d\n", block, lst->items[block].size);
    DPRINTF("items %d %d [%d]\n", lst->items[block].value[0], lst->items[block].value[1], lst->items[block].value[2]);
    
    /* return same iterator - now it points on next element */
    return it;
}

/* call this function at free time, to optimizate structure */
result_t list_optimize(struct list_t *lst)
{
    (void)lst;
    return 0;
}



/* get element by index */
int32_t list_at(struct list_t *lst, int32_t index)
{
    iterator_t it = list_move(lst, used_head(lst), index);
    return list_get(lst, it);
}
