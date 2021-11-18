.PHONY = all clean cliente medico balcao
CC = gcc
LINKERFLAG = -g -lm

obj = build
src = src
bin = bin


all: dirs cliente medico balcao
	export MAXCLIENTES=50
	export MAXMEDICOS=50

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

dirs:
	@echo "Making dirs..."
	mkdir -p ${obj} ${bin}

clean_mac:
	@echo "Cleaning up..."
	rm -rvf ${obj}/*.o ${bin}/*
	cp prof/classificador_mac bin/classificador

clean:
	@echo "Cleaning up..."
	rm -rvf ${obj}/*.o ${bin}/*
	cp prof/classificador bin/classificador
	
