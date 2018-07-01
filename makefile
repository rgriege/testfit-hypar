libtest.so: test.c
	gcc -fPIC -shared test.c -o libtest.so

.PHONY: clean
clean:
	rm -f libtest.so
