.PHONY = all clean cliente medico balcao classificador
CC = gcc
LINKERFLAG = -lm


all: cliente medico balcao classificador

%.o: %.c
	@echo "Creating object.."
	${CC} -c src/$< -o build/$@

cliente: cliente.o util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

medico: medico.o util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

balcao: balcao.o util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

classificador: classificador.o util.o
	echo "Checking.."
	${CC} ${LINKERFLAG} build/$< -o bin/$@

clean:
	@echo "Cleaning up..."
	rm -rvf build/*.o bin/*
	
