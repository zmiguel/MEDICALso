.PHONY = all clean cliente medico balcao classificador
CC = gcc
LINKERFLAG = -g -lm

obj = build
src = src
bin = bin


all: dirs cliente medico balcao classificador

${obj}/%.o: ${src}/%.c
	@echo "Creating object.."
	${CC} -g -c $< -o $@

cliente: ${obj}/cliente.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} $^ -o ${bin}/$@

medico: ${obj}/medico.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} $^ -o ${bin}/$@

balcao: ${obj}/balcao.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} $^ -o ${bin}/$@

classificador: ${obj}/classificador.o ${obj}/util.o
	@echo "Checking.."
	${CC} ${LINKERFLAG} $^ -o ${bin}/$@

dirs:
	@echo "Making dirs..."
	mkdir -p ${obj} ${bin}

clean:
	@echo "Cleaning up..."
	rm -rvf ${obj}/*.o ${bin}/*
	
