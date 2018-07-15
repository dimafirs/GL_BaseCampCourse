#include <stddef.h>

#define LIST_NODE_INIT(node) { &(node), &(node)}

#define LIST_NODE(node) \
    struct list_node node = LIST_NODE_INIT(node)

/* Sooooo tasty macro... */
#define container_of(ptr, type, member) ({                  \
        const typeof( ((type *)0)->member ) *sptr = (ptr);   \
        (type *) ( (char *)sptr - offsetof(type,member) ); })

struct list_node{
    struct list_node *prev;
    struct list_node *next;
};

static inline void INIT_LIST_NODE(struct list_node *node){
    node->prev = node;
    node->next = node;
}

/* add_between() - add @new node between @prev and @next */
static inline void add_between(struct list_node *new, 
                               struct list_node *prev, 
                               struct list_node *next)
{
    next->prev=new;
    new->next = next;
    prev->next = new;
    new->prev = prev;   
}

/* list_add() - add list_node @new after list_node @node */
static inline void list_add(struct list_node *new,
                            struct list_node *node)
{
    add_between(new, node, node->next);
}

/* list_add_tail() - add @new node before @node */
static inline void list_add_tail(struct list_node *new,
                                 struct list_node *node)
{
    add_between(new, node->prev, node);
}

/* list_replace() - replace @old node with @new */
static inline void list_replace(struct list_node *new, 
                                struct list_node *old)
{
    old->prev->next = new;
    new->next = old->next;
    new->prev = old->prev;
    old->next->prev = new;
}

/* list_del() - delete @node from list (without free memory) */
static inline void list_del(struct list_node *node){
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/* list_move() - move @node to new position - after @place */
static inline void list_move(struct list_node *node,
                             struct list_node *place)
{
    list_del(node);
    list_add(node, place);
}

/* list_move_tail() - move @node to new position - before @place */
static inline void list_move_tail(struct list_node *node,
                             struct list_node *place)
{
    list_del(node);
    list_add_tail(node, place);
}
 
/* list_is_last() - return non-zero value if @node is last element 
 *                  in @head list 
 */
static inline int list_is_last(struct list_node *node,
                               struct list_node *head)
{
    return (node->next) == head;
}

/* list_is_empty() - return non-zero value if @node list is empty */
static inline int list_is_empty(struct list_node *node){
    return (node->next) == node;
}

/* list_is_singular() - return non-zero value if @list consist of 
 *                      two element, that are circulary connected */
static inline int list_is_singular(struct list_node *list){
    return ( !(list_is_empty) && (list->next == list->prev) );
}

/* list_cut() - replace part from @list between @list(without) and
 *              @pos(with), and place it to list with head @new_list 
 */
static inline void list_cut(struct list_node *list, 
                            struct list_node *pos,
                            struct list_node *new_list)
{
    struct list_node *tmp_node = pos->next;

    list->next->prev = new_list;
    new_list->next = list->next;
    new_list->prev = pos;
    pos->next = new_list;

    list->next = tmp_node;
    tmp_node->prev = list;
}

/* list_splice() - join two lists together: @list place in the beginning
 * of @head list (after @head node) 
 */
static inline void list_splice(struct list_node *list, 
                               struct list_node *head)
{
    struct list_node *fst_node = list->next;
    struct list_node *lst_node = list->prev;
    
    fst_node->prev = head;
    lst_node->next = head->next;
    
    head->next->prev = lst_node;
    head->next = fst_node;
}

/* list_splice_tail() - join two lists together: @list place in the beginning
 * of @head list (before @head node) 
 */
static inline void list_splice_tail(struct list_node *list, 
                               struct list_node *head)
{
    struct list_node *fst_node = list->next;
    struct list_node *lst_node = list->prev;
    
    fst_node->prev = head->prev;
    lst_node->next = head;
    
    head->prev->next = fst_node;
    head->prev = lst_node;
}

/* list_entry() macro return entry where @member is situated */
#define list_entry(ptr, type, member)  \
    container_of(ptr, type, member)
/* list_first_entry() macro return first entry after list_node
 * under @ptr 
 */
#define list_first_entry(ptr, type, member) \
    list_entry(ptr->next, type, member)

/* list_first_entry() macro return previous entry before 
 *  list_node under @ptr 
 */
#define list_last_entry(ptr, type, member) \
	list_entry(ptr->prev, type, member)

/* list_add_entry() macro add @new node to list after @head */
#define list_add_entry(new, head) \
    list_add(new, head) 

/* list_add_tail_entry_tail() macro add @new node to list before @head */
#define list_add_entry_tail(new, head) \
    list_add_tail(new, head) 

/* list_replace_entry() macro replace @old node with @new */
#define list_replace_entry(new, old) \
    list_replace(new, old)

/* list_del_entry() macro del @entry from list*/ 
#define list_del_entry(entry) \
    list_del(entry)

/* list_move_entry() macro replace @node and put this after @place */
#define list_move_entry(node, place) \
    list_move(node, place)

/* list_move_entry() macro replace @node and put this before @place */
#define list_move_tail_entry(node, place) \
    list_move_tail(node, place)

/* is_last() macro return non-zero if @node is last node in @list*/
#define is_last(node, list) \
    list_is_last(node, list)

/* is_empty() macro return non-zero if @list is empty (single-node) */
#define is_empty(list) \
    list_is_empty(list)

/* list_cut_entry() macro allow you to cut part between @list and @pos,
 * and put it to @new_list
 */
#define list_cut_entry(list, pos, new_list) \
    list_cut(list, pos, new_list)

/* list_splice_entries() macro allow you to connect two lists (@list and @head)
 * together(@list place in the beginning of @head list (after @head node) 
 */
#define list_splies_entries(list, head)  \
    list_splice(list, head)

/* list_splice_tail_entries() macro allow you to connect two lists 
 * (@list and @head) together (@list place before a @head list) 
 */
#define list_splies_tail_entries(list, head)  \
    list_splice_tail(list, head)

/* list_for_each() macro interprets in cycle that allow you to
 * to iterate list under @head in forward order 
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/* list_for_each_rev() macro interprets in cycle that allow you to
 * to iterate list under @head in reverse order 
 */
#define list_for_each_rev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

