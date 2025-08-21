todo: todo.c
	cc -o todo todo.c -lncurses

clean:
	rm -f todo

install: todo
	mkdir -p /usr/local/bin
	cp -f todo /usr/local/bin

uninstall:
	rm -f /usr/local/bin/todo

.PHONY:
	clean install uninstall
