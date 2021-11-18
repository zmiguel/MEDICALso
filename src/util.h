// Estrutura para guardar os utentes
struct utente{
  char nome[256];
  char especialidade[256];
  int prioridade;
} typedef Utente;

struct administrador{
  char nome[256];
} typedef Administrador;

// Estrutura para guardar os administradores
struct especialista{
  char nome[256];
  char especialidade[256];
} typedef Especialista;

// Estrutura para comunicação do utente para o balcao
struct utente_balcao{
  int pid;
  char nome[256];
  char msg[256];
} typedef U_B;

// Estrutura para comunicação do balcao para o utente
struct balcao_utente{
  int pid;
  char msg[256];
} typedef B_U;

// Estrutura para comunicação do medico para o balcao
struct medico_balcao{
  int pid;
  char nome[256];
  char especialidade[256];
} typedef M_B;

// Estrutura para comunicação do balcao para o medico
struct balcao_medico{
  int pid;
  char msg[256];
} typedef B_M;

// Estrutura para comunicação entre utente e o medico
struct utente_medico{
  int pid;
  char msg[256];
} typedef Consulta;

int sum(int one, int two);