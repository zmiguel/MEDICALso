.PHONY = all clean cliente medico balcao classificador
CC = gcc
LINKERFLAG = -lm


all: cliente medico balcao classificador

build/%.o: src/%.c
	@echo "Creating object.."
	${CC} -c $< -o $@

cliente: build/cliente.o build/util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

medico: build/medico.o build/util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

balcao: build/balcao.o build/util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

classificador: build/classificador.o build/util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

clean:
	@echo "Cleaning up..."
	rm -rvf build/*.o bin/*
	
