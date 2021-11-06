struct utente
{
  char nome[256];
  char especialidade[256];
  int prioridade;
} typedef Utente;

struct administrador
{
  char nome[256];
} typedef Administrador;

struct especialista
{
  char nome[256];
  char especialidade[256];
} typedef Especialista;


int sum(int one, int two);