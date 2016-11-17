all: rm cp ls

rm:
	gcc -c simul_utils.c
	gcc -c rm_mod.c
	gcc -o rm_mod rm_mod.o simul_utils.o
	ar crv simul_utils.a simul_utils.o
	ranlib simul_utils.a
	gcc -o rm_mod rm_mod.o simul_utils.a -Wall
cp:
	gcc -c simul_utils.c
	gcc -c cp_mod.c
	gcc -o cp_mod cp_mod.o simul_utils.o
	ar crv simul_utils.a simul_utils.o
	ranlib simul_utils.a
	gcc -o cp_mod cp_mod.o simul_utils.a -Wall
ls:
	gcc -c simul_utils.c
	gcc -c ls_mod.c
	gcc -o ls_mod ls_mod.o simul_utils.o
	ar crv simul_utils.a simul_utils.o
	ranlib simul_utils.a
	gcc -o ls_mod ls_mod.o simul_utils.a -Wall

