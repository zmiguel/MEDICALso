# Tasks to be done

## Utente

### Condições

- Só executa se o balcão estiver a correr

### Inicio

- Pede nome do utilizador como argumento linha comandos (argv)
- Pede sintomas fazendo uma pergunta au utilizador

### Continuação

- Envia sintomas para o balcão, para ser analisado pelo classificador
- balcão atribui uma especialidade e prioridade.
- balcão devolve:

  - especialidade
  - prioridade
  - nr de utendes a sua frente nessa especialidade
  - nr de especialistas nessa especialidade

- utente aguarda até ser encaminhado para um especialista (mesmo quando nao ha ninguem a frente)

### Atendimento

- balcão informa que é a sua vez
- liga o cliente ao medico
- abre linha de comunicação por texto (pergunta resposta) entre cliente e medico
- quando o cliente diz "adeus" termina a consulta

## Especialista

### Condições

- Só executa quando o balcão estiver a correr

### Inicio

- Pede ao medico o seu nome e especialidade como argumentos de linha de comandos (./medico <nome> <especialidade>)

### Continuação

- Informa o balcão que este medico existe

### Atendimento

- Aguarda que o balcão lhe atribua um utente
- Se disser "adeus" a consulta termina
- Se disser "sair" termina a sua ligação ao sistema e deixa de estar disponível

## Balcão

### Inicio

- Ao ser executado não é necessário qualquer input para estar tudo a funcionar.

### Execução

- informa sempre que existerem novos eventos:

  - novo utente
  - classificação do utente
  - quem o esta a atender
  - consulta terminada e por quem
  - especialista que se juntou e especialidade
  - especialista que saiu e especialidade

- A cada 30 segundos imprimir informação das listas de espera com todas as especialidades
- Permitir que o administrador envie comandos para controlar o balcão
  - `utentes` -> lista os utentes em fila de espera, a especialidade, e a sua prioridade. E também os utente que estão a ser atendidos e por quem
  - `especialists` -> lista os especialistas e o seu estado actual
  - `delut X` -> remove o utente X, informando o cliente e terminando-lhe o programa (só para utentes em fila de espera)
  - `delesp X` -> remove o especialista X, informando o programa medico e terminando-lhe o programa (só caso nao esteja a atender ninguém)
  `freq N` -> passa a apresentar a informação das listas de espera de N em N segundos
  `encerra` -> termina o programa, avisando os cliente e medicos que vai fechar!

# Metas

## Meta 1

- Planear  e  definir  as estruturas de  dados responsáveis  por  gerir  as  definições  de  funcionamento  no cliente, médico e serviço. Definir os vários header files com constantes simbólicas que registem os valores por omissão comuns e específicos do cliente e servidor bem como as estruturas de dados relevantes
- Implementar  a  parte  do balcão  relativa  à  classificação  da  especialidade  e  respetiva  prioridade.  Para  tal  deve assumir que os dados dos sintomas são obtidos diretamente pelo utilizador administrador do programa balcão.
- Implementar no balcão a obtenção dos valores das variáveis de ambiente e concretizar as estruturas de dados do balcão
