#include "string_tree.h"
#include "compat.h"
#include <string.h>
#include <stdlib.h>

struct string_tree *string_tree_search(struct rb_root *root, const char *s,
				       int n)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct string_tree *data = rb_entry(node, struct string_tree, rbn);
		int result;

		if(n == -1) {
			result = name_cmp(s, data->s);
		} else {
			result = name_cmp_n(s, data->s, n);
			if(result == 0)
				result = n - data->len;
		}

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

struct string_tree *string_tree_search2(struct rb_root *root, const char *s1,
					int s1len, const char *s2)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct string_tree *data = rb_entry(node, struct string_tree, rbn);
		int result;

		result = name_cmp_n(s1, data->s, s1len);
		if(result == 0) {
			if(s2)
				result = name_cmp(s2, data->s + s1len);
			else
				result = s1len - data->len;
		}

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

int string_tree_insert(struct rb_root *root, struct string_tree *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct string_tree *this = rb_entry(*new, struct string_tree, rbn);
		int result = name_cmp(data->s, this->s);

		parent = *new;
		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else
			return -1;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->rbn, parent, new);
	rb_insert_color(&data->rbn, root);

	return 0;
}

int string_tree_add(struct rb_root *root, struct string_tree *st, const char *s)
{
	st->len = strlen(s);
	st->s = malloc(st->len + 1);
	if(!st->s) {
		perror("malloc");
		return -1;
	}

	memcpy(st->s, s, st->len);
	st->s[st->len] = 0;

	if(string_tree_insert(root, st) < 0) {
		free(st->s);
		return -1;
	}
	return 0;
}

void string_tree_free(struct rb_root *root, struct string_tree *st)
{
	string_tree_rm(root, st);
	free(st->s);
}
