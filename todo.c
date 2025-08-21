#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define FILENAME "TODO"

#define TODO_PREFIX "TODO: "
#define DONE_PREFIX "DONE: "
#define PREFIX_LEN 6

#define PROMPT ">> "
#define PROMPT_LEN 3

struct task {
	char *title;
	struct task *prev;
	struct task *next;
};

struct task_list {
	struct task *head;
	struct task *tail;
	struct task *curr;
	size_t size;
};

struct task_list *create_list()
{
	struct task_list *list = (struct task_list *) malloc(sizeof(struct task_list));
	list->head = NULL;
	list->tail = NULL;
	list->curr = NULL;
	list->size = 0;
}

struct task *create_task(const char *task_title)
{
	struct task *new_task = (struct task *) malloc(sizeof(struct task));
	new_task->title = (char *) malloc(sizeof(char) * strlen(task_title) + 1);
	strcpy(new_task->title, task_title);
	new_task->next = NULL;
	new_task->prev = NULL;
	return new_task;
}

void insert_above(struct task_list *list, struct task *new_task)
{
	if (!list->curr) {
		list->head = new_task;
		list->tail = new_task;
	} else if (!list->curr->prev) {
		list->curr->prev = new_task;
		new_task->next = list->curr;
		list->head = new_task;
	} else {
		new_task->prev = list->curr->prev;
		new_task->next = list->curr;
		list->curr->prev->next = new_task;
		list->curr->prev = new_task;
	}

	list->curr = new_task;
	list->size += 1;
}

void insert_below(struct task_list *list, struct task *new_task)
{
	if (!list->curr) {
		list->head = new_task;
		list->tail = new_task;
	} else if (!list->curr->next) {
		list->curr->next = new_task;
		new_task->prev = list->curr;
		list->tail = new_task;
	} else {
		new_task->next = list->curr->next;
		new_task->prev = list->curr;
		list->curr->next->prev = new_task;
		list->curr->next = new_task;
	}

	list->curr = new_task;
	list->size += 1;
}

/*
 * Don't forget to keep current task somewhere!
 * Wrong usage of this function may cause memory leaks.
 */
void forget_curr(struct task_list *list)
{
	if (list->curr == list->head && list->curr == list->tail) {
		list->curr = NULL;
		list->head = NULL;
		list->tail = NULL;
	} else if (list->curr == list->head) {
		list->curr = list->curr->next;
		list->head = list->curr;
		list->curr->prev = NULL;
	} else if (list->curr == list->tail) {
		list->curr = list->curr->prev;
		list->tail = list->curr;
		list->curr->next = NULL;
	} else {
		list->curr->next->prev = list->curr->prev;
		list->curr->prev->next = list->curr->next;
		list->curr = list->curr->next;
	}

	list->size -= 1;
}

void transfer_task(struct task_list *from, struct task_list *to)
{
	if (!from->curr)
		return;

	struct task *to_transfer = from->curr;
	forget_curr(from);

	if (!to->tail) {
		to->head = to_transfer;
		to->curr = to_transfer;
		to_transfer->next = NULL;
		to_transfer->prev= NULL;
	} else {
		to->tail->next = to_transfer;
		to_transfer->prev = to->tail;
		to_transfer->next = NULL;
	}

	to->tail = to_transfer;
	to->size += 1;
}

void swap_tasks(struct task *first_task, struct task *second_task)
{
	if (!first_task || !second_task)
		return;

	char *temp_title = (char *) malloc(sizeof(char) * strlen(first_task->title) + 1);
	strcpy(temp_title, first_task->title);

	free(first_task->title);
	first_task->title = (char *) malloc(sizeof(char) * strlen(second_task->title) + 1);
	strcpy(first_task->title, second_task->title);

	free(second_task->title);
	second_task->title = (char *) malloc(sizeof(char) * strlen(temp_title) + 1);
	strcpy(second_task->title, temp_title);

	free(temp_title);
}

void delete_curr(struct task_list *list)
{
	if (!list->curr)
		return;
	
	struct task *to_free = list->curr;
	forget_curr(list);
	free(to_free->title);
	free(to_free);
}

void free_list(struct task_list *list)
{
	if (!list)
		return;

	if (list->head) {
		struct task *curr = list->head;
		struct task *to_free;
		while (curr) {
			to_free = curr;
			curr = curr->next;
			free(to_free->title);
			free(to_free);
		}
	}

	free(list);
}

int is_task(const char *str)
{
	if (strlen(str) < PREFIX_LEN)
		return 0;

	char prefix[PREFIX_LEN + 1];
	for (int i = 0; i < PREFIX_LEN; ++i)
		prefix[i] = str[i];

	if (strcmp(prefix, TODO_PREFIX) == 0)
		return 1;
	else if (strcmp(prefix, DONE_PREFIX) == 0)
		return 2;
	return 0;
}

void format_str(char *str)
{
	int curr = PREFIX_LEN;
	do {
		str[curr - PREFIX_LEN] = str[curr];
	} while (str[curr++] != '\0');
}

void read_list(const char *filename, struct task_list *todo, struct task_list *done)
{
	FILE *list_file = fopen(filename, "r");

	if (!list_file)
		return;

	int capacity = 16;
	char *buffer = (char *) malloc(sizeof(char) * capacity);
	int curr = 0;

	char c;
	while ((c = fgetc(list_file)) != EOF) {
		if (c == '\n') {
			buffer[curr] = '\0';
			if (is_task(buffer) == 1) {
				format_str(buffer);
				insert_below(todo, create_task(buffer));
			} else if (is_task(buffer) == 2) {
				format_str(buffer);
				insert_below(done, create_task(buffer));
			}
			curr = 0;
			continue;
		}

		buffer[curr++] = c;

		if (curr >= capacity) {
			capacity = capacity * 2;
			buffer = (char *) realloc(buffer, sizeof(char) * capacity);
		}
	}

	todo->curr = todo->head;
	done->curr = done->head;

	free(buffer);
	fclose(list_file);
}

void write_list(const char *filename, struct task_list *todo, struct task_list *done)
{
	FILE *list_file = fopen(filename, "w");

	if (!list_file)
		return;

	struct task *curr_task;
	for (curr_task = todo->head; curr_task != NULL; curr_task = curr_task->next)
		fprintf(list_file, TODO_PREFIX "%s\n", curr_task->title);

	for (curr_task = done->head; curr_task != NULL; curr_task = curr_task->next)
		fprintf(list_file, DONE_PREFIX "%s\n", curr_task->title);

	fclose(list_file);
}

char *get_input()
{
	int max_x, max_y, x, y;
	getmaxyx(stdscr, max_y, max_x);
	x = PROMPT_LEN;
	y = 0;

	int capacity = 16;
	char *input = (char *) malloc(sizeof(char) * capacity);
	int curr = 0;

	char c;
	while (c = getch()) {
		if (x >= max_x) {
			x -= max_x;
			y += 1;
		}

		if (c == '\x1B') {
			free(input);
			return NULL;
		} else if (c == '\n') {
			input[curr] = '\0';
			break;
		} else if ((c == KEY_BACKSPACE || c == KEY_DC || c == 127) && curr != 0) {
			if (y > 0 && x == 0) {
				y -= 1;
				x = max_x - 1;
			} else if (x + max_x * y > PROMPT_LEN) {
				x -= 1;
			}
			move(y, x);
			addch(' ');
			move(y, x);
			curr -= 1;
			input[curr] = '\0';
			continue;
		} else if (c < 32 || c == 127) {
			continue;
		}

		addch(c);
		input[curr++] = c;
		x += 1;

		if (curr >= capacity) {
			capacity = capacity * 2;
			input = (char *) realloc(input, sizeof(char) * capacity);
		}
	}

	return input;
}

int main(int argc, char *argv[])
{
	if (argc > 2 || argc == 2 && strcmp(argv[1], "--help") == 0) {
		fprintf(stderr, "Usage: %s <TODO>        '%s' by the default\n", argv[0], FILENAME);
		exit(1);
	}

	char *filename = FILENAME;
	if (argc == 2)
		filename = argv[1];

	struct task_list *todo = create_list(); 
	struct task_list *done = create_list(); 
	struct task_list *context = todo;
	char context_char = ' ';

	read_list(filename, todo, done);

	initscr();
	raw();
	curs_set(0);
	timeout(16);
	noecho();

	struct task *curr_task;
	char key = 0;
	int quit = 0;
	int changed = 0;
	
	char *message = "";

	while (!quit) {
		erase();

		printw("%s\n\n", message);

		if (context == todo)
			printw("[TODO]  DONE \n");
		else
			printw(" TODO  [DONE]\n");
		printw("------------- ");
		printw("(%zu)\n", context->size);

		for (curr_task = context->head; curr_task != NULL; curr_task = curr_task->next) {
			if (curr_task == context->curr) {
				attron(A_REVERSE);
				printw("- [%c] %s \n", context_char, curr_task->title);
				attroff(A_REVERSE);
				continue;
			}
			printw("- [%c] %s \n", context_char, curr_task->title);
		}

		switch (key) {
		case '\t':
			if (context == todo) {
				context = done;
				context_char = 'x';
			} else {
				context = todo;
				context_char = ' ';
			}
			message = "";
			break;
		case 'j':
			message = "";
			if (context->curr && context->curr->next)
				context->curr = context->curr->next;
			break;
		case 'k':
			message = "";
			if (context->curr && context->curr->prev)
				context->curr = context->curr->prev;
			break;
		case 'J':
			message = "";
			if (context->curr && context->curr->next) {
				swap_tasks(context->curr, context->curr->next);
				context->curr = context->curr->next;
				changed = 1;
			}
			break;
		case 'K':
			message = "";
			if (context->curr && context->curr->prev) {
				swap_tasks(context->curr, context->curr->prev);
				context->curr = context->curr->prev;
				changed = 1;
			}
			break;
		case 'g':
			context->curr = context->head;
			message = "";
			break;
		case 'G':
			context->curr = context->tail;
			message = "";
			break;
		case 'd':
			if (!context->curr) {
				message = "Nothing to delete";
				break;
			}
				
			timeout(-1);
			mvprintw(0, 0, "\n");
			mvprintw(0, 0, "Delete? [Y/n]: ");
			char choice = getch();
			timeout(16);

			if (choice != '\n' && choice != 'Y' && choice != 'y') {
				message = "Not deleted";
				break;
			}

			delete_curr(context);
			message = "Deleted";
			changed = 1;
			break;
		case 'x':
			if (!context->curr)
				break;

			if (context == todo) {
				transfer_task(todo, done);
				message = "Done";
			} else {
				transfer_task(done, todo);
				message = "Not done";
			}
			changed = 1;
			break;
		case 'O':
		case 'o':
			if (context == done) {
				message = "Can't add a new task here";
				break;
			}

			timeout(-1);
			mvprintw(0, 0, "\n");
			mvprintw(0, 0, PROMPT);
			char *title = get_input();
			timeout(16);

			if (!title) {
				message = "Aborted";
				break;
			}

			if (key == 'o')
				insert_below(context, create_task(title));
			else
				insert_above(context, create_task(title));
			free(title);
			message = "Added a new task";
			changed = 1;
			break;
		case 'w':
			write_list(filename, todo, done);
			message = "Written";
			changed = 0;
			break;
		case 'q':
			if (changed) {
				timeout(-1);
				mvprintw(0, 0, "\n");
				mvprintw(0, 0, "You have some changes. Exit anyway? [y/N]: ");
				char choice = getch();
				timeout(16);

				if (choice != 'y' && choice != 'Y') {
					message = "Aborted";
					break;
				}
			}
			quit = 1;
		}

		key = getch();
		refresh();
	}

	free_list(todo);
	free_list(done);

	endwin();

	return 0;
}
