#include <list>
#include <iostream>
#include "assert.h"

#ifndef TEST_CPP_REALIZATION
    #define TEST_CPP_REALIZATION
#endif

#include "mylist.h"

struct list_t
{
    std::list<int32_t> x;
};

int is_correct(iterator_t it)
{
    return ((it).it != ((it).lst->end()));
}

/* standart initializators */
list_t *list_create(int32_t capacity) 
{
    (void)capacity;
    return new list_t;
}

list_t *list_create_from_array(int32_t *array, int32_t array_len, int32_t capacity) 
{
    list_t *lst = list_create(capacity);
    
    for (int32_t i = 0; i < array_len; ++i) 
    {
        lst->x.push_back(array[i]);
    }
    
    return lst;
}

result_t list_free(struct list_t *lst) 
{
    delete lst;
    return 0;
}

result_t list_reserve(struct list_t *lst, int32_t capacity) 
{
    (void)lst; 
    (void)capacity;
    return 0;
}

/* helping functions */
int32_t list_size(struct list_t *lst) 
{
    return lst->x.size();
}

/* iterators */
iterator_t list_head(struct list_t *lst) 
{
    return {lst->x.begin(), &lst->x};
}

iterator_t list_tail(struct list_t *lst) 
{
    return {std::prev(lst->x.end()), &lst->x};
}

iterator_t list_next(struct list_t *lst, iterator_t it) 
{
    return {std::next(it.it), &lst->x};
}

iterator_t list_prev(struct list_t *lst, iterator_t it) 
{
    if (it.it == lst->x.begin())
    {
        return {lst->x.end(), &lst->x};
    }
    return {std::prev(it.it), &lst->x};
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
    (void)lst;
    return *it.it;
}

/* insertion and deletion of elements */
iterator_t list_insert(struct list_t *lst, iterator_t it, int32_t value) 
{
    lst->x.insert(it.it, value);
    return {std::next(it.it), &lst->x};
}

iterator_t list_remove(struct list_t *lst, iterator_t it) 
{
    iterator_t res = list_next(lst, it);
    lst->x.erase(it.it);
    return res;
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
    iterator_t it = list_move(lst, {lst->x.begin(), &lst->x}, index);
    return *it.it;
}
