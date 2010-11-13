
CC = gcc
LIBS = `pkg-config --libs --cflags glib-2.0 gtk+-2.0`

compile: translator.c handler.c connector.c config.h
	$(CC) $(LIBS) translator.c handler.c connector.c -o translator

run: compile
	./translator

tar:
	-test -e translator && rm -rf translator
	mkdir translator
	for file in `ls | grep -xv "translator"`; do cp $$file ./translator; done
	tar -cvf translator.tar.gz ./translator
	rm -rf translator
