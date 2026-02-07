#include "user/user.h"
#include "kernel/param.h"
//#include "kernel/proc.h"

static char *last_alloc = 0x0;

char* center_name(char name[16]) {
  char* new = malloc(17);
  last_alloc = new;
  uint64 str_len = 0;
  while (1) {
    if (str_len == 16) break;
    if (name[str_len] == 0) break;
    str_len++;
  }

  uint64 spaces_left = (16 - str_len) / 2;
  uint64 spaces_right = spaces_left + ((16 - str_len) % 2);

  uint64 counter = 0;

  for (uint64 i = 0; i < spaces_left; i++) {
    new[counter] = ' ';
    counter++;
  }
  for (uint64 i = 0; i < str_len; i++) {
    new[counter] = name[i];
    counter++;
  }
  for (uint64 i = 0; i < spaces_right; i++) {
    new[counter] = ' ';
    counter++;
  }
  new[16] = 0;

  return new;
}

int main() {
  int pids[NPROC];

  ps(pids, 0);

  printf("PID |       Name       | Privilege | Running | Parent PID\n");
  printf("---------------------------------------------------------\n");

  static char placeholder[16];
  memset(placeholder, 0, 16);
  placeholder[0] = 0;
  char *placeholder_name = center_name(placeholder);

  for (int i = 0; i < NPROC; i++) {
    if (pids[i] == 0) break;

    struct procdata data;
    pinfo(pids[i], &data);

    printf("%d   | %s |     %s     |    %s    | %d\n", data.pid, *data.name ? center_name(data.name) : placeholder_name, data.intended_state == 1 ? "S" : "U", data.killed == 0 ? "1" : "0", \
      data.parent_pid);

    free(last_alloc);
  }

  printf("\n\n");

  exit(0);
}
