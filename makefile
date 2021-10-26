.PHONY = all clean cliente medico balcao classificador
CC = gcc
LINKERFLAG = -lm

obj = build
src = src


all: cliente medico balcao classificador

${obj}/%.o: ${src}/%.c
	@echo "Creating object.."
	${CC} -c $< -o $@

cliente: ${obj}/cliente.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} ${obj}/$< -o ${src}/$@

medico: ${obj}/medico.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} ${obj}/$< -o ${src}/$@

balcao: ${obj}/balcao.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} ${obj}/$< -o ${src}/$@

classificador: ${obj}/classificador.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} ${obj}/$< -o ${src}/$@

clean:
	@echo "Cleaning up..."
	rm -rvf ${obj}/*.o ${bin}/*
	
