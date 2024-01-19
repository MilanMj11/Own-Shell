#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <readline/readline.h>
// nix-shell -p readline
// gcc final.c -o final -lreadline

#define HISTORY_SIZE 50
char *history[HISTORY_SIZE];
int history_count = 0;

int dimensiune();
void print_array();
int impartire_big();
int execute_big();
void print_array();

void _help() {
  printf("Comenzi disponibile:\n");
  printf("_cd <nume_director> : schimba directorul in cel precizat\n");
  printf("_mkdir <nume_director> : creaza un director cu numele precizat\n");
  printf("_pwd : afiseaza calea directorului curent\n");
  printf("_touch <nume_fisier> : creaza un fisier cu numele precizat\n");
  printf("_grep <pattern> <nume_fisier> : \n");
  printf("_clear : curata termanalul\n");
  printf("_history : afiseaza istoricul comenzilor\n");
  printf("_ls : listeaza numele fisierelor din directorul curent\n");
  printf("exit : \n");
  printf("_cp <nume_fisier_1> <nume_fisier_2> : copiaza continutul primului "
         "fisier in cel de al doilea\n");
  printf("_rm <nume_fisier> : elimina fisierul precizat\n");
  printf("_rmdir <nume_director> : elimina directorul precizat\n");
}

void free_array(char **arr) {
  if (arr == NULL) {
    return;
  }
  for (int i = 0; arr[i] != NULL; i++) {
    free(arr[i]);
  }
  free(arr);
}

char *ltrim(char *s) {
  while (isspace(*s))
    s++;
  return s;
}

char *rtrim(char *s) {
  char *back = s + strlen(s);
  while (isspace(*--back))
    ;
  *(back + 1) = '\0';
  return s;
}

char *trim(char *s) { return rtrim(ltrim(s)); }

char **trim_array(char **arr) {
  for (int i = 0; i < dimensiune(arr); i++) {
    arr[i] = trim(arr[i]);
  }
  return arr;
}

/// ------------------ ADD HISTORY -------------------
void add_history(char *command) {
  if (history_count < HISTORY_SIZE) {
    history[history_count++] = strdup(command);
    // strdup folosit pt creare de copie dinamica
  } else {
    free(history[0]);
    for (int i = 0; i < HISTORY_SIZE - 1; ++i) {
      history[i] = history[i + 1];
    }
    history[HISTORY_SIZE - 1] = strdup(command);
  }
}
/// ------------------ ADD HISTORY -------------------

/// ------------------ SHOW HISTORY -------------------
int show_history(char** args) {
  if (dimensiune(args) != 0) {
    printf("Format gresit\n");
    return -1;
  }
  for (int i = 0; i < history_count; ++i) {
    printf("%s\n", history[i]);
  }
  return 0;
}
/// ------------------ SHOW HISTORY -------------------

/// ------------------ LIST ---------------------------
int _ls(char** args) {
  if (dimensiune(args) != 0) {
    printf("Format gresit\n");
    return -1;
  }
  // scandir(calea directorului, adresa la care vor fi stocate fisierele -
  // struct dirent, pointer la functie de filtrare - null, pointer la functie de
  // comparare - alphasort sortare alfabetica)
  struct dirent **files;
  int nr_files = scandir(".", &files, NULL, alphasort);
  if (nr_files < 0) {
    perror("Nu s-a reusit scanarea directorului");
    return -1;
  }
  for (int i = 0; i < nr_files; ++i) {
    printf("%s\n", files[i]->d_name);
    free(files[i]);
  }
  free(files);
  return 0;
}
/// ------------------ LIST -------------------------

///-------------------- COPY FILE -------------------
int _cp(char **args) {
  int sursa, dest;
  if (dimensiune(args) != 2) {
    printf("format gresit");
    return -1;
  }
  sursa = open(args[0], O_RDONLY);
  if (sursa == -1) {
    printf("Eroare la deschiderea fisierului sursa\n");
    return -1;
  }

  dest = open(args[1], O_WRONLY);
  if (dest == -1) {
    printf("Eroare la deschiderea fisierului destinatie\n");
    close(sursa);
    return -1;
  }

  char buffer[2000];
  int bytes;

  while ((bytes = read(sursa, buffer, sizeof(buffer))) > 0) {
    if (write(dest, buffer, bytes) != bytes) {
      close(sursa);
      close(dest);
      return 1;
    }
  }
  close(sursa);
  close(dest);
  printf("Copiere reusita\n");
  return 0;
}
///-------------------- COPY FILE -------------------

/// ------------------ REMOVE FILE -------------------
int _rm(char **args) {
  if (dimensiune(args) != 1) {
    printf("Format gresit\n");
    return -1;
  }
  if (remove(args[0]) == 0) {
    printf("Fisier sters cu succes\n");
  } else {
    printf("Eroare la stergerea fisierului\n");
    return -1;
  }
  return 0;
}
/// ------------------ REMOVE FILE -------------------

/// ------------------ REMOVE DIRECTORY --------------
int _rmdir(char **args) {
  if (dimensiune(args) != 1) {
    printf("Format gresit\n");
    return -1;
  }

  char cwd[1024];

  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getcwd");
    return -1;
  }

  char path[1024];
  snprintf(path, sizeof(path), "%s/%s", cwd, args[0]);

  printf("%s\n", path);

  if (rmdir(path) == 0) {
    printf("Fisier sters cu succes\n");
  } else {
    printf("Eroare la stergerea fisierului\n");
    return -1;
  }
  return 0;
}
/// ------------------ REMOVE DIRECTORY --------------

/// ------------------ SHOW DIRECTORY ----------------
int _pwd(char** args) {
  if (dimensiune(args) != 0) {
    printf("Format gresit\n");
    return -1;
  }
  char buffer[1024];
  /// getcwd copiaza numele fisierului curent in buffer;
  if (getcwd(buffer, sizeof(buffer)) != NULL)
    printf("Current directory: %s\n", buffer);
  else {
    perror("Eroare la apelarea getcwd");
    return -1;
  }
  return 0;
}
/// ------------------ SHOW DIRECTORY ----------------

/// ------------------ CLEAR ----------------
int _clear(char** args) {
  if (dimensiune(args) != 0) {
    printf("Format gresit\n");
    return -1;
  }
  // "\33": Represents the escape character in octal notation (equivalent to
  // \x1B in hexadecimal or \e in some systems).
  // "[H": Moves the cursor to the home position (top-left corner of the
  // screen).
  // "[2J": Clears the entire screen.
  if (write(1, "\33[H\33[2J", 7) == -1) {
    printf("Eroare la write");
    return -1;
  }
  return 0;
}
/// ------------------ CLEAR ----------------

/// ------------------ CREATE FILE -------------------
int create_file(const char *filename) {
  int fd = open(filename, O_CREAT | O_WRONLY, 0644);
  if (fd == -1) {
    perror("open");
    return 1;
  }
  // inchid fisierul dupa ce l-am deschis
  close(fd);
  return 0;
}

int _touch(char **args) {
  if (dimensiune(args) != 1) {
    printf("Format gresit\n");
    return -1;
  }
  if (create_file(args[0]) == -1) {
    perror("Eroare la crearea fisierului");
    return -1;
  }
  printf("Fisierul %s a fost creat cu succes\n", args[0]);
  return 0;
}
/// ------------------ CREATE FILE -------------------

/// ------------------ CHANGE DIRECTORY -------------------
int change_directory(const char *dir_name) {
  if (dir_name[0] == '\0') {
    printf("Expected an argument for the directory's name!\n");
    return -1;
  } else {
    if (chdir(dir_name) != 0) {
      printf("Eroare la schimbarea directorului!\n");
      return -1;
    }
  }
  return 0;
}

int _cd(char **args) {
  if (dimensiune(args) != 1) {
    printf("Format gresit\n");
    return -1;
  }
  if (change_directory(args[0]) == -1) {
    // perror("Eroare la schimbarea directorului");
    return -1;
  }
  return 0;
}
/// ------------------ CHANGE DIRECTORY -------------------

/// ------------------ MAKE DIRECTORY -------------------
int create_directory(const char *dirname) {
  if (mkdir(dirname, S_IRWXU) == 0) {
    printf("Directory \"%s\" created successfully.\n", dirname);
    return 0;
  } else {
    perror("create_directory");
    return -1;
  }
}

int _mkdir(char **args) {
  if (dimensiune(args) != 1) {
    printf("Format gresit\n");
    return -1;
  }

  if (create_directory(args[0]) == -1) {
    perror("Eroare la crearea directorului");
    return -1;
  }

  return 0;
}
/// ------------------ MAKE DIRECTORY -------------------

/// ------------------ GREP -----------------------------

int find_patterns(const char *pattern, FILE *file) {
  char line[256];

  while (fgets(line, sizeof(line), file) != NULL) {
    if (strstr(line, pattern) != NULL) {
      printf("%s", line);
    }
  }
  return 0;
}

int _grep(char **args) {

  if (dimensiune(args) != 2) {
    printf("Format gresit\n");
    return -1;
  }

  char *pattern = (args[0] + 1);
  char *filename = args[1];

  /// trim the pattern by the '\"';

  pattern[strlen(pattern) - 1] = '\0';

  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    perror("Eroare la deschiderea fisierului");
    return -1;
  }

  if (find_patterns(pattern, file) == -1) {
    perror("Eroare la find_patterns in _grep");
    return -1;
  }

  return 0;
}

/// ------------------ GREP -----------------------------

char **impartire_comenzi(char *line) {
  int buffer_size = 100;
  char **comenzi = malloc(sizeof(char *) * buffer_size);
  char *dupe_line = strdup(line);
  char *token = strtok(dupe_line, " ");
  int contor_comenzi = 0;

  if (comenzi == NULL) {
    printf("eroare alocarea memoriei");
    return NULL;
  }

  while (token != NULL) {
    comenzi[contor_comenzi] = strdup(token);
    contor_comenzi++;

    if (contor_comenzi >= buffer_size) {
      buffer_size += 100;
      comenzi = realloc(comenzi, sizeof(char *) * buffer_size);
      if (comenzi == NULL) {
        printf("eroare alocarea memoriei");
        return NULL;
      }
    }

    token = strtok(NULL, " ");
  }
  comenzi[contor_comenzi] = NULL;
  return comenzi;
}

int execute(char *command) {
  /// ----------------- IMPARTIRE IN COMANDA SI ARGUMENTE -------------
  char **comenzi_big = NULL;
  char **cuvinte = impartire_comenzi(command);
  int nr_arg = dimensiune(cuvinte);
  char **args = malloc(sizeof(char *) * (nr_arg) );
  int i = 0;
  int returnat = -2;
  for (i = 0; i < (nr_arg - 1); i++) {
    args[i] = strdup(cuvinte[i + 1]);
    int dim = strlen(args[i]);
    args[i][dim] = '\0';
    //args = realloc(args, sizeof(char *) );
  }
  args[i] = NULL;
  char *comanda = strdup(cuvinte[0]);
  /// ----------------- IMPARTIRE IN COMANDA SI ARGUMENTE -------------

  if (strstr(command, ">") != NULL) {
    impartire_big(command, &comenzi_big);
    trim_array(comenzi_big);

    if (dimensiune(comenzi_big) > 1) { // pentru >
      if (dimensiune(comenzi_big) != 2) {
        printf("Format gresit\n");
        return -1;
      }
      char *fisier = comenzi_big[1];
      char *comanda = comenzi_big[0];
      returnat = execute_big(comanda, fisier);
      free_array(cuvinte);
      free_array(args);
      return returnat;
    }
  }

  if (strcmp(comanda, "_history") == 0) {
      returnat = show_history(args);
  }
  if (strcmp(comanda, "_grep") == 0) {
    returnat = _grep(args);
  }
  if (strcmp(comanda, "_mkdir") == 0) {
    returnat = _mkdir(args);
  }
  if (strcmp(comanda, "_cd") == 0) {
    returnat = _cd(args);
  }
  if (strcmp(comanda, "_touch") == 0) {
    returnat = _touch(args);
  }
  if (strcmp(comanda, "_clear") == 0) {
    returnat = _clear(args);
  }
  if (strcmp(comanda, "_pwd") == 0) {
    returnat = _pwd(args);
  }
  if (strcmp(comanda, "_ls") == 0)
    returnat = _ls(args);

  if (strcmp(comanda, "_rm") == 0)
    returnat = _rm(args);

  if (strcmp(comanda, "_rmdir") == 0)
    returnat = _rmdir(args);

  if (strcmp(comanda, "_cp") == 0)
    returnat = _cp(args);

  free_array(cuvinte);
  free_array(args);
  // free_array(desp_spatii);
  return returnat;
}

/// ----------------- FUNCTII AJUTATOARE -------------
int dimensiune(char **x) {
  int dim = 0;
  if (x == NULL)
    return dim;
  while (x[dim] != NULL) {
    dim++;
  }
  return dim;
}

void print_array(char **array) {
  for (int i = 0; i < dimensiune(array); i++) {
    printf("%s\n", array[i]);
  }
}

int impartire_and(char *line, char ***rezultat) {
  char *delimiter = "&&";
  char *start = line;
  int i = 0;

  while (1) {
    char *token = strstr(start, delimiter);

    if (token == NULL) {
      token = start + strlen(start);
    }

    size_t length = token - start;
    *rezultat = realloc(*rezultat, sizeof(char *) * (i + 1));
    if (*rezultat == NULL) {
      perror("realloc");
      return -1;
    }

    (*rezultat)[i] = malloc(length + 1);
    if ((*rezultat)[i] == NULL) {
      perror("malloc");
      return -1;
    }

    strncpy((*rezultat)[i], start, length);
    (*rezultat)[i][length] = '\0';

    i++;

    if (*token == '\0') {
      break;
    }
    start = token + strlen(delimiter);
  }

  (*rezultat)[i] = NULL;
  return 0;
}

int impartire_or(char *line, char ***rezultat) {
  char *delimiter = "||";
  char *start = line;
  int i = 0;

  while (1) {
    char *token = strstr(start, delimiter);

    if (token == NULL) {
      token = start + strlen(start);
    }

    size_t length = token - start;
    *rezultat = realloc(*rezultat, sizeof(char *) * (i + 1));
    if (*rezultat == NULL) {
      perror("realloc");
      return -1;
    }

    (*rezultat)[i] = malloc(length + 1);
    if ((*rezultat)[i] == NULL) {
      perror("malloc");
      return -1;
    }

    strncpy((*rezultat)[i], start, length);
    (*rezultat)[i][length] = '\0';

    i++;

    if (*token == '\0') {
      break;
    }
    start = token + strlen(delimiter);
  }

  (*rezultat)[i] = NULL;
  return 0;
}

int impartire_big(char *line, char ***rezultat) {
  char *delimiter = ">";
  char *start = strdup(line);
  int i = 0;

  while (1) {
    char *token = strstr(start, delimiter);

    if (token == NULL) {
      token = start + strlen(start);
    }

    size_t length = token - start;
    *rezultat = realloc(*rezultat, sizeof(char *) * (i + 1));
    if (*rezultat == NULL) {
      perror("realloc");
      return -1;
    }

    (*rezultat)[i] = malloc(length + 1);
    if ((*rezultat)[i] == NULL) {
      perror("malloc");
      return -1;
    }

    strncpy((*rezultat)[i], start, length);
    (*rezultat)[i][length] = '\0';

    i++;

    if (*token == '\0') {
      break;
    }
    start = token + strlen(delimiter);
  }

  (*rezultat)[i] = NULL;
  return 0;
}

char **append_arrays(char **arr1, char **arr2) {
  int size2 = dimensiune(arr2);
  int size1 = dimensiune(arr1);
  if (arr1 == NULL)
    arr1 = malloc(sizeof(char *) * size2);
  else
    arr1 = realloc(arr1, sizeof(char *) * (size1 + size2));

  for (int i = 0; i < size2; i++) {
    arr1[i + size1] = strdup(arr2[i]);
  }
  arr1[size1 + size2] = NULL;
  return arr1;
}

char **add_element(char **arr, char *element) {
  int size = dimensiune(arr);
  if (arr == NULL)
    arr = malloc(sizeof(char *) * 1);
  else
    arr = realloc(arr, sizeof(char *) * (size + 1));
  arr[size] = strdup(element);
  arr[size + 1] = NULL;
  return arr;
}

int execute_big(char *comanda, char *fisier) { // pentru >
  int stdout_copy = dup(STDOUT_FILENO);
  int file = open(fisier, O_WRONLY | O_CREAT, 0666);
  // use the 0666 file mode, which gives read and write permissions to the
  // owner, group, and others.
  if (file < 0) {
    printf("open");
    return -1;
  }
  // Redirecționează ieșirea standard către fișier
  dup2(file, STDOUT_FILENO);
  execute(comanda);
  close(file);
  dup2(stdout_copy, STDOUT_FILENO);
  return 0;
}
/// ----------------- FUNCTII AJUTATOARE -------------


int main(void) {
  char *line;

  while (1) {
    char **comenzi_and = NULL;
    char **comenzi_or = NULL;
    char **comenzi = NULL;
    char **semne = NULL;
    char **desp_spatii = NULL;
    line = readline("MMIT shell> ");

    if (line[0] == '\0')
      continue;

    add_history(line);

    if (strcmp(line, "_help") == 0) {
      _help();
      continue;
    }

    if (strcmp(line, "exit") == 0)
      break;



/// ----------------- IMPARTIRE LINIE DUPA && || -------------
    impartire_and(line, &comenzi_and);
    trim_array(comenzi_and);

    for (int i = 0; i < dimensiune(comenzi_and); i++) {
      impartire_or(comenzi_and[i], &comenzi_or);
      trim_array(comenzi_or);
      comenzi = append_arrays(comenzi, comenzi_or);
    }
/// ----------------- IMPARTIRE LINIE DUPA && || -------------

/// ----------------- IMPARTIRE LINIE DUPA SPATII -------------
    char *and = "&&";
    char * or = "||";
    desp_spatii = impartire_comenzi(line);

    for (int i = 0; i < dimensiune(desp_spatii); i++) {
      if (strcmp(desp_spatii[i], "&&") == 0) {
        semne = add_element(semne, and);
      }
      if (strcmp(desp_spatii[i], "||") == 0) {
        semne = add_element(semne, or);
      }
    }
/// ----------------- IMPARTIRE LINIE DUPA SPATII -------------

    int dim = dimensiune(comenzi);
    if (comenzi[dimensiune(comenzi)-1][0] == '\0') dim--;

    if (dimensiune(semne) > 0) {
      if(dim - dimensiune(semne) != 1){
        printf("Eroare: Numarul de comenzi incorect\n");
        break;
      }
      int idx = 0;
      int idx_comenzi = 1;
      int returnat = execute(comenzi[0]);
      while (idx < dimensiune(semne)) {
        if (strcmp(semne[idx], and) == 0 && returnat == 0) {
          returnat = execute(comenzi[idx_comenzi]);

          if (returnat != 0)
            break;
        }
        if (strcmp(semne[idx], or) == 0 && returnat != 0) {
          returnat = execute(comenzi[idx_comenzi]);
          if (returnat == 0)
            break;
        }
        idx_comenzi++;
        idx++;
      }
    }
    else execute(line);
  }
  return 0;
}
