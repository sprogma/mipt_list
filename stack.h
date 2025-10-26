#ifndef LIST_H
#define LIST_H


struct list_t;

typedef iterator_t int32_t;
typedef result_t uint64_t;

/* standart initializators */
result_t list_init(struct list_t *, int32_t capacity);

result_t list_from_array(struct list_t *, int32_t *array, int32_t array_len, int32_t capacity);

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


/* insertion and deletion of elements */
result_t list_insert(struct list_t *, iterator_t, int32_t);

result_t list_delete(struct list_t *, iterator_t);


/* call this function at free time, to optimizate structure */
result_t list_optimize(struct list_t *);


/* get element by index */
result_t list_at(struct list_t *, int index);


#endif
