#ifndef LIST_H
#define LIST_H

#include "inttypes.h"

#ifdef TEST_CPP_REALIZATION
    #include <list>
    typedef struct {std::list<int32_t>::iterator it; std::list<int32_t> *lst; } iterator_t;
#else
    typedef int32_t iterator_t;
#endif



#ifndef TEST_CPP_REALIZATION
#ifdef __cplusplus
extern "C" {
#endif
#endif


struct list_t;

typedef uint64_t result_t;

int is_correct(iterator_t it);

/* standart initializators */
struct list_t * list_create(int32_t capacity);

struct list_t * list_create_from_array(int32_t *array, int32_t array_len, int32_t capacity);

result_t list_free(struct list_t *);

result_t list_reserve(struct list_t *, int32_t capacity);


/* helping functions */
int32_t list_size(struct list_t *);


/* iterators */
iterator_t list_head(struct list_t *);
iterator_t list_tail(struct list_t *);
iterator_t list_next(struct list_t *, iterator_t);
iterator_t list_prev(struct list_t *, iterator_t);
iterator_t list_move(struct list_t *, iterator_t, int32_t steps); // may be negative
int32_t list_get(struct list_t *, iterator_t);


/* insertion and deletion of elements */
iterator_t list_insert(struct list_t *, iterator_t, int32_t);

iterator_t list_remove(struct list_t *lst, iterator_t it);


/* call this function at free time, to optimizate structure */
result_t list_optimize(struct list_t *);


/* get element by index */
int32_t list_at(struct list_t *lst, int32_t index);

#ifndef TEST_CPP_REALIZATION
#ifdef __cplusplus
} // extern "C"
#endif
#endif

#endif
