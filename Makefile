#
# Makefile
#
# by Andreas Steinmetz, 2020
#
# This file is put in the public domain. Have fun!
#
libmb85rs64v.a: libmb85rs64v.c mb85rs64v.h
	gcc -Wall -Os -c libmb85rs64v.c
	ar -rcuU libmb85rs64v.a libmb85rs64v.o

clean:
	rm -f libmb85rs64v.a libmb85rs64v.o
