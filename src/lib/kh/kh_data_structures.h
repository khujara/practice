#ifndef KH_DATA_STRUCTURES_H
#define KH_DATA_STRUCTURES_H

typedef struct SortEntry
{
	f32 key;
	u32 ind;
} SortEntry;


KH_INTERN u32
hash_key_from_djb2(char *str)
{
	u32 res = 5381;
	char *at = str;
	while(*at++)
	{
		res = ((res << 5) + res) + *at; /* hash * 33 + *at */
	}

	return(res);
}

KH_INTERN u32
hash_key_from_djb2a(char *str)
{
	u32 res = 5381;
	char *at = str;
	while(*at++)
	{
		res = (((res << 5) + res) ^ (*at)); /* hash * 33 + *at */
	}

	return(res);
}

KH_INTERN u32
hash_key_from_dotnet(char *str)
{

	u32 length = string_length(str);
	u32 hash1 = (5381<<16) + 5381;
	u32 hash2 = hash1;

	u32 *pint = (u32 *)str;
	i32 len;
	for(len = length; len > 1; len -= 8)
	{
		hash1 = ((hash1 << 5) + hash1 + (hash1 >> 27) ^ pint[0]);
		hash2 = ((hash2 << 5) + hash2 + (hash2 >> 27) ^ pint[1]);
		pint += 2;
	}

	if(len > 0)
	{
		hash1 = ((hash1 << 5) + hash1 + (hash1 >> 27)) ^ pint[0];
	}

	u32 res = hash1 + (hash2 * 1566083941);
	return(res);
}

inline u32
sort_key_to_u32(f32 key)
{
	u32 res = *(u32 *)&key;
	if(res & 0x80000000)
	{
		res = ~res;
	}
	else
	{
		res |= 0x80000000;
	}
	return(res);
}

KH_INTERN void
radix_sort(u32 count, SortEntry *src, SortEntry *dst)
{
	kh_assert(src);
	kh_assert(dst);

	const u32 size = 256;
	for(u32 b = 0; b < 32; b += 8)
	{
		u32 sort_keys_off[size] = {};
		for(u32 i = 0; i < count; ++i)
		{
			u32 radix_val = sort_key_to_u32(src[i].key);
			u32 radix_piece = (radix_val >> b) & 0xFF;
			++sort_keys_off[radix_piece];
		}

		u32 total = 0;
		for(u32 sk_i = 0; sk_i < array_count(sort_keys_off); ++sk_i)
		{
			u32 c = sort_keys_off[sk_i];
			sort_keys_off[sk_i] = total;
			total += c;
		}

		for(u32 j = 0; j < count; ++j)
		{
			u32 radix_val = sort_key_to_u32(src[j].key);
			u32 radix_piece = (radix_val >> b) & 0xFF;
			dst[sort_keys_off[radix_piece]++] = src[j];
		}

		KH_SWAP(src, dst, SortEntry, tmp);
	}
}


#if 0
#define LINKED_LIST_TO_LINEAR(res, tmp, head, next_name, type) \
{ \
	type *out = (type *)tmp; \
	for(type *i = head; i; i = i->next_name) \
	{ \
		*out++ = i; \
	} \
	res = (type *)tmp; \
}

/*
---------------------------
	NOTE: LINKED LIST DEFINES
---------------------------
*/
	
#define LINKED_LIST_INSERT_FIRST(head, res, next) \
	kh_assert(res); \
	res->next = (head); \
	(head) = (res)

#define LINKED_LIST_REMOVE_FIRST(res, head, next) \
	(res) = (head); \
	(head) = (head)->next

#define LINKED_LIST_GET_LENGTH(res, head, next, type) \
	res = 0; \
	type *cur; \
	for(cur = head; cur; cur = cur->next) \
	{ \
		++res; \
	}

#define LINKED_LIST_INSERT_NEXT(res, node, next) \
	kh_assert(res); \
	res->next = node->next; \
	node->next = res

#define LINKED_LIST_REMOVE_NEXT(res, node, next) \
	res = node->next; \
	if(res) \
	{ \
		node->next = res->next; \
		res->next = 0; \
	}

#define LINKED_LIST_REMOVE(head, res, next, type) \
	if(res == head)  \
	{ \
		LINKED_LIST_REMOVE_FIRST(res, head, next); \
	} \
	else \
	{ \
		type *cur; \
		for(cur = head; cur; cur = cur->next) \
		{ \
			if(cur->next == res) \
			{ \
				cur->next = res->next; \
				break; \
			} \
		} \
	}

#define LINKED_LIST_REVERSE(head, next, type) \
	type *cur;  \
	type *prev = 0; \
	for(cur = head; cur; ) \
	{ \
		type *tmp_n = cur->next; \
		cur->next = prev; \
		prev = cur; \
		cur = tmp_n; \
	} \
	head = prev

/*
---------------------------
	NOTE: LINKED LIST CIRCULAR DEFINES
---------------------------
*/

#define LINKED_LIST_CIRC_INIT(head, next) \
	kh_assert(head); \
	head->next = head

#define LINKED_LIST_CIRC_INSERT_FIRST(head, res, next) \
	kh_assert(res); \
	res->next = head->next; \
	head->next = res

#define LINKED_LIST_CIRC_REMOVE_FIRST(head, res, next) \
	if(head->next != head) \
	{ \
		res = head->next; \
		if(res && res != head) \
		{ \
			head->next = res->next; \
		} \
	}

/*
---------------------------
	NOTE: DOUBLY LINKED LIST DEFINES
---------------------------
*/


#define DOUBLY_LINKED_LIST_GET_LENGTH(res, head, next, type) LINKED_LIST_GET_LENGTH(res. head, next, type)

#define DOUBLY_LINKED_LIST_INSERT_FIRST(head, res, next, prev) \
	kh_assert(res); \
	head->prev = res; \
	res->next = head; \
	head = res

#define DOUBLY_LINKED_LIST_REMOVE_FIRST(res, head, next, prev) \
	(res) = (head); \
	(head) = res->next; \
	(head)->prev = 0

#define DOUBLY_LINKED_LIST_INSERT_NEXT(res, node, next, prev) \
	kh_assert(res); \
	res->prev = node; \
	res->next = node->next; \
	if(res->next) \
	{ \
		res->next->prev = res; \
	} \
	node->next = res

#define DOUBLY_LINKED_LIST_INSERT_PREV(head, res, node, next, prev) \
	kh_assert(res); \
	if(node == head) \
	{ \
		DOUBLY_LINKED_LIST_INSERT_FIRST(head, res, next, prev); \
	} \
	else \
	{ \
		res->next = node; \
		res->prev = node->prev; \
		res->prev->next = res; \
		node->prev = res; \
	}

#define DOUBLY_LINKED_LIST_REMOVE_NEXT(res, node, next, prev) \
	res = node->next; \
	if(res) \
	{ \
		node->next = res->next; \
		node->next->prev = node; \
		res->next = 0; \
		res->prev = 0; \
	}

#define DOUBLY_LINKED_LIST_REMOVE_PREV(res, node, next, prev) \
	res = node->prev; \
	if(res) \
	{ \
		node->prev = res->prev; \
		node->prev->next = node; \
		res->prev = 0; \
		res->next = 0; \
	}

#define DOUBLY_LINKED_LIST_REMOVE(head, res, next, prev, type) \
	if(res == head)  \
	{ \
		DOUBLY_LINKED_LIST_REMOVE_FIRST(res, head, next, prev); \
	} \
	else \
	{ \
		type *cur; \
		for(cur = head; cur; cur = cur->next) \
		{ \
			if(cur->next == res) \
			{ \
				cur->next = res->next; \
				if(cur->next) \
				{ \
					cur->next->prev = cur; \
				} \
				break; \
			} \
		} \
	}


/*
---------------------------
	NOTE: DOUBLY LINKED LIST CIRCULAR  DEFINES
---------------------------
*/


#define DOUBLY_LINKED_CIRC_LIST_INIT(head, next, prev) \
	head->prev = head; \
	head->next = head;

#define DOUBLY_LINKED_CIRC_LIST_INSERT(head, res, next, prev) \
	kh_assert(res); \
	res->next = head->next; \
	res->prev = head; \
	res->next->prev = res; \
	res->prev->next = res

// TODO(flo)
#define DOUBLY_LINKED_CIRC_LIST_REMOVE(res)


/*
---------------------------
	NOTE: FREE LIST DEFINES
---------------------------
*/

#define FREE_LIST_INSERT(res, free_ptr, next_free, allocation) \
	res = free_ptr; \
	if(res) \
	{ \
		free_ptr = res->next_free; \
	} \
	else \
	{ \
		res = allocation; \
	}

// TODO(flo)
#define FREE_LIST_REMOVE(res, free_ptr, next_free) \
	if(res) \
	{ \
		res->next_free = free_ptr; \
		free_ptr = res; \
	}

/*
---------------------------
	NOTE: STACK DEFINES
---------------------------
*/

// TODO(flo): linked list stack

#define STACK_LIST_PUSH(res, top, next) \
	res->next = top; \
	top = res

#define STACK_LIST_POP(res, top, next) \
	kh_assert(top); \
	res = top; \
	top = top->next


#define STACK_LIST_PEEK(res, top) res = top

#define STACK_ARRAY_PUSH(arr, top, res) \
	if(top < array_count(arr)) \
 	{ \
		arr[top++] = res; \
	}

#define STACK_ARRAY_POP(res, arr, top) \
	if(top > 0) \
	{ \
		res = arr[--top]; \
	}

#define STACK_ARRAY_PEEK(res, arr, top) res = arr[top - 1]

#define STACK_PTR_PUSH(arr, size, top, res) \
	if(top < size) \
	{ \
		arr[top++] = res; \
	}

#define STACK_PTR_POP(res, arr, top) \
	if(top > 0) \
	{ \
		res = arr + --top; \
	}

#define STACK_PTR_PEEK(res, arr, top) res = arr + (top - 1)

/*
---------------------------
	NOTE: QUEUE DEFINES
---------------------------
*/

// TODO(flo): this defines seem useless find better one

#define QUEUE_ARR_MAKE(front, rear) front = rear = 0;

#define QUEUE_ARR_ENQUEUE(arr, res, rear, count, size) \
	if(count < size) \
	{ \
		arr[rear++] = res; \
		if(rear == size) \
		{ \
			rear = 0; \
		} \
		count++; \
	}

#define QUEUE_ARR_DEQUEUE(arr, res, front, count, size) \
	if(count > 0) \
	{ \
		res = arr[front++]; \
		if(front == size) \
		{ \
			front = 0; \
		} \
		count--; \
	}

#define QUEUE_NEXT_ENQUEUE_INDEX(rear, size) (rear + 1) % size
#define QUEUE_NEXT_DEQUEUE_INDEX(front, size) (front + 1) % size

#define QUEUE_NEXT_ENQUEUE_INDEX_POW2(rear, size) (rear + 1) & (size - 1)
#define QUEUE_NEXT_DEQUEUE_INDEX_POW2(front, size) (front + 1) & (size - 1)

#define QUEUE_LIST_ENQUEUE(res, next, front, rear) \
	res->next = 0; \
	if(!front && !rear) \
	{ \
		front = rear = res; \
	} \
	else \
	{ \
		rear->next = res; \
		rear = res; \
	}

#define QUEUE_LIST_DEQUEUE(res, next, front, rear) \
	res = front; \
	if(front) \
	{ \
		if(front == rear) \
		{ \
			front = rear = 0; \
		} \
		else \
		{ \
			front = front->next; \
		} \
	}

/*
	m = array_count(hash)
	DIVISION METHOD : hash(k) = k % m or if m = 2^r hash(k) k & (m - 1)
	MULTIPLICATION METHOD : hash(k) = ((a*k) % 2^w) >> w - r where w = number of bits for the computation model (32 or 64), and m = 2^r so m must be power of two, a = random bit number (must be odd and not close to power of two but between 2^r-1 and 2^r). ie : ((a * k), % MAX_INT64) >> 64 - 8
	if m = 256 (aka 2^8), a must be odd between 128 and 256 but not close
	UNIVERSAL METHOD : ((a*k + b) % p) % m, where a and b must be random between 0 and p-1, and p must be a prime number bigger than the size of the universe

	Open Addressing Hash : we have not implemented this yet (chaining hash and mixing have been), but we need a additional flag (HasBeenDelete) in addition to our flag (IsEmpty) for searching. when we insert we treat the two flags in the same way but when we search if we encouter the delete flag we continue to iterate
*/

struct hash_element
{
	hash_element *next_in_hash;
	u32 datas;
};

struct hash_str
{
	hash_element *ptr[256];
};

inline hash_element *
get_element_in_hash(hash_str *hash, u32 datas)
{
	u32 hash_value = datas;
	// u32 hash_slot = hash_value % array_count(hash->ptr);
	u32 hash_slot = hash_value & (array_count(hash->ptr) - 1); 
	kh_assert(hash_slot < array_count(hash->ptr));

	hash_element **el = &hash->ptr[hash_slot];
	while(*el && datas != (*el)->datas)
	{
		el = &(*el)->next_in_hash;
	}

	hash_element *res = *el;
	if(!res)
	{
		// TODO(flo): free list here instead of alloc
		res = alloc(sizeof(hash_element));
		res->datas = datas;

		res->next_in_hash = *el;
		*el = res;
	}
	return(res);
}



/*
---------------------------
	NOTE: TREES DEFINES
---------------------------
*/

#define BS_TREE_GET_ROOT_FOR(res, root, left_child, right_child, key, compared) \
	for(;root;) \
	{ \
		res = root; \
		if(compared <= root->key) \
		{ \
			root = root->left_child; \
		} \
		else \
		{ \
			root = root->right_child; \
		} \
	}

/*
---------------------------
	NOTE: GRAPHS DEFINES
---------------------------
*/

/*
---------------------------
	NOTE: SORTS DEFINES
---------------------------
*/
/*
---------------------------
	NOTE: STATE DEFINES
---------------------------
*/

#define STATE_ENTER(callbacK_func, ...) callbacK_func(## __VA_ARGS__)
#define STATE_EXIT()
#define STATE_EXECUTE()
#endif //0
#endif //KH_DATA_STRUCTURES_H






