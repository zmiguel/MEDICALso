// Estrutura para guardar os utentes
struct utente{
  char nome[256];
  char especialidade[256];
  int prioridade;
  int pid;
  int em_consulta;
} typedef Utente;

struct especialista{
  char nome[256];
  char especialidade[256];
  int pid;
  int em_consulta;
  unsigned int ts;
} typedef Especialista;

// Estrutura para comunicação dos cliente para o balcao
struct cliente_balcao{
  int pid;
  int tipo;
  char nome[256];
  char msg[256];
  unsigned int ts;
} typedef C_B;

// Estrutura para comunicação do balcao para o utente
struct balcao_utente{
  int pid;
  int tipo;
  char especialidade[256];
  int prioridade;
  int num_utentes;
  int num_especialistas;
  char msg[256];
} typedef B_U;

// Estrutura para comunicação do balcao para o medico
struct balcao_medico{
  int pid;
  int tipo;
  char msg[256];
} typedef B_M;

// Estrutura para comunicação entre utente e o medico
struct utente_medico{
  int pid;
  char msg[1000];
} typedef Consulta;
